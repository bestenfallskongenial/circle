// vc_h264_parser.h
//
// FINAL VERSION DONT CHANGE
// seems to work as intended

#ifndef _vc_h264_parser_h
#define _vc_h264_parser_h

#include <circle/types.h>

#define MAX_VIDEOS 8
#define MAX_TEXTURES 8

#define TEX_SIZE (1024*1024*4)  	                // 4194304 byte size of texture ( .bpm ) files

#define MAX_FRAMES 2048                                 // Maximum frames configuration

#define H264_PROFILE_BASELINE 66                        // H.264 Profile IDs
#define H264_PROFILE_MAIN     77
#define H264_PROFILE_HIGH     100

#define NAL_TYPE_SLICE 1                                // NAL unit types
#define NAL_TYPE_IDR   5
#define NAL_TYPE_SEI   6
#define NAL_TYPE_SPS   7
#define NAL_TYPE_PPS   8

#define STOREDEBUG_WHITESPACE 255
#define STOREDEBUG_NEWLINE '\n'
#define MMAL_MAX_DEBUG_FILE_LENGTH (1024 * 64)

#define MMAL_FORMAT_EXTRADATA_MAX_SIZE 128              // Maximum size of the format extradata. //

class CH264Parser
{
public:
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CONSTRUCTOR / DECONSTRUCTOR
//----------------------------------------------------------------------------------------------------------------------------------------------------
    CH264Parser(void);
    ~CH264Parser(void);
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              USER API
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool            ParseInitialize         (       int     max_textures,
                                                u32     max_tex_size,
                                                int     max_videos,  
                                                int     max_frames,
                                                u16     max_width,
                                                u16     max_height,
                                                u8      max_profile,
                                                u8      max_level);
bool            ParseVideo              (       int     file_index,
                                                char*   buffer_array[], 
                                                size_t  size_array[]);
bool            ParseBPM                (       int     file_index,
                                                char*   buffer_array[],
                                                size_t  size_array[]);
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
private:
        void    ParserStoreLog                              (   int video_index,const char* label, 
                                                                u32 value1 = STOREDEBUG_WHITESPACE, 
                                                                u32 value2 = STOREDEBUG_WHITESPACE);
        void    ParserStoreMsg                              (   int video_index,const void* tx_msg, u32 total_size, const char* label);
size_t          FindNextStartCode                           (   u8*             data, 
                                                                size_t          pos, 
                                                                size_t          size) const;
bool            ParseSPS                                    (   u8*             sps_data, 
                                                                size_t          sps_size, 
                                                                u16*            width, 
                                                                u16*            height, 
                                                                u8*             profile, 
                                                                u8*             level) const;
u32             ReadExpGolomb                               (   u8*             data, 
                                                                size_t*         bit_offset) const;
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
public: 
        int     m_max_textures;
        u32     m_max_tex_size;

        int     m_max_videos;
        int     m_max_frames;
        u16     m_max_width;
        u16     m_max_height;
        u8      m_max_profile;
        u8      m_max_level;
        // ---------------- Video metadata per stream -------------------------------------------------------------------------------------------------------------------------
        u16     m_video_width[MAX_VIDEOS];                                              // Parsed width (pixels) from SPS for each video stream
        u16     m_video_height[MAX_VIDEOS];                                             // Parsed height (pixels) from SPS for each video stream 
        u8      m_vid_profile[MAX_VIDEOS];                                              // Parsed H.264 profile_idc for each video stream (e.g. 66=Baseline, 77=Main, 100=High)
        u8      m_vid_level[MAX_VIDEOS];                                                // Parsed H.264 level_idc for each video stream (e.g. 0x29=Level 4.1)
        bool    m_vid_is_valid[MAX_VIDEOS];                                             // Validity flag: true if SPS+PPS successfully parsed for this video stream
        // ---------------- Frame storage per stream --------------------------------------------------------------------------------------------------------------------------
        void*   m_frame_address[MAX_VIDEOS][MAX_FRAMES];                                // Pointer to each parsed IDR frame (points directly to start code of the NAL)
                                                                                        // Indexed by [video_index][frame_index]
        size_t  m_framelenght[MAX_VIDEOS][MAX_FRAMES];                                  // Length (in bytes) of each parsed IDR frame
                                                                                        // Indexed by [video_index][frame_index]    
        size_t  m_idr_offset[MAX_VIDEOS];
        int     m_frame_count[MAX_VIDEOS];                                              // Number of IDR frames found for this video stream
        // ---------------- Extradata (SPS+PPS) per stream --------------------------------------------------------------------------------------------------------------------
        u8      m_extradata[MAX_VIDEOS][MMAL_FORMAT_EXTRADATA_MAX_SIZE];                // Raw Annex-B extradata buffer containing SPS+PPS for each video stream
        size_t  m_extradata_len[MAX_VIDEOS];                                            // Length (in bytes) of the extradata buffer for each video stream
        bool    m_extradata_valid[MAX_VIDEOS];                                          // Validity flag: true if extradata is ready for SetPortInfo
        // ---------------- Debug logging per stream --------------------------------------------------------------------------------------------------------------------------
        char    m_DebugCharArray[MAX_VIDEOS][MMAL_MAX_DEBUG_FILE_LENGTH] = { 0 };       // Debug log string buffer for each video stream
                                                                                        // Contains SPS/metadata logs, extradata hex dump, IDR frame addresses/lengths
        u32     m_CharIndex[MAX_VIDEOS] = { 0 };                                        // Current write index in the debug log buffer for each video stream
        // ---------------- Per-texture metadata arrays -----------------------------------------------------------------------------------------------------------------------
        bool    m_tex_valid[MAX_TEXTURES];                                              // true if header passed all BMP checks
        u32     m_tex_file_size[MAX_TEXTURES];                                          // value of the BMP “file size” field
        u32     m_tex_data_offset[MAX_TEXTURES];                                        // value of the BMP “pixel data” offset field
        u16     m_tex_width[MAX_TEXTURES];                                              // decoded width from the BMP header
        u16     m_tex_height[MAX_TEXTURES];                                             // decoded height from the BMP header
        u32     m_tex_image_size[MAX_TEXTURES];                                         // decoded image-data byte count (width×height×3)
};

#endif // _vc_h264_parser_h