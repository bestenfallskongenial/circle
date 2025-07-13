// display.cpp

bool            CKernel::display_startup_screen     (CUBE_STATE_T *state)
void            CKernel::display_append_modes       () 

// filesystem.cpp

bool            CKernel::filesystem_open_file       (   const char *pTitle)		
unsigned        CKernel::filesystem_load_file       (   char *buffer, 
                                                        unsigned bufferSize, 
                                                        bool callback)
bool            CKernel::filesystem_close_file      ()
int             CKernel::filesystem_process_files   (   char* fileNameArray[], 
                                                        unsigned totalLoadedBytes[], 
                                                        char** bufferArray, 
                                                        int maxFiles, 
                                                        int successfulLoaded, 
                                                        unsigned fileSize, 
                                                        bool callback)
bool            CKernel::filesystem_mount           (	const char* deviceName, 
                                                        char* vshaderFileNames[], 
                                                        unsigned vStotalLoadedBytes[], 
                                                        int maxVshaderFiles,
                                                        char* fshaderFileNames[], 
                                                        unsigned fStotalLoadedBytes[], 
                                                        int maxFshaderFiles,
                                                        char* textureFileNames[], 
                                                        unsigned tXtotalLoadedBytes[], 
                                                        int maxTextureFiles,
                                                        char* videoFileNames[]  , 
                                                        unsigned vItotalLoadedBytes[], 
                                                        int maxVideoFiles,     
                                                        bool callback)          
bool            CKernel::filesystem_save_log_file   (   const char* deviceName, 
                                                        const char* filename, 
                                                        const CString& str_to_save)
bool            CKernel::filesystem_IsValidFileType (   const char* pFileName, const char* extension)
unsigned        CKernel::filesystem_ScanRootDir     (   char** fileArray, 
                                                        const char* extension, 
                                                        unsigned maxFiles)
bool            CKernel::filesystem_update_USB      (   const char* deviceType)
void            CKernel::filesystem_remove_USB      (   CDevice *pDevice, void *pContext)
bool            CKernel::filesystem_load_kernel     (   const char* deviceName, 
                                                        const char* filename, 
                                                        unsigned bufferIndex)
bool            CKernel::filesystem_save_kernel     (   const char* deviceName, 
                                                        const char* filename, 
                                                        unsigned bufferIndex)

// gfx.cpp

void            CKernel::gfx_shader_log             (   GLint shader, int shaderIndex)
void            CKernel::gfx_program_log            (   GLint shader, int program_index)
void            CKernel::gfx_check                  (   const char *file, unsigned line)
void            CKernel::gfx_init_OGL               (   CUBE_STATE_T *state)
void            CKernel::gfx_init_vshaders          (   CUBE_STATE_T *state, int fromFile, int toFile)
void            CKernel::gfx_init_fshaders          (   CUBE_STATE_T *state, int fromFile, int toFile)
void            CKernel::gfx_init_programs          (   CUBE_STATE_T *state, int fromFile, int toFile)
void            CKernel::gfx_init_uniforms          (   CUBE_STATE_T *state, int fromFile, int toFile)
void            CKernel::gfx_init_textures          (   CUBE_STATE_T *state, int fromFile, int toFile)
void            CKernel::gfx_init_v_buffer          (   CUBE_STATE_T *state)
void            CKernel::gfx_render_shader_a        (   CUBE_STATE_T *state)
void            CKernel::gfx_render_shader_b        (   CUBE_STATE_T* state)

// io.cpp

void            CKernel::io_init_pickup_buffer      ()
void            CKernel::io_read_ADC                ()
void            CKernel::io_event_button_A          (   BUTTONS::TEvent Event, void *pParam)
void            CKernel::io_event_button_B          (   BUTTONS::TEvent Event, void *pParam)

// LED.cpp

void            CKernel::LED_circle_color        ()
void            CKernel::LED_circle_while_load ()
void            CKernel::LED_update              ()

// memory.cpp

bool            CKernel::memory_allocate            ()                                                               
void            CKernel::memory_clean_up            ()
char**          CKernel::memory_init_buffer         (   size_t count, size_t bufferSize)                            // do i really need you to?                   
char**          CKernel::memory_init_dma_buffer     (   size_t count, 
                                                        size_t bufferSize,
                                                        char** blockBaseOut,
                                                        char** rawBlockOut,
                                                        size_t* alignedSizeOut)                                     // do i really need you to alline this extreme?
void            CKernel::memory_clear_buffer        (   char** buffers, size_t count)
void            CKernel::memory_clear_dma_buffer    (   char** buffers, char* rawBlock)

// menu.cpp

void            CKernel::menu_general               ()                                                              // i need to tame this monster!
void            CKernel::menu_mode_default_A        ()                                                              // to big, to complex
void            CKernel::menu_mode_default_B        ()                                                              // enums / structs??
void            CKernel::menu_mode_assign_A         ()
void            CKernel::menu_mode_assign_B         ()
void            CKernel::menu_mode_assign_LFO       ()

// parser.cpp

void            CKernel::parser_teture_bmp          (   int fromFile, int toFile)
void            CKernel::parser_debug               ()                                                              // wrapper for parser and string generator and store to file

// util.cpp

void            CKernel::util_prep_parameters       ()                                                              // helper for startup
int             CKernel::util_choose_program        ()
void            CKernel::util_store_program         ()
int             CKernel::util_choose_texture        ()
void            CKernel::util_random_vec8           (uint32_t seed)
void            CKernel::util_calculate_BPM         (unsigned long buttonTime, unsigned long clockTime)
void            CKernel::util_determine_bpm_source  ()
void            CKernel::util_update_predicted_beat ()
void            CKernel::util_LFO                   ()
void            CKernel::util_audio_energy          (float adcvalue)
bool            CKernel::util_check_for_update      ()
void            CKernel::util_save_modes_file       ()
void            CKernel::GenerateH264ParserInfo     (   int     video_index,                                        // stores to string debug
                                                        char*   buffer_video[],
                                                        size_t  buffer_size[],    
                                                        u16     video_width[],
                                                        u16     video_height[],
                                                        u8      video_profile[],
                                                        void*   frame_addresses[][MAX_FRAMES],
                                                        size_t  length_of_frames[][MAX_FRAMES],
                                                        int     number_of_frames[],
                                                        bool    is_video_valid[])
void            CKernel::debug_parser               ()                                                              // unused!