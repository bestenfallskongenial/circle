#include "kernel.h"
#include "global.h"
/*
extern "C" {
    void vc_host_get_vchi_state(VCHI_INSTANCE_T *initialise_instance, VCHI_CONNECTION_T **connection);
}
*/

void            CKernel::util_prep_parameters       ()
{
    for ( int i=0; i <= DEFAULT_SLOT; i++)
    {
    mode_storage_buffers[LF1_WAVE][i] = 0;
    mode_storage_buffers[LF2_WAVE][i] = 1;
    mode_storage_buffers[LF1_MULT][i] = 3;
    mode_storage_buffers[LF2_MULT][i] = 3;
    }
}
int             CKernel::util_choose_program        ()
{
                static int usedShader = 0;  // Static variable to maintain value between function calls
                int calculated = output_int_value[ADC_SELECT_PRG] * FSH_LOADED_NEW / 1024;  //???
                // Only update if the calculated index points to a valid u_program_handle
                if (m_shaderStatusFlags[calculated ]==true /*&& g_menu_mode_new == 0*/) usedShader = calculated;

                return usedShader;
}

void            CKernel::util_store_program         () 
{
                // 1. SHADER CHANGE CHECK
                if (gl_current_prg != last_gl_current_prg) 
                    {    
                    // SIMPLE: Use program slot if stored, otherwise DEFAULT_SLOT
                    current_buffer = shader_has_stored_params[gl_current_prg] ?
                                    gl_current_prg : DEFAULT_SLOT;
                    last_gl_current_prg = gl_current_prg;
                    }

                // 2. STORE PARAMETERS
                if (shader_has_stored_params[gl_current_prg] == false && is_hold_for_2_sec_a == true && is_hold_for_2_sec_b == true )
                    {  
                    // SIMPLE: Copy DEFAULT_SLOT contents to this program's slot
                    memcpy(&mode_storage_buffers[0][gl_current_prg],
                        &mode_storage_buffers[0][DEFAULT_SLOT],
                        16 * sizeof(int));
                    
                    shader_has_stored_params[gl_current_prg] = true;
                    current_buffer = gl_current_prg;  // Now use program's slot
                    is_hold_for_2_sec_a = false;
                    is_hold_for_2_sec_b = false;
                    }

                // 3. DELETE STORED PARAMETERS
                
        else  if (shader_has_stored_params[gl_current_prg] == true && is_hold_for_2_sec_a == true && is_hold_for_2_sec_b == true ) // -really else and not only if??
                    {  
                    shader_has_stored_params[gl_current_prg] = false;
                    current_buffer = DEFAULT_SLOT;  // Back to default
                    is_hold_for_2_sec_a = false;
                    is_hold_for_2_sec_b = false;
                    }
}

int             CKernel::util_choose_texture        ()
{
                static int usedTexture = 0;
                if (TEX_LOADED_NEW != 0) 
                    {
                    int calculated = output_int_value[ADC_SELECT_TEX] * (TEX_LOADED_NEW - 1) / 1024;
                    usedTexture = calculated;
                    }
                return usedTexture;
}

void            CKernel::util_random_vec8           (uint32_t seed)                                               // create 8 unique normalised to 1.0 float and to 1024 int values
{
                const int max_int = 1024;
                const float scale = 1.0f / 4294967295.0f;
                uint32_t x = seed;

                x ^= x << 13; x ^= x >> 17; x ^= x << 5;
                random_float_value[0] = (float)x * scale;
                random_int_value[0] = (uint32_t)(random_float_value[0] * max_int);

                x ^= x << 13; x ^= x >> 17; x ^= x << 5;
                random_float_value[1] = (float)x * scale;
                random_int_value[1] = (uint32_t)(random_float_value[1] * max_int);

                x ^= x << 13; x ^= x >> 17; x ^= x << 5;
                random_float_value[2] = (float)x * scale;
                random_int_value[2] = (uint32_t)(random_float_value[2] * max_int);

                x ^= x << 13; x ^= x >> 17; x ^= x << 5;
                random_float_value[3] = (float)x * scale;
                random_int_value[3] = (uint32_t)(random_float_value[3] * max_int);

                x ^= x << 13; x ^= x >> 17; x ^= x << 5;
                random_float_value[4] = (float)x * scale;
                random_int_value[4] = (uint32_t)(random_float_value[4] * max_int);

                x ^= x << 13; x ^= x >> 17; x ^= x << 5;
                random_float_value[5] = (float)x * scale;
                random_int_value[5] = (uint32_t)(random_float_value[5] * max_int);

                x ^= x << 13; x ^= x >> 17; x ^= x << 5;
                random_float_value[6] = (float)x * scale;
                random_int_value[6] = (uint32_t)(random_float_value[6] * max_int);

                x ^= x << 13; x ^= x >> 17; x ^= x << 5;
                random_float_value[7] = (float)x * scale;
                random_int_value[7] = (uint32_t)(random_float_value[7] * max_int);
}

