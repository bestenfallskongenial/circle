#include "kernel.h"
#include "global.h"
/*
void            CKernel::io_init_pickup_buffer      ()
{
                menu_pickup_flag[0]  = m_MCP300X.DoSingleEndedConversionRaw(4);
                menu_pickup_flag[1]  = m_MCP300X.DoSingleEndedConversionRaw(5);
                menu_pickup_flag[2]  = m_MCP300X.DoSingleEndedConversionRaw(6);
                menu_pickup_flag[3]  = m_MCP300X.DoSingleEndedConversionRaw(7);
                menu_pickup_flag[4]  = m_MCP300X.DoSingleEndedConversionRaw(4);
                menu_pickup_flag[5]  = m_MCP300X.DoSingleEndedConversionRaw(5);
                menu_pickup_flag[6]  = m_MCP300X.DoSingleEndedConversionRaw(6);
                menu_pickup_flag[7]  = m_MCP300X.DoSingleEndedConversionRaw(7);
                menu_pickup_flag[8]  = m_MCP300X.DoSingleEndedConversionRaw(4);
                menu_pickup_flag[9]  = m_MCP300X.DoSingleEndedConversionRaw(5);
                menu_pickup_flag[10] = m_MCP300X.DoSingleEndedConversionRaw(6);
                menu_pickup_flag[11] = m_MCP300X.DoSingleEndedConversionRaw(7);
}
*/
void            CKernel::io_read_ADC                () 
{
                const float maxA = 1023.0;
                const int scaleFactors[3] = {   2047,       // 2.5V max (1023 * 2)
                                                1551,       // 3.3V max (1023 * 1.515555...)
                                                1023    };  // 5.0V max       

                static int  bufferInt[ADC_CHANNELS][ADC_BUFFER] = { 0 };
                        
                static int i;

                for (unsigned channel = 0; channel < ADC_CHANNELS; ++channel)  // Loop through each channel and read its raw value 
                    {
                    bufferInt[channel][i] = m_MCP300X.DoSingleEndedConversionRaw(channel);  // or 1023?!
                    if(bufferInt[channel][i] > 1023) bufferInt[channel][i] = 1023;


                    adc_raw_value[channel]      = ( bufferInt[channel][0] +
                                                	bufferInt[channel][1] +
                                                	bufferInt[channel][2] +
                                                	bufferInt[channel][3]) >>2 ; 

                    adc_int_value[channel]      = ( adc_raw_value[channel] * scaleFactors[attenuation] ) / 1023;                                
                    
                    adc_float_value[channel] 	= (	adc_int_value[channel]) / (1024.0f);    // maxA +1 or GLfloat normalize the raw value to GLfloat 0.0 to 1.0
                    }
                i = (i + 1) & 3;
}


