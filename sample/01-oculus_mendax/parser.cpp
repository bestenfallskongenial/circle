#include "kernel.h"
#include "global.h"

void            CKernel::parser_teture_bmp          (   int fromFile, int toFile) // we need to do something here about the logfile!!!
{
                CString log_message;

//              log_message.Format("Tex# Status   Filesize Offset Dimension BMP-Size    Filename\n");
//                                  Tex# Status   Size     Offset Dimension Filesize
//              g_log_string.Append(log_message);
                
                for (int i = fromFile; i < toFile; ++i)
                    {
                    if ( m_bufferTexture[i][0] == 'B' && m_bufferTexture[i][1] == 'M' &&
                    
                        (static_cast<uint8_t>(m_bufferTexture[i][2]) |                          // Extract file size (little-endian)
                        (static_cast<uint8_t>(m_bufferTexture[i][3]) << 8) |
                        (static_cast<uint8_t>(m_bufferTexture[i][4]) << 16) |
                        (static_cast<uint8_t>(m_bufferTexture[i][5]) << 24)) <= TEX_SIZE &&
                    
                        (static_cast<uint8_t>(m_bufferTexture[i][14]) |                         // Extract header size (little-endian)
                        (static_cast<uint8_t>(m_bufferTexture[i][15]) << 8) |
                        (static_cast<uint8_t>(m_bufferTexture[i][16]) << 16) |
                        (static_cast<uint8_t>(m_bufferTexture[i][17]) << 24)) == 40 &&
                    
                        (static_cast<uint8_t>(m_bufferTexture[i][26]) |                         // Extract color planes (little-endian)
                        (static_cast<uint8_t>(m_bufferTexture[i][27]) << 8)) == 1 &&
                    
                        (static_cast<uint8_t>(m_bufferTexture[i][28]) |                         // Extract bits per pixel (little-endian)
                        (static_cast<uint8_t>(m_bufferTexture[i][29]) << 8)) == 24 &&
                    
                        (static_cast<uint8_t>(m_bufferTexture[i][30]) |                         // Extract compression method (little-endian)
                        (static_cast<uint8_t>(m_bufferTexture[i][31]) << 8) |
                        (static_cast<uint8_t>(m_bufferTexture[i][32]) << 16) |
                        (static_cast<uint8_t>(m_bufferTexture[i][33]) << 24)) == 0 &&
                    
                        (static_cast<uint8_t>(m_bufferTexture[i][18]) |                         // Extract width (little-endian)
                        (static_cast<uint8_t>(m_bufferTexture[i][19]) << 8) |
                        (static_cast<uint8_t>(m_bufferTexture[i][20]) << 16) |
                        (static_cast<uint8_t>(m_bufferTexture[i][21]) << 24)) *
                    
                        (static_cast<uint8_t>(m_bufferTexture[i][22]) |                         // Extract height (little-endian)
                        (static_cast<uint8_t>(m_bufferTexture[i][23]) << 8) |
                        (static_cast<uint8_t>(m_bufferTexture[i][24]) << 16) |
                        (static_cast<uint8_t>(m_bufferTexture[i][25]) << 24)) * 3 ==
                    
                        (static_cast<uint8_t>(m_bufferTexture[i][34]) |                         // Extract image size (little-endian)
                        (static_cast<uint8_t>(m_bufferTexture[i][35]) << 8) |
                        (static_cast<uint8_t>(m_bufferTexture[i][36]) << 16) |
                        (static_cast<uint8_t>(m_bufferTexture[i][37]) << 24) ))
                        {
                        TEX_FILE_STATUS[i] =      true; // Valid BMP header
                        
                        TEX_FILE_SIZE[i] =       static_cast<uint8_t>(m_bufferTexture[i][2]) |       // Extract file size (little-endian)
                                           (static_cast<uint8_t>(m_bufferTexture[i][3]) << 8) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][4]) << 16) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][5]) << 24);
                        
                        TEX_FILE_BM_OFFSET[i] =   static_cast<uint8_t>(m_bufferTexture[i][10]) |      // Extract data offset (little-endian)
                                           (static_cast<uint8_t>(m_bufferTexture[i][11]) << 8) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][12]) << 16) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][13]) << 24);
                        
                        TEX_FILE_X_DIM[i] =     static_cast<uint8_t>(m_bufferTexture[i][18]) |      // Extract width (little-endian)
                                           (static_cast<uint8_t>(m_bufferTexture[i][19]) << 8) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][20]) << 16) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][21]) << 24);
                        
                        TEX_FILE_Y_DIM[i] =     static_cast<uint8_t>(m_bufferTexture[i][22]) |      // Extract height (little-endian)
                                           (static_cast<uint8_t>(m_bufferTexture[i][23]) << 8) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][24]) << 16) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][25]) << 24);
                        
                        TEX_FILE_BM_SIZE[i] =     static_cast<uint8_t>(m_bufferTexture[i][34]) |      // Extract image size (little-endian)
                                           (static_cast<uint8_t>(m_bufferTexture[i][35]) << 8) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][36]) << 16) |
                                           (static_cast<uint8_t>(m_bufferTexture[i][37]) << 24);
                        }
