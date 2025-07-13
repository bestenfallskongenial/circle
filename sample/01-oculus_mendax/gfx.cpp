#undef _OLG_DEBUG_

#include "kernel.h"
#include "global.h"
#include "bcm_host.h" 

void            CKernel::gfx_shader_log             (   GLint shader, int shaderIndex)
{
                GLint success;
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // Get compilation status
                
                m_shaderStatusFlags[shaderIndex] = (success == GL_TRUE); // Store status in our flags array
}

void            CKernel::gfx_program_log            (   GLint shader, int program_index) 
{
                int internal_index =0;
                if ( program_index > 0 ) 
                    {
                    internal_index = program_index - 1;
                    }

                GLint success;
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                
                CString log_message;
                log_message.Format( "----------------------------------------------------------------\n"
                                    "Program %d link status: %s\n\n", 
                                    program_index,
                                    (success == GL_TRUE ? "Success" : "Failed"));
                g_log_string.Append(log_message);

                char name[27];  // 40 chars + null terminator
                strncpy(name, &m_bufferFshader[program_index][2], 26);

                name[26] = '\0';
                log_message.Format( "%s Filename: %s Size %u\n"
                                    "----------------------------------------------------------------\n", 
                                    name, SCANED_FILES_FSH[internal_index],
                                    FSH_LOADED_BYTES[program_index]);
                g_log_string.Append(log_message);

                char log[1024];
                glGetProgramInfoLog(shader, sizeof(log), NULL, log);
                
                log_message.Format(" %s\n", log);    
                g_log_string.Append(log_message);  // Append the u_program_handle log to the global string
                
                GLint numUniforms;
                glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &numUniforms);    // Query and append active uniforms information
                
                for (GLint i = 0; i < numUniforms; ++i) 
                    {
                    char name[256];
                    GLsizei length;
                    GLint size;
                    GLenum type;
                    glGetActiveUniform(shader, i, sizeof(name), &length, &size, &type, name);
                    
                    GLint location = glGetUniformLocation(shader, name);
                    
                    log_message.Format("Uniform   #%-2d: name=%-10s, size=%-2d, type=0x%-4x, location=%-2d\n",
                                i, name, size, type, location);
                                
                    g_log_string.Append(log_message);  // Append uniform info to the global string
                    }
                g_log_string.Append("\n");
                
                GLint numAttributes;
                glGetProgramiv(shader, GL_ACTIVE_ATTRIBUTES, &numAttributes);   // Query and append active attributes information
                
                for (GLint i = 0; i < numAttributes; ++i) 
                    {
                    char name[256];
                    GLsizei length;
                    GLint size;
                    GLenum type;
                    glGetActiveAttrib(shader, i, sizeof(name), &length, &size, &type, name);
                    
                    GLint location = glGetAttribLocation(shader, name);
                    
                    log_message.Format("Attribute #%-2d: name=%-10s, size=%-2d, type=0x%-4x, location=%-2d\n\n",
                                i, name, size, type, location);
                    g_log_string.Append(log_message);  // Append attribute info to the global string
                    }
                //    g_log_string.Append("\n");
}

