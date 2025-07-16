#include "kernel.h"
#include <circle/string.h> 
#include <stdint.h>
/*
< extern        < type                 < name                                   < size
        <const
*/   
extern          CString                 g_barsString;
extern          CString                 g_log_string;
extern          CString                 g_error_string;
extern          CString                 g_modes;

extern          bool                    noTargetFPS;
extern          bool                    adc_pot_routing;
extern          bool                    resetFlag;
extern          bool                    g_show_bank;

extern          bool                    is_hold_for_2_sec_a;
extern          bool                    is_hold_for_2_sec_b;

extern          bool                    is_hold_for_4_sec_a;
extern          bool                    is_hold_for_4_sec_b;
extern          bool                    timer_mode;
extern          bool                    single_tex_mode;

extern          float                   current_fps;
extern          unsigned long           last_swap_duration;
extern          int                     last_gl_current_prg;  // Store last active shader program

extern          int                     current_buffer;

extern          int                     VSH_LOADED_OLD;
extern          int                     FSH_LOADED_OLD;
extern          int                     TEX_LOADED_OLD;
extern          int                     VID_LOADED_OLD;
extern          int                     VSH_LOADED_NEW;
extern          int                     FSH_LOADED_NEW;
extern          int                     TEX_LOADED_NEW;
extern          int                     VID_LOADED_NEW;   

extern          int                     scanned_vsh;
extern          int                     scanned_fsh;
extern          int                     scanned_tex;
extern          int                     scanned_vid; 

extern            unsigned totalBytesReadSD;
extern            unsigned totalBytesReadUSB;

extern bool           a_is_hold;
extern bool           b_is_hold;

extern          int                     gl_current_prg; // display_debug value
extern          int                     gl_current_tex;
extern          int                     linked_programs;
extern          int                     audio_source_channel;

extern          int                     g_menu_mode_new;                    // Current menu mode (0, 1, or 2)
extern          int                     g_menu_mode_old;
extern          int                     g_current_texture;
extern          int                     active_bpm_channel;
extern          int                     attenuation;
extern          unsigned                hFile;  // extern          bool                    showMenuFlag;

extern          unsigned long           start_time_fps_calculation;
extern          unsigned long           end_time_fps_calculation;
extern          unsigned long           total_time_fps_calculation;

extern          unsigned long           currentTime;
extern          unsigned long           targetTime;
extern          unsigned long           delayTime;


extern          float                   util_audio_smooth_band[4]; 
extern          float                   GLtime;
extern          float                   opaque;

//  Define file names on the sd card and usb stick
extern  const   char                    *VID__LOG_NAMES                          [VID_SD+VID_USB];

extern          char                   *SCANED_FILES_VSH                         [40];
extern          char                   *SCANED_FILES_FSH                         [40];
extern          char                   *SCANED_FILES_TEX                         [40];
extern          char                   *SCANED_FILES_VID                         [40];

extern          unsigned                VSH_LOADED_BYTES                        [VSH_SD + VSH_USB];
extern          unsigned                FSH_LOADED_BYTES                        [FSH_SD + FSH_USB];
extern          unsigned                TEX_LOADED_BYTES                        [TEX_SD + TEX_USB];
extern          unsigned                VID_LOADED_BYTES                        [VID_SD + VID_USB]; 
// Define arrays for bitmap metadata
extern          bool                    TEX_FILE_STATUS                         [TEX_SD + TEX_USB];                                
extern          uint32_t                TEX_FILE_SIZE                           [TEX_SD + TEX_USB];           // File size 
extern          uint32_t                TEX_FILE_BM_OFFSET                      [TEX_SD + TEX_USB];           // Offset 
extern          uint32_t                TEX_FILE_X_DIM                          [TEX_SD + TEX_USB];           // Width 
extern          uint32_t                TEX_FILE_Y_DIM                          [TEX_SD + TEX_USB];           // Height 
extern          uint32_t                TEX_FILE_BM_SIZE                        [TEX_SD + TEX_USB];           // Image size 
//  Define the multiplier for my LFO ( static when used inside the LFO function )
extern          int                     multiplier                              [7];
extern          unsigned long           last_circle_buffer                      [LFO_INSTANCES];
extern          unsigned long           next_circle_buffer                      [LFO_INSTANCES];
extern          float                   LFO_float_output                        [LFO_INSTANCES];
extern          int                     last_multiplier                         [LFO_INSTANCES];

extern          int                     adc_raw_value                           [ADC_CHANNELS];
extern          int                     adc_int_value                           [ADC_CHANNELS];
extern          float                   adc_float_value                         [ADC_CHANNELS];
extern          int                     output_int_value                        [ADC_CHANNELS];
extern          float                   output_float_value                      [ADC_CHANNELS];                    // Array of output values per channel
extern          int                     random_int_value                        [ADC_CHANNELS];
extern          float                   random_float_value                      [ADC_CHANNELS];                    // Array of u_seed_a values per channel

//extern          float                   float_lfo_output                        [LFO_INSTANCES];         // realtime output for 2 x 4 lfo waves ( sine, saw, tri, ... )
extern          int                     LFO_int_output                          [LFO_INSTANCES];
extern          bool                    m_shaderStatusFlags                     [FSH_SD+FSH_USB]; 
extern          bool                    menu_pickup_flag                        [ADC_CHANNELS+ADC_CHANNELS];                 // Array tracking if pot value has been "picked up"
extern          bool                    input_threshold_flag                    [ADC_CHANNELS];             // Array tracking threshold state per channel
extern          unsigned long           resultBPM                               [2];
extern          int                     g_input_mode                            [ADC_CHANNELS];             // Array storing operation mode for each channel
extern          int                     threshold_high                          [ADC_CHANNELS];                    // Array of upper threshold values per channel
extern          int                     threshold_low                           [ADC_CHANNELS];                     // Array of lower threshold values per channel
extern          int                     menu_pickup_buffer                      [ADC_CHANNELS+ADC_CHANNELS];               // Array storing previous pot values for pickup detection
extern          unsigned long           current_time_for_bpm;                      // Array storing timing information per channel
extern          unsigned long           next_beat_time                          [2];                     // Next beat timestamp for BPM mode
extern          unsigned long           buffer_interval                         [2];
extern          unsigned long           last_valid_bpm_calculation              [2];
extern          unsigned long           last_valid_bpm_buffer                   [2];
extern          unsigned long           intervalBuffer                          [2][3];
extern          unsigned char           led_col_new                        [20][3];
extern  const   unsigned char           rgb_tab                               [49][3]; 

extern          unsigned long           waveTable                               [WAVEFORMS][WAVESAMPLES];


extern          unsigned long elapsedMicroseconds[LFO_INSTANCES];
extern          unsigned long cycleLength[LFO_INSTANCES];
extern          int sampleIndex[LFO_INSTANCES];

extern          int LFO_mode[4];

extern          int sensitivity_new;
extern          int sensitivity_old;

extern          unsigned long     loaded_bytes_kernel[2];

extern          int mode_storage_buffers[16][33];
extern          int shader_has_stored_params[33];
extern          int audio_sample[2];

extern          CString h264_info;  // <- needs to be global, no return of this function!

/*
new menu_input_mode

[8 ( for adc inputs ) + 4 ( LFO ) + 4 ( extra mode flags )][progs on sd + progs on usb]
 menu_input_mode        LFO_mode    mode_storage_buffers[FRM_MODE][current_buffer],mode_storage_buffers[TEX_MODE][current_buffer],  adc_attenuation    

eventual also the variable buffers ***_old???
*/

