#include "kernel.h"
#include "global.h"

#include <circle/string.h> 
#include <stdint.h>
/*
                < type                 < name                                   < size                      < content/value
                                                                                                                    <apostroph
*/   
                CString                 g_barsString;
                CString                 g_log_string;
                CString                 g_error_string;
                CString                 g_modes;

                unsigned                hFile                                                               =     0;  // ?? empty ??
                bool                    noTargetFPS                                                         =  true;
                bool                    adc_pot_routing                                                     = false;
                bool                    resetFlag                                                           = false;
                bool                    g_show_bank                                                         = false;

                bool                    is_hold_for_2_sec_a                                                 = false;
                bool                    is_hold_for_2_sec_b                                                 = false;
                bool                    is_hold_for_4_sec_a                                                 = false;
                bool                    is_hold_for_4_sec_b                                                 = false;
                bool                    timer_mode                                                          = false;
                bool                    single_tex_mode                                                     = false;
                int                     VSH_LOADED_OLD                                                      =     0;
                int                     FSH_LOADED_OLD                                                      =     0;
                int                     TEX_LOADED_OLD                                                      =     0;
                int                     VID_LOADED_OLD                                                      =     0;
                int                     VSH_LOADED_NEW                                                      =     0;
                int                     FSH_LOADED_NEW                                                      =     0;
                int                     TEX_LOADED_NEW                                                      =     0;
                int                     VID_LOADED_NEW                                                      =     0;

                int                     scanned_vsh                                                      =     0;
                int                     scanned_fsh                                                      =     0;
                int                     scanned_tex                                                      =     0;
                int                     scanned_vid                                                      =     0;

            unsigned totalBytesReadSD  = 0;
            unsigned totalBytesReadUSB = 0;
            