void            CKernel::gfx_check                  (   const char *file, unsigned line)
{
                static unsigned error_count = 0;
                static bool summary_written = false;  // New static flag
                const unsigned ERROR_THRESHOLD = 1024;
                
                if (resetFlag && !summary_written)
                    {
                    summary_written = true;  // Set flag before writing summary
                    
                    CTimer *pTimer = CTimer::Get();
                    unsigned ticks = pTimer->GetTicks();
                    CString log_message;
                    
                    log_message.Format("\n=== Final System Status [%5u.%02u] ===\n", ticks / HZ, ticks % HZ);
                    g_error_string.Append(log_message);
                    
                    log_message.Format("Total Errors Encountered: %u\n", error_count);
                    g_error_string.Append(log_message);
                    
                    GLint value;
                    glGetIntegerv(GL_CURRENT_PROGRAM, &value);
                    log_message.Format("Current Program: %d\n", value);
                    g_error_string.Append(log_message);
                    
                    glGetIntegerv(GL_ACTIVE_TEXTURE, &value);
                    log_message.Format("Active Texture Unit: GL_TEXTURE%d\n", value - GL_TEXTURE0);
                    g_error_string.Append(log_message);
                    
                    GLint viewport[4];
                    glGetIntegerv(GL_VIEWPORT, viewport);
                    log_message.Format("Viewport: x=%d, y=%d, width=%d, height=%d\n", 
                        viewport[0], viewport[1], viewport[2], viewport[3]);
                    g_error_string.Append(log_message);
                    
                    GLint fb;
                    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
                    log_message.Format("Current Framebuffer: %d\n", fb);
                    g_error_string.Append(log_message);
                    
                    log_message.Format("\n=== End Status Report ===\n\n");
                    g_error_string.Append(log_message);
                    
                    return;
                    }

                GLenum error = glGetError();
                if (error != GL_NO_ERROR)
                    {
                    CString log_message;
                    CTimer *pTimer = CTimer::Get();
                    unsigned ticks = pTimer->GetTicks();
                    
                    const char* error_str;
                    const char* severity;
                    
                    switch(error) 
                        {
                        case GL_INVALID_ENUM:               
                            error_str = "GL_INVALID_ENUM"; 
                            severity = "WARNING ";
                            break;
                        case GL_INVALID_VALUE:              
                            error_str = "GL_INVALID_VALUE"; 
                            severity = "WARNING ";
                            break;
                        case GL_INVALID_OPERATION:          
                            error_str = "GL_INVALID_OPERATION"; 
                            severity = "WARNING ";
                            break;
                        case GL_OUT_OF_MEMORY:              
                            error_str = "GL_OUT_OF_MEMORY"; 
                            severity = "CRITICAL";
                            resetFlag = true;
                            break;
                        case GL_INVALID_FRAMEBUFFER_OPERATION: 
                            error_str = "GL_INVALID_FRAMEBUFFER_OPERATION"; 
                            severity = "CRITICAL";
                            resetFlag = true;
                            break;
                        default:                            
                            error_str = "UNKNOWN_ERROR"; 
                            severity = "WARNING ";
                            break;
                        }
                    
                    log_message.Format("[%5u.%02u][%-8s] OpenGL Error: %-30s (0x%x) at %s:%u\n", 
                        ticks / HZ,
                        ticks % HZ,
                        severity,
                        error_str,
                        error,
                        file,
                        line);
                        
                    g_error_string.Append(log_message);
                    
                    error_count++;
                    if (error_count >= ERROR_THRESHOLD)
                        {
                        resetFlag = true;
                        }
                    }
}

