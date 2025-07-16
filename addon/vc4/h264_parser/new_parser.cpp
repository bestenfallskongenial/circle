//----------------------------------------------------------------------------------------------------------------------------------------------------
//              vc_h264_parser.cpp 
//----------------------------------------------------------------------------------------------------------------------------------------------------


//
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
bool CH264Parser::ParseVideo(   int     video_index,
                                char*   buffer_array[], 
                                size_t  size_array[], 
                                u16     vid_width[], 
                                u16     vid_height[], 
                                u8      vid_profile[],
                                u8      vid_level[],
                                void*   frame_addresses[][MAX_FRAMES],
                                size_t  length_of_frames[][MAX_FRAMES],
                                int     number_of_frames[],
                                bool    is_video_valid[])
{
    // Get the specific buffer and size for this video using the video_index
    u8* data = (u8*)buffer_array[video_index];
    size_t size = size_array[video_index];
    
    // FIX 1: Skip non-standard header byte if present
    if (size > 4 && data[0] != 0 && data[1] == 0 && data[2] == 0) {
        // Skip the first byte which appears to be a non-standard header
        data++;
        size--;
    }
    

    u8* last_sps = nullptr;
    u8* last_pps = nullptr;

    size_t pos = 0;
    
    // Initialize validation state
    bool found_sps = false;
    bool found_pps = false;
    bool found_idr = false;
    
    // Initialize output values
    vid_width[video_index] = 0;
    vid_height[video_index] = 0;
    vid_profile[video_index] = 0;
    is_video_valid[video_index] = false;
    number_of_frames[video_index] = 0;
    
    // First pass: Parse metadata
    while (pos < size - 3) {
        // Find start code
        pos = FindNextStartCode(data, pos, size);
        if (pos >= size - 3) break;
        
        // Determine start code length (3 or 4 bytes)
        size_t start_code_len = (data[pos+2] == 1) ? 3 : 4;
        
        // Get NAL unit type
        u8 nal_type = data[pos + start_code_len] & 0x1F;
        
        // Find next start code to calculate NAL unit size
        size_t next_pos = FindNextStartCode(data, pos + start_code_len, size);
        size_t nal_size = (next_pos == size) ? (size - pos - start_code_len) : (next_pos - pos - start_code_len);
        
        // Process based on NAL type
        if (nal_type == NAL_TYPE_SPS) {
            // FIX 2: Create a clean copy of SPS data
            u8 clean_sps[1024]; 
            size_t clean_idx = 0;
            
            // Skip the NAL header byte (first byte)
            // FIX 3: Process SPS data starting after NAL header
            for (size_t i = 1; i < nal_size && clean_idx < sizeof(clean_sps); i++) {
                // FIX 4: Skip emulation prevention bytes (0x03 after two 0x00)
                if (i >= 3 && 
                    data[pos + start_code_len + i - 2] == 0 && 
                    data[pos + start_code_len + i - 1] == 0 && 
                    data[pos + start_code_len + i] == 3) {
                    continue; // Skip the emulation prevention byte
                }
                
                // Copy regular data byte
                clean_sps[clean_idx++] = data[pos + start_code_len + i];
            }
            
            // Parse the clean SPS data
            if (ParseSPS(clean_sps, clean_idx, 
                      &vid_width[video_index], &vid_height[video_index], &vid_profile[video_index], &vid_level[video_index])) {
                found_sps = true;
            }
        } 
        else if (nal_type == NAL_TYPE_PPS) {
            found_pps = true;
        } 
        else if (nal_type == NAL_TYPE_IDR) {
            found_idr = true;
        }
        
        // Move to next NAL unit
        pos = next_pos;
    }
    
    // Basic validation - check if we found SPS and PPS
    is_video_valid[video_index] = found_sps && found_pps;
    
    // Extract I-frame data
    int frame_count = 0;
    pos = 0;
  
    while (pos < size - 3 && frame_count < MAX_FRAMES) {
        // Find start code
        pos = FindNextStartCode(data, pos, size);
        if (pos >= size - 3) break;
        
        // Determine start code length
        size_t start_code_len = (data[pos+2] == 1) ? 3 : 4;
        
        // Get NAL unit type
        u8 nal_type = data[pos + start_code_len] & 0x1F;
        
        // Only process IDR frames (I-frames)
        if (nal_type == NAL_TYPE_IDR) {
            // Store frame start address
            frame_addresses[video_index][frame_count] = data + pos;
            
            // Find next start code
            size_t next_pos = FindNextStartCode(data, pos + start_code_len, size);
            
            // Calculate and store frame length
            if (next_pos < size) {
                length_of_frames[video_index][frame_count] = next_pos - pos;
            } else {
                // Last frame in file
                length_of_frames[video_index][frame_count] = size - pos;
            }
            
            frame_count++;
            pos = next_pos;
        } else {
            // Skip to next NAL unit
            size_t next_pos = FindNextStartCode(data, pos + start_code_len, size);
            pos = next_pos;
        }
    }

    // Store total number of frames found
    number_of_frames[video_index] = frame_count;
    
    return true;
}

