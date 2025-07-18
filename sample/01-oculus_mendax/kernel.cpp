#include "kernel.h"     
#include "global.h"                                    
#include <circle/string.h>                                  
#include <circle/memory.h>                                  

#include <linux/kernel.h>
#include <linux/delay.h>                // ????????????????????????????????

#include <assert.h>

#include "bcm_host.h"  

#define LOG_NAME "VCSM"

                static  const char              FromKernel[] = "kernel";
                
                CKernel::CKernel (void)
                :	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	                m_Timer (&m_Interrupt),
	                m_Logger (LOGLEVEL, &m_Timer),
                    m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	                m_USBHCI (&m_Interrupt, &m_Timer, TRUE),
	                m_VCHIQ (CMemorySystem::Get (), &m_Interrupt),
                    m_SharedMemory(),
                    m_H264Decoder(),
                    m_H264Parser(),
                    m_bStorageAttached (FALSE),
	                m_pFileSystem (0),
                    m_Watchdog(),
	                m_SPIMaster (SPI_CLOCK_SPEED, 0, 0, SPI_MASTER_DEVICE),
	                m_MCP300X (&m_SPIMaster, VREF, SPI_CHIP_SELECT, SPI_CLOCK_SPEED),
                    m_NeoPixels (SD_LINES_MASK, LED_COUNT),
                    m_GPIOManager (&m_Interrupt),
                    m_ChipSelectPin(CS_PIN, GPIOModeOutput),
	                m_Button_A (SW_PIN_A), m_Button_B (SW_PIN_B),

                    state{},

                    m_bufferVideo(nullptr),
                    m_videoBlockBase(nullptr),
                    m_videoRawBlock(nullptr),
                    m_videoBlockSize(0),

                    m_bufferFrameBufferA(nullptr),
                    m_frameBlockBaseA(nullptr),
                    m_frameRawBlockA(nullptr),
                    m_frameBlockSizeA(0),

                    m_bufferFrameBufferB(nullptr),
                    m_frameBlockBaseB(nullptr),
                    m_frameRawBlockB(nullptr),
                    m_frameBlockSizeB(0),

                    m_bufferTexture(nullptr),

                    m_bufferKernel(nullptr),

                    m_bufferVshader(nullptr), 
                    m_bufferFshader(nullptr),

                    m_USBhasLoadOnes(false),                                // we load only one time from usb, sorry user!
                    m_resetFlag(false)
                    {
	                m_ActLED.Blink (5);	// show we are alive
                    }
                CKernel::~CKernel (void)
                    {
                    memory_clean_up(); 
                    }
boolean         CKernel::Initialize (void)
{
	            boolean bOK = TRUE;
	            if (bOK) { bOK = m_Screen.Initialize (); }
	            if (bOK) { bOK = m_Serial.Initialize (115200); }
                if (bOK) { CDevice *pTarget = m_DeviceNameService.GetDevice(m_Options.GetLogDevice(), FALSE);
                    if (pTarget == 0)
                        {
                        pTarget = &m_Screen;
                        }
                    bOK = m_Logger.Initialize(pTarget);
                    }
                if (bOK) {  bOK = m_Interrupt.Initialize (); }
	            if (bOK) {  bOK = display_startup_screen (&state); }
	            if (bOK) {  bOK = m_Timer.Initialize (); }
	            if (bOK) {  bOK = m_EMMC.Initialize (); }    
	            if (bOK) {  bOK = m_USBHCI.Initialize (); 
                            m_USBHCI.UpdatePlugAndPlay();}


                if (bOK) {  m_Timer.MsDelay(1000);
                            bOK = memory_allocate(); } 

                if (bOK) {  bOK = m_VCHIQ.Initialize (); }
                if (bOK) {  m_Timer.MsDelay(200);
                            bcm_host_init(); }

                if (bOK) {  m_Timer.MsDelay(200);
                            gfx_init_OGL(&state); }

                if (bOK) {  m_Timer.MsDelay(200);
                            m_SharedMemory.VCSMInitialize   (); 
                            m_SharedMemory.VCSMimportMemory (m_videoBlockBase, m_videoBlockSize, &m_VCSMinputHandle);
                            m_SharedMemory.VCSMLockMemory   (m_VCSMinputHandle, &m_VCSMinputPtr);
                            m_SharedMemory.VCSMimportMemory (m_frameBlockBaseA, m_frameBlockSizeA, &m_VCSMoutputHandleA);
                            m_SharedMemory.VCSMLockMemory   (m_VCSMoutputHandleA, &m_VCSMoutputPtrA);
                            m_SharedMemory.VCSMimportMemory (m_frameBlockBaseB, m_frameBlockSizeB, &m_VCSMoutputHandleB);
                            m_SharedMemory.VCSMLockMemory   (m_VCSMoutputHandleB, &m_VCSMoutputPtrB);
                         } 
/*                                                            
                if (bOK) { m_H264Decoder.MMALinitialize           (     m_VCSMinputHandle,         // my input buffer handle from smem
                                                                        m_videoBlockSize,           // my allocated input buffer size 
                                                                        m_VCSMoutputHandleA,       // my output buffer handle a from smem 
                                                                        m_VCSMoutputHandleB,       // my output buffer handle b from smem
                                                                        m_frameBlockSizeA,          // my allocated output buffer size
                                                                        640,            // 
                                                                        480, 
                                                                        state.display,      // <-- add this
                                                                        state.context);       // <-- and this)
                }
*/      
                if (bOK) { m_Watchdog.Start(TIMEOUT); }
	            if (bOK) { m_SPIMaster.Initialize (); }
                if (bOK) { m_ChipSelectPin.Write(LOW); }
	            if (bOK) { bOK = m_Button_A.Initialize ();
                           m_Button_A.RegisterEventHandler (io_event_button_A, this); }
	            if (bOK) { bOK = m_Button_B.Initialize ();
                           m_Button_B.RegisterEventHandler (io_event_button_B, this);  }

	            return bOK;
}