void            CKernel::gfx_init_OGL               (   CUBE_STATE_T *state)
{
                int32_t success = 0;
                EGLBoolean result;
                EGLint num_config;

                static EGL_DISPMANX_WINDOW_T nativewindow;

                DISPMANX_UPDATE_HANDLE_T dispman_update;
                VC_RECT_T dst_rect;
                VC_RECT_T src_rect;

                static const EGLint attribute_list[] =
                    {
                    EGL_RED_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_BLUE_SIZE, 8,
                    EGL_ALPHA_SIZE, 8,
                    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                    EGL_NONE
                    };
   
                static const EGLint context_attributes[] = 
                    {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
                    };

                EGLConfig config;
                
                state->display = eglGetDisplay                      (   EGL_DEFAULT_DISPLAY     );  // get an EGL display connection
#ifdef _OLG_DEBUG_                
                assert(state->display!=EGL_NO_DISPLAY);
                check();
#endif // _OLG_DEBUG_
                result = eglInitialize                              (   state->display,         // initialize the EGL display connection
                                                                        NULL, 
                                                                        NULL                    );
#ifdef _OLG_DEBUG_
                assert(EGL_FALSE != result);//?
                check();
#endif // _OLG_DEBUG_
                result = eglChooseConfig                            (   state->display,         // get an appropriate EGL frame buffer configuration 
                                                                        attribute_list, 
                                                                        &config, 
                                                                        1, 
                                                                        &num_config             );
#ifdef _OLG_DEBUG_                
                assert(EGL_FALSE != result); //?
                check();
#endif // _OLG_DEBUG_
                result = eglBindAPI(EGL_OPENGL_ES_API);                                         // get an appropriate EGL frame buffer configuration
#ifdef _OLG_DEBUG_
                assert(EGL_FALSE != result);//?
                check();
#endif // _OLG_DEBUG_
                state->context = eglCreateContext                   (   state->display,         // create an EGL rendering context
                                                                        config, 
                                                                        EGL_NO_CONTEXT, 
                                                                        context_attributes      );
#ifdef _OLG_DEBUG_
                assert(state->context!=EGL_NO_CONTEXT);//?
                check();
#endif // _OLG_DEBUG_
                success = graphics_get_display_size                 (   0 /* LCD */,            // create an EGL window surface
                                                                        &state->screen_width, 
                                                                        &state->screen_height   );
#ifdef _OLG_DEBUG_
                assert( success >= 0 );
#endif // _OLG_DEBUG_
                dst_rect.x = 0;
                dst_rect.y = 0;
                dst_rect.width = state->screen_width;
                dst_rect.height = state->screen_height;
      
                src_rect.x = 0;
                src_rect.y = 0;
                src_rect.width = state->screen_width << 16;
                src_rect.height = state->screen_height << 16;        

                state->dispman_display = vc_dispmanx_display_open   (   0 /* LCD */ );
                dispman_update = vc_dispmanx_update_start( 0 );
      
                state->dispman_element = vc_dispmanx_element_add    (   dispman_update, 
                                                                        state->dispman_display,
                                                                        0/*layer*/, 
                                                                        &dst_rect, 
                                                                        0/*src*/,
                                                                        &src_rect, 
                                                                        DISPMANX_PROTECTION_NONE, /*protection*/
                                                                        0 /*alpha*/, 
                                                                        0 /*clamp*/, 
                                                                        DISPMANX_NO_ROTATE /*transform*/ );   // was 0/*transform*/ before DISPMANX_ROTATE_90 
      
                nativewindow.element = state->dispman_element;
                nativewindow.width = state->screen_width;
                nativewindow.height = state->screen_height;

                vc_dispmanx_update_submit_sync                      (   dispman_update  );
#ifdef _OLG_DEBUG_
                check();
#endif // _OLG_DEBUG_            
                state->surface = eglCreateWindowSurface             (   state->display, 
                                                                        config, 
                                                                        &nativewindow, 
                                                                        NULL            );
#ifdef _OLG_DEBUG_
                assert(state->surface != EGL_NO_SURFACE);//?
                check();
#endif // _OLG_DEBUG_                
                result = eglMakeCurrent                             (   state->display,     // connect the context to the surface
                                                                        state->surface, 
                                                                        state->surface, 
                                                                        state->context  );
#ifdef _OLG_DEBUG_
                assert(EGL_FALSE != result);//?
                check();
#endif // _OLG_DEBUG_
                glClearColor(0.15f, 0.25f, 0.35f, 1.0f);                                    // Set background color and clear buffers
                glClear( GL_COLOR_BUFFER_BIT );
#ifdef _OLG_DEBUG_
                check();
#endif // _OLG_DEBUG_            
}

void            CKernel::gfx_init_vshaders          (   CUBE_STATE_T *state, int fromFile, int toFile)    // Function to initialize Vertex Shaders
{
                for (int i = fromFile; i < toFile; i++) 
                    {
                    const char *SourcePrtVshader = m_bufferVshader[0]; // will be later maybe changed to multible instances of vshader

                    state->gl_vsh_id[i] = glCreateShader(GL_VERTEX_SHADER);
                    glShaderSource(state->gl_vsh_id[i], 1, &SourcePrtVshader, 0);  // will be later maybe changed to multible instances of vshader
                    glCompileShader(state->gl_vsh_id[i]);

                //  check();          
                    }
}

