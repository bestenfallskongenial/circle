#include "kernel.h"
#include "global.h"

void            CKernel::display_LoadScreenTexVidShd( int mode)
{
                    const char* clearScreen = "\x1b[H\x1b[J";               // first we clear the screen

                    m_Screen.Write(clearScreen, strlen(clearScreen));

                    switch(mode)
                    {
                        case 0:
                        m_Screen.Write("\n Load Vertex Shader Files   /", strlen("\n Load Vertex Shader Files   /"));
                        break;
                        case 1:
                        m_Screen.Write("\n Load Fragment Shader Files /", strlen("\n Load Fragment Shader Files /"));
                        break;
                        case 2:
                        m_Screen.Write("\n Load BMP Texture Files     /", strlen("\n Load BMP Texture Files     /"));
                        break;
                        case 3:
                        m_Screen.Write("\n Load H264 Video Files      /", strlen("\n Load H264 Video Files      /"));
                        break;
                        case 4:
                        m_Screen.Write("\n Load IMG Firmware Files    /", strlen("\n Load IMG Firmware Files    /"));
                        break;                                                                        
                    }

                    LED_circle_color()                                      // circle the led colors 

}
bool            CKernel::LoadScreenTexVid()
{

    
}


bool            CKernel::display_startup_screen(CUBE_STATE_T *state)
{                   CString info;
                    const char* clearScreen = "\x1b[H\x1b[J";

                    m_Screen.Write(clearScreen, strlen(clearScreen));

                    m_Screen.Write("\n", 1);

                    info.Format("\n\n  Machine Model: %s V%u.%u SoC Type: %s | RAM Size: %u MB\n\n", 
                    m_MachineInfo.GetMachineName(), m_MachineInfo.GetModelMajor(), m_MachineInfo.GetModelRevision(),m_MachineInfo.GetSoCName(), m_MachineInfo.GetRAMSize());
                    m_Screen.Write(info, info.GetLength());

                    info.Format("    CPU Speed: %s | SoC Max Temperature: %u Celsius\n\n", 
                    m_Options.GetCPUSpeed() == CPUSpeedMaximum ? "Maximum" : "Low", m_Options.GetSoCMaxTemp());
                    m_Screen.Write(info, info.GetLength());

                    info.Format("    Core: %3u MHz | ARM: %3u MHz\n\n    EMMC: %3u MHz | EMMC2: %3u MHz | UART: %3u MHz\n\n", 
                    m_MachineInfo.GetClockRate(CLOCK_ID_CORE)/ 1000000, m_MachineInfo.GetClockRate(CLOCK_ID_ARM)/ 1000000,
                    m_MachineInfo.GetClockRate(CLOCK_ID_EMMC)/ 1000000, m_MachineInfo.GetClockRate(CLOCK_ID_EMMC2)/ 1000000,
                    m_MachineInfo.GetClockRate(CLOCK_ID_UART)/ 1000000);
                    m_Screen.Write(info, info.GetLength());

                    info.Format("    DMA Channel: %u\n\n", m_MachineInfo.AllocateDMAChannel(DMA_CHANNEL_NORMAL));
                    m_Screen.Write(info, info.GetLength());

                    info.Format("    USB Power Delay: %u ms | USB Full Speed: %s\n\n", 
                    m_Options.GetUSBPowerDelay(), m_Options.GetUSBFullSpeed() ? "Yes" : "No");
                    m_Screen.Write(info, info.GetLength());

                    return true;
}

