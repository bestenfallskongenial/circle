#ifndef _kernel_h
#define _kernel_h

#include <circle/actled.h>                                  // onboard LED
#include <circle/koptions.h>                                // rasppi hardware options
#include <circle/machineinfo.h>
#include <circle/devicenameservice.h>						
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>

#include <circle/memory.h>										// new for dma allocated buffers!
#include <circle/new.h>												// new for dma allocated buffers!

#include "/home/bestenfalls/circle/addon/SDCard/emmc.h"

#include <circle/usb/usbhcidevice.h>
#include <circle/fs/fat/fatfs.h>

/*
#include <vc4/interface/khronos/include/EGL/egl.h>
#include <vc4/interface/khronos/include/EGL/eglext.h>
#include <vc4/interface/khronos/include/GLES2/gl2.h>
*/
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/EGL/egl.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES/gl.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES2/gl2.h"

#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/EGL/eglext.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES/glext.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES2/gl2ext.h"
/*
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES2/gl2.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/EGL/egl.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/EGL/eglext.h"
*/
#include <circle/sched/scheduler.h>                         // multitasking ?!
#include <vc4/vchiq/vchiqdevice.h>
#include "vc4/vchiq/vchiq_arm.h"
// my video player
#include <interface/vcos/vcos.h>  // For VCHI_INSTANCE_T
#include <vc4/vchi/vchi.h>       // For VCHI_CONNECTION_T
/*
// Add the external function declaration

*/
#include <vc4/vc_sm/vc_sm_class.h>							// VideoCore Shared Memory							
#include <vc4/h264_decoder/vc_h264_decoder.h>					// VideoCore H.264 Decoder for fiexd Resolution Annex B Raw H.264 Videos
#include <vc4/h264_parser/vc_h264_parser.h>					// Parser for the H.264 Files

#include <circle/types.h>                                   // 

#include <circle/bcmwatchdog.h> // new watchdog

#include <circle/spimaster.h>

#include <sensor/mcp300x.h>									// adc

#include <WS28XX/ws2812oversmi.h> 

#include <circle/gpiomanager.h>
#include <circle/gpiopin.h>
#include <sensor/buttons.h>

// #define 		DEBUGMODE              false    // enables verbose 

#define 		VIDEO_WIDTH				640
#define			VIDEO_HEIGHT			480
#define 		MAX_FRAMES 				2048
#define 		BASELINE_PROFILE		66		// Baseline

#define			TIMEOUT					10
#define 		LOGLEVEL				4	// 0: only panic 1: also errors 2: also warnings 3: also notices 4: also debug output (default))
#define         ADC_CHANNELS         	8   // number of adc input channels
#define 		ADC_BUFFER			 	4	// number of buffer cells for smoothing
#define 		NUMBER_OF_MODES			6	// adc, trigger, bpm, LFO 1, LFO 2, audio ( for now )
#define 		ADC_SELECT_PRG			7	// adc channel to choose the glsl-u_program_handle
#define			ADC_SELECT_TEX			6	// adc channel to choose the texture
#define			ADC_INPUT_CLK			5	// adc channel use as clock ! BIGGER THAN 3 !

#define 		DEFAULT_SLOT			32

#define 		LFO_PARAMETERS			4
#define 		LFO_INSTANCES			2

#define 		R     					0
#define 		G   					1  
#define 		B    					2

#define			M_1						1
#define			M_2						2
#define			M_L						3
#define			A25						4
#define			A33						5
#define			A50  					6
#define			TRG						7
#define			BPM						8
#define			LF1						9
#define			LF2						10
#define			CLK						11
#define			A_0						12
#define			A_1						13
#define			A_2						14
#define			A_3						15

#define CH0_MODE 0
#define CH1_MODE 1
#define CH2_MODE 2
#define CH3_MODE 3
#define CH4_MODE 4
#define CH5_MODE 5
#define CH6_MODE 6
#define CH7_MODE 7

#define LF1_WAVE 8
#define LF2_WAVE 9
#define LF1_MULT 10
#define LF2_MULT 11
#define FRM_MODE 12
#define TEX_MODE 13 
#define CLK_MODE 14
#define DMY_MODE 15

#define AUD_MODE 5

#define 		TOLERANCE 				8

#define         VSH_SD             		1	// max number of u_vertex shader on sd
#define         FSH_SD             		2	// max number of fragment shader on sd
#define         TEX_SD             		2	// max number of textures on sd
#define         VID_SD             		2	// max number of videos on sd