void            CKernel::util_calculate_BPM         (unsigned long buttonTime, unsigned long clockTime) 
{
                static unsigned long lastTime[2];
                static unsigned long timeBuffer[2][4] = {{0}};  
                static int timeIndex[2] = {0};

                if (buttonTime != lastTime[0])                                                          // Process button u_time (instance 0)
                    {
                    timeBuffer[0][timeIndex[0]] = buttonTime;
        
                    intervalBuffer[0][0] = timeBuffer[0][1] - timeBuffer[0][0];
                    intervalBuffer[0][1] = timeBuffer[0][2] - timeBuffer[0][1];
                    intervalBuffer[0][2] = timeBuffer[0][3] - timeBuffer[0][2];
                if(     intervalBuffer[0][1] < intervalBuffer[0][0] * 1.25f 
                    &&  intervalBuffer[0][2] < intervalBuffer[0][0] * 1.25f
                    &&  intervalBuffer[0][0] < intervalBuffer[0][2] * 1.25f ) 
                        {
                        unsigned long averageInterval = (intervalBuffer[0][0] + intervalBuffer[0][1] + intervalBuffer[0][2]) / 3;
            
                        resultBPM[0] = 60000000 / averageInterval;
            
                        buffer_interval[0] = averageInterval;
                        last_valid_bpm_calculation[0]  = m_Timer.GetClockTicks();
                        }
                    lastTime[0] = buttonTime;

                    timeIndex[0] = (timeIndex[0] + 1) % 4;    
                    }
                if (clockTime != lastTime[1])                                                           // Process clock u_time (instance 1)
                    {
                    timeBuffer[1][timeIndex[1]] = clockTime;
        
                    intervalBuffer[1][0] = timeBuffer[1][1] - timeBuffer[1][0];
                    intervalBuffer[1][1] = timeBuffer[1][2] - timeBuffer[1][1];
                    intervalBuffer[1][2] = timeBuffer[1][3] - timeBuffer[1][2];
                    if(     intervalBuffer[1][1] < intervalBuffer[1][0] * 1.25f 
                        &&  intervalBuffer[1][2] < intervalBuffer[1][0] * 1.25f
                        &&  intervalBuffer[1][0] < intervalBuffer[1][2] * 1.25f ) 
                        {
                        unsigned long averageInterval = (intervalBuffer[1][0] + intervalBuffer[1][1] + intervalBuffer[1][2]) / 3;
            
                        resultBPM[1] = 60000000 / averageInterval;
            
                        buffer_interval[1] = averageInterval;
                        last_valid_bpm_calculation[1]  = m_Timer.GetClockTicks();
                        }
                    lastTime[1] = clockTime;

                    timeIndex[1] = (timeIndex[1] + 1) % 4;    
                    }
}

void            CKernel::util_determine_bpm_source  ()
{
                if (last_valid_bpm_calculation[0] > last_valid_bpm_calculation[1])                                              // Compare last_valid_bpm_calculation[0] and last_valid_bpm_calculation[1] to decide which one is newer
                    {
                    active_bpm_channel = 0;                                                             // Choose channel 0 as the active BPM source
                    }
                    if (last_valid_bpm_calculation[1] > last_valid_bpm_calculation[0] /* && g_input_mode[5] == 2 */ ) 
                    {
                    active_bpm_channel = 1;                                                             // Choose channel 1 as the active BPM source
                    }
}