bool CH264Parser::CreateExtradata(const uint8_t* data,
                                  size_t         size,
                                  uint8_t*       out,
                                  size_t*        out_length)
{
    if (!data || size < 8) return false;

    static const uint8_t sc[4] = {0,0,0,1};

    size_t pos = 0;
    size_t sps_off = 0, pps_off = 0;
    size_t sps_len = 0, pps_len = 0;

    // 1) find SPS (NAL type 7)
    while (true) {
        size_t off = FindNextStartCode(data, pos, size);
        if (off >= size) return false;

        size_t sc_len = (data[off+2] == 1) ? 3 : 4;
        uint8_t nal_type = data[off + sc_len] & 0x1F;

        if (nal_type == 7) { // SPS
            sps_off = off;
            pos = off + sc_len;
            break;
        }
        pos = off + sc_len;
    }

    // 2) find PPS (NAL type 8)
    while (true) {
        size_t off = FindNextStartCode(data, pos, size);
        if (off >= size) return false;

        size_t sc_len = (data[off+2] == 1) ? 3 : 4;
        uint8_t nal_type = data[off + sc_len] & 0x1F;

        if (nal_type == 8) { // PPS
            pps_off = off;
            pos = off + sc_len;
            break;
        }
        pos = off + sc_len;
    }

    // 3) calculate SPS/PPS lengths (exclude their start codes)
    size_t sc_sps = (data[sps_off+2] == 1) ? 3 : 4;
    size_t sc_pps = (data[pps_off+2] == 1) ? 3 : 4;

    sps_len = pps_off - (sps_off + sc_sps);

    // find the next NAL to know where PPS ends
    size_t next_after_pps = FindNextStartCode(data, pps_off + sc_pps, size);
    if (next_after_pps > size) next_after_pps = size;

    pps_len = next_after_pps - (pps_off + sc_pps);

    // 4) build Annex-B extradata: [00 00 00 01][SPS][00 00 00 01][PPS]
    size_t out_pos = 0;
    memcpy(out + out_pos, sc, 4);                        out_pos += 4;
    memcpy(out + out_pos, data + sps_off + sc_sps, sps_len); out_pos += sps_len;
    memcpy(out + out_pos, sc, 4);                        out_pos += 4;
    memcpy(out + out_pos, data + pps_off + sc_pps, pps_len); out_pos += pps_len;

    *out_length = out_pos;
    return true;
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
    if (value1 == STOREDEBUG_WHITESPACE && value2 == STOREDEBUG_WHITESPACE) {
        m_DebugCharArray[m_CharIndex++] = '\n';
        m_DebugCharArray[m_CharIndex]   = '\0';
        return;
    }

    // If value is valid, write it
    if (value1 != STOREDEBUG_WHITESPACE) {
        m_DebugCharArray[m_CharIndex++] = ' ';
        m_DebugCharArray[m_CharIndex++] = '0';
        m_DebugCharArray[m_CharIndex++] = 'x';
        for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) {
            char hex = "0123456789ABCDEF"[(value1 >> (i * 4)) & 0xF];
            m_DebugCharArray[m_CharIndex++] = hex;
        }
    }

    // If second value is valid, write it
    if (value2 != STOREDEBUG_WHITESPACE) {
        m_DebugCharArray[m_CharIndex++] = ' ';
        m_DebugCharArray[m_CharIndex++] = '0';
        m_DebugCharArray[m_CharIndex++] = 'x';
        for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) {
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
    for (u32 i = 0; i < total_size; ++i) {
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
            (data[pos] == 0 && data[pos+1] == 0 && data[pos+2] == 0 && data[pos+3] == 1)) {
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
    while (1) {
        if (bit_pos == 8) {
            bit_pos = 0;
            byte_offset++;
        }
        
        if ((data[byte_offset] & (0x80 >> bit_pos)) != 0) {
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
    for (size_t i = 0; i < leadingZeroBits; i++) {
        result <<= 1;
        if (bit_pos == 8) {
            bit_pos = 0;
            byte_offset++;
        }
        
        if ((data[byte_offset] & (0x80 >> bit_pos)) != 0) {
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
    if (sps_size < 3) return false;

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
    if (*profile >= 100) {
        // chroma_format_idc
        u32 chroma_format_idc = ReadExpGolomb(sps_data, &bit_offset);

        if (chroma_format_idc == 3) {
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

        if (seq_scaling_matrix_present_flag) {
            // Simple approximation for scaling matrix
            bit_offset += 8;
        }
    }

    // log2_max_frame_num_minus4
    ReadExpGolomb(sps_data, &bit_offset);

    // pic_order_cnt_type
    u32 pic_order_cnt_type = ReadExpGolomb(sps_data, &bit_offset);

    if (pic_order_cnt_type == 0) {
        // log2_max_pic_order_cnt_lsb_minus4
        ReadExpGolomb(sps_data, &bit_offset);
    } else if (pic_order_cnt_type == 1) {
        // delta_pic_order_always_zero_flag
        bit_offset++;

        // offset_for_non_ref_pic
        ReadExpGolomb(sps_data, &bit_offset);

        // offset_for_top_to_bottom_field
        ReadExpGolomb(sps_data, &bit_offset);

        // num_ref_frames_in_pic_order_cnt_cycle
        u32 num_ref_frames_in_pic_order_cnt_cycle = ReadExpGolomb(sps_data, &bit_offset);

        for (u32 i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
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

    if (frame_mbs_only_flag) {
        *height = (pic_height_in_map_units_minus1 + 1) * 16;
    } else {
        *height = (pic_height_in_map_units_minus1 + 1) * 32;

        // mb_adaptive_frame_field_flag
        bit_offset++;
    }

    // direct_8x8_inference_flag
    bit_offset++;

    // frame_cropping_flag
    u8 frame_cropping_flag = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
    bit_offset++;

    if (frame_cropping_flag) {
        // Apply cropping to the dimensions
        u32 frame_crop_left_offset = ReadExpGolomb(sps_data, &bit_offset);
        u32 frame_crop_right_offset = ReadExpGolomb(sps_data, &bit_offset);
        u32 frame_crop_top_offset = ReadExpGolomb(sps_data, &bit_offset);
        u32 frame_crop_bottom_offset = ReadExpGolomb(sps_data, &bit_offset);

        // Adjust width and height based on cropping
        *width -= (frame_crop_left_offset + frame_crop_right_offset) * 2;

        if (frame_mbs_only_flag) {
            *height -= (frame_crop_top_offset + frame_crop_bottom_offset) * 2;
        } else {
            *height -= (frame_crop_top_offset + frame_crop_bottom_offset) * 4;
        }
    }

    // NEW: VUI timing info for framerate
    {
        u8 vui = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
        bit_offset++;
        m_timing_info_present[vid] = vui;
        if (vui) {
            // skip aspect_ratio_flag and any SAR
            u8 ar = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
            bit_offset++;
            if (ar) {
                ReadExpGolomb(sps_data, &bit_offset);
                ReadExpGolomb(sps_data, &bit_offset);
            }
            // timing_info_present_flag
            u8 ti = (sps_data[bit_offset/8] >> (7 - (bit_offset % 8))) & 0x01;
            bit_offset++;
            if (ti) {
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