#define         VSH_USB            		0	// max number of u_vertex shader on usb
#define         FSH_USB           		32	// max number of fragment shader on usb
#define         TEX_USB            		8	// max number of textures on usb
#define         VID_USB            		8	// max number of videos on usb

#define			KERNEL_SIZE				(1024*1024*2) 	// 2mb must be sufficient here ( i confused the kernel size by the factor 10 )
#define         VSH_SIZE                (1024*32) 		// 32768 size of u_vertex shader files
#define         FSH_SIZE                (1024*32) 		// 32768 size of fragment shader files
#define         TEX_SIZE                (1024*1024*4)  	// 4194304	// size of texture ( .bpm ) files
#define         VID_SIZE                (1024*1024*8) 	// 16777216// size of video ( .264 ) files
#define 		FRAME_SIZE 				(1024*1024) 	// 1MB, which comfortably fits a 640x480x3 frame

#define         TARGET_FPS              24      // desired frame rate !!! becomes menu !!!
#define 		MIN_BPM					10		// min bpm for adc selector 
#define			MAX_BPM					240 	// max bpm for adc selector

#define         WAVEFORMS             	4		// number of lfo waves
#define         WAVESAMPLES            	256  	// number of samples per lfo waves
#define         PARTITIONSD	            "emmc1-1"	// partition sd
#define         PARTITIONUSB	        "umsd1-1"	// partition usb

// LOGFILE NAMES 

#define 		FILENAME_GL_LOG			"LOG-GLES.txt"
#define 		FILENAME_GLERROR		"LOG-GLER.txt"
#define 		FILENAME_VCSM_LOG		"LOG-VCSM.txt"
#define 		FILENAME_MMAL_LOG		"LOG-MMAL.txt"

#define			FILENAME_MODES			"MODES.txt"
#define 		FILENAME_PARSER0		"FRMT-0.txt"
#define 		FILENAME_PARSER1		"FRMT-1.txt"
#define 		FILENAME_PARSER2		"FRMT-2.txt"
#define 		FILENAME_PARSER3		"FRMT-3.txt"
#define 		FILENAME_KERNEL			"kernel.img"
#define 		CHUNK_SIZE				1024

#define 		CLEAR_SCREEN			"\x1b[H\x1b[J"
#define 		CURSOR_OFF				"\x1b[?25l"

#define         LED_PIN		           	19		// pin led ( 8 .. 23 )
#define         CS_PIN                 	12      // pin for menu input selector
#define         SW_PIN_A	           	13      // button A ( lower )
#define         SW_PIN_B	          	6       // button B ( upper )
#define         LED_COUNT		       	4		// number of LEDs on the stripes
#define         GPIO_TO_SD_LINE(pin)	((pin) - 8)							// dont touch!
#define         SD_LINE1		      	GPIO_TO_SD_LINE (LED_PIN)			// dont touch!
#define         SD_LINE_TO_MASK(line)	(1 << (line))						// dont touch!
#define         SD_LINES_MASK		 	(  SD_LINE_TO_MASK (SD_LINE1) )		// dont touch!

#define         CHANNEL			    	7		                    // 7 ( for mcp3008 )
#define         VREF			 		5.0f	                    // Reference voltage 5 Volt
#define         SPI_MASTER_DEVICE	 	0		                    // 0
#define         SPI_CHIP_SELECT		 	0		                    // 0
#define         SPI_CLOCK_SPEED	   		1000000		                // Hz

//#define       check() 				assert(glGetError() == 0)  						// macro for GL ES2.0 error check
#define         check() 				gfx_check(__FILE__, __LINE__) 	// my own assertiion implimentation

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);

	TShutdownMode Run (void);