/*                      else
                        {
                        TEX_FILE_STATUS[i]    = false;
                        TEX_FILE_SIZE[i]     = 0;
                        TEX_FILE_BM_OFFSET[i] = 0;
                        TEX_FILE_X_DIM[i]   = 0;
                        TEX_FILE_Y_DIM[i]   = 0;
                        TEX_FILE_BM_SIZE[i]   = 0;
                        }   
                    log_message.Format("%-2d   %-6s   %-8d 0x%-4x %-4dx%-4d %-8d    %s\n",
                    i,
                    (TEX_FILE_STATUS[i] ? "Valid" : "Failed"),
                    TEX_FILE_SIZE[i], 
                    TEX_FILE_BM_OFFSET[i],
                    TEX_FILE_X_DIM[i],
                    TEX_FILE_Y_DIM[i],
                    TEX_FILE_BM_SIZE[i],
                    SCANED_FILES_TEX[i]);
                    
                    g_log_string.Append(log_message);
                    */
                    }
}

void            CKernel::parser_debug               ()
{
                m_H264Parser.ParseVideo(0,
                                        m_bufferVideo,
                                        VID_LOADED_BYTES,
                                        VID_WIDTH,
                                        VID_HEIGHT,
                                        VID_PROFILE,
                                        FRAME_ADDRESSES,
                                        FRAME_LENGTH,
                                        FRAME_COUNT,
                                        VID_IS_VALID
                                        );

                GenerateH264ParserInfo  (0,
                                        m_bufferVideo,
                                        VID_LOADED_BYTES,
                                        VID_WIDTH,
                                        VID_HEIGHT,
                                        VID_PROFILE,
                                        FRAME_ADDRESSES,
                                        FRAME_LENGTH,
                                        FRAME_COUNT,
                                        VID_IS_VALID
                                        );

                filesystem_save_log_file( "emmc1-1", FILENAME_PARSER0, h264_info);

                m_H264Parser.ParseVideo(1,
                                        m_bufferVideo,
                                        VID_LOADED_BYTES,
                                        VID_WIDTH,
                                        VID_HEIGHT,
                                        VID_PROFILE,
                                        FRAME_ADDRESSES,
                                        FRAME_LENGTH,
                                        FRAME_COUNT,
                                        VID_IS_VALID
                                        );

                GenerateH264ParserInfo  (1,
                                        m_bufferVideo,
                                        VID_LOADED_BYTES,
                                        VID_WIDTH,
                                        VID_HEIGHT,
                                        VID_PROFILE,
                                        FRAME_ADDRESSES,
                                        FRAME_LENGTH,
                                        FRAME_COUNT,
                                        VID_IS_VALID
                                        );

                filesystem_save_log_file( "emmc1-1", FILENAME_PARSER1, h264_info);        
/*
                m_H264Parser.ParseVideo(2,
                                        m_bufferVideo,
                                        VID_LOADED_BYTES,
                                        VID_WIDTH,
                                        VID_HEIGHT,
                                        VID_PROFILE,
                                        FRAME_ADDRESSES,
                                        FRAME_LENGTH,
                                        FRAME_COUNT,
                                        VID_IS_VALID
                                        );

                GenerateH264ParserInfo  (2,
                                        m_bufferVideo,
                                        VID_LOADED_BYTES,
                                        VID_WIDTH,
                                        VID_HEIGHT,
                                        VID_PROFILE,
                                        FRAME_ADDRESSES,
                                        FRAME_LENGTH,
                                        FRAME_COUNT,
                                        VID_IS_VALID
                                        );

                filesystem_save_log_file( "emmc1-1", FILENAME_PARSER2, h264_info);

                m_H264Parser.ParseVideo(3,
                                        m_bufferVideo,
                                        VID_LOADED_BYTES,
                                        VID_WIDTH,
                                        VID_HEIGHT,
                                        VID_PROFILE,
                                        FRAME_ADDRESSES,
                                        FRAME_LENGTH,
                                        FRAME_COUNT,
                                        VID_IS_VALID
                                        );

                GenerateH264ParserInfo  (3,
                                        m_bufferVideo,
                                        VID_LOADED_BYTES,
                                        VID_WIDTH,
                                        VID_HEIGHT,
                                        VID_PROFILE,
                                        FRAME_ADDRESSES,
                                        FRAME_LENGTH,
                                        FRAME_COUNT,
                                        VID_IS_VALID
                                        );

                filesystem_save_log_file( "emmc1-1", FILENAME_PARSER3, h264_info);    
*/
}
