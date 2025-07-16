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
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
// In vc_h264_parser.h, update the helper signature:
bool            CreateExtradata                             (   const uint8_t* data,
                                                                size_t         size,
                                                                uint8_t*       out,
                                                                size_t*        out_length)

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

public: // members
};




#endif // _vc_h264_parser_h