bool           a_is_hold = false;
bool           b_is_hold = false;

               float                   current_fps                                                         =     0.0;
               unsigned long           last_swap_duration                                                  =     0;
               int last_gl_current_prg = -1;  // Store last active shader program

                int                     current_buffer = 0;

                int                     gl_current_prg                                                      =     0; // display_debug value
                int                     gl_current_tex                                                      =     0;
                int                     linked_programs                                                     =     0;
                int                     audio_source_channel                                                =    -1;                // Tracks which channel is audio source (-1 if none)

                int                     active_bpm_channel                                                  =     0;
                int                     g_menu_mode_new                                                     =     0;                        // Current menu mode (0, 1, or 2)
                int                     g_menu_mode_old                                                     =     0;
                int                     g_current_texture                                                   =     0;
                int                     attenuation                                                         =     2;
                unsigned long           start_time_fps_calculation                                          =     0;
                unsigned long           end_time_fps_calculation                                            =     0;
                unsigned long           total_time_fps_calculation                                          =     0;
                
                unsigned long           currentTime = 0;
                unsigned long           targetTime = 0;
                unsigned long           delayTime = 0;
                
                float                   util_audio_smooth_band                  [4]                         =  {0.0f};

                float                   GLtime                                                              =  0.0f;
                float                   opaque                                                              =  1.0f;

        const   char                    *VID__LOG_NAMES                          [VID_SD+VID_USB]                   = { "VIDLOG00.264",   "VIDLOG01.264",  "VIDLOG02.264",   "VIDLOG03.264", "VIDLOG04.264",   "VIDLOG05.264",  "VIDLOG06.264",   "VIDLOG07.264"    };

                char                   *SCANED_FILES_VSH                         [40]                       = {   0    };
                char                   *SCANED_FILES_FSH                         [40]                       = {   0    };
                char                   *SCANED_FILES_TEX                         [40]                       = {   0    };
                char                   *SCANED_FILES_VID                         [40]                       = {   0    };

                unsigned                VSH_LOADED_BYTES                        [VSH_SD + VSH_USB]          = {   0    };
                unsigned                FSH_LOADED_BYTES                        [FSH_SD + FSH_USB]          = {   0    };
                unsigned                TEX_LOADED_BYTES                        [TEX_SD + TEX_USB]          = {   0    };
                unsigned                VID_LOADED_BYTES                        [VID_SD + VID_USB]          = {   0    }; 
                // H.264 Video Parser
                u16                     VID_WIDTH                               [VID_SD + VID_USB]          = {   0    };
                u16                     VID_HEIGHT                              [VID_SD + VID_USB]          = {   0    };
                u8                      VID_PROFILE                             [VID_SD + VID_USB]          = {   0    };

                void*                   FRAME_ADDRESSES                         [VID_SD + VID_USB][MAX_FRAMES] = { 0 };
                size_t                  FRAME_LENGTH                            [VID_SD + VID_USB][MAX_FRAMES] = { 0 };
                int                     FRAME_COUNT                             [VID_SD + VID_USB]             = { 0 };
                bool                    VID_IS_VALID                            [VID_SD + VID_USB]             = { false };
                
                bool                    TEX_FILE_STATUS                         [TEX_SD + TEX_USB]          = {   0    }; // Define arrays for bitmap metadata                                
                uint32_t                TEX_FILE_SIZE                           [TEX_SD + TEX_USB]          = {   0    };           // File size 
                uint32_t                TEX_FILE_BM_OFFSET                      [TEX_SD + TEX_USB]          = {   0    };           // Offset 
                uint32_t                TEX_FILE_X_DIM                          [TEX_SD + TEX_USB]          = {   0    };           // Width 
                uint32_t                TEX_FILE_Y_DIM                          [TEX_SD + TEX_USB]          = {   0    };           // Height 
                uint32_t                TEX_FILE_BM_SIZE                        [TEX_SD + TEX_USB]          = {   0    };           // Image size 

                int                     multiplier                              [7]                         = {   64, 32, 16, 8, 4, 2, 1 };

                unsigned long           last_circle_buffer                      [LFO_INSTANCES]             = {   0    };
                unsigned long           next_circle_buffer                      [LFO_INSTANCES]             = {   0    };
                float                   LFO_float_output                        [LFO_INSTANCES]             = {   0    };
                int                     LFO_int_output                          [LFO_INSTANCES]             = {   0    };
                int                     last_multiplier                         [LFO_INSTANCES]             = {   0    };


                int                   adc_raw_value                             [ADC_CHANNELS]              = { 0 };
                int                   adc_int_value                             [ADC_CHANNELS]              = { 0 };
                float                 adc_float_value                           [ADC_CHANNELS]              = { 0 };
                int                   output_int_value                          [ADC_CHANNELS]              = { 0 };
                float                 output_float_value                        [ADC_CHANNELS]              = { 0 };                    // Array of output values per channel
                int                   random_int_value                          [ADC_CHANNELS]              = { 0 };
                float                 random_float_value                        [ADC_CHANNELS]              = { 0 };                    // Array of u_seed_a values per channel

                int                   g_input_mode                              [ADC_CHANNELS]              = { 0 };             // Array storing operation mode for each channel
                bool                  m_shaderStatusFlags                       [FSH_SD+FSH_USB]; 
                int                   menu_pickup_buffer                        [ADC_CHANNELS+ADC_CHANNELS]              = { 0 };               // Array storing previous pot values for pickup detection
                unsigned long         current_time_for_bpm                                                  = 0;                      // Array storing timing information per channel
                bool                  menu_pickup_flag                          [ADC_CHANNELS+ADC_CHANNELS]              = { 0 };                 // Array tracking if pot value has been "picked up"
                bool                  input_threshold_flag                      [ADC_CHANNELS]              = { 0 };             // Array tracking threshold state per channel
                unsigned long         next_beat_time                            [2]                         = { 0 };                     // Next beat timestamp for BPM mode
                unsigned long         buffer_interval                           [2]                         = { 0 };
                unsigned long         last_valid_bpm_calculation                [2]                         = { 0 };
                unsigned long         last_valid_bpm_buffer                     [2]                         = { 0 };
                unsigned long         resultBPM                                 [2]                         = { 0 };
                unsigned long         intervalBuffer                            [2][3]                      = { 0 };
                int                   threshold_high                            [ADC_CHANNELS]              = {   320,  320,  320,  320,  320,  320,  320,  320 };                    // Array of upper threshold values per channel
                int                   threshold_low                             [ADC_CHANNELS]              = {   128,  128,  128,  128,  128,  128,  128,  128 };                     // Array of lower threshold values per channel
               unsigned char          led_col_new                               [20][3]                     = { 0 };

         const   unsigned char        rgb_tab                                   [49][3]                     = {
{ 0, 0, 0},// 0: off/black
{48,32, 0},// 1: pure green (menu1)
{ 0, 0,48},// 2: yellow (menu2)
{ 0,48, 0},// 3: green (LFO base)

{ 0, 0,48},// 4: deep blue (ADC 2.5V)
{ 0,16,48},// 5: medium blue (ADC 3.3V)
{ 0,24,48},// 6: light blue (ADC 5V)

{56,40, 0},// 7: orange (TRG)
{32,48,32},// 8: cyan (BPM)
   
{ 0,48,24},// 9: green variant (LFO)
{ 0,40,40},// 10: green variant (LFO)

{48,48, 0},// 11: orange variant (TRG)
{ 8, 0,48},// 12: pure red (audio)
{16, 0,48},// 13: red variant (audio)
{24, 0,48},// 14: red variant (audio)
{32, 0,48},// 15: red variant (audio)
 //  {  40,  0,   48 },
{48, 0, 0},// 17: Full red
{ 0,42, 6},// 18: Strong green with touch blue
{12, 0,48},// 19: Strong blue with touch red
{36,48, 0},// 20: Yellow-green
{ 0,24,42},// 21: Turquoise
{48, 6,36},// 22: Purple-red
{ 6,48,12},// 23: Bright green
{42, 0,48},// 24: Purple
{48,30, 0},// 25: Orange
{ 0,48,36},// 26: Blue-green
{36,12,48},// 27: Purple-blue
{48, 0,24},// 28: Dark red
{12,42, 0},// 29: Yellow-green
{ 0,18,48},// 30: Medium blue
{48,36, 6},// 31: Light orange
{24, 0,42},// 32: Dark purple
{ 6,48,30},// 33: Turquoise-green
{42,12, 0},// 34: Brown-red
{ 0,36,48},// 35: Light blue
{48, 0,36},// 36: Pink-red
{18,42, 6},// 37: Forest green
{ 0,12,48},// 38: Deep blue
{48,30, 0},// 39: Orange
{ 6, 0,42},// 40: Deep purple
{36,48,12},// 41: Yellow
{ 0,24,48},// 42: Ocean blue
{42, 6, 0},// 43: Dark red
{12,48,36},// 44: Mint green
{48, 0,42},// 45: Pink
{ 0,36, 6},// 46: Green
{30,12,48},// 47: Violet
{48,42, 0},// 48: Yellow-orange
{ 6, 0,48} // 49: Deep blue         15+32
};