TShutdownMode   CKernel::Run(void)
{
            m_Timer.MsDelay(500);
m_H264Decoder.MMALinitialize           (                                m_VCSMinputHandle,         // my input buffer handle from smem
                                                                        m_videoBlockSize,           // my allocated input buffer size 
                                                                        m_VCSMoutputHandleA,       // my output buffer handle a from smem 
                                                                        m_VCSMoutputHandleB,       // my output buffer handle b from smem
                                                                        m_frameBlockSizeA,          // my allocated output buffer size
                                                                        640,            // 
                                                                        480, 
                                                                        state.display,      // <-- add this
                                                                        state.context);       // <-- and this)

                CString test;
                CString test1;

                util_prep_parameters();
                
                const char* cursoroff = "\x1b[?25l";
                m_Screen.Write(cursoroff, strlen(cursoroff));

                if (util_check_for_update())    // Call util_check_for_update() before entering the loop
                    {
                    return ShutdownReboot;      // If the update was successful, proceed with reboot
                    }

            m_H264Parser.ParseInitialize(   8, 
                                            TEX_SIZE, 
                                            8, 
                                            MAX_FRAMES, 
                                            VIDEO_WIDTH, 
                                            VIDEO_HEIGHT, 
                                            BASELINE_PROFILE, 
                                            41 );            

                m_Timer.MsDelay(500);

                if( filesystem_mount("emmc1-1", 
                    SCANED_FILES_VSH,  VSH_LOADED_BYTES, VSH_SD,
                    SCANED_FILES_FSH,  FSH_LOADED_BYTES, FSH_SD,
                    SCANED_FILES_TEX,  TEX_LOADED_BYTES, TEX_SD,
                    SCANED_FILES_VID,  VID_LOADED_BYTES, VID_SD))
                    {                   
                    gfx_init_v_buffer(&state);

                    parser_bmp(TEX_LOADED_OLD,TEX_LOADED_NEW);
                    parser_h264(VID_LOADED_OLD,VID_LOADED_NEW);
    
                    gfx_init_vshaders(&state, VSH_LOADED_OLD, VSH_LOADED_NEW);
                    gfx_init_fshaders(&state, FSH_LOADED_OLD, FSH_LOADED_NEW);
                    gfx_init_programs(&state, FSH_LOADED_OLD, FSH_LOADED_NEW);
                    gfx_init_uniforms(&state, FSH_LOADED_OLD, FSH_LOADED_NEW);
                    gfx_init_textures(&state, TEX_LOADED_OLD, TEX_LOADED_NEW);

                    m_Watchdog.Start(TIMEOUT);

                //  parser_debug(VID_LOADED_OLD, VID_LOADED_NEW); 

                    m_Timer.MsDelay(100);
 
                    VSH_LOADED_OLD = VSH_LOADED_NEW;    // also 0 at bytesize calc beacuse i loaded only here
                    FSH_LOADED_OLD = FSH_LOADED_NEW;
                    TEX_LOADED_OLD = TEX_LOADED_NEW;    // we need to reset the couter casues issues later at byte size calculation
                    VID_LOADED_OLD = VID_LOADED_NEW;
                    }
                    m_Timer.MsDelay(500);
                    CString bufferVCSM = m_SharedMemory.m_DebugCharArray;
                    CString bufferMMAL = m_H264Decoder.m_DebugCharArray;
                    filesystem_save_log_file( "emmc1-1", FILENAME_VCSM_LOG, bufferVCSM);
                    filesystem_save_log_file( "emmc1-1", FILENAME_MMAL_LOG, bufferMMAL);

                while (m_resetFlag == false)                                                  // mainloop
                    {
                    start_time_fps_calculation = m_Timer.GetClockTicks();                   // for the "fps break"
                    m_Watchdog.Start(8);

                    if(filesystem_update_USB("umsd1") == true && m_USBhasLoadOnes == false)         // checks and loads from usb
                        {
                    //  TEX_LOADED_OLD = 0;     // we need to reset the couter
                    //  TEX_LOADED_NEW = 0;     // that the brandlogo is removed

                        if( filesystem_mount("umsd1-1",                                                 // 1. scan the filenames!
                            SCANED_FILES_VSH, VSH_LOADED_BYTES, VSH_USB,
                            SCANED_FILES_FSH, FSH_LOADED_BYTES, FSH_USB,
                            SCANED_FILES_TEX, TEX_LOADED_BYTES, TEX_USB, 
                            SCANED_FILES_VID, VID_LOADED_BYTES, VID_USB))
                            {
                            parser_bmp(TEX_LOADED_OLD,TEX_LOADED_NEW);
                            parser_h264(VID_LOADED_OLD,VID_LOADED_NEW);

                            gfx_init_fshaders(&state, FSH_LOADED_OLD, FSH_LOADED_NEW);
                            gfx_init_programs(&state, FSH_LOADED_OLD, FSH_LOADED_NEW);
                            gfx_init_uniforms(&state, FSH_LOADED_OLD, FSH_LOADED_NEW);
                            gfx_init_textures(&state, TEX_LOADED_OLD, TEX_LOADED_NEW); 

                            filesystem_save_log_file( "umsd1-1", FILENAME_GL_LOG, g_log_string);

                            m_USBhasLoadOnes = true;
                            m_Timer.MsDelay(100);
                            }
                        }

                    gfx_render_shader_a(&state);

                    gl_current_prg = util_choose_program();
                    gl_current_tex = util_choose_texture();

                    util_store_program();

                    io_read_ADC();

                    m_Button_A.Update ();
                    m_Button_B.Update ();
                    m_ChipSelectPin.Write(adc_pot_routing); // change to occationally call, not each u_time 

                    switch (mode_storage_buffers[FRM_MODE][current_buffer])
                        {
                        case true:  GLtime = output_int_value[4]/36.0f;                     break;
                        case false: GLtime = start_time_fps_calculation / 1000000.0f;       break;
                        }
                    util_random_vec8(start_time_fps_calculation);
 
                    menu_general();
                    
                    menu_mode_default_A();
                    menu_mode_default_B();
                    menu_mode_assign_A();
                    menu_mode_assign_B();
                    menu_mode_assign_LFO();

                    display_append_modes();

                    LED_update();

                    util_calculate_BPM(m_Button_B.GetLastEventTime(), current_time_for_bpm);
                    util_update_predicted_beat();
                    util_determine_bpm_source();
                    util_LFO();                 

                    if ( is_hold_for_4_sec_a && is_hold_for_4_sec_b ) m_resetFlag = true;
// i like to do it as function !!!
                    // FPS limiting using previous swap time prediction
                    currentTime = m_Timer.GetClockTicks();
                    targetTime = start_time_fps_calculation + (1000000 / TARGET_FPS);
                    if (noTargetFPS == true && targetTime > (currentTime + last_swap_duration))
                        {
                        delayTime = (targetTime - (currentTime + last_swap_duration)) / 1000;
                        m_Timer.MsDelay(delayTime);
                        }
                    // Measure swap time and calculate FPS
                    unsigned long before_swap = m_Timer.GetClockTicks();
                    gfx_render_shader_b(&state);
                    last_swap_duration = m_Timer.GetClockTicks() - before_swap;
                    // Calculate actual FPS including everything
                    unsigned long total_frame_time = m_Timer.GetClockTicks() - start_time_fps_calculation;
                    current_fps = 1000000.0f / total_frame_time;
                }
                util_save_modes_file();    
                filesystem_save_log_file( "umsd1-1", FILENAME_MODES, g_modes);
                check();                                                                        // one last check?

                filesystem_save_log_file( "umsd1-1", FILENAME_GLERROR, g_error_string);             // anyway - store the logfile!

                return ShutdownReboot;
}