void            CKernel::gfx_init_fshaders          (   CUBE_STATE_T *state, int fromFile, int toFile)    // Function to initialize Fragment Shaders 
{
                for (int i = fromFile; i < toFile; i++) 
                    {
                    const char *SourcePrtFshader = m_bufferFshader[i];

                    state->gl_fsh_id[i] = glCreateShader(GL_FRAGMENT_SHADER);
                    glShaderSource(state->gl_fsh_id[i], 1, &SourcePrtFshader, 0);
                    glCompileShader(state->gl_fsh_id[i]);
                    gfx_shader_log(state->gl_fsh_id[i], i);
                //  check();
                    }
}

void            CKernel::gfx_init_programs          (   CUBE_STATE_T *state, int fromFile, int toFile)
{
                CString debug;
                
                for (int i = fromFile; i < toFile; i++) 
                {
                    if (m_shaderStatusFlags[i]) 
                        {
                        state->gl_prg_id[i] = glCreateProgram();
                        glAttachShader(state->gl_prg_id[i], state->gl_vsh_id[0]);
                        glAttachShader(state->gl_prg_id[i], state->gl_fsh_id[i]);
                        glLinkProgram(state->gl_prg_id[i]);

                        GLint linkStatus;
                        glGetProgramiv(state->gl_prg_id[i], GL_LINK_STATUS, &linkStatus);

                        gfx_program_log(state->gl_prg_id[i],i);
                    //  check();

                        if (linkStatus == GL_FALSE) 
                            {
                            glDeleteProgram(state->gl_prg_id[i]);
                            state->gl_prg_id[i] = 0;
                            m_shaderStatusFlags[i] = false;
                            }
                        else
                            {
                            linked_programs++;  // Count valid programs for util_choose_program()       ????????????
                            }
                        }
                m_Watchdog.Start(TIMEOUT*3); // new watchdog        
                }
}

void            CKernel::gfx_init_uniforms          (   CUBE_STATE_T *state, int fromFile, int toFile)    // Function to initialize Uniforms 
{
                CString debug;
                //  check();
                for (int i = fromFile; i < toFile; i++) 
                    {
                //  check();
                    if (m_shaderStatusFlags[i])
                        {
                        glUseProgram(state->gl_prg_id[i]); // NEW FOR DEBUG!!

                    //  check();
                        state->gl_vtx           = glGetAttribLocation( state->gl_prg_id[i], "vertex" );

                        state->u_time[i]        = glGetUniformLocation(state->gl_prg_id[i], "time" );
                        state->u_tres[i]        = glGetUniformLocation(state->gl_prg_id[i], "tres" );
                        state->u_seed[i]        = glGetUniformLocation(state->gl_prg_id[i], "seed" );
                        state->u_aud[i]         = glGetUniformLocation(state->gl_prg_id[i], "audio" );
                        state->u_col[i]         = glGetUniformLocation(state->gl_prg_id[i], "color" );
                        state->u_par_a[i]       = glGetUniformLocation(state->gl_prg_id[i], "par_a" );
                        state->u_par_b[i]       = glGetUniformLocation(state->gl_prg_id[i], "par_b" );

                        state->u_tex_l[i]       = glGetUniformLocation(state->gl_prg_id[i], "tex_l" );


                        state->u_tex_id[i][0]   = glGetUniformLocation(state->gl_prg_id[i], "tex[0]" );
                        state->u_tex_id[i][1]   = glGetUniformLocation(state->gl_prg_id[i], "tex[1]" );
                        state->u_tex_id[i][2]   = glGetUniformLocation(state->gl_prg_id[i], "tex[2]" );
                        state->u_tex_id[i][3]   = glGetUniformLocation(state->gl_prg_id[i], "tex[3]" );
                        state->u_tex_id[i][4]   = glGetUniformLocation(state->gl_prg_id[i], "tex[4]" );
                        state->u_tex_id[i][5]   = glGetUniformLocation(state->gl_prg_id[i], "tex[5]" );
                        state->u_tex_id[i][6]   = glGetUniformLocation(state->gl_prg_id[i], "tex[6]" );
                        state->u_tex_id[i][7]   = glGetUniformLocation(state->gl_prg_id[i], "tex[7]" );
                        state->u_tex_id[i][8]   = glGetUniformLocation(state->gl_prg_id[i], "tex[8]" );
                        state->u_tex_id[i][9]   = glGetUniformLocation(state->gl_prg_id[i], "tex[9]" );
                    //  check();
                        }
                    }
}