#include "kernel.h"
#include "global.h"
#include "wavetable.h"

#include <circle/string.h> 
#include <stdint.h>

unsigned long           waveTable                               [WAVEFORMS][WAVESAMPLES]    = {{  0,    0,    1,    1,    2,    4,    6,    8,   10,   12,   15,   19,   22,   26,   30,   34,
                                                                                                                 39,   44,   49,   55,   60,   66,   73,   79,   86,   93,  101,  108,  116,  124,  133,  141,
                                                                                                                150,  159,  168,  177,  187,  197,  207,  217,  227,  238,  249,  259,  270,  282,  293,  304,
                                                                                                                316,  327,  339,  351,  363,  375,  387,  399,  412,  424,  436,  449,  461,  474,  486,  499,
                                                                                                                512,  524,  537,  549,  562,  574,  587,  599,  611,  624,  636,  648,  660,  672,  684,  696,
                                                                                                                707,  719,  730,  741,  753,  764,  774,  785,  796,  806,  816,  826,  836,  846,  855,  864,
                                                                                                                873,  882,  890,  899,  907,  915,  922,  930,  937,  944,  950,  957,  963,  968,  974,  979,
                                                                                                                984,  989,  993,  997, 1001, 1004, 1008, 1011, 1013, 1015, 1017, 1019, 1021, 1022, 1022, 1023,
                                                                                                               1023, 1023, 1022, 1022, 1021, 1019, 1017, 1015, 1013, 1011, 1008, 1004, 1001,  997,  993,  989,
                                                                                                                984,  979,  974,  968,  963,  957,  950,  944,  937,  930,  922,  915,  907,  899,  890,  882,
                                                                                                                873,  864,  855,  846,  836,  826,  816,  806,  796,  785,  774,  764,  753,  741,  730,  719,
                                                                                                                707,  696,  684,  672,  660,  648,  636,  624,  611,  599,  587,  574,  562,  549,  537,  524,
                                                                                                                512,  499,  486,  474,  461,  449,  436,  424,  412,  399,  387,  375,  363,  351,  339,  327,
                                                                                                                316,  304,  293,  282,  270,  259,  249,  238,  227,  217,  207,  197,  187,  177,  168,  159,
                                                                                                                150,  141,  133,  124,  116,  108,  101,   93,   86,   79,   73,   66,   60,   55,   49,   44,
                                                                                                                 39,   34,   30,   26,   22,   19,   15,   12,   10,    8,    6,    4,    2,    1,    1,    0  },
                                                                                                             {    0,    8,   16,   24,   32,   40,   48,   56,   64,   72,   81,   89,   97,  105,  113,  121,
                                                                                                                129,  137,  145,  153,  161,  169,  177,  185,  193,  201,  209,  217,  226,  234,  242,  250,
                                                                                                                258,  266,  274,  282,  290,  298,  306,  314,  322,  330,  338,  346,  354,  362,  371,  379,
                                                                                                                387,  395,  403,  411,  419,  427,  435,  443,  451,  459,  467,  475,  483,  491,  499,  507,
                                                                                                                516,  524,  532,  540,  548,  556,  564,  572,  580,  588,  596,  604,  612,  620,  628,  636,
                                                                                                                644,  652,  661,  669,  677,  685,  693,  701,  709,  717,  725,  733,  741,  749,  757,  765,
                                                                                                                773,  781,  789,  797,  806,  814,  822,  830,  838,  846,  854,  862,  870,  878,  886,  894,
                                                                                                                902,  910,  918,  926,  934,  942,  951,  959,  967,  975,  983,  991,  999, 1007, 1015, 1023,
                                                                                                               1015, 1007,  999,  991,  983,  975,  967,  959,  951,  943,  935,  927,  919,  911,  903,  895,
                                                                                                                887,  879,  871,  863,  855,  847,  839,  831,  823,  815,  807,  799,  791,  783,  775,  767,
                                                                                                                759,  751,  743,  735,  727,  719,  711,  703,  695,  687,  679,  671,  663,  655,  647,  639,
                                                                                                                631,  623,  615,  607,  599,  591,  583,  575,  567,  559,  551,  543,  535,  527,  519,  512,
                                                                                                                504,  496,  488,  480,  472,  464,  456,  448,  440,  432,  424,  416,  408,  400,  392,  384,
                                                                                                                376,  368,  360,  352,  344,  336,  328,  320,  312,  304,  296,  288,  280,  272,  264,  256,
                                                                                                                248,  240,  232,  224,  216,  208,  200,  192,  184,  176,  168,  160,  152,  144,  136,  128,
                                                                                                                120,  112,  104,   96,   88,   80,   72,   64,   56,   48,   40,   32,   24,   16,    8,    0  },
                                                                                                             {    0,    4,    8,   12,   16,   20,   24,   28,   32,   36,   40,   44,   48,   52,   56,   60,
                                                                                                                 64,   68,   72,   76,   80,   84,   88,   92,   96,  100,  104,  108,  112,  116,  120,  124,
                                                                                                                128,  132,  136,  140,  144,  148,  152,  156,  160,  164,  168,  173,  177,  181,  185,  189,
                                                                                                                193,  197,  201,  205,  209,  213,  217,  221,  225,  229,  233,  237,  241,  245,  249,  253,
                                                                                                                257,  261,  265,  269,  273,  277,  281,  285,  289,  293,  297,  301,  305,  309,  313,  317,
                                                                                                                321,  325,  329,  333,  337,  341,  345,  349,  353,  357,  361,  365,  369,  373,  377,  381,
                                                                                                                385,  389,  393,  397,  401,  405,  409,  413,  417,  421,  425,  429,  433,  437,  441,  445,
                                                                                                                449,  453,  457,  461,  465,  469,  473,  477,  481,  485,  489,  493,  497,  501,  505,  509,
                                                                                                                514,  518,  522,  526,  530,  534,  538,  542,  546,  550,  554,  558,  562,  566,  570,  574,
                                                                                                                578,  582,  586,  590,  594,  598,  602,  606,  610,  614,  618,  622,  626,  630,  634,  638,
                                                                                                                642,  646,  650,  654,  658,  662,  666,  670,  674,  678,  682,  686,  690,  694,  698,  702,
                                                                                                                706,  710,  714,  718,  722,  726,  730,  734,  738,  742,  746,  750,  754,  758,  762,  766,
                                                                                                                770,  774,  778,  782,  786,  790,  794,  798,  802,  806,  810,  814,  818,  822,  826,  830,
                                                                                                                834,  838,  842,  846,  850,  855,  859,  863,  867,  871,  875,  879,  883,  887,  891,  895,
                                                                                                                899,  903,  907,  911,  915,  919,  923,  927,  931,  935,  939,  943,  947,  951,  955,  959,
                                                                                                                963,  967,  971,  975,  979,  983,  987,  991,  995,  999, 1003, 1007, 1011, 1015, 1019, 1023  },
                                                                                                             { 1023, 1019, 1015, 1011, 1007, 1003,  999,  995,  991,  987,  983,  979,  975,  971,  967,  963,
                                                                                                                959,  955,  951,  947,  943,  939,  935,  931,  927,  923,  919,  915,  911,  907,  903,  899,
                                                                                                                895,  891,  887,  883,  879,  875,  871,  867,  863,  859,  855,  850,  846,  842,  838,  834,
                                                                                                                830,  826,  822,  818,  814,  810,  806,  802,  798,  794,  790,  786,  782,  778,  774,  770,
                                                                                                                766,  762,  758,  754,  750,  746,  742,  738,  734,  730,  726,  722,  718,  714,  710,  706,
                                                                                                                702,  698,  694,  690,  686,  682,  678,  674,  670,  666,  662,  658,  654,  650,  646,  642,
                                                                                                                638,  634,  630,  626,  622,  618,  614,  610,  606,  602,  598,  594,  590,  586,  582,  578,
                                                                                                                574,  570,  566,  562,  558,  554,  550,  546,  542,  538,  534,  530,  526,  522,  518,  514,
                                                                                                                509,  505,  501,  497,  493,  489,  485,  481,  477,  473,  469,  465,  461,  457,  453,  449,
                                                                                                                445,  441,  437,  433,  429,  425,  421,  417,  413,  409,  405,  401,  397,  393,  389,  385,
                                                                                                                381,  377,  373,  369,  365,  361,  357,  353,  349,  345,  341,  337,  333,  329,  325,  321,
                                                                                                                317,  313,  309,  305,  301,  297,  293,  289,  285,  281,  277,  273,  269,  265,  261,  257,
                                                                                                                253,  249,  245,  241,  237,  233,  229,  225,  221,  217,  213,  209,  205,  201,  197,  193,
                                                                                                                189,  185,  181,  177,  173,  168,  164,  160,  156,  152,  148,  144,  140,  136,  132,  128,
                                                                                                                124,  120,  116,  112,  108,  104,  100,   96,   92,   88,   84,   80,   76,   72,   68,   64,
                                                                                                                 60,   56,   52,   48,   44,   40,   36,   32,   28,   24,   20,   16,   12,    8,    4,    0  } };


unsigned long     loaded_bytes_kernel[2] = { 0 };                                                                                                               
         
        unsigned long elapsedMicroseconds[LFO_INSTANCES];
        unsigned long cycleLength[LFO_INSTANCES];
        int sampleIndex[LFO_INSTANCES];

        int LFO_mode[LFO_PARAMETERS] = { 0,3,3,3 };

        int sensitivity_new = 0;
        int sensitivity_old = 0;


         int mode_storage_buffers[16][33] = {0};
         int shader_has_stored_params[33] = { 0 };
         int audio_sample[2] = { 0,0};

         CString h264_info;  // <- needs to be global, no return of this function!

/*
new menu_input_mode

[8 ( for adc inputs ) + 4 ( LFO ) + 4 ( extra mode flags )][progs on sd + progs on usb]
 menu_input_mode        LFO_mode    mode_storage_buffers[FRM_MODE][current_buffer],mode_storage_buffers[TEX_MODE][current_buffer],  adc_attenuation    

eventual also the variable buffers ***_old???
*/