struct CUBE_STATE_T
	{
	// EGL Window
   	uint32_t screen_width;
   	uint32_t screen_height;
 
   	DISPMANX_ELEMENT_HANDLE_T dispman_element;
   	DISPMANX_DISPLAY_HANDLE_T dispman_display;

   	EGLDisplay display;
   	EGLSurface surface;
   	EGLContext context;
	// GL Handles
   	GLuint gl_vsh_id[VSH_SD+VSH_USB];
   	GLuint gl_fsh_id[FSH_SD+FSH_USB];

   	GLuint gl_prg_id[FSH_SD+FSH_USB];

	GLuint gl_tex_id[TEX_SD+TEX_USB];
	GLint  u_tex_id[FSH_SD+FSH_USB][TEX_SD+TEX_USB];

   	GLuint gl_buf; // buffer
	GLuint gl_vtx; // vertex

	GLint u_time[FSH_SD+FSH_USB];
	GLint u_tres[FSH_SD+FSH_USB];
	GLint u_seed[FSH_SD+FSH_USB];	
	GLint u_aud[FSH_SD+FSH_USB];
	GLint u_col[FSH_SD+FSH_USB];

	GLint u_par_a[FSH_SD+FSH_USB];
	GLint u_par_b[FSH_SD+FSH_USB];
    GLint u_tex_l[FSH_SD+FSH_USB];
	};

	bool 			memory_allocate				();											// 
    void 			memory_clean_up				();
	char** 			memory_init_buffer			(	size_t count, size_t bufferSize);
 // char**			memory_init_dma_buffer		(	size_t count, size_t bufferSize);
	char** 			memory_init_dma_buffer		(	size_t count, size_t bufferSize,
                                       				char** blockBaseOut,
                                       				char** rawBlockOut,
                                       				size_t* alignedSizeOut);

	void 			memory_clear_buffer			(	char** buffers, size_t count);
 // void			memory_clear_dma_buffer		(	char** buffers, size_t count);
	void			memory_clear_dma_buffer		(	char** buffers, char* rawBlock);

	bool 			filesystem_open_file		(	const char *pTitle	);
	unsigned 		filesystem_load_file		(	char *buffer, unsigned bufferSize, int mode);		
	bool 			filesystem_close_file		();
	int 			filesystem_process_files	(  	char* fileNameArray[], 
													unsigned totalLoadedBytes[], 
													char** bufferArray, 
                            						int maxFiles, int successfulLoaded, 
													unsigned TEX_FILE_SIZE, int mode);
	bool 			filesystem_save_log_file	(	const char* deviceName, const char* filename, const CString& str_to_save);
	bool 			filesystem_update_USB		(	const char* deviceType);
	bool 			filesystem_mount			(	const char* deviceName, 
                              						char* vshaderFileNames[], 
													unsigned vStotalLoadedBytes[], int maxVshaderFiles,
                              						char* fshaderFileNames[], 
													unsigned fStotalLoadedBytes[], int maxFshaderFiles,
                              						char* textureFileNames[], 
													unsigned tXtotalLoadedBytes[], int maxTextureFiles,
                                            		char* videoFileNames[]  , 
													unsigned vItotalLoadedBytes[], int maxVideoFiles);
	bool			filesystem_IsValidFileType	(const char* pFileName, const char* extension);
	unsigned		filesystem_ScanRootDir		(char** fileArray, const char* extension, unsigned maxFiles);
	bool			filesystem_load_kernel		(const char* deviceName, const char* filename, unsigned bufferIndex);
	bool			filesystem_save_kernel		(const char* deviceName, const char* filename, unsigned bufferIndex);
private:
	static void 	filesystem_remove_USB 		(	CDevice *pDevice, void *pContext);	
public:
//  void 			io_init_pickup_buffer		();
	void 			io_read_ADC					();
//  void 			io_set_LED_RGB				( 	int LEDnumber, int red, int green, int blue);
	void			LED_circle_color			();
//  void			LED_circle_while_load		();
//  void 			io_set_color				(int adc_channel);
	void 			LED_update				();
private:
	static void 	io_event_button_A 			(	BUTTONS::TEvent Event, void *pParam);			// to simplify?
	static void 	io_event_button_B 			(	BUTTONS::TEvent Event, void *pParam);			// to simplify?