void            CKernel::util_update_predicted_beat ()
{
                unsigned long currentTime = m_Timer.GetClockTicks(); // Get the current u_time in clock ticks

                // Update for instance 0
                if (currentTime >= next_beat_time[0]) 
                {
                    next_beat_time[0] += buffer_interval[0]; // Predict the next beat u_time
                }
                if (currentTime >= next_circle_buffer[0]) 
                    {
                    last_circle_buffer[0] = next_circle_buffer[0];
                    next_circle_buffer[0] = next_circle_buffer[0] + (buffer_interval[active_bpm_channel] * last_multiplier[0]); // multiplier[mode_storage_buffers[LF1_MULT][current_buffer]]);
                    
                    last_multiplier[0]          = multiplier[mode_storage_buffers[LF1_MULT][current_buffer]];
                    }
                // Handle BPM changes for instance 0
                if ((last_valid_bpm_buffer[0] != last_valid_bpm_calculation[0]))
                    {
                    next_beat_time[0]           = last_valid_bpm_calculation[0]; // Reset to current time for new BPM

                    last_valid_bpm_buffer[0]    = last_valid_bpm_calculation[0];
                    }
                if (/*(last_valid_bpm_buffer[0] != last_valid_bpm_calculation[0]) ||*/ last_multiplier[0] != multiplier[mode_storage_buffers[LF1_MULT][current_buffer]])
                    {
                    last_circle_buffer[0]       = last_valid_bpm_calculation[active_bpm_channel];
                    next_circle_buffer[0]       = last_valid_bpm_calculation[active_bpm_channel] + (buffer_interval[active_bpm_channel] * last_multiplier[0]); // multiplier[mode_storage_buffers[LF1_MULT][current_buffer]]);
                    
                    last_multiplier[0]          = multiplier[mode_storage_buffers[LF1_MULT][current_buffer]];
                    }
                // Update for instance 1
                if (currentTime >= next_beat_time[1]) 
                    {
                    next_beat_time[1] += buffer_interval[1]; // Predict the next beat u_time
                    }
                if (currentTime >= next_circle_buffer[1]) 
                    {
                    last_circle_buffer[1] = next_circle_buffer[1];
                    next_circle_buffer[1] = next_circle_buffer[1] + (buffer_interval[active_bpm_channel] * last_multiplier[1] ); // multiplier[mode_storage_buffers[LF2_MULT][current_buffer]]);

                    last_multiplier[1]          = multiplier[mode_storage_buffers[LF2_MULT][current_buffer]];
                    }
                // Handle BPM changes for instance 1
                if ((last_valid_bpm_buffer[1] != last_valid_bpm_calculation[1]))
                    {
                    next_beat_time[1]           = last_valid_bpm_calculation[1]; // Reset to current time for new BPM

                    last_valid_bpm_buffer[1]    = last_valid_bpm_calculation[1];
                    }
                if (/*(last_valid_bpm_buffer[1] != last_valid_bpm_calculation[1]) ||*/ last_multiplier[1] != multiplier[mode_storage_buffers[LF2_MULT][current_buffer]])
                    {
                    last_circle_buffer[1]       = last_valid_bpm_calculation[active_bpm_channel];
                    next_circle_buffer[1]       = last_valid_bpm_calculation[active_bpm_channel] + (buffer_interval[active_bpm_channel] * last_multiplier[1]); // multiplier[mode_storage_buffers[LF2_MULT][current_buffer]]);
                    
                    last_multiplier[1]          = multiplier[mode_storage_buffers[LF2_MULT][current_buffer]];
                    }

}

