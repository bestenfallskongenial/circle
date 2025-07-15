#include "kernel.h"
#include "global.h"

void            CKernel::LED_circle_color        ()                    // simple display_debug / feedback function
{
                static int counter = 0;

                m_NeoPixels.SetLED (SD_LINE1, 2, (waveTable[0][1*counter%255]>>4), (waveTable[0][2*counter%255]>>4), (waveTable[0][3*counter%255]>>4) );
                m_NeoPixels.SetLED (SD_LINE1, 1, (waveTable[0][2*counter%255]>>4), (waveTable[0][3*counter%255]>>4), (waveTable[0][1*counter%255]>>4) );
                m_NeoPixels.SetLED (SD_LINE1, 3, (waveTable[0][3*counter%255]>>4), (waveTable[0][1*counter%255]>>4), (waveTable[0][2*counter%255]>>4) );
                m_NeoPixels.SetLED (SD_LINE1, 0, (waveTable[0][2*counter%255]>>4), (waveTable[0][1*counter%255]>>4), (waveTable[0][3*counter%255]>>4) );

                m_NeoPixels.Update ();

                counter++;
}
void            CKernel::LED_update              ()            // important, i use the order 0 to 12 ( 0-7 for leds / 8-11 for mode 1 / 12-15 for mode 2 / 16-19 for lfo)
{                               // but: the leds are mapped as 0->3, 1->1, 2->2, 3->0
                switch (g_menu_mode_new)
                    {
                    case 0:                         // default mode 
                        switch (g_show_bank)        // display leds of
                            {
                            case 0:                     // bank 0 ( 0-3 )
                            m_NeoPixels.SetLED (SD_LINE1, 2, led_col_new[ 0][0], led_col_new[ 0][1], led_col_new[ 0][2]);
                            m_NeoPixels.SetLED (SD_LINE1, 1, led_col_new[ 1][0], led_col_new[ 1][1], led_col_new[ 1][2]);
                            m_NeoPixels.SetLED (SD_LINE1, 3, led_col_new[ 2][0], led_col_new[ 2][1], led_col_new[ 2][2]);
                            m_NeoPixels.SetLED (SD_LINE1, 0, led_col_new[ 3][0], led_col_new[ 3][1], led_col_new[ 3][2]);
                            
                            m_NeoPixels.Update ();
                            break;
                            case 1:                     // bank 1 ( 4-7 )
                            m_NeoPixels.SetLED (SD_LINE1, 2, led_col_new[ 4][0], led_col_new[ 4][1], led_col_new[ 4][2]);
                            m_NeoPixels.SetLED (SD_LINE1, 1, led_col_new[ 5][0], led_col_new[ 5][1], led_col_new[ 5][2]);
                            m_NeoPixels.SetLED (SD_LINE1, 3, led_col_new[ 6][0], led_col_new[ 6][1], led_col_new[ 6][2]);
                            m_NeoPixels.SetLED (SD_LINE1, 0, led_col_new[ 7][0], led_col_new[ 7][1], led_col_new[ 7][2]);        
                            
                            m_NeoPixels.Update ();
                            break;
                            }
                            break;
                    case 1:                         // assign 0-3 mode  
                    m_NeoPixels.SetLED (SD_LINE1, 2, led_col_new[ 8][0], led_col_new[ 8][1], led_col_new[ 8][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 1, led_col_new[ 9][0], led_col_new[ 9][1], led_col_new[ 9][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 3, led_col_new[10][0], led_col_new[10][1], led_col_new[10][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 0, led_col_new[11][0], led_col_new[11][1], led_col_new[11][2]);
                
                    m_NeoPixels.Update ();
                            break;
                    case 2:                         // assign 4-7 mode
                    m_NeoPixels.SetLED (SD_LINE1, 2, led_col_new[12][0], led_col_new[12][1], led_col_new[12][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 1, led_col_new[13][0], led_col_new[13][1], led_col_new[13][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 3, led_col_new[14][0], led_col_new[14][1], led_col_new[14][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 0, led_col_new[15][0], led_col_new[15][1], led_col_new[15][2]);
                
                    m_NeoPixels.Update ();
                    break;
                    case 3:                         // setup lfo mode 
                    m_NeoPixels.SetLED (SD_LINE1, 2, led_col_new[16][0], led_col_new[16][1], led_col_new[16][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 1, led_col_new[17][0], led_col_new[17][1], led_col_new[17][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 3, led_col_new[18][0], led_col_new[18][1], led_col_new[18][2]);
                    m_NeoPixels.SetLED (SD_LINE1, 0, led_col_new[19][0], led_col_new[19][1], led_col_new[19][2]);
                
                    m_NeoPixels.Update ();
                    break;
                    }
}