void            CKernel::io_event_button_A          (   BUTTONS::TEvent Event, void *pParam) // Button Y ( lower )
{
                CKernel *pThis = static_cast<CKernel *>(pParam);
                switch (Event)
                    {
                    case BUTTONS::EventSwitchDown:                                  
                    
                    break;

                    case BUTTONS::EventSwitchUp:

                    if (!(g_menu_mode_new == 1 || g_menu_mode_new == 3))
                        {
                        g_menu_mode_new = 0;
                        is_hold_for_2_sec_a = false;
                        is_hold_for_4_sec_a = false;
                        opaque = 1.0f;
                        adc_pot_routing = false;
                    //  m_ChipSelectPin.Write(false);
                        }
                        a_is_hold = false;

                        break;

                    case BUTTONS::EventSwitchClick:  

                    if (g_menu_mode_new == 1) 
                        {
                        g_menu_mode_new = 3;
                    //  g_menu_mode_old = 3;
                        }
                    else if (g_menu_mode_new == 3) 
                        {
                        g_menu_mode_new = 1;
                    //  g_menu_mode_old = 1;
                        }

                    break;

                    case BUTTONS::EventSwitchDoubleClick:
                //  if ( g_menu_mode_new == 0 ) mode_storage_buffers[FRM_MODE][current_buffer] = !mode_storage_buffers[FRM_MODE][current_buffer];;           // switch between internal time and adc time 
                    if ( g_menu_mode_new != 0 && audio_source_channel != -1 ) sensitivity_new = (sensitivity_new + 1) % 5; // activate mode for lfo 
                    if ( g_menu_mode_new != 0 && audio_source_channel == -1 ) attenuation = (attenuation + 1) % 3;
                   
                    break;
                    
                    case BUTTONS::EventSwitchTripleClick:
                    
                    if ( g_menu_mode_new != 0 ) mode_storage_buffers[TEX_MODE][current_buffer] = !mode_storage_buffers[TEX_MODE][current_buffer];
                    
                    
                    break;
                    
                    case BUTTONS::EventSwitchHold:
                    
                        a_is_hold = true;
                        g_menu_mode_new = 2;    
                        opaque = 0.5f;
                    //  g_menu_mode_old = 2;
                        g_show_bank = 1;
                        adc_pot_routing = true;
                    //  m_ChipSelectPin.Write(true);
                    
                    switch (pThis->m_Button_A.GetHoldQuarterSeconds())
                        {
                        case 4:
                      is_hold_for_2_sec_a = true;
                        break;     
                        case 12:    
                    //  noTargetFPS =! noTargetFPS;  
                        is_hold_for_4_sec_a = true;                     
                        
                        break;
                        
                        default:                                                    
                        
                        return;
                        }                                   										
                    
                    break;
                    
                    default:
                    
                    break;
                    }
}
void            CKernel::io_event_button_B          (   BUTTONS::TEvent Event, void *pParam) // Button X ( upper )
{
                CKernel *pThis = static_cast<CKernel *>(pParam);
                switch (Event)
                    {
                    case BUTTONS::EventSwitchDown:                                  
                    
                    break;

                    case BUTTONS::EventSwitchUp:

                    if (!(g_menu_mode_new == 2))
                        {     										
                        g_menu_mode_new = 0;
                        is_hold_for_2_sec_b = false;
                        is_hold_for_4_sec_b = false;
                        opaque = 1.0f;
                        adc_pot_routing = false;
                    //  m_ChipSelectPin.Write(false);
                        }
                    b_is_hold = false;    

                    break;

                    case BUTTONS::EventSwitchClick:                                 
                    
                    break;
                    case BUTTONS::EventSwitchDoubleClick:
                //  if ( g_menu_mode_new == 2 ) mode_storage_buffers[FRM_MODE][current_buffer] = !mode_storage_buffers[FRM_MODE][current_buffer];;           // switch between internal time and adc time 
                    break;
                    
                    case BUTTONS::EventSwitchTripleClick:                           

                    if ( g_menu_mode_new == 2 ) mode_storage_buffers[FRM_MODE][current_buffer] = !mode_storage_buffers[FRM_MODE][current_buffer];;           // switch between internal time and adc time                    
                    
                    break;
                    
                    case BUTTONS::EventSwitchHold:
                    
                    b_is_hold = true;
                    if ( g_menu_mode_new != 3 )    g_menu_mode_new = 1;    
                //  if ( g_menu_mode_new != 3 )    g_menu_mode_old = 1;
                    opaque = 0.5f;
                    g_show_bank = 0;
                    adc_pot_routing = true;
                //  m_ChipSelectPin.Write(true);

                    switch (pThis->m_Button_B.GetHoldQuarterSeconds())
                        {
                        case 4:
                     is_hold_for_2_sec_b = true;
                        break;
                        case 12:    
                        
                        is_hold_for_4_sec_b = true;                     
                        
                        break;
                        
                        default:                                                    
                        
                        return;
                        }                                   										
                    
                    break;
                    
                    default:
                    
                    break;
                    }
}
