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
bool    CH264Parser::ParseInitialize (  int         max_textures,
                                        u32         max_tex_size,
                                        int         max_videos,  
                                        int         max_frames,
                                        u16         max_width,
                                        u16         max_height,
                                        u8          max_profile,
                                        u8          max_level)
{
    m_max_textures = max_textures;
    m_max_tex_size = max_tex_size;

    m_max_videos  = max_videos;
    m_max_frames  = max_frames;
    m_max_width   = max_width;
    m_max_height  = max_height;
    m_max_profile = max_profile;
    m_max_level   = max_level;

    if ( max_videos != MAX_VIDEOS ) return false;

    return true;
}
bool CH264Parser::ParseVideo(    int     file_index, char*   buffer_array[], size_t  size_array[])
{
    m_CharIndex[file_index] = 0;
    memset(m_DebugCharArray[file_index], 
                0, 
                sizeof m_DebugCharArray[file_index]);

    u8* data  = (u8*)buffer_array[file_index];
    size_t size = size_array[file_index];

    
    if (size > 4 && data[0] != 0 && data[1] == 0 && data[2] == 0)                                                   // Skip potential non-standard leading byte
        {
        data++;
        size--;
        }

    ParserStoreLog(file_index,"Parser / Frame Log for Video  ", file_index);

    m_video_width[file_index]    = 0;                                                                              // Reset metadata & log buffer index
    m_video_height[file_index]   = 0;
    m_vid_profile[file_index]    = 0;
    m_vid_level[file_index]      = 0;
    m_vid_is_valid[file_index]   = false;
    m_frame_count[file_index]    = 0;
    m_extradata_valid[file_index] = false;
    m_extradata_len[file_index]   = 0;

    size_t sps_off = 0, pps_off = 0;                                                                                // SPS/PPS offsets for extradata
    size_t sps_len = 0, pps_len = 0;
    bool found_sps = false;
    bool found_pps = false;
    
    size_t pos = 0;                                                                                                 // --- First pass: find SPS/PPS ---
    while (pos < size - 3) 
        {
        pos = FindNextStartCode(data, pos, size);
        if (pos >= size - 3) break;

        size_t sc_len = (data[pos + 2] == 1) ? 3 : 4;
        u8 nal_type = data[pos + sc_len] & 0x1F;

        size_t next_pos = FindNextStartCode(data, pos + sc_len, size);
        size_t nal_size = (next_pos == size)
                        ? (size - pos - sc_len)
                        : (next_pos - pos - sc_len);
        if (nal_type == NAL_TYPE_SPS && !found_sps) 
            {
            sps_off = pos;  
            u8 clean_sps[1024];                                                                                     // Clean SPS
            size_t clean_idx = 0;
            for (size_t i = 1; i < nal_size && clean_idx < sizeof(clean_sps); i++) {
                if (i >= 3 &&
                    data[pos + sc_len + i - 2] == 0 &&
                    data[pos + sc_len + i - 1] == 0 &&
                    data[pos + sc_len + i] == 3)
                    continue;
                clean_sps[clean_idx++] = data[pos + sc_len + i];
            }   
            if (ParseSPS(   clean_sps, clean_idx,                                                                   // Parse SPS -> width/height/profile/level
                            &m_video_width[file_index],
                            &m_video_height[file_index],
                            &m_vid_profile[file_index],
                            &m_vid_level[file_index])) {
                found_sps = true;
                // log parsed SPS info
                ParserStoreLog(file_index,"\nSPS width/height   ", m_video_width[file_index], m_video_height[file_index]);
                ParserStoreLog(file_index,"SPS profile/level  ", m_vid_profile[file_index], m_vid_level[file_index]);
            }
        }
        else if (nal_type == NAL_TYPE_PPS && !found_pps) 
            {
            pps_off = pos;
            found_pps = true;
            }
        if (found_sps && found_pps) break;
        pos = next_pos;
        }
    
    if (found_sps && found_pps)                                                                                     // --- Build extradata ---
        {
        size_t sc_sps = (data[sps_off+2] == 1) ? 3 : 4;
        size_t sc_pps = (data[pps_off+2] == 1) ? 3 : 4;
        sps_len = pps_off - (sps_off + sc_sps);

        size_t next_after_pps = FindNextStartCode(data, pps_off + sc_pps, size);
        if (next_after_pps > size) next_after_pps = size;
        pps_len = next_after_pps - (pps_off + sc_pps);

        static const u8 sc[4] = {0,0,0,1};
        size_t out_pos = 0;

        memcpy(m_extradata[file_index] + out_pos, sc, 4); out_pos += 4;
        memcpy(m_extradata[file_index] + out_pos, data + sps_off + sc_sps, sps_len); out_pos += sps_len;
        memcpy(m_extradata[file_index] + out_pos, sc, 4); out_pos += 4;
        memcpy(m_extradata[file_index] + out_pos, data + pps_off + sc_pps, pps_len); out_pos += pps_len;

        m_extradata_len[file_index]   = out_pos;
        m_extradata_valid[file_index] = true;
        m_vid_is_valid[file_index]    = true;

        ParserStoreMsg(file_index,m_extradata[file_index], m_extradata_len[file_index], "EXTRADATA SPS+PPS\n");  // log full extradata hex dump
    }
    int frame_idx = 0;                                                                                              // --- Second pass: find IDR frames ---
    pos = 0;
    while (pos < size - 3 && frame_idx < MAX_FRAMES) 
        {
        pos = FindNextStartCode(data, pos, size);
        if (pos >= size - 3)
            {
            break;
            }
        size_t sc_len = (data[pos + 2] == 1) ? 3 : 4;
        u8 nal_type = data[pos + sc_len] & 0x1F;

        if (nal_type == NAL_TYPE_IDR) 
            {
            m_frame_address[file_index][frame_idx] = (void*)(data + pos);                                          // save pointer + length
            size_t next_pos = FindNextStartCode(data, pos + sc_len, size);
            if (next_pos < size)
                {
                m_framelenght[file_index][frame_idx] = next_pos - pos;
                }
            else
                {
                m_framelenght[file_index][frame_idx] = size - pos;
                }    
            ParserStoreLog(file_index,"IDR frame addr/len", (u32)m_frame_address[file_index][frame_idx], (u32)m_framelenght[file_index][frame_idx]);// log IDR frame pointer + length
            frame_idx++;
            pos = next_pos;
            }
        else 
            {
            pos = FindNextStartCode(data, pos + sc_len, size);
            }
        }
    m_frame_count[file_index] = frame_idx;

    ParserStoreLog(file_index,"\nParsed Frames           ",frame_idx);
    // Check resolution
    if (m_video_width[file_index]  != m_max_width || m_video_height[file_index] != m_max_height)
        {
        m_vid_is_valid[file_index] = false;
        }
    // Check profile
    if (m_vid_profile[file_index] != m_max_profile)
        {
        m_vid_is_valid[file_index] = false;
        }
    // Check level
    if (m_vid_level[file_index] != m_max_level)
        {
        m_vid_is_valid[file_index] = false;
        }
     if (m_vid_is_valid[file_index])
        {   
        ParserStoreLog(file_index,"\nMetaData Valid for Video",file_index);
        }
    else    
        {   
        ParserStoreLog(file_index,"\nMetaData Invalid for Video",file_index);
        }

        return m_vid_is_valid[file_index];
}
bool CH264Parser::ParseBPM          (int file_index, char* buffer_array[], size_t size_array[])
{
    m_CharIndex[file_index] = 0;
    memset(m_DebugCharArray[file_index], 
                0, 
                sizeof m_DebugCharArray[file_index]);

    u8*    data = reinterpret_cast<u8*>(buffer_array[file_index]);
    size_t size = size_array[file_index];

    // — initialize log entry for this texture —
    ParserStoreLog(file_index, "=== BMP header parse start ===", file_index);
/*
    if (size < 38)
    {
        ParserStoreLog(file_index, "BMP too small to parse", 
                       static_cast<u32>(size));
        return m_tex_valid[file_index] = false;
    }
*/
    // read fields
    u32 fileSize    = data[2]  | (data[3]<<8)  | (data[4]<<16)  | (data[5]<<24);
    u32 dataOffset  = data[10] | (data[11]<<8) | (data[12]<<16) | (data[13]<<24);
    u32 headerSize  = data[14] | (data[15]<<8) | (data[16]<<16) | (data[17]<<24);
    u16 planes      = data[26] | (data[27]<<8);
    u16 bpp         = data[28] | (data[29]<<8);
    u32 compression = data[30] | (data[31]<<8) | (data[32]<<16) | (data[33]<<24);
    u32 width       = data[18] | (data[19]<<8) | (data[20]<<16) | (data[21]<<24);
    u32 height      = data[22] | (data[23]<<8) | (data[24]<<16) | (data[25]<<24);
    u32 imgSize     = data[34] | (data[35]<<8) | (data[36]<<16) | (data[37]<<24);


    // after you've decoded width/height:
if ((width  & 3) != 0 ||    // not multiple of 4
    (height & 3) != 0)
{
  // fail alignment test
  m_tex_valid[file_index] = false;
  ParserStoreLog(file_index,
                 "BMP dim not 4-aligned",
                 width, height);
  return false;
}
    // log raw header fields
    ParserStoreLog(file_index, "BMP fileSize/dataOffset  ", fileSize, dataOffset);
    ParserStoreLog(file_index, "BMP headerSize/planes    ", headerSize, planes);
    ParserStoreLog(file_index, "BMP bpp/compression      ", bpp, compression);
    ParserStoreLog(file_index, "BMP width/height         ", width, height);
    ParserStoreLog(file_index, "BMP imgSize              ", imgSize);

    // (optional) dump the first 38 bytes of the header
    ParserStoreMsg(file_index, data, 38, "BMP Header Hex Dump");

    // validate
    bool ok = 
         data[0]=='B' && data[1]=='M'
      && fileSize   <= m_max_tex_size
      && headerSize == 40
      && planes     == 1
      && bpp        == 24
      && compression== 0
      && width*height*3 == imgSize;

    m_tex_valid[file_index]       = ok;
    m_tex_file_size[file_index]   = fileSize;
    m_tex_data_offset[file_index] = dataOffset;
    m_tex_width[file_index]       = static_cast<u16>(width);
    m_tex_height[file_index]      = static_cast<u16>(height);
    m_tex_image_size[file_index]  = imgSize;

    // final status
//  ParserStoreLog(file_index,
//                 ok ? "BMP header VALID" : "BMP header FAILED",
//                 STOREDEBUG_WHITESPACE,
//                 STOREDEBUG_WHITESPACE);

    ParserStoreLog(file_index,
                   ok ? "BMP header VALID" : "BMP header FAILED");

    return ok;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
void            CH264Parser::ParserStoreLog              ( int file_index, const char* label, u32 value1, u32 value2 )
{
    // Always write the label
    for (const char* p = label; *p; ++p)
        m_DebugCharArray[file_index][m_CharIndex[file_index]++] = *p;

    // If both values are placeholders, stop here
    if (value1 == STOREDEBUG_WHITESPACE && value2 == STOREDEBUG_WHITESPACE) 
        {
        m_DebugCharArray[file_index][m_CharIndex[file_index]++] = '\n';
        m_DebugCharArray[file_index][m_CharIndex[file_index]]   = '\0';
        return;
        }
    // If value is valid, write it
    if (value1 != STOREDEBUG_WHITESPACE) 
        {
        m_DebugCharArray[file_index][m_CharIndex[file_index]++] = ' ';
        m_DebugCharArray[file_index][m_CharIndex[file_index]++] = '0';
        m_DebugCharArray[file_index][m_CharIndex[file_index]++] = 'x';
        for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i)
            {
            char hex = "0123456789ABCDEF"[(value1 >> (i * 4)) & 0xF];
            m_DebugCharArray[file_index][m_CharIndex[file_index]++] = hex;
            }
        }
    // If second value is valid, write it
    if (value2 != STOREDEBUG_WHITESPACE) 
        {
        m_DebugCharArray[file_index][m_CharIndex[file_index]++] = ' ';
        m_DebugCharArray[file_index][m_CharIndex[file_index]++] = '0';
        m_DebugCharArray[file_index][m_CharIndex[file_index]++] = 'x';
        for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) 
            {
            char hex = "0123456789ABCDEF"[(value2 >> (i * 4)) & 0xF];
            m_DebugCharArray[file_index][m_CharIndex[file_index]++] = hex;
            }
        }
    // Terminate
    m_DebugCharArray[file_index][m_CharIndex[file_index]++] = '\n';
    m_DebugCharArray[file_index][m_CharIndex[file_index]]   = '\0';
}
void            CH264Parser::ParserStoreMsg              ( int file_index,const void* tx_msg, u32 total_size, const char* label )
{   
    // insert leading newline
    m_DebugCharArray[file_index][m_CharIndex[file_index]] = '\n';
    m_CharIndex[file_index]++;
    // copy label
    for (const char* p = label; *p; ++p) 
        {
        m_DebugCharArray[file_index][m_CharIndex[file_index]] = *p;
        m_CharIndex[file_index]++;
        }
    // next line please
    m_DebugCharArray[file_index][m_CharIndex[file_index]] = '\n';
    m_CharIndex[file_index]++;
    // hex dump, 16 bytes per line
    const unsigned char* b = (const unsigned char*)tx_msg;
    for (u32 i = 0; i < total_size; ++i) 
        {
        if (i && (i % 16) == 0) 
            {
            m_DebugCharArray[file_index][m_CharIndex[file_index]] = '\n';
            m_CharIndex[file_index]++;
            }
        unsigned char v = b[i];

        char hi = "0123456789ABCDEF"[v >> 4];
        m_DebugCharArray[file_index][m_CharIndex[file_index]] = hi;
        m_CharIndex[file_index]++;

        char lo = "0123456789ABCDEF"[v & 0xF];
        m_DebugCharArray[file_index][m_CharIndex[file_index]] = lo;
        m_CharIndex[file_index]++;

        m_DebugCharArray[file_index][m_CharIndex[file_index]] = ' ';
        m_CharIndex[file_index]++;
        }
    // newline + terminator
    m_DebugCharArray[file_index][m_CharIndex[file_index]] = '\n';
    m_CharIndex[file_index]++;
    m_DebugCharArray[file_index][m_CharIndex[file_index]] = '\n';
    m_CharIndex[file_index]++;    
    m_DebugCharArray[file_index][m_CharIndex[file_index]] = '\0';
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

    return true;
}