void            CKernel::util_LFO                   ()
{
                unsigned long currentTime = m_Timer.GetClockTicks(); // Get the current u_time in clock ticks

                elapsedMicroseconds[0] =   currentTime - last_circle_buffer[0];
                cycleLength[0] = next_circle_buffer[0] - last_circle_buffer[0]; // Total length of the current cycle

            //    sampleIndex[0] = (elapsedMicroseconds[0] * 255) / cycleLength[0];
                int indexA = (elapsedMicroseconds[0] * 255) / cycleLength[0];
                sampleIndex[0] = indexA > 255 ? 255 : indexA;


                LFO_float_output[0] = (float)waveTable[mode_storage_buffers[LF1_WAVE][current_buffer]][sampleIndex[0]] / 1023.0f;
                LFO_int_output  [0] =        waveTable[mode_storage_buffers[LF1_WAVE][current_buffer]][sampleIndex[0]];

                elapsedMicroseconds[1] =   currentTime - last_circle_buffer[1];
                cycleLength[1] = next_circle_buffer[1] - last_circle_buffer[1]; // Total length of the current cycle

            //    sampleIndex[1] = (elapsedMicroseconds[1] * 255) / cycleLength[1];  // was 256
                int indexB = (elapsedMicroseconds[1] * 255) / cycleLength[1];
                sampleIndex[1] = indexB > 255 ? 255 : indexB;

                LFO_float_output[1] = (float)waveTable[mode_storage_buffers[LF2_WAVE][current_buffer]][sampleIndex[1]] / 1023.0f; // was 0123.0f
                LFO_int_output  [1] =        waveTable[mode_storage_buffers[LF2_WAVE][current_buffer]][sampleIndex[1]];
}   

void            CKernel::util_audio_energy          (float adcvalue) 
{   
                const int maxBuffer = 33;

                const int buffer_size_table[4][5] = 
                    {
                    {33, 25, 17,  9,  5},  // Channel 0 (lowest frequency)  
                    {25, 19, 13,  7,  4},  // Channel 1
                    {17, 13,  9,  5,  3},  // Channel 2
                    { 9,  7,  5,  3,  2}   // Channel 3 (highest frequency)
                    };
                    
                static float band0[maxBuffer] = {0};                    // Static ring buffers for each frequency band
                static float band1[maxBuffer] = {0};
                static float band2[maxBuffer] = {0};
                static float band3[maxBuffer] = {0};
                static unsigned char indexBand0 = 0;                    // Static indices for ring buffers
                static unsigned char indexBand1 = 0;
                static unsigned char indexBand2 = 0;
                static unsigned char indexBand3 = 0;

                if ( sensitivity_new != sensitivity_old )
                    {
                    util_audio_smooth_band[0] = 0;
                    util_audio_smooth_band[1] = 0;
                    util_audio_smooth_band[2] = 0;
                    util_audio_smooth_band[3] = 0;

                    indexBand0 = 0;    // Reset ring buffer indices too
                    indexBand1 = 0;
                    indexBand2 = 0;
                    indexBand3 = 0;

                    sensitivity_old = sensitivity_new;
                    }   
                band0[indexBand0] = adcvalue;                           // Update ring buffers with the new ADC value
                band1[indexBand1] = adcvalue;
                band2[indexBand2] = adcvalue;
                band3[indexBand3] = adcvalue;
                for (unsigned char i = 0; i < buffer_size_table[0][sensitivity_new]; ++i)     // Averaging the buffer contents
                    {
                    util_audio_smooth_band[0] += band0[i];
                    }
                for (unsigned char i = 0; i < buffer_size_table[1][sensitivity_new]; ++i) 
                    {
                    util_audio_smooth_band[1] += band1[i];
                    }
                for (unsigned char i = 0; i < buffer_size_table[2][sensitivity_new]; ++i) 
                    {
                    util_audio_smooth_band[2] += band2[i];
                    }
                for (unsigned char i = 0; i < buffer_size_table[3][sensitivity_new]; ++i) 
                    {
                    util_audio_smooth_band[3] += band3[i];
                    }
                util_audio_smooth_band[0] /= buffer_size_table[0][sensitivity_new];
                util_audio_smooth_band[1] /= buffer_size_table[1][sensitivity_new];
                util_audio_smooth_band[2] /= buffer_size_table[2][sensitivity_new];
                util_audio_smooth_band[3] /= buffer_size_table[3][sensitivity_new];

                indexBand0 = (indexBand0 + 1) % buffer_size_table[0][sensitivity_new];        // Update indices
                indexBand1 = (indexBand1 + 1) % buffer_size_table[1][sensitivity_new];
                indexBand2 = (indexBand2 + 1) % buffer_size_table[2][sensitivity_new];
                indexBand3 = (indexBand3 + 1) % buffer_size_table[3][sensitivity_new];
}