void            CKernel::gfx_init_textures          (   CUBE_STATE_T *state, int fromFile, int toFile)
{
                CString log_message;
                parser_teture_bmp(TEX_LOADED_OLD, TEX_LOADED_NEW);
                
                int validTextureCount = 0;  // Counter for valid textures only

                for (int i = fromFile; i < toFile; i++)
                    {
                    if(TEX_FILE_STATUS[i] == true)
                        {
                        glGenTextures(1, &state->gl_tex_id[validTextureCount]);  // Use counter instead of i
                        glBindTexture(GL_TEXTURE_2D, state->gl_tex_id[validTextureCount]);
                    //  check();
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    //  check();
                        GLvoid* bitmapData = &m_bufferTexture[i][TEX_FILE_BM_OFFSET[i]];

                        glTexImage2D(GL_TEXTURE_2D, 
                                   0, 
                                   GL_RGB, 
                                   TEX_FILE_X_DIM[i],
                                   TEX_FILE_Y_DIM[i], 
                                   0, 
                                   GL_RGB, 
                                   GL_UNSIGNED_BYTE, 
                                   bitmapData);
                    //  check();
                        glBindTexture(GL_TEXTURE_2D, 0);
                    //  check();
                        validTextureCount++;  // Increment only after successful texture creation
                        }
                    m_Watchdog.Start(TIMEOUT);       // new watchdog
                    }

// !!! i need to move this out of the function again. its a hotfix, intent is to compose the bitmap section of the log file !!!

                log_message.Format( "----------------------------------------------------------------\n"
                                    "Tex# Status   Filesize Offset Dimension BMP-Size    Filename\n");
                g_log_string.Append(log_message);
                for( int i = 0;i < TEX_LOADED_NEW-1; ++i )
                    {
                    log_message.Format("%-2d   %-6s   %-8d 0x%-4x %-4dx%-4d %-8d    %s\n",
                            i,
                            (TEX_FILE_STATUS[i] ? "Valid" : "Failed"),
                            TEX_FILE_SIZE[i], 
                            TEX_FILE_BM_OFFSET[i],
                            TEX_FILE_X_DIM[i],
                            TEX_FILE_Y_DIM[i],
                            TEX_FILE_BM_SIZE[i],
                            SCANED_FILES_TEX[i]);
                            
                            g_log_string.Append(log_message);
                    }
// !!! here the function continues 

                // Store the actual number of valid textures for later use
                TEX_LOADED_NEW = validTextureCount;
}