void            CKernel::display_append_modes() 
{
                CString line;

                char tempBuffer[27];

                memcpy(tempBuffer, &m_bufferFshader[gl_current_prg][2], 26);

                tempBuffer[26] = '\0';

                CString shaderPart;
                
                shaderPart = tempBuffer;

                m_Screen.Write(CLEAR_SCREEN, strlen(CLEAR_SCREEN));                
                
                const char* table_attenuator[3]= { "2.5", "3.3", "5.0" };

                const char* table_waveform[4] = { "Sin","Saw","Tr1","Tr2" };

                const char* table_intensity[5] = { "12.5%", "25%", "50%", "75%", "100%" };

//                                             0     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15
                const char* table_menu[16]={ "ADC","TRG","BPM","LF1","LF2","AUD","   ","   ","   ","---"," | ","*  ","  *","TEX","FRM","PRG"};

                if ( g_menu_mode_new == 1 || g_menu_mode_new == 2 || g_menu_mode_new == 3)
                    {                
                    line.Format(    "\n\n"
                                    "   --------------------------\n"
                                    "   %s\n"
                                    "   --------------------------\n"
                                    "    %s %s>A0  A1<%s %s%s\n"
                                    "    %s %s>A2  A3<%s %s%s\n\n"
                                    "    %s %s>B0  B1<%s %s%s\n"
                                    "    %s %s>B2  B3<%s %s%s\n\n"
                                    "        %s>L1  L2<%s    %s\n"
                                    "        %3d>D1  D2<%-3d    %s\n\n"
                                    "   %s %3u BPM  CLK %-3u %s\n\n"
                                    "        %s CV  As %s\n" 
                                    "   --------------------------\n"
                                    "    Prg %2d of %2d%sFPS %.2f\n"
                                    "   --------------------------\n",
                                    (const char *)shaderPart,
                                    table_menu[8],
                                    (   mode_storage_buffers[CH0_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH1_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH2_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH3_MODE][current_buffer]==AUD_MODE) && 
                                        mode_storage_buffers[CH0_MODE][current_buffer]!=AUD_MODE  // Changed 3 to 5
                                    ? table_menu[9] : table_menu[mode_storage_buffers[CH0_MODE][current_buffer]], 
                                    (   mode_storage_buffers[CH0_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH1_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH2_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH3_MODE][current_buffer]==AUD_MODE) && 
                                        mode_storage_buffers[CH1_MODE][current_buffer]!=AUD_MODE  // Changed 3 to 5
                                    ? table_menu[9] : table_menu[mode_storage_buffers[CH1_MODE][current_buffer]], 
                                    table_menu[8],
                                    (g_menu_mode_new == 1) ? table_menu[10] : table_menu[8],

                                    table_menu[8],
                                    (   mode_storage_buffers[CH0_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH1_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH2_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH3_MODE][current_buffer]==AUD_MODE) && 
                                        mode_storage_buffers[CH2_MODE][current_buffer]!=AUD_MODE  // Changed 3 to 5
                                    ? table_menu[9] : table_menu[mode_storage_buffers[CH2_MODE][current_buffer]], 
                                    (   mode_storage_buffers[CH0_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH1_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH2_MODE][current_buffer]==AUD_MODE || 
                                        mode_storage_buffers[CH3_MODE][current_buffer]==AUD_MODE) && 
                                        mode_storage_buffers[CH3_MODE][current_buffer]!=AUD_MODE  // Changed 3 to 5
                                    ? table_menu[9] : table_menu[mode_storage_buffers[CH3_MODE][current_buffer]], 
                                    table_menu[8],
                                    (g_menu_mode_new == 1) ? table_menu[10] : table_menu[8],

                                    (mode_storage_buffers[FRM_MODE][current_buffer] ? table_menu[14] : table_menu[8]),  
                                    table_menu[mode_storage_buffers[CH4_MODE][current_buffer]], 
                                    table_menu[mode_storage_buffers[CH5_MODE][current_buffer]], 
                                    table_menu[8],
                                    (g_menu_mode_new == 2) ? table_menu[10] : table_menu[8],

                                    (mode_storage_buffers[TEX_MODE][current_buffer] ? table_menu[13] : table_menu[8]),  
                                    table_menu[mode_storage_buffers[CH6_MODE][current_buffer]], 
                                    table_menu[mode_storage_buffers[CH7_MODE][current_buffer]], 
                                    table_menu[15],  
                                    (g_menu_mode_new == 2) ? table_menu[10] : table_menu[8],

                                    table_waveform[mode_storage_buffers[LF1_WAVE][current_buffer]],  
                                    table_waveform[mode_storage_buffers[LF2_WAVE][current_buffer]],
                                    (g_menu_mode_new == 3) ? table_menu[10] : table_menu[8],
                                    multiplier[mode_storage_buffers[LF1_MULT][current_buffer]],
                                    multiplier[mode_storage_buffers[LF2_MULT][current_buffer]],
                                    (g_menu_mode_new == 3) ? table_menu[10] : table_menu[8],

                                    (active_bpm_channel == 0) ? table_menu[12] : table_menu[8], resultBPM[0], 
                                    resultBPM[1], (active_bpm_channel == 1) ? table_menu[11] : table_menu[8], 
                                    table_attenuator[attenuation], table_intensity[sensitivity_new],
                                    gl_current_prg, (linked_programs-1), 
                                    (shader_has_stored_params[gl_current_prg] == true) ? table_menu[11] : table_menu[8],
                                    current_fps);             

                        m_Screen.Write(line, strlen(line));
                    }    
}