bool            CKernel::util_check_for_update      ()
{
                CString line;    

                // Read physical pin state - 0 means pressed, 1 means released
                unsigned pinStateA = CGPIOPin(SW_PIN_A, GPIOModeInputPullUp).Read();

                // If button A is pressed (pinStateA == 0)  
                if (pinStateA == 0)  
                {
                    unsigned long startTime = m_Timer.GetTicks();
                    unsigned long duration = 5000000; // 5 seconds window for confirmation

                    // Wait for button B press for confirmation
                    while (m_Timer.GetTicks() - startTime < duration)
                    {
                        unsigned pinStateB = CGPIOPin(SW_PIN_B, GPIOModeInputPullUp).Read();
                        
            //          unsigned long elapsed = m_Timer.GetTicks() - startTime;
            //          unsigned long remaining = duration - elapsed;

            //          unsigned long secondsRemaining = remaining / 1000000; // Convert microseconds to second

                        m_Screen.Write(CLEAR_SCREEN, strlen(CLEAR_SCREEN));
                        m_Screen.Write( "\n\n  do you want to update? press X\n"
                                        "  reset in 5 Seconds\n\n"
                                        "  prepare the USB Stick with the new Firmware File\n\n",
                                strlen( "\n\n  do you want to update? press X\n"
                                        "  reset in 5 Seconds\n\n"
                                        "  prepare the USB Stick with the new Firmware File\n\n" ));      
            //          m_Screen.Write(line, strlen(line));
                        m_Timer.MsDelay(100);  // Update display every 100ms
                        
                        if (pinStateB == 0)  // If button B is pressed, start update process
                        {
                            m_Screen.Write("  okay, lets do it\n\n", strlen("okay, lets do it\n\n"));

                            // Load current firmware from SD to buffer 0
                            if (!filesystem_load_kernel(PARTITIONSD, FILENAME_KERNEL, 0))
                            {
                                m_Screen.Write("  problem with the old firmware file / sd card - please try again\n", 
                                        strlen("  problem with the old firmware file / sd card - please try again\n"));
                                m_Timer.MsDelay(2000);  
                                return false;                      
                            }

                            line.Format ("  %u Byte loaded from SD\n\n",  loaded_bytes_kernel[0]);
                            m_Screen.Write(line, strlen(line));

                            m_Screen.Write("  insert USB stick\n\n", 
                                    strlen("  insert USB stick\n\n"));

                            // Load new firmware from USB to buffer 1
                            if (!filesystem_load_kernel(PARTITIONUSB, FILENAME_KERNEL, 1))
                            {
                                m_Screen.Write("  problem with the new firmware file / USB stick - please try again\n", 
                                        strlen("  problem with the old firmware file / USB stick - please try again\n"));            
                                m_Timer.MsDelay(2000);
                                return false;                
                            }

                            line.Format ("  %u Byte loaded from USB\n\n",  loaded_bytes_kernel[1]);
                            m_Screen.Write(line, strlen(line));

                            // Write new firmware from buffer 1 to SD
                            if (!filesystem_save_kernel(PARTITIONSD, FILENAME_KERNEL, 1))
                                {
                                m_Screen.Write("  Error ! Restoring the firmware\n", 
                                        strlen("  Error ! Restoring the firmware\n"));             
                                if (!filesystem_save_kernel(PARTITIONSD, FILENAME_KERNEL, 0))
                                    {
                                    m_Screen.Write("  Writing / Restoring the Firmware Failed - you need to restore it manually !!!\n", 
                                            strlen("  Writing / Restoring the Firmware Failed - you need to restore it manually !!!\n"));             
                                    m_Timer.MsDelay(2000);
                                    return false;    
                                    }
                                }
                            m_Screen.Write("  update successful - system will reboot now\n", 
                                    strlen("  update successful - system will reboot now\n"));      
                            m_Timer.MsDelay(2000);
                            return true;   
                        }
                    }
                    return false; 
                } 
                return false;      
}

