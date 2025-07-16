// vc_h264_parser.h
//
// FINAL VERSION DONT CHANGE
// seems to work as intended

#ifndef _vc_h264_parser_h
#define _vc_h264_parser_h

#include <circle/types.h>

// Maximum frames configuration
#define MAX_FRAMES 2048

// H.264 Profile IDs
#define H264_PROFILE_BASELINE 66
#define H264_PROFILE_MAIN     77
#define H264_PROFILE_HIGH     100

// NAL unit types
#define NAL_TYPE_SLICE 1
#define NAL_TYPE_IDR   5
#define NAL_TYPE_SEI   6
#define NAL_TYPE_SPS   7
#define NAL_TYPE_PPS   8

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
                                                                                                                        
/*
                                                                                                                        // Parse an H.264 video buffer using external storag
bool            ParseVideo                                  (   int             video_index,                            // video_index: Index of video in the arrays
                                                                char*           buffer_array[],                         // buffer_array: Array of pointers to H.264 video data
                                                                size_t          size_array[],                           // size_array: Array of sizes of video data in bytes
                                                                u16             vid_width[],                            // vid_width: Array to store parsed video width [video_index]
                                                                u16             vid_height[],                           // vid_height: Array to store parsed video height [video_index]
                                                                u8              vid_profile[],                          // vid_profile: Array to store parsed video profile [video_index]
                                                                u8              vid_level[],                            // NEW! NEED TO BE ADAPTED IN THE PROJECT
                                                                void*           frame_addresses[][MAX_FRAMES],          // frame_addresses: 2D array to store I-frame addresses [video_index][frame_index]
                                                                size_t          length_of_frames[][MAX_FRAMES],         // length_of_frames: 2D array to store I-frame sizes [video_index][frame_index]
                                                                int             number_of_frames[],                     // number_of_frames: Array to store frame count per video [video_index]
                                                                bool            is_video_valid[]);                      // is_video_valid: Array to store basic validity check [video_index]
                                                                                                                        // Returns true if parsing completed, false on error
// In vc_h264_parser.h, update the helper signature:
*/
bool            ParseVideo                                  (   int             video_index,
                                                                const uint8_t*  buffer,
                                                                size_t          size    )
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
private:
    // Helper methods
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

public: 
/* 
members 

u16         m_video_width[max_vid]                  //  stores the x resolution of the parsed video
u16         m_video_height[max_vid]                 //  stores the y resolution of the parsed video
u8          m_vid_profile[max_vid]                  //  stores the profile of the parsed video
u8          m_vid_level[max_vid]                    //  stores the level of the parsed video
void*       m_frame_address[max_vid][max_frames]    //  stores the addresses / pointers to the idr nal unit ( frames ) of the parsed video
size_t      m_framelenght[max_vid][max_frames]      //  stores the lengths of the idr nal units of the parsed video
int         m_frame_count[max_vid]                  //  stores the number of idr units / frames of the parsed video
bool        m_vid_is_valid[max_vid]                 //

uint8_t     m_extradata[max_vid][max_extradata]     //
size_t      m_extradata_len[max_vid]                //
bool        m_extradata_valid[max_vid]              //
*/
// ---------------- Video metadata per stream -----------------------------------------------------------------------------------------------------------------
u16 m_video_width[MAX_VIDEOS];                                  // Parsed width (pixels) from SPS for each video stream
u16 m_video_height[MAX_VIDEOS];                                 // Parsed height (pixels) from SPS for each video stream 
u8  m_vid_profile[MAX_VIDEOS];                                  // Parsed H.264 profile_idc for each video stream (e.g. 66=Baseline, 77=Main, 100=High)
u8  m_vid_level[MAX_VIDEOS];                                    // Parsed H.264 level_idc for each video stream (e.g. 0x29=Level 4.1)
bool m_vid_is_valid[MAX_VIDEOS];                                // Validity flag: true if SPS+PPS successfully parsed for this video stream
// ---------------- Frame storage per stream ------------------------------------------------------------------------------------------------------------------
void* m_frame_address[MAX_VIDEOS][MAX_FRAMES];                  // Pointer to each parsed IDR frame (points directly to start code of the NAL)
                                                                // Indexed by [video_index][frame_index]
size_t m_framelenght[MAX_VIDEOS][MAX_FRAMES];                   // Length (in bytes) of each parsed IDR frame
                                                                // Indexed by [video_index][frame_index]    
int m_frame_count[MAX_VIDEOS];                                  // Number of IDR frames found for this video stream
// ---------------- Extradata (SPS+PPS) per stream ------------------------------------------------------------------------------------------------------------
uint8_t m_extradata[MAX_VIDEOS][MAX_EXTRADATA];                 // Raw Annex-B extradata buffer containing SPS+PPS for each video stream
size_t m_extradata_len[MAX_VIDEOS];                             // Length (in bytes) of the extradata buffer for each video stream
bool m_extradata_valid[MAX_VIDEOS];                             // Validity flag: true if extradata is ready for SetPortInfo
// ---------------- Debug logging per stream ------------------------------------------------------------------------------------------------------------------
char m_DebugCharArray[MAX_VIDEOS][MMAL_MAX_DEBUG_FILE_LENGTH];  // Debug log string buffer for each video stream
                                                                // Contains SPS/metadata logs, extradata hex dump, IDR frame addresses/lengths
u32 m_CharIndex[MAX_VIDEOS];                                    // Current write index in the debug log buffer for each video stream


};

#endif // _vc_h264_parser_h