void            CKernel::gfx_init_v_buffer          (   CUBE_STATE_T *state)                              // Function to initialize Buffers 
{
                static const GLfloat vertex_data[] = 
                    {  -1.0,-1.0, 1.0, 1.0,
                        1.0,-1.0, 1.0, 1.0,
                        1.0, 1.0, 1.0, 1.0,
                       -1.0, 1.0, 1.0, 1.0  };

                glClearColor(0.0, 1.0, 1.0, 1.0);

                glGenBuffers(1, &state->gl_buf);
            //  check();

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            //  check();
                
                glViewport(0, 0, state->screen_width, state->screen_height);                        // Prepare viewport
            //  check();
                
                glBindBuffer(GL_ARRAY_BUFFER, state->gl_buf);                                          // Upload vertex data to a buffer
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
                glVertexAttribPointer(state->gl_vtx, 4, GL_FLOAT, 0, 16, 0);
                glEnableVertexAttribArray(state->gl_vtx);
            //  check();
}
void            CKernel::gfx_render_shader_a        (   CUBE_STATE_T *state)
{
                glBindFramebuffer(GL_FRAMEBUFFER,0);    // Now render to the main frame buffer
        
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);   // Clear the background (not really necessary I suppose)
                //  check();
                glBindBuffer(GL_ARRAY_BUFFER, state->gl_buf);
                //  check();

                glUseProgram ( state->gl_prg_id[gl_current_prg] );

                // check();
    
                GLuint cx = state->screen_width;
                GLuint cy = state->screen_height;

                if(state->u_time[gl_current_prg] != -1) glUniform1f(    state->u_time[gl_current_prg], GLtime);
                if(state->u_tres[gl_current_prg]!= -1 ) glUniform2f(    state->u_tres[gl_current_prg], cx, cy);
                if(state->u_seed[gl_current_prg] != -1) glUniform4f(    state->u_seed[gl_current_prg], 
                                                                        random_float_value[0], 
                                                                        random_float_value[1], 
                                                                        random_float_value[2], 
                                                                        random_float_value[3]);
                if(state->u_aud[gl_current_prg]!= -1 ) glUniform4f(     state->u_aud[gl_current_prg], 
                                                                        util_audio_smooth_band[0], 
                                                                        util_audio_smooth_band[1], 
                                                                        util_audio_smooth_band[2], 
                                                                        util_audio_smooth_band[3]);
                if(state->u_col[gl_current_prg] != -1) glUniform4f(     state->u_col[gl_current_prg], 0.0f, 0.0f, 0.0f, opaque);
                if(state->u_par_a[gl_current_prg] != -1 ) glUniform4f(  state->u_par_a[gl_current_prg], 
                                                                        output_float_value[0], 
                                                                        output_float_value[1], 
                                                                        output_float_value[2], 
                                                                        output_float_value[3]);
                if(state->u_par_b[gl_current_prg] != -1 ) glUniform4f(  state->u_par_b[gl_current_prg], 
                                                                        output_float_value[4], 
                                                                        output_float_value[5], 
                                                                        output_float_value[6], 
                                                                        output_float_value[7]);
                if(state->u_tex_l[gl_current_prg] != -1) glUniform1i(   state->u_tex_l[gl_current_prg], TEX_LOADED_NEW);

                switch(mode_storage_buffers[TEX_MODE][current_buffer]) 
                    {
                    case false:     // Original mode
                        for (int i = 0; i < TEX_LOADED_NEW; i++) 
                            {
                            glActiveTexture(GL_TEXTURE0+i);
                            glBindTexture(GL_TEXTURE_2D, state->gl_tex_id[i]);
                            if (state->u_tex_id[gl_current_prg][i] != -1)
                                {
                                glUniform1i(state->u_tex_id[gl_current_prg][i], i);
                                }
                        //  check();
                            }
                        break;

                    case true:      // Single texture mode
                        switch(TEX_LOADED_NEW) 
                            {
                            case 0:     // No textures - skip entirely
                                break;
                                
                            case 1:     // Single texture - only bind one
                                glActiveTexture(GL_TEXTURE0);
                                glBindTexture(GL_TEXTURE_2D, state->gl_tex_id[gl_current_tex]);
                                if (state->u_tex_id[gl_current_prg][0] != -1)
                                    glUniform1i(state->u_tex_id[gl_current_prg][0], 0);
                            //  check();
                                break;
                                
                            default:    // Two or more textures - bind current and next
                                glActiveTexture(GL_TEXTURE0);
                                glBindTexture(GL_TEXTURE_2D, state->gl_tex_id[gl_current_tex]);
                                if (state->u_tex_id[gl_current_prg][0] != -1)
                                    glUniform1i(state->u_tex_id[gl_current_prg][0], 0);
                            //  check();
                                
                                glActiveTexture(GL_TEXTURE1);
                                glBindTexture(GL_TEXTURE_2D, state->gl_tex_id[gl_current_tex + 1]);
                                if (state->u_tex_id[gl_current_prg][1] != -1)
                                    glUniform1i(state->u_tex_id[gl_current_prg][1], 1);
                            //  check();
                                break;
                            }
                        break;
                    }

                glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
            //  check();

                glBindBuffer( GL_ARRAY_BUFFER, 0 );

                glFlush();

                if (noTargetFPS==true)
                    {
                    glFinish();
                //  check();
                    }
}

void            CKernel::gfx_render_shader_b        (   CUBE_STATE_T* state)
{
                eglSwapBuffers(state->display, state->surface);
            //  check();
}