void            CKernel::util_save_modes_file       ()    // whats up here???
{
                CString temp_string;

                const char* table_attenuator[3]= { "2.5", "3.3", "5.0" };
                const char* table_waveform[4] = { "Sin","Saw","Tr1","Tr2" };
                const char* table_intensity[5] = { "12.5%", "25%", "50%", "75%", "100%" };
                const char* table_menu[NUMBER_OF_MODES]={ "ADC","TRG","BPM","LF1","LF2","AUD"};
                const char* file_header =   "# --------------------------------------------------------------------------------\n"
                                            "# Mode Config File\n"
                                            "# --------------------------------------------------------------------------------\n"
                                            "# How to Edit the Config File:\n"
                                            "# Add/Remove '#' or whole lines\n"
                                            "# Valid Entries for CV XY are:\n"
                                            "#  ADC - Direct CV-In\n"
                                            "#  TRG - Trigger Synced Randomizer\n"
                                            "#  BPM - Beat Synced Randomizer\n"
                                            "#  LF1 - LFO 1\n"
                                            "#  LF2 - LFO 2\n"
                                            "#  AUD - 4 Band Audio Energy Analyzer\n"
                                            "#  ( is not availeble for CV BX! )\n\n"
                                            "# Valid Entries for LFO 1 & 2:\n"
                                            "#  Sin / Saw / TR1 / TR2 ( rising/falling Triangle )\n"
                                            "# Valid Entries for Divider 1 & 2:\n"
                                            "#  1 / 2 / 4 / 8 / 16 / 32 / 64\n"
                                            "# Valid Entries Frame/Texture Mode:\n"
                                            "#  ON / OFF\n\n";
                g_modes.Append(file_header);

                temp_string.Format( "# --------------------------------------------------------------------------------\n"
                                    "#  Global Settings\n" 
                                    "# --------------------------------------------------------------------------------\n"
                                    "  BPM:                 %-3u\n"
                                    "  CLK:                 %-3u\n\n"
                                    "  CV-Attenuation:      %s Volt\n"
                                    "  Audio-Sensisitivity: %s\n",
                                    resultBPM[0], 
                                    resultBPM[1],
                                    table_attenuator[attenuation],
                                    table_intensity[sensitivity_new]);

                g_modes.Append(temp_string);

                for(int i = 0; i < linked_programs; i++) 
                    {
                    temp_string.Format( "# --------------------------------------------------------------------------------\n"
                                        "#  filename: %s\n"
                                        "# --------------------------------------------------------------------------------\n", SCANED_FILES_FSH[i+1]);
                    g_modes.Append(temp_string);
                    g_modes.Append("\n");

                    temp_string.Format( "  CV A0: %s\n"
                                        "  CV A1: %s\n"
                                        "  CV A2: %s\n"
                                        "  CV A3: %s\n\n",table_menu[mode_storage_buffers[CH0_MODE][i]],
                                                        table_menu[mode_storage_buffers[CH1_MODE][i]],
                                                        table_menu[mode_storage_buffers[CH2_MODE][i]],
                                                        table_menu[mode_storage_buffers[CH3_MODE][i]]);
                    g_modes.Append(temp_string);

                    temp_string.Format( "  CV B0: %s\n"
                                        "  CV B1: %s\n"
                                        "  CV B2: %s\n"
                                        "  CV B3: %s\n\n",table_menu[mode_storage_buffers[CH4_MODE][i]],
                                                        table_menu[mode_storage_buffers[CH5_MODE][i]],
                                                        table_menu[mode_storage_buffers[CH6_MODE][i]],
                                                        table_menu[mode_storage_buffers[CH7_MODE][i]]);
                    g_modes.Append(temp_string);       

                    temp_string.Format( "  Wave    LFO 1: %s\n"
                                        "  Divider LFO 1: %d\n\n"
                                        "  Wave    LFO 2: %s\n"
                                        "  Divider LFO 2: %d\n\n",table_waveform[mode_storage_buffers[LF1_WAVE][i]],
                                                                    multiplier[mode_storage_buffers[LF1_MULT][i]],
                                                                table_waveform[mode_storage_buffers[LF2_WAVE][i]],
                                                                    multiplier[mode_storage_buffers[LF2_MULT][i]]);
                    g_modes.Append(temp_string);  
                    
                    temp_string.Format("  Frame   Mode: %s\n", (mode_storage_buffers[FRM_MODE][i] ? "ON" : "OFF"));
                    g_modes.Append(temp_string);
            
                    temp_string.Format("  Texture Mode: %s\n", (mode_storage_buffers[TEX_MODE][i] ? "ON" : "OFF"));
                    g_modes.Append(temp_string);

                    g_modes.Append("\n");
                    }
}
// Generate H.264 parser information string for debugging
void            CKernel::GenerateH264ParserInfo     (   int     video_index,
                                                        char*   buffer_video[],
                                                        size_t  buffer_size[],    
                                                        u16     video_width[],
                                                        u16     video_height[],
                                                        u8      video_profile[],
                                                        void*   frame_addresses[][MAX_FRAMES],
                                                        size_t  length_of_frames[][MAX_FRAMES],
                                                        int     number_of_frames[],
                                                        bool    is_video_valid[])
{
                // CString h264_info;  // <- needs to be global, no return of this function!
                h264_info = "";
                // Create header
                const char* file_header = 
                "# --------------------------------------------------------------------------------\n"
                "# H.264 Video Parse Information (From CH264Parser)\n"
                "# --------------------------------------------------------------------------------\n\n";
                h264_info.Append(file_header);

                // Get profile name
                const char* profile_name = "Unknown";
                if (video_profile[video_index] == H264_PROFILE_BASELINE) {
                profile_name = "Baseline";
                } else if (video_profile[video_index] == H264_PROFILE_MAIN) {
                profile_name = "Main";
                } else if (video_profile[video_index] == H264_PROFILE_HIGH) {
                profile_name = "High";
                }

                // Add video metadata
                CString temp_string;
                temp_string.Format(
                "Video Index:    %d\n"
                "Valid:          %s\n"
                "Resolution:     %dx%d\n"
                "Profile:        %d (%s)\n"
                "Frame Count:    %d\n\n",
                video_index,
                is_video_valid[video_index] ? "Yes" : "No",
                video_width[video_index],
                video_height[video_index],
                video_profile[video_index],
                profile_name,
                number_of_frames[video_index]);
                h264_info.Append(temp_string);

                // Add frame information
                h264_info.Append("# --------------------------------------------------------------------------------\n");
                h264_info.Append("# Frame Information (I-frames only)\n");
                h264_info.Append("# --------------------------------------------------------------------------------\n");
                h264_info.Append("# Frame | Address     | Size       | Offset     \n");
                h264_info.Append("# --------------------------------------------------------------------------------\n");

                // Calculate base address for offset calculation
                uintptr_t base_address = reinterpret_cast<uintptr_t>(buffer_video[video_index]);

                // List ALL frame details
                for (int i = 0; i < number_of_frames[video_index]; i++)
                {
                uintptr_t frame_addr = reinterpret_cast<uintptr_t>(frame_addresses[video_index][i]);
                size_t frame_size = length_of_frames[video_index][i];
                ptrdiff_t offset = frame_addr - base_address;

                temp_string.Format("  %-5d | 0x%08X | %-10d | %-10d\n", 
                i, frame_addr, frame_size, offset);
                h264_info.Append(temp_string);
                }

                h264_info.Append("\n");
                }

void            CKernel::debug_parser               ()
{
                CString temp;
                temp.Format("\n is vid valid %-8d %-8d \n"
                "size of video   %-8u %-8u \n"
                "number frames   %-8d %-8d \n",
                VID_IS_VALID[0],VID_IS_VALID[1],
                VID_LOADED_BYTES[0],VID_LOADED_BYTES[1],
                FRAME_COUNT[0],FRAME_COUNT[1]);
                m_Screen.Write(temp, strlen(temp));
}