public:
	void			util_prep_parameters		();
	int 			util_choose_program			();
	int 			util_choose_texture			();
	void			util_store_program			(); 
	void			util_random_vec8			(uint32_t seed); 
	void 			util_calculate_BPM			(	unsigned long buttonTime, unsigned long clockTime);
	void 			util_determine_bpm_source	();
	void			util_update_predicted_beat	();
  	void 			util_LFO					();
	void 			util_audio_energy			(	float adcvalue	);		

	float			util_get_adc_bpm			(	int adcValue	);
	void			util_calculate_filesize		();
	bool 			util_check_for_update		();
                 
	void 			util_save_modes_file		();

	void   			gfx_check					(	const char *file, unsigned line);
	void			gfx_shader_log				(	GLint shader, int shaderIndex);
	void 			gfx_program_log				(	GLint shader, int program_index) ;				// is to change / remove
	void 			gfx_init_OGL				(	CUBE_STATE_T *state);  		// ****
	void 			gfx_init_vshaders			(	CUBE_STATE_T *state, int fromFile, int toFile);	// ****
	void 			gfx_init_fshaders			(	CUBE_STATE_T *state, int fromFile, int toFile);	// ****
	void 			gfx_init_programs			(	CUBE_STATE_T *state, int fromFile, int toFile);	// ****
	void 			gfx_init_uniforms			(	CUBE_STATE_T *state, int fromFile, int toFile);	// ****
	void 			gfx_init_textures			(	CUBE_STATE_T *state, int fromFile, int toFile);	// possible change 
	void 			gfx_init_v_buffer			(	CUBE_STATE_T *state);							// possible change
	void 			gfx_render_shader_a			(	CUBE_STATE_T *state);							// possible change
	void 			gfx_render_shader_b			(	CUBE_STATE_T *state);	

	bool			display_startup_screen		(	CUBE_STATE_T *state);
	void			display_LoadScreenTexVidShd ( int mode );
	void 			display_debug				( 	CUBE_STATE_T *state, int level );					// display_debug menu 
	void 			display_append_modes		(); // display_debug output
	void 			display_print_log_file		();
    void			display_print_File_names 	(const char* prefix, char** array_to_print, int max_files);

	void            menu_general				();
    void            menu_mode_default_A			();
    void            menu_mode_default_B			();
    void            menu_mode_assign_A			();
    void            menu_mode_assign_B			();
	void			menu_mode_assign_LFO		();
//  void			menu_mode_assign_audio		();	
//  void			debug						();

	void 			parser_h264					(int fromFile, int toFile);
	void			parser_bmp					(int fromFile, int toFile);
	void 			GenerateH264ParserInfo		(	int     file_index );
	void 			GenerateBmpParserInfo		(	int     file_index );



private:
	CActLED			    m_ActLED;
	CKernelOptions		m_Options;
	CMachineInfo		m_MachineInfo;
	CDeviceNameService	m_DeviceNameService;
	CScreenDevice		m_Screen;
	CSerialDevice		m_Serial;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer			    m_Timer;
	CLogger			    m_Logger;
	CEMMCDevice			m_EMMC;
	CUSBHCIDevice		m_USBHCI;
	CVCHIQDevice		m_VCHIQ;

	CMemorySystem		m_Memory;
// my Video Player
    CVCSharedMemory 	m_SharedMemory;
    CH264Decoder 		m_H264Decoder;
    CH264Parser 		m_H264Parser;

	volatile boolean	m_bStorageAttached;
	CFATFileSystem		*m_pFileSystem;

  	CScheduler		    m_Scheduler;

	CBcmWatchdog       	m_Watchdog; // Watchdog instance

	CSPIMaster		    m_SPIMaster;
	CMCP300X		    m_MCP300X;

	CWS2812OverSMI		m_NeoPixels;

	CGPIOManager		m_GPIOManager;		// not needed in polling mode

	CGPIOPin 			m_ChipSelectPin;  	// Add this line for the chip select pin

	BUTTONS				m_Button_A;
	BUTTONS				m_Button_B;

	CUBE_STATE_T  		state;

	char** 				m_bufferVideo;
	char* 				m_videoBlockBase;
	char* 				m_videoRawBlock;
    size_t 				m_videoBlockSize;

	char**				m_bufferFrameBufferA;
	char* 				m_frameBlockBaseA;
	char* 				m_frameRawBlockA;
    size_t 				m_frameBlockSizeA;

	char**				m_bufferFrameBufferB;
	char* 				m_frameBlockBaseB;
	char* 				m_frameRawBlockB;
    size_t 				m_frameBlockSizeB;	

    char** 				m_bufferTexture;
	char* 				m_textureBlockBase;
	char* 				m_textureRawBlock;
    size_t 				m_textureBlockSize;

	char**				m_bufferKernel;
	char** 				m_bufferVshader;
    char** 				m_bufferFshader;


    // VideoCore handles
    u32  m_videoBufferHandles[VID_SD + VID_USB];    // Handles for video buffers
    u32  m_frameBufferHandlesA;                   // Handles for frame buffers
	u32  m_frameBufferHandlesB;  

	u32 m_VCSMinputHandle;
	u32 m_VCSMoutputHandleA;
	u32 m_VCSMoutputHandleB;

	u32 m_VCSMinputPtr;
	u32 m_VCSMoutputPtrA;
	u32 m_VCSMoutputPtrB;

	bool m_USBhasLoadOnes;
	bool m_resetFlag;
};

#endif