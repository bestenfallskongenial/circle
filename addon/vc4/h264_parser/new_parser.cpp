//----------------------------------------------------------------------------------------------------------------------------------------------------
//              vc_h264_parser.cpp 
//----------------------------------------------------------------------------------------------------------------------------------------------------
// FINAL VERSION DONT CHANGE
// seems to work as intended
#include "vc_h264_parser.h"
#include <string.h>
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CONSTRUCTOR / DECONSTRUCTOR
//----------------------------------------------------------------------------------------------------------------------------------------------------
CH264Parser::CH264Parser(void)
{
    // Nothing to initialize - we use external storage
}
CH264Parser::~CH264Parser(void)
{
    // Nothing to clean up - we don't own any memory
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              USER API
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool CH264Parser::ParseVideo(int video_index,
                             const uint8_t* buffer,
                             size_t size)
{
    const uint8_t* data = buffer;
    // Skip potential non-standard leading byte
    if (size > 4 && data[0] != 0 && data[1] == 0 && data[2] == 0) 
        {
        data++;
        size--;
        }
    // Reset metadata & log buffer index
    m_video_width[video_index]    = 0;
    m_video_height[video_index]   = 0;
    m_vid_profile[video_index]    = 0;
    m_vid_level[video_index]      = 0;
    m_vid_is_valid[video_index]   = false;
    m_frame_count[video_index]    = 0;
    m_extradata_valid[video_index] = false;
    m_extradata_len[video_index]   = 0;
    // SPS/PPS offsets for extradata
    size_t sps_off = 0, pps_off = 0;
    size_t sps_len = 0, pps_len = 0;
    bool found_sps = false;
    bool found_pps = false;
    // --- First pass: find SPS/PPS ---
    size_t pos = 0;
    while (pos < size - 3) 
        {
        pos = FindNextStartCode(data, pos, size);
        if (pos >= size - 3) 
            {
            break;
            }
        size_t sc_len = (data[pos + 2] == 1) ? 3 : 4;
        uint8_t nal_type = data[pos + sc_len] & 0x1F;

        size_t next_pos = FindNextStartCode(data, pos + sc_len, size);
        size_t nal_size = (next_pos == size)
                        ? (size - pos - sc_len)
                        : (next_pos - pos - sc_len);

        if (nal_type == NAL_TYPE_SPS && !found_sps) 
            {
            sps_off = pos;
            // Clean SPS
            uint8_t clean_sps[1024];
            size_t clean_idx = 0;
            for (size_t i = 1; i < nal_size && clean_idx < sizeof(clean_sps); i++) 
                {
                if (i >= 3 &&
                    data[pos + sc_len + i - 2] == 0 &&
                    data[pos + sc_len + i - 1] == 0 &&
                    data[pos + sc_len + i] == 3)
                    continue;
                clean_sps[clean_idx++] = data[pos + sc_len + i];
                }
            // Parse SPS -> width/height/profile/level
            if (ParseSPS(clean_sps, clean_idx,
                         &m_video_width[video_index],
                         &m_video_height[video_index],
                         &m_vid_profile[video_index],
                         &m_vid_level[video_index])) 
                         {
                        found_sps = true;
                        // log parsed SPS info
                        ParserstoreLog("SPS width/height",
                                    m_video_width[video_index],
                                    m_video_height[video_index]);
                        ParserstoreLog("SPS profile/level",
                                    m_vid_profile[video_index],
                                    m_vid_level[video_index]);
                        }
            }
        else if (nal_type == NAL_TYPE_PPS && !found_pps) 
            {
            pps_off = pos;
            found_pps = true;
            }
        if (found_sps && found_pps) 
            {
            break;
            }
        pos = next_pos;
    }
    // --- Build extradata ---
    if (found_sps && found_pps) 
        {
        size_t sc_sps = (data[sps_off+2] == 1) ? 3 : 4;
        size_t sc_pps = (data[pps_off+2] == 1) ? 3 : 4;
        sps_len = pps_off - (sps_off + sc_sps);

        size_t next_after_pps = FindNextStartCode(data, pps_off + sc_pps, size);
        if (next_after_pps > size) next_after_pps = size;
        pps_len = next_after_pps - (pps_off + sc_pps);

        static const uint8_t sc[4] = {0,0,0,1};
        size_t out_pos = 0;
        memcpy(m_extradata[video_index] + out_pos, sc, 4); out_pos += 4;
        memcpy(m_extradata[video_index] + out_pos, data + sps_off + sc_sps, sps_len); out_pos += sps_len;
        memcpy(m_extradata[video_index] + out_pos, sc, 4); out_pos += 4;
        memcpy(m_extradata[video_index] + out_pos, data + pps_off + sc_pps, pps_len); out_pos += pps_len;

        m_extradata_len[video_index]   = out_pos;
        m_extradata_valid[video_index] = true;
        m_vid_is_valid[video_index]    = true;

        // log full extradata hex dump
        ParserstoreMsg(m_extradata[video_index],
                       m_extradata_len[video_index],
                       "EXTRADATA SPS+PPS");
        }
    // --- Second pass: find IDR frames ---
    int frame_idx = 0;
    pos = 0;
    while (pos < size - 3 && frame_idx < MAX_FRAMES) 
        {
        pos = FindNextStartCode(data, pos, size);
        if (pos >= size - 3)
            {
            break;
            }
        size_t sc_len = (data[pos + 2] == 1) ? 3 : 4;
        uint8_t nal_type = data[pos + sc_len] & 0x1F;

        if (nal_type == NAL_TYPE_IDR) 
            {
            // save pointer + length
            m_frame_address[video_index][frame_idx] = (void*)(data + pos);
            size_t next_pos = FindNextStartCode(data, pos + sc_len, size);
            if (next_pos < size)
                {
                m_framelenght[video_index][frame_idx] = next_pos - pos;
                }
            else
                {
                m_framelenght[video_index][frame_idx] = size - pos;
                }
            // log IDR frame pointer + length
            ParserstoreLog("IDR frame addr/len",
                           (u32)(uintptr_t)m_frame_address[video_index][frame_idx],
                           (u32)m_framelenght[video_index][frame_idx]);
            frame_idx++;
            pos = next_pos;
            } 
        else 
            {
            pos = FindNextStartCode(data, pos + sc_len, size);
            }
        }
    m_frame_count[video_index] = frame_idx;
    return m_vid_is_valid[video_index];
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
void            CH264Parser::ParserstoreLog              (   const char* label, u32 value1, u32 value2)
{
    // Always write the label
    for (const char* p = label; *p; ++p)
        m_DebugCharArray[m_CharIndex++] = *p;

    // If both values are placeholders, stop here
    if (value1 == STOREDEBUG_WHITESPACE && value2 == STOREDEBUG_WHITESPACE) 
        {
        m_DebugCharArray[m_CharIndex++] = '\n';
        m_DebugCharArray[m_CharIndex]   = '\0';
        return;
        }
    // If value is valid, write it
    if (value1 != STOREDEBUG_WHITESPACE) 
        {
        m_DebugCharArray[m_CharIndex++] = ' ';
        m_DebugCharArray[m_CharIndex++] = '0';
        m_DebugCharArray[m_CharIndex++] = 'x';
        for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i)
            {
            char hex = "0123456789ABCDEF"[(value1 >> (i * 4)) & 0xF];
            m_DebugCharArray[m_CharIndex++] = hex;
            }
        }
    // If second value is valid, write it
    if (value2 != STOREDEBUG_WHITESPACE) 
        {
        m_DebugCharArray[m_CharIndex++] = ' ';
        m_DebugCharArray[m_CharIndex++] = '0';
        m_DebugCharArray[m_CharIndex++] = 'x';
        for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) 
            {
            char hex = "0123456789ABCDEF"[(value2 >> (i * 4)) & 0xF];
            m_DebugCharArray[m_CharIndex++] = hex;
            }
        }
    // Terminate
    m_DebugCharArray[m_CharIndex++] = '\n';
    m_DebugCharArray[m_CharIndex]   = '\0';
}
void            CH264Parser::ParserstoreMsg              (   const void* tx_msg, u32 total_size, const char* label)
{   
    // insert leading newline
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;
    // copy label
    for (const char* p = label; *p; ++p) 
        {
        m_DebugCharArray[m_CharIndex] = *p;
        m_CharIndex++;
        }
    // next line please
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;
    // hex dump, 16 bytes per line
    const unsigned char* b = (const unsigned char*)tx_msg;
    for (u32 i = 0; i < total_size; ++i) 
        {
        if (i && (i % 16) == 0) 
            {
            m_DebugCharArray[m_CharIndex] = '\n';
            m_CharIndex++;
            }
        unsigned char v = b[i];

        char hi = "0123456789ABCDEF"[v >> 4];
        m_DebugCharArray[m_CharIndex] = hi;
        m_CharIndex++;

        char lo = "0123456789ABCDEF"[v & 0xF];
        m_DebugCharArray[m_CharIndex] = lo;
        m_CharIndex++;

        m_DebugCharArray[m_CharIndex] = ' ';
        m_CharIndex++;
        }
    // newline + terminator
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;    
    m_DebugCharArray[m_CharIndex] = '\0';
}
size_t CH264Parser::FindNextStartCode(u8* data, size_t pos, size_t size) const
{
    while (pos < size - 3) {
        if ((data[pos] == 0 && data[pos+1] == 0 && data[pos+2] == 1) ||
            (data[pos] == 0 && data[pos+1] == 0 && data[pos+2] == 0 && data[pos+3] == 1)) 
            {
            return pos;
            }
        pos++;
    }
    return size; // No more start codes found
}
u32 CH264Parser::ReadExpGolomb(u8* data, size_t* bit_offset) const
{
    size_t leadingZeroBits = 0;
    size_t offset = *bit_offset;
    size_t byte_offset = offset / 8;
    size_t bit_pos = offset % 8;
    
    // Count leading zeros
    while (1) 
        {
        if (bit_pos == 8) 
            {
            bit_pos = 0;
            byte_offset++;
            }
        
        if ((data[byte_offset] & (0x80 >> bit_pos)) != 0) 
            {
            break;
            }
        leadingZeroBits++;
        bit_pos++;
        offset++;
        }
    offset++; // Skip the stop bit
    bit_pos = offset % 8;
    byte_offset = offset / 8;
    // Read the coefficient bits
    u32 result = 0;
    for (size_t i = 0; i < leadingZeroBits; i++) 
        {
        result <<= 1;
        if (bit_pos == 8) 
            {
            bit_pos = 0;
            byte_offset++;
            }
        
        if ((data[byte_offset] & (0x80 >> bit_pos)) != 0) 
            {
            result |= 1;
            }
        bit_pos++;
        offset++;
        }
    
    result = (1 << leadingZeroBits) - 1 + result;
    *bit_offset = offset;
    return result;
}
bool CH264Parser::ParseSPS(u8* sps_data, size_t sps_size, u16* width, u16* height, u8* profile, u8* level) const
{
    // Ensure we have enough data
    if (sps_size < 3) 
        {
        return false;
        }
    // First byte is profile_idc
    *profile = sps_data[0];

    // NEW: store level_idc
    *level = sps_data[2];
    // END NEW

    // Skip constraint_set flags and level_idc (3 bytes total)
    size_t bit_offset = 24; // Skip 3 bytes (profile + constraint flags + level)

    // seq_parameter_set_id
    ReadExpGolomb(sps_data, &bit_offset);

    // FIX 5: Process all profiles without restriction
    // Handle high profile specific parameters if needed
    if (*profile >= 100) 
        {
        // chroma_format_idc
        u32 chroma_format_idc = ReadExpGolomb(sps_data, &bit_offset);

        if (chroma_format_idc == 3) 
            {
            // separate_colour_plane_flag
            bit_offset++; // Skip 1 bit
            }

        // bit_depth_luma_minus8
        ReadExpGolomb(sps_data, &bit_offset);

        // bit_depth_chroma_minus8
        ReadExpGolomb(sps_data, &bit_offset);

        // qpprime_y_zero_transform_bypass_flag
        bit_offset++; // Skip 1 bit

        // seq_scaling_matrix_present_flag
        u8 seq_scaling_matrix_present_flag = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
        bit_offset++;

        if (seq_scaling_matrix_present_flag) 
            {
            // Simple approximation for scaling matrix
            bit_offset += 8;
            }
        }

    // log2_max_frame_num_minus4
    ReadExpGolomb(sps_data, &bit_offset);

    // pic_order_cnt_type
    u32 pic_order_cnt_type = ReadExpGolomb(sps_data, &bit_offset);

    if (pic_order_cnt_type == 0) 
        {
        // log2_max_pic_order_cnt_lsb_minus4
        ReadExpGolomb(sps_data, &bit_offset);
        } 
    else if (pic_order_cnt_type == 1) 
        {
        // delta_pic_order_always_zero_flag
        bit_offset++;

        // offset_for_non_ref_pic
        ReadExpGolomb(sps_data, &bit_offset);

        // offset_for_top_to_bottom_field
        ReadExpGolomb(sps_data, &bit_offset);

        // num_ref_frames_in_pic_order_cnt_cycle
        u32 num_ref_frames_in_pic_order_cnt_cycle = ReadExpGolomb(sps_data, &bit_offset);

        for (u32 i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) 
            {
            // offset_for_ref_frame[i]
            ReadExpGolomb(sps_data, &bit_offset);
            }
        }

    // max_num_ref_frames
    ReadExpGolomb(sps_data, &bit_offset);

    // gaps_in_frame_num_value_allowed_flag
    bit_offset++;

    // pic_width_in_mbs_minus1
    u32 pic_width_in_mbs_minus1 = ReadExpGolomb(sps_data, &bit_offset);

    // pic_height_in_map_units_minus1
    u32 pic_height_in_map_units_minus1 = ReadExpGolomb(sps_data, &bit_offset);

    // frame_mbs_only_flag
    u8 frame_mbs_only_flag = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
    bit_offset++;

    // Calculate the dimensions
    *width = (pic_width_in_mbs_minus1 + 1) * 16;

    if (frame_mbs_only_flag) 
        {
        *height = (pic_height_in_map_units_minus1 + 1) * 16;
        } 
    else 
        {
        *height = (pic_height_in_map_units_minus1 + 1) * 32;

        // mb_adaptive_frame_field_flag
        bit_offset++;
        }

    // direct_8x8_inference_flag
    bit_offset++;

    // frame_cropping_flag
    u8 frame_cropping_flag = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
    bit_offset++;

    if (frame_cropping_flag) 
        {
        // Apply cropping to the dimensions
        u32 frame_crop_left_offset = ReadExpGolomb(sps_data, &bit_offset);
        u32 frame_crop_right_offset = ReadExpGolomb(sps_data, &bit_offset);
        u32 frame_crop_top_offset = ReadExpGolomb(sps_data, &bit_offset);
        u32 frame_crop_bottom_offset = ReadExpGolomb(sps_data, &bit_offset);

        // Adjust width and height based on cropping
        *width -= (frame_crop_left_offset + frame_crop_right_offset) * 2;

        if (frame_mbs_only_flag) 
            {
            *height -= (frame_crop_top_offset + frame_crop_bottom_offset) * 2;
            } 
        else 
            {
            *height -= (frame_crop_top_offset + frame_crop_bottom_offset) * 4;
            }
        }

    // NEW: VUI timing info for framerate
    {
        u8 vui = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
        bit_offset++;
        m_timing_info_present[vid] = vui;
        if (vui) 
            {
            // skip aspect_ratio_flag and any SAR
            u8 ar = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
            bit_offset++;
            if (ar) 
                {
                ReadExpGolomb(sps_data, &bit_offset);
                ReadExpGolomb(sps_data, &bit_offset);
                }
            // timing_info_present_flag
            u8 ti = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
            bit_offset++;
            if (ti) 
                {
                // num_units_in_tick (32 bits)
                u32 nu = (sps_data[bit_offset/8]   << 24)
                       | (sps_data[bit_offset/8+1] << 16)
                       | (sps_data[bit_offset/8+2] <<  8)
                       |  sps_data[bit_offset/8+3];
                bit_offset += 32;
                // time_scale (32 bits)
                u32 ts = (sps_data[bit_offset/8]   << 24)
                       | (sps_data[bit_offset/8+1] << 16)
                       | (sps_data[bit_offset/8+2] <<  8)
                       |  sps_data[bit_offset/8+3];
                bit_offset += 32;
                // fixed_frame_rate_flag
                u8 ff = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
                m_num_units_in_tick[vid]   = nu;
                m_time_scale_value [vid]   = ts;
                m_fixed_frame_rate [vid]   = ff;
                }
            }
    }
    // END NEW

    return true;
}