#include "kernel.h"
#include "global.h"

void            CKernel::menu_general               ()
{
                if (g_menu_mode_new != g_menu_mode_old) 
                    {

                    for(int i = 0; i < 16; i++) 
                        {
                        menu_pickup_flag[i] = false;
                        }
                    g_menu_mode_old = g_menu_mode_new;
                    }
}

void            CKernel::menu_mode_default_A        ()  // default function for the cannels 
{
                int chn = 0;
                
                int brightness  = 0;

                int brightnessA = 0;
                int brightnessB = 0;
                int brightnessC = 0;
                int brightnessD = 0;

                unsigned long current_time = m_Timer.GetClockTicks();
                if ( audio_source_channel != -1 )
                    {
                        util_audio_energy(adc_float_value[audio_source_channel]);
                        output_float_value[0] = util_audio_smooth_band[0];
                        output_float_value[1] = util_audio_smooth_band[1];
                        output_float_value[2] = util_audio_smooth_band[2];
                        output_float_value[3] = util_audio_smooth_band[3];

                        brightnessA = (util_audio_smooth_band[0] * 63) + 1;  // gives 1-63
                        brightnessB = (util_audio_smooth_band[1] * 63) + 1;
                        brightnessC = (util_audio_smooth_band[2] * 63) + 1;
                        brightnessD = (util_audio_smooth_band[3] * 63) + 1;

                                led_col_new[0][R] = rgb_tab[A_0 + sensitivity_new][R] + ((rgb_tab[A_0 + sensitivity_new][R] != 0) * brightnessA);
                                led_col_new[0][G] = rgb_tab[A_0 + sensitivity_new][G] + ((rgb_tab[A_0 + sensitivity_new][G] != 0) * brightnessA);
                                led_col_new[0][B] = rgb_tab[A_0 + sensitivity_new][B] + ((rgb_tab[A_0 + sensitivity_new][B] != 0) * brightnessA);

                                led_col_new[1][R] = rgb_tab[A_0 + sensitivity_new][R] + ((rgb_tab[A_0 + sensitivity_new][R] != 0) * brightnessB);
                                led_col_new[1][G] = rgb_tab[A_0 + sensitivity_new][G] + ((rgb_tab[A_0 + sensitivity_new][G] != 0) * brightnessB);
                                led_col_new[1][B] = rgb_tab[A_0 + sensitivity_new][B] + ((rgb_tab[A_0 + sensitivity_new][B] != 0) * brightnessB);

                                led_col_new[2][R] = rgb_tab[A_0 + sensitivity_new][R] + ((rgb_tab[A_0 + sensitivity_new][R] != 0) * brightnessC);
                                led_col_new[2][G] = rgb_tab[A_0 + sensitivity_new][G] + ((rgb_tab[A_0 + sensitivity_new][G] != 0) * brightnessC);
                                led_col_new[2][B] = rgb_tab[A_0 + sensitivity_new][B] + ((rgb_tab[A_0 + sensitivity_new][B] != 0) * brightnessC);

                                led_col_new[3][R] = rgb_tab[A_0 + sensitivity_new][R] + ((rgb_tab[A_0 + sensitivity_new][R] != 0) * brightnessD);
                                led_col_new[3][G] = rgb_tab[A_0 + sensitivity_new][G] + ((rgb_tab[A_0 + sensitivity_new][G] != 0) * brightnessD);
                                led_col_new[3][B] = rgb_tab[A_0 + sensitivity_new][B] + ((rgb_tab[A_0 + sensitivity_new][B] != 0) * brightnessD);                                                                                                                                                                                                                        
                    }
                else
                    {
                    chn = 0;

                    switch(mode_storage_buffers[CH0_MODE][current_buffer]) 
                        {
                        case 0:  // g_input_mode = 0 - Direct ADC
                            output_float_value[0] = adc_float_value[0];
                            output_int_value[0]   = adc_int_value[0];

                                brightness = output_int_value[0]>> 4;

                                    led_col_new[chn][R] = rgb_tab[A25 + attenuation][R] + ((rgb_tab[A25 + attenuation][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[A25 + attenuation][G] + ((rgb_tab[A25 + attenuation][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[A25 + attenuation][B] + ((rgb_tab[A25 + attenuation][B] != 0) * brightness);
                        break;
                    
                        case 1:  // g_input_mode = 1 - Threshold trigger
                        if ( adc_int_value[0] >= threshold_high[0] && !input_threshold_flag[0]) 
                            {
                            output_float_value[0] = random_float_value[0];
                            output_int_value[0]   = random_int_value[0];

                                brightness = output_int_value[0]>> 4;

                                    led_col_new[chn][R] = rgb_tab[TRG][R] + ((rgb_tab[TRG][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[TRG][G] + ((rgb_tab[TRG][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[TRG][B] + ((rgb_tab[TRG][B] != 0) * brightness);

                            input_threshold_flag[0] = true;
                            }
                        else if (adc_int_value[0] < threshold_low[0]) 
                            {
                            input_threshold_flag[0] = false;
                            }
                        break;

                        case 2:  // BPM trigger
                        if (current_time >= next_beat_time[active_bpm_channel]) 
                            {
                            output_float_value[0] = random_float_value[0];
                            output_int_value[0]   = random_int_value[0];

                                led_col_new[chn][R] = 159;
                                led_col_new[chn][G] = 159;
                                led_col_new[chn][B] = 159;
                            }
                        else
                            {
                                brightness = output_int_value[0]>> 4; 

                                    led_col_new[chn][R] = rgb_tab[BPM][R] + ((rgb_tab[BPM][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[BPM][G] + ((rgb_tab[BPM][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[BPM][B] + ((rgb_tab[BPM][B] != 0) * brightness);
                            }
                        break;
                        case 3:
                            output_float_value[0] = LFO_float_output[0];
                            output_int_value[0]   = LFO_int_output[0];

                                brightness = LFO_int_output[0]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF1][R] + ((rgb_tab[LF1][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF1][G] + ((rgb_tab[LF1][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF1][B] + ((rgb_tab[LF1][B] != 0) * brightness);
                        break;
                        case 4:
                            output_float_value[0] = LFO_float_output[1];
                            output_int_value[0]   = LFO_int_output[1];

                                brightness = LFO_int_output[1]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF2][R] + ((rgb_tab[LF2][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF2][G] + ((rgb_tab[LF2][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF2][B] + ((rgb_tab[LF2][B] != 0) * brightness);
                        break;
                        }
    // normal channel 1
                    chn = 1;

                    switch(mode_storage_buffers[CH1_MODE][current_buffer]) 
                        {
                        case 0:  // g_input_mode = 0 - Direct ADC
                            output_float_value[1] = adc_float_value[1];
                            output_int_value[1]   = adc_int_value[1];

                                brightness = output_int_value[1]>> 4;

                                    led_col_new[chn][R] = rgb_tab[A25 + attenuation][R] + ((rgb_tab[A25 + attenuation][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[A25 + attenuation][G] + ((rgb_tab[A25 + attenuation][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[A25 + attenuation][B] + ((rgb_tab[A25 + attenuation][B] != 0) * brightness);
                        break;
                    
                        case 1:  // g_input_mode = 1 - Threshold trigger
                        if ( adc_int_value[1] >= threshold_high[1] && !input_threshold_flag[1]) 
                            {
                            output_float_value[1] = random_float_value[1];
                            output_int_value[1]   = random_int_value[1];

                                brightness = output_int_value[1]>> 4;

                                    led_col_new[chn][R] = rgb_tab[TRG][R] + ((rgb_tab[TRG][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[TRG][G] + ((rgb_tab[TRG][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[TRG][B] + ((rgb_tab[TRG][B] != 0) * brightness);

                            input_threshold_flag[1] = true;
                            }
                        else if (adc_int_value[1] < threshold_low[1]) 
                            {
                            input_threshold_flag[1] = false;
                            }
                        break;
                        
                        case 2:  // BPM trigger
                        if (current_time >= next_beat_time[active_bpm_channel]) 
                            {
                            output_float_value[1] = random_float_value[1];
                            output_int_value[1]   = random_int_value[1];

                                led_col_new[chn][R] = 159;
                                led_col_new[chn][G] = 159;
                                led_col_new[chn][B] = 159;
                            }
                        else
                            {
                                brightness = output_int_value[1]>> 4;

                                    led_col_new[chn][R] = rgb_tab[BPM][R] + ((rgb_tab[BPM][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[BPM][G] + ((rgb_tab[BPM][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[BPM][B] + ((rgb_tab[BPM][B] != 0) * brightness);
                            }
                        break;               
                        case 3:
                            output_float_value[1] = LFO_float_output[0];
                            output_int_value[1]   = LFO_int_output[0];

                                brightness = LFO_int_output[0]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF1][R] + ((rgb_tab[LF1][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF1][G] + ((rgb_tab[LF1][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF1][B] + ((rgb_tab[LF1][B] != 0) * brightness);
                        break;
                        case 4:
                            output_float_value[1] = LFO_float_output[1];
                            output_int_value[1]   = LFO_int_output[1];                            

                                brightness = LFO_int_output[1]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF2][R] + ((rgb_tab[LF2][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF2][G] + ((rgb_tab[LF2][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF2][B] + ((rgb_tab[LF2][B] != 0) * brightness);
                        break;
                        }
    // normal channel 2
                    chn = 2;

                    switch(mode_storage_buffers[CH2_MODE][current_buffer]) 
                        {
                        case 0:  // g_input_mode = 0 - Direct ADC
                            output_float_value[2] = adc_float_value[2];
                            output_int_value[2]   = adc_int_value[2];

                                brightness = output_int_value[2]>> 4;

                                    led_col_new[chn][R] = rgb_tab[A25 + attenuation][R] + ((rgb_tab[A25 + attenuation][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[A25 + attenuation][G] + ((rgb_tab[A25 + attenuation][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[A25 + attenuation][B] + ((rgb_tab[A25 + attenuation][B] != 0) * brightness);                          
                        break;
                    
                        case 1:  // g_input_mode = 1 - Threshold trigger
                        if ( adc_int_value[2] >= threshold_high[2] && !input_threshold_flag[2]) 
                            {
                            output_float_value[2] = random_float_value[2];
                            output_int_value[2]   = random_int_value[2];

                                brightness = output_int_value[2]>> 4;

                                    led_col_new[chn][R] = rgb_tab[TRG][R] + ((rgb_tab[TRG][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[TRG][G] + ((rgb_tab[TRG][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[TRG][B] + ((rgb_tab[TRG][B] != 0) * brightness);

                            input_threshold_flag[2] = true;
                            }
                        else if (adc_int_value[2] < threshold_low[2]) 
                            {
                            input_threshold_flag[2] = false;
                            }
                        break;

                        case 2:  // BPM trigger
                        if (current_time >= next_beat_time[active_bpm_channel]) 
                            {
                            output_float_value[2] = random_float_value[2];
                            output_int_value[2]   = random_int_value[2];

                                led_col_new[chn][R] = 159;
                                led_col_new[chn][G] = 159;
                                led_col_new[chn][B] = 159;
                            }
                        else
                            {
                                brightness = output_int_value[2]>> 4;

                                    led_col_new[chn][R] = rgb_tab[BPM][R] + ((rgb_tab[BPM][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[BPM][G] + ((rgb_tab[BPM][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[BPM][B] + ((rgb_tab[BPM][B] != 0) * brightness);
                            }
                        break;
                        case 3:
                            output_float_value[2] = LFO_float_output[0];
                            output_int_value[2]   = LFO_int_output[0];

                                brightness = LFO_int_output[0]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF1][R] + ((rgb_tab[LF1][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF1][G] + ((rgb_tab[LF1][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF1][B] + ((rgb_tab[LF1][B] != 0) * brightness);
                        break;
                        case 4:
                            output_float_value[2] = LFO_float_output[1];
                            output_int_value[2]   = LFO_int_output[1];

                                brightness = LFO_int_output[1]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF2][R] + ((rgb_tab[LF2][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF2][G] + ((rgb_tab[LF2][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF2][B] + ((rgb_tab[LF2][B] != 0) * brightness);
                        break;                        
                        }
    // normal channel 3
                    chn = 3;

                    switch(mode_storage_buffers[CH3_MODE][current_buffer]) 
                        {
                        case 0:  // g_input_mode = 0 - Direct ADC
                            output_float_value[3] = adc_float_value[3];
                            output_int_value[3]   = adc_int_value[3];

                                brightness = output_int_value[3]>> 4;

                                    led_col_new[chn][R] = rgb_tab[A25 + attenuation][R] + ((rgb_tab[A25 + attenuation][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[A25 + attenuation][G] + ((rgb_tab[A25 + attenuation][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[A25 + attenuation][B] + ((rgb_tab[A25 + attenuation][B] != 0) * brightness);                                  
                        break;
                    
                        case 1:  // g_input_mode = 1 - Threshold trigger
                        if ( adc_int_value[3] >= threshold_high[3] && !input_threshold_flag[3]) 
                            {
                            output_float_value[3] = random_float_value[3];
                            output_int_value[3]   = random_int_value[3];

                                brightness = output_int_value[3]>> 4;

                                    led_col_new[chn][R] = rgb_tab[TRG][R] + ((rgb_tab[TRG][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[TRG][G] + ((rgb_tab[TRG][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[TRG][B] + ((rgb_tab[TRG][B] != 0) * brightness);

                            input_threshold_flag[3] = true;
                            }
                        else if (adc_int_value[3] < threshold_low[3]) 
                            {
                            input_threshold_flag[3] = false;
                            }
                        break;

                        case 2:  // BPM trigger
                        if (current_time >= next_beat_time[active_bpm_channel]) 
                            {
                            output_float_value[3] = random_float_value[3];
                            output_int_value[3]   = random_int_value[3];

                                led_col_new[chn][R] = 159;
                                led_col_new[chn][G] = 159;
                                led_col_new[chn][B] = 159;
                            }
                        else
                            {
                                brightness = output_int_value[3]>> 4;

                                    led_col_new[chn][R] = rgb_tab[BPM][R] + ((rgb_tab[BPM][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[BPM][G] + ((rgb_tab[BPM][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[BPM][B] + ((rgb_tab[BPM][B] != 0) * brightness);
                            }
                        break;
                        case 3:
                            output_float_value[3] = LFO_float_output[0];
                            output_int_value[3]   = LFO_int_output[0];

                                brightness = LFO_int_output[0]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF1][R] + ((rgb_tab[LF1][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF1][G] + ((rgb_tab[LF1][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF1][B] + ((rgb_tab[LF1][B] != 0) * brightness);
                        break;
                        case 4:
                            output_float_value[3] = LFO_float_output[1];
                            output_int_value[3]   = LFO_int_output[1];

                                brightness = LFO_int_output[1]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF2][R] + ((rgb_tab[LF2][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF2][G] + ((rgb_tab[LF2][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF2][B] + ((rgb_tab[LF2][B] != 0) * brightness);
                        break;                        
                        }
                    }
}


void            CKernel::menu_mode_default_B        ()  // default function for the cannels 4-7
{
                unsigned long current_time = m_Timer.GetClockTicks();

                int brightness  = 0;

// normal channel 4

                int chn = 4;

                switch(mode_storage_buffers[CH4_MODE][current_buffer]) 
                    {
                    case 0:  // g_input_mode = 0 - Direct ADC special - frame/time input

                            if ( g_menu_mode_new == 0 ) output_float_value[4] = adc_float_value[4];
                            if ( g_menu_mode_new == 0 ) output_int_value[4]   = adc_int_value[4];

                                brightness = output_int_value[4]>> 4;

                                    led_col_new[chn][R] = rgb_tab[A25 + attenuation][R] + ((rgb_tab[A25 + attenuation][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[A25 + attenuation][G] + ((rgb_tab[A25 + attenuation][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[A25 + attenuation][B] + ((rgb_tab[A25 + attenuation][B] != 0) * brightness); 
                    break;

                    case 1:  // g_input_mode = 1 - Threshold trigger
                    if ( adc_int_value[4] >= threshold_high[4] && !input_threshold_flag[4]) 
                        {
                        output_float_value[4] = random_float_value[4];
                        output_int_value[4]   = random_int_value[4];

                            brightness = output_int_value[4]>> 4;

                                led_col_new[chn][R] = rgb_tab[TRG][R] + ((rgb_tab[TRG][R] != 0) * brightness);
                                led_col_new[chn][G] = rgb_tab[TRG][G] + ((rgb_tab[TRG][G] != 0) * brightness);
                                led_col_new[chn][B] = rgb_tab[TRG][B] + ((rgb_tab[TRG][B] != 0) * brightness);

                        input_threshold_flag[4] = true;
                        }
                    else if (adc_int_value[4] < threshold_low[4]) 
                        {
                        input_threshold_flag[4] = false;
                        }
                    break;

                    case 2:  // BPM trigger
                    if (current_time >= next_beat_time[active_bpm_channel]) 
                        {
                        output_float_value[4] = random_float_value[4];
                        output_int_value[4]   = random_int_value[4];

                            led_col_new[chn][R] = 159;
                            led_col_new[chn][G] = 159;
                            led_col_new[chn][B] = 159;
                        }
                    else
                        {
                            brightness = output_int_value[4]>> 4;

                                led_col_new[chn][R] = rgb_tab[BPM][R] + ((rgb_tab[BPM][R] != 0) * brightness);
                                led_col_new[chn][G] = rgb_tab[BPM][G] + ((rgb_tab[BPM][G] != 0) * brightness);
                                led_col_new[chn][B] = rgb_tab[BPM][B] + ((rgb_tab[BPM][B] != 0) * brightness);
                        }
                    break;
                    case 3:
                        output_float_value[4] = LFO_float_output[0];
                        output_int_value[4]   = LFO_int_output[0];

                            brightness = LFO_int_output[0]>> 4;

                                led_col_new[chn][R] = rgb_tab[LF1][R] + ((rgb_tab[LF1][R] != 0) * brightness);
                                led_col_new[chn][G] = rgb_tab[LF1][G] + ((rgb_tab[LF1][G] != 0) * brightness);
                                led_col_new[chn][B] = rgb_tab[LF1][B] + ((rgb_tab[LF1][B] != 0) * brightness);
                    break;
                    case 4:
                        output_float_value[4] = LFO_float_output[1];
                        output_int_value[4]   = LFO_int_output[1];

                            brightness = LFO_int_output[1]>> 4;

                                led_col_new[chn][R] = rgb_tab[LF2][R] + ((rgb_tab[LF2][R] != 0) * brightness);
                                led_col_new[chn][G] = rgb_tab[LF2][G] + ((rgb_tab[LF2][G] != 0) * brightness);
                                led_col_new[chn][B] = rgb_tab[LF2][B] + ((rgb_tab[LF2][B] != 0) * brightness);
                    break;                        
                    }
// normal channel 5
                chn = 5;

                switch(mode_storage_buffers[CH5_MODE][current_buffer]) 
                    {
                    case 0:  // g_input_mode = 0 - Direct ADC
                        output_float_value[5] = adc_float_value[5];
                        output_int_value[5]   = adc_int_value[5];

                            brightness = output_int_value[5]>> 4;

                                led_col_new[chn][R] = rgb_tab[A25 + attenuation][R] + ((rgb_tab[A25 + attenuation][R] != 0) * brightness);
                                led_col_new[chn][G] = rgb_tab[A25 + attenuation][G] + ((rgb_tab[A25 + attenuation][G] != 0) * brightness);
                                led_col_new[chn][B] = rgb_tab[A25 + attenuation][B] + ((rgb_tab[A25 + attenuation][B] != 0) * brightness);                                   
                    break;

                    case 1:  // g_input_mode = 1 - Threshold trigger special clk input
                    if ( adc_int_value[5] >= threshold_high[5] && !input_threshold_flag[5]) 
                        {
                        output_float_value[5] = random_float_value[5];
                        output_int_value[5]   = random_int_value[5];

                            led_col_new[chn][R] = 223;
                            led_col_new[chn][G] = 223;
                            led_col_new[chn][B] = 223;

                        current_time_for_bpm = current_time;

                        input_threshold_flag[5] = true;
                        }
                    else if (adc_int_value[5] < threshold_low[5]) 
                        {
                        input_threshold_flag[5] = false;
                        }
                    else
                        {
                            brightness = output_int_value[5]>> 4;
                                led_col_new[chn][R] = (rgb_tab[CLK][R] + brightness);
                                led_col_new[chn][G] = (rgb_tab[CLK][G] + brightness);
                                led_col_new[chn][B] = (rgb_tab[CLK][B] + brightness);
                        }

                    break;

                    case 2:  // BPM trigger
                    if (current_time >= next_beat_time[active_bpm_channel]) 
                        {
                        output_float_value[5] = random_float_value[5];
                        output_int_value[5]   = random_int_value[5];

                            led_col_new[chn][R] = 159;
                            led_col_new[chn][G] = 159;
                            led_col_new[chn][B] = 159;
                        }
                    else
                        {
                            brightness = output_int_value[5]>> 4;

                                led_col_new[chn][R] = rgb_tab[BPM][R] + ((rgb_tab[BPM][R] != 0) * brightness);
                                led_col_new[chn][G] = rgb_tab[BPM][G] + ((rgb_tab[BPM][G] != 0) * brightness);
                                led_col_new[chn][B] = rgb_tab[BPM][B] + ((rgb_tab[BPM][B] != 0) * brightness);
                        }
                    break;
                    case 3:
                        output_float_value[5] = LFO_float_output[0];
                        output_int_value[5]   = LFO_int_output[0];

                            brightness = LFO_int_output[0]>> 4;

                                led_col_new[chn][R] = rgb_tab[LF1][R] + ((rgb_tab[LF1][R] != 0) * brightness);
                                led_col_new[chn][G] = rgb_tab[LF1][G] + ((rgb_tab[LF1][G] != 0) * brightness);
                                led_col_new[chn][B] = rgb_tab[LF1][B] + ((rgb_tab[LF1][B] != 0) * brightness);
                    break;
                    case 4:
                        output_float_value[5] = LFO_float_output[1];
                        output_int_value[5]   = LFO_int_output[1];

                            brightness = LFO_int_output[1]>> 4;

                                led_col_new[chn][R] = rgb_tab[LF2][R] + ((rgb_tab[LF2][R] != 0) * brightness);
                                led_col_new[chn][G] = rgb_tab[LF2][G] + ((rgb_tab[LF2][G] != 0) * brightness);
                                led_col_new[chn][B] = rgb_tab[LF2][B] + ((rgb_tab[LF2][B] != 0) * brightness);
                    break;                        
                    }
// normal channel 6
                chn = 6;

                if (!menu_pickup_flag[12] && 
                    (adc_raw_value[6] > menu_pickup_buffer[12] - TOLERANCE) &&
                    (adc_raw_value[6] < menu_pickup_buffer[12] + TOLERANCE)) 
                    {
                    menu_pickup_flag[12] = true;
                    }
                else if (menu_pickup_flag[12] || mode_storage_buffers[CH6_MODE][current_buffer] != 0)
                    {    
                    switch(mode_storage_buffers[CH6_MODE][current_buffer]) 
                        {
                        case 0:  // g_input_mode = 0 - Direct ADC
                            if ( g_menu_mode_new == 0 ) output_float_value[6] = adc_float_value[6];
                            if ( g_menu_mode_new == 0 ) output_int_value[6]   = adc_int_value[6];
                            if ( single_tex_mode == true ) output_int_value[6] = adc_raw_value[6];

                                brightness = output_int_value[6]>> 4;

                                    led_col_new[chn][R] = rgb_tab[A25 + attenuation][R] + ((rgb_tab[A25 + attenuation][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[A25 + attenuation][G] + ((rgb_tab[A25 + attenuation][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[A25 + attenuation][B] + ((rgb_tab[A25 + attenuation][B] != 0) * brightness);                                 
                        break;
                
                        case 1:  // g_input_mode = 1 - Threshold trigger
                        if ( adc_int_value[6] >= threshold_high[6] && !input_threshold_flag[6]) 
                            {
                            output_float_value[6] = random_float_value[6];
                            output_int_value[6]   = random_int_value[6];

                                brightness = output_int_value[6]>> 4;

                                    led_col_new[chn][R] = rgb_tab[TRG][R] + ((rgb_tab[TRG][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[TRG][G] + ((rgb_tab[TRG][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[TRG][B] + ((rgb_tab[TRG][B] != 0) * brightness);

                            input_threshold_flag[6] = true;
                            }
                        else if (adc_int_value[6] < threshold_low[6]) 
                            {
                            input_threshold_flag[6] = false;
                            }
                        break;

                        case 2:  // BPM trigger
                        if (current_time >= next_beat_time[active_bpm_channel]) 
                            {
                            output_float_value[6] = random_float_value[6];
                            output_int_value[6]   = random_int_value[6];

                                led_col_new[chn][R] = 159;
                                led_col_new[chn][G] = 159;
                                led_col_new[chn][B] = 159;
                            }
                        else
                            {
                                brightness = output_int_value[6]>> 4;

                                    led_col_new[chn][R] = rgb_tab[BPM][R] + ((rgb_tab[BPM][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[BPM][G] + ((rgb_tab[BPM][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[BPM][B] + ((rgb_tab[BPM][B] != 0) * brightness);
                            }                            
                        break;
                        case 3:
                            output_float_value[6] = LFO_float_output[0];
                            output_int_value[6]   = LFO_int_output[0];

                                brightness = LFO_int_output[0]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF1][R] + ((rgb_tab[LF1][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF1][G] + ((rgb_tab[LF1][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF1][B] + ((rgb_tab[LF1][B] != 0) * brightness);
                        break;
                        case 4:
                            output_float_value[6] = LFO_float_output[1];
                            output_int_value[6]   = LFO_int_output[1];

                                brightness = LFO_int_output[1]>> 4;
                                
                                    led_col_new[chn][R] = rgb_tab[LF2][R] + ((rgb_tab[LF2][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF2][G] + ((rgb_tab[LF2][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF2][B] + ((rgb_tab[LF2][B] != 0) * brightness);
                        break;                        
                        }        
                if ( g_menu_mode_new == 0 ) menu_pickup_buffer[12] = adc_raw_value[6];
                        }
                    else
                        {
                        led_col_new[chn][R] = (rgb_tab[M_1][R]);
                        led_col_new[chn][G] = (rgb_tab[M_1][G]);
                        led_col_new[chn][B] = (rgb_tab[M_1][B]);    

                    //  if ( g_menu_mode_new == 0 ) menu_pickup_buffer[12] = adc_int_value[6];
                        }
                        
    // normal channel 7
                chn = 7;

                if (!menu_pickup_flag[13] && 
                    (adc_raw_value[7] > menu_pickup_buffer[13] - TOLERANCE) &&
                    (adc_raw_value[7] < menu_pickup_buffer[13] + TOLERANCE)) 
                    {
                    menu_pickup_flag[13] = true;
                    }
                else if (menu_pickup_flag[13] || mode_storage_buffers[CH7_MODE][current_buffer] != 0)
                    {    
                    switch(mode_storage_buffers[CH7_MODE][current_buffer]) 
                        {
                        case 0:  // g_input_mode = 0 - Direct ADC special shader select
                                if ( g_menu_mode_new == 0 ) output_float_value[7] = adc_float_value[7];
                                if ( g_menu_mode_new == 0 ) output_int_value[7]   = adc_raw_value[7];
                                

                                brightness = output_int_value[7]>> 4;
                                    led_col_new[chn][R] = rgb_tab[A_3 + gl_current_prg][R] + ((rgb_tab[A_3 + gl_current_prg][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[A_3 + gl_current_prg][G] + ((rgb_tab[A_3 + gl_current_prg][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[A_3 + gl_current_prg][B] + ((rgb_tab[A_3 + gl_current_prg][B] != 0) * brightness);

                        break;
                        case 1:  // g_input_mode = 1 - Threshold trigger
                        if ( adc_int_value[7] >= threshold_high[7] && !input_threshold_flag[7]) 
                            {
                            output_float_value[7] = random_float_value[7];
                                output_int_value[7] = random_int_value[7];

                                brightness = output_int_value[7]>> 4;

                                    led_col_new[chn][R] = rgb_tab[TRG][R] + ((rgb_tab[TRG][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[TRG][G] + ((rgb_tab[TRG][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[TRG][B] + ((rgb_tab[TRG][B] != 0) * brightness);

                            input_threshold_flag[7] = true;
                            }
                        else if (adc_int_value[7] < threshold_low[7]) 
                            {
                            input_threshold_flag[7] = false;
                            }
                        break;

                        case 2:  // BPM trigger
                        if (current_time >= next_beat_time[active_bpm_channel]) 
                            {
                            output_float_value[7] = random_float_value[7];
                            output_int_value[7]   = random_int_value[7];

                                led_col_new[chn][R] = 159;
                                led_col_new[chn][G] = 159;
                                led_col_new[chn][B] = 159;
                            }
                        else
                            {
                                brightness = output_int_value[7]>> 4;

                                    led_col_new[chn][R] = rgb_tab[BPM][R] + ((rgb_tab[BPM][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[BPM][G] + ((rgb_tab[BPM][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[BPM][B] + ((rgb_tab[BPM][B] != 0) * brightness);
                            }
                        break;
                        case 3:
                            output_float_value[7] = LFO_float_output[0];
                            output_int_value[7]   = LFO_int_output[0];

                                brightness = LFO_int_output[0]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF1][R] + ((rgb_tab[LF1][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF1][G] + ((rgb_tab[LF1][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF1][B] + ((rgb_tab[LF1][B] != 0) * brightness);
                        break;
                        case 4:
                            output_float_value[7] = LFO_float_output[1];
                            output_int_value[7]   = LFO_int_output[1];

                                brightness = LFO_int_output[1]>> 4;

                                    led_col_new[chn][R] = rgb_tab[LF2][R] + ((rgb_tab[LF2][R] != 0) * brightness);
                                    led_col_new[chn][G] = rgb_tab[LF2][G] + ((rgb_tab[LF2][G] != 0) * brightness);
                                    led_col_new[chn][B] = rgb_tab[LF2][B] + ((rgb_tab[LF2][B] != 0) * brightness);
                        break;                        
                        }                       
                if ( g_menu_mode_new == 0 ) menu_pickup_buffer[13] = adc_raw_value[7];
                }
                else
                    {
                    led_col_new[chn][R] = (rgb_tab[M_1][R]);
                    led_col_new[chn][G] = (rgb_tab[M_1][G]);
                    led_col_new[chn][B] = (rgb_tab[M_1][B]);    
                //    if ( g_menu_mode_new == 0 ) menu_pickup_buffer[13] = adc_int_value[7];
                    }  
}

void            CKernel::menu_mode_assign_A         ()  // mapping to the A channels ( 4 to 7 )
{
                int chn = 8;
                int new_mode;
                int brightness  = 0;

                if (g_menu_mode_new == 1)
                    {
// map function 0
                    new_mode = (adc_raw_value[4] * NUMBER_OF_MODES) >> 10;

                    if  (!menu_pickup_flag[0] && new_mode == mode_storage_buffers[CH0_MODE][current_buffer])
                        {
                        menu_pickup_flag[0] = true;
                        }
                    else if (menu_pickup_flag[0]) 
                        {
                            if (mode_storage_buffers[CH0_MODE][current_buffer] == AUD_MODE) { audio_source_channel = -1; }

                            if (new_mode == AUD_MODE) 
                                {                      // Trying to enable audio mode
                                if (audio_source_channel == -1) 
                                    {     // No audio source active
                                    audio_source_channel = 0;         // Take audio source
                                    }
                                else 
                                    {                             // Audio source already taken
                                    new_mode = mode_storage_buffers[CH0_MODE][current_buffer];      // Keep previous mode
                                    }
                                }
                        mode_storage_buffers[CH0_MODE][current_buffer] = new_mode;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[0])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_1][R]);
                            led_col_new[chn][G] = (rgb_tab[M_1][G]);
                            led_col_new[chn][B] = (rgb_tab[M_1][B]);
                        }
                    else 
                        {
                        brightness = output_int_value[0]>> 4;

                            led_col_new[chn][R] = rgb_tab[A50 + new_mode][R] + ((rgb_tab[A50 + new_mode][R] != 0) * brightness);
                            led_col_new[chn][G] = rgb_tab[A50 + new_mode][G] + ((rgb_tab[A50 + new_mode][G] != 0) * brightness);
                            led_col_new[chn][B] = rgb_tab[A50 + new_mode][B] + ((rgb_tab[A50 + new_mode][B] != 0) * brightness);
                        }                                
// map function 1                               
                    chn = 9;
                    new_mode = (adc_raw_value[5] * NUMBER_OF_MODES) >> 10;

                    if (!menu_pickup_flag[1] && new_mode == mode_storage_buffers[CH1_MODE][current_buffer])
                        {
                        menu_pickup_flag[1] = true;
                        }
                    else if (menu_pickup_flag[1]) 
                        {
                            if (mode_storage_buffers[CH1_MODE][current_buffer] == AUD_MODE) { audio_source_channel = -1; }
                            if (new_mode == AUD_MODE) 
                                {                      // Trying to enable audio mode
                                if (audio_source_channel == -1) 
                                    {     // No audio source active
                                    audio_source_channel = 1;         // Take audio source
                                    } 
                                else 
                                    {                             // Audio source already taken
                                    new_mode = mode_storage_buffers[CH1_MODE][current_buffer];      // Keep previous mode
                                    }
                                }
                        mode_storage_buffers[CH1_MODE][current_buffer] = new_mode;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[1])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_1][R]);
                            led_col_new[chn][G] = (rgb_tab[M_1][G]);
                            led_col_new[chn][B] = (rgb_tab[M_1][B]);
                        }
                    else 
                        {
                        brightness = output_int_value[1]>> 4;

                            led_col_new[chn][R] = rgb_tab[A50 + new_mode][R] + ((rgb_tab[A50 + new_mode][R] != 0) * brightness);
                            led_col_new[chn][G] = rgb_tab[A50 + new_mode][G] + ((rgb_tab[A50 + new_mode][G] != 0) * brightness);
                            led_col_new[chn][B] = rgb_tab[A50 + new_mode][B] + ((rgb_tab[A50 + new_mode][B] != 0) * brightness);
                        }                                 
// map function 2
                    chn = 10;

                    new_mode = (adc_raw_value[6] * NUMBER_OF_MODES) >> 10;
                    if (!menu_pickup_flag[2] && new_mode == mode_storage_buffers[CH2_MODE][current_buffer])
                        {
                        menu_pickup_flag[2] = true;
                        }
                    else if (menu_pickup_flag[2]) 
                        {
                            if (mode_storage_buffers[CH2_MODE][current_buffer] == AUD_MODE) { audio_source_channel = -1; }
                            if (new_mode == AUD_MODE) 
                                {                      // Trying to enable audio mode
                                if (audio_source_channel == -1) 
                                    {     // No audio source active
                                    audio_source_channel = 2;         // Take audio source
                                    } 
                                else 
                                    {                             // Audio source already taken
                                    new_mode = mode_storage_buffers[CH2_MODE][current_buffer];      // Keep previous mode
                                    }
                                }
                        mode_storage_buffers[CH2_MODE][current_buffer] = new_mode;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[2])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_1][R]);
                            led_col_new[chn][G] = (rgb_tab[M_1][G]);
                            led_col_new[chn][B] = (rgb_tab[M_1][B]);
                        }
                    else 
                        {
                        brightness = output_int_value[2]>> 4;

                            led_col_new[chn][R] = rgb_tab[A50 + new_mode][R] + ((rgb_tab[A50 + new_mode][R] != 0) * brightness);
                            led_col_new[chn][G] = rgb_tab[A50 + new_mode][G] + ((rgb_tab[A50 + new_mode][G] != 0) * brightness);
                            led_col_new[chn][B] = rgb_tab[A50 + new_mode][B] + ((rgb_tab[A50 + new_mode][B] != 0) * brightness);
                        }                                 
// map function 3
                    chn = 11;

                    new_mode = (adc_raw_value[7] * NUMBER_OF_MODES) >> 10;
                    if (!menu_pickup_flag[3] && new_mode == mode_storage_buffers[CH3_MODE][current_buffer])
                        {
                        menu_pickup_flag[3] = true;
                        }
                    else if (menu_pickup_flag[3]) 
                        {
                            if (mode_storage_buffers[CH3_MODE][current_buffer] == AUD_MODE) { audio_source_channel = -1; }
                            if (new_mode == AUD_MODE) 
                                {                      // Trying to enable audio mode
                                if (audio_source_channel == -1) 
                                    {     // No audio source active
                                    audio_source_channel = 3;         // Take audio source
                                    } 
                                else 
                                    {                             // Audio source already taken
                                    new_mode = mode_storage_buffers[CH3_MODE][current_buffer];      // Keep previous mode
                                    }
                                }
                        mode_storage_buffers[CH3_MODE][current_buffer] = new_mode;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[3])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_1][R]);
                            led_col_new[chn][G] = (rgb_tab[M_1][G]);
                            led_col_new[chn][B] = (rgb_tab[M_1][B]);
                        }
                    else 
                        {
                        brightness = output_int_value[3]>> 4;

                            led_col_new[chn][R] = rgb_tab[A50 + new_mode][R] + ((rgb_tab[A50 + new_mode][R] != 0) * brightness);
                            led_col_new[chn][G] = rgb_tab[A50 + new_mode][G] + ((rgb_tab[A50 + new_mode][G] != 0) * brightness);
                            led_col_new[chn][B] = rgb_tab[A50 + new_mode][B] + ((rgb_tab[A50 + new_mode][B] != 0) * brightness);
                        }                                 
                    }
}

void            CKernel::menu_mode_assign_B         ()  // mapping to the A channels ( 4 to 7 )
{
                int chn = 12;

                int new_mode;

                int brightness  = 0;

                if (g_menu_mode_new == 2) 
                    {
// map function 4
                    new_mode = (adc_raw_value[4] * ( NUMBER_OF_MODES - 1) ) >> 10;
                    if  (!menu_pickup_flag[4] && new_mode == mode_storage_buffers[CH4_MODE][current_buffer])
                        {
                        menu_pickup_flag[4] = true;
                        }
                    else if (menu_pickup_flag[4]) 
                        {
                        mode_storage_buffers[CH4_MODE][current_buffer] = new_mode;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[4])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_2][R]);
                            led_col_new[chn][G] = (rgb_tab[M_2][G]);
                            led_col_new[chn][B] = (rgb_tab[M_2][B]);
                        }
                    else 
                        {
                        brightness = output_int_value[4]>> 4;

                            led_col_new[chn][R] = rgb_tab[A50 + new_mode][R] + ((rgb_tab[A50 + new_mode][R] != 0) * brightness);
                            led_col_new[chn][G] = rgb_tab[A50 + new_mode][G] + ((rgb_tab[A50 + new_mode][G] != 0) * brightness);
                            led_col_new[chn][B] = rgb_tab[A50 + new_mode][B] + ((rgb_tab[A50 + new_mode][B] != 0) * brightness);
                        }                                 
// map function 5
                    chn = 13;

                    new_mode = (adc_raw_value[5] * ( NUMBER_OF_MODES - 1) ) >> 10;
                    if (!menu_pickup_flag[5] && new_mode == mode_storage_buffers[CH5_MODE][current_buffer])
                        {
                        menu_pickup_flag[5] = true;
                        }
                    else if (menu_pickup_flag[5]) 
                        {
                        mode_storage_buffers[CH5_MODE][current_buffer] = new_mode;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[5])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_2][R]);
                            led_col_new[chn][G] = (rgb_tab[M_2][G]);
                            led_col_new[chn][B] = (rgb_tab[M_2][B]);
                        }
                    else 
                        {
                        brightness = output_int_value[5]>> 4;

                            led_col_new[chn][R] = rgb_tab[A50 + new_mode][R] + ((rgb_tab[A50 + new_mode][R] != 0) * brightness);
                            led_col_new[chn][G] = rgb_tab[A50 + new_mode][G] + ((rgb_tab[A50 + new_mode][G] != 0) * brightness);
                            led_col_new[chn][B] = rgb_tab[A50 + new_mode][B] + ((rgb_tab[A50 + new_mode][B] != 0) * brightness);
                        }                                 
// map function 6
                    chn = 14;

                    new_mode = (adc_raw_value[6] * ( NUMBER_OF_MODES - 1 )) >> 10;
                    if (!menu_pickup_flag[6] && new_mode == mode_storage_buffers[CH6_MODE][current_buffer])
                        {
                        menu_pickup_flag[6] = true;
                        }
                    else if (menu_pickup_flag[6]) 
                        {
                        mode_storage_buffers[CH6_MODE][current_buffer] = new_mode;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[6])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_2][R]);
                            led_col_new[chn][G] = (rgb_tab[M_2][G]);
                            led_col_new[chn][B] = (rgb_tab[M_2][B]);
                        }
                    else 
                        {
                        brightness = output_int_value[6]>> 4;

                            led_col_new[chn][R] = rgb_tab[A50 + new_mode][R] + ((rgb_tab[A50 + new_mode][R] != 0) * brightness);
                            led_col_new[chn][G] = rgb_tab[A50 + new_mode][G] + ((rgb_tab[A50 + new_mode][G] != 0) * brightness);
                            led_col_new[chn][B] = rgb_tab[A50 + new_mode][B] + ((rgb_tab[A50 + new_mode][B] != 0) * brightness);
                        }                                 
// map function 7
                    chn = 15;

                    new_mode = (adc_raw_value[7] * ( NUMBER_OF_MODES - 1 )) >> 10;
                    if (!menu_pickup_flag[7] && new_mode == mode_storage_buffers[CH7_MODE][current_buffer])
                        {
                        menu_pickup_flag[7] = true;
                        }
                    else if (menu_pickup_flag[7]) 
                        {
                        mode_storage_buffers[CH7_MODE][current_buffer] = new_mode;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[7])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_2][R]);
                            led_col_new[chn][G] = (rgb_tab[M_2][G]);
                            led_col_new[chn][B] = (rgb_tab[M_2][B]);
                        }
                    else 
                        {
                        brightness = output_int_value[7]>> 4;

                            led_col_new[chn][R] = rgb_tab[A50 + new_mode][R] + ((rgb_tab[A50 + new_mode][R] != 0) * brightness);
                            led_col_new[chn][G] = rgb_tab[A50 + new_mode][G] + ((rgb_tab[A50 + new_mode][G] != 0) * brightness);
                            led_col_new[chn][B] = rgb_tab[A50 + new_mode][B] + ((rgb_tab[A50 + new_mode][B] != 0) * brightness);
                        }                                 
                    }
}

void            CKernel::menu_mode_assign_LFO       ()  // mapping the LFO-parameters
{
                int chn = 16;

                if (g_menu_mode_new == 3 ) 
                    {
                    int waveformA = (adc_raw_value[4] * ( WAVEFORMS )) >> 10;

                    if (!menu_pickup_flag[8] && (waveformA == mode_storage_buffers[LF1_WAVE][current_buffer])) 
                        {
                        menu_pickup_flag[8] = true;
                        }
                    else if (menu_pickup_flag[8]) 
                        {
                        mode_storage_buffers[LF1_WAVE][current_buffer] = waveformA;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[8])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_L][R]);
                            led_col_new[chn][G] = (rgb_tab[M_L][G]);
                            led_col_new[chn][B] = (rgb_tab[M_L][B]);
                        }
                    else 
                        {
                            led_col_new[chn][R] = (rgb_tab[LF1][R]);
                            led_col_new[chn][G] = (rgb_tab[LF1][G]);
                            led_col_new[chn][B] = (rgb_tab[LF1][B]);
                        }    
                    chn = 17;

                    int waveformB = (adc_raw_value[5] * ( WAVEFORMS )) >> 10;

                    if (!menu_pickup_flag[9] && (waveformB == mode_storage_buffers[LF2_WAVE][current_buffer])) 
                        {
                        menu_pickup_flag[9] = true;
                        }
                    else if (menu_pickup_flag[9]) 
                        {
                        mode_storage_buffers[LF2_WAVE][current_buffer] = waveformB;   // Process new value after pickup
                        }
                    if (!menu_pickup_flag[9])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_L][R]);
                            led_col_new[chn][G] = (rgb_tab[M_L][G]);
                            led_col_new[chn][B] = (rgb_tab[M_L][B]);
                        }
                    else 
                        {
                            led_col_new[chn][R] = (rgb_tab[LF2][R]);
                            led_col_new[chn][G] = (rgb_tab[LF2][G]);
                            led_col_new[chn][B] = (rgb_tab[LF2][B]);
                        }                       
                    chn = 18;

                    int multiplyA = (adc_raw_value[6] * ( 7 )) >> 10;

                    if (!menu_pickup_flag[10] && (multiplyA == mode_storage_buffers[LF1_MULT][current_buffer])) 
                        {
                        menu_pickup_flag[10] = true;
                        }
                    else if (menu_pickup_flag[10]) 
                        {
                        mode_storage_buffers[LF1_MULT][current_buffer] = multiplyA;   // Process new value after pickup
                        }      
                    if (!menu_pickup_flag[10])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_L][R]);
                            led_col_new[chn][G] = (rgb_tab[M_L][G]);
                            led_col_new[chn][B] = (rgb_tab[M_L][B]);
                        }
                    else 
                        {
                            led_col_new[chn][R] = (rgb_tab[LF1][R]);
                            led_col_new[chn][G] = (rgb_tab[LF1][G]);
                            led_col_new[chn][B] = (rgb_tab[LF1][B]);
                        }                                   
                    chn = 19;

                    int multiplyB = (adc_raw_value[7] * ( 7 )) >> 10;

                    if (!menu_pickup_flag[11] && (multiplyB == mode_storage_buffers[LF2_MULT][current_buffer])) 
                        {
                        menu_pickup_flag[11] = true;
                        }
                    else if (menu_pickup_flag[11]) 
                        {
                        mode_storage_buffers[LF2_MULT][current_buffer] = multiplyB;   // Process new value after pickup
                        }    
                    if (!menu_pickup_flag[11])
                        {
                            led_col_new[chn][R] = (rgb_tab[M_L][R]);
                            led_col_new[chn][G] = (rgb_tab[M_L][G]);
                            led_col_new[chn][B] = (rgb_tab[M_L][B]);
                        }
                    else 
                        {
                            led_col_new[chn][R] = (rgb_tab[LF2][R]);
                            led_col_new[chn][G] = (rgb_tab[LF2][G]);
                            led_col_new[chn][B] = (rgb_tab[LF2][B]);
                        }                                                                                                          
                    }
}