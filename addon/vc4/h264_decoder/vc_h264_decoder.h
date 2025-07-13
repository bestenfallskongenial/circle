//----------------------------------------------------------------------------------------------------------------------------------------------------
//              h264_decoder_class.h 
//----------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef _H264_DECODER_CLASS_H
#define _H264_DECODER_CLASS_H


#include <circle/types.h> 

#include "h264_decoder_defs.h"

#include "interface/vcos/vcos.h"
// #include <vc4/interface/vcinclude/common.h>
#include <vc4/vchi/vchi.h>  
#include <vc4/interface/khronos/include/EGL/egl.h>
#include <vc4/interface/khronos/include/GLES/gl.h>
#include <vc4/interface/khronos/include/GLES2/gl2.h>
#include <vc4/interface/khronos/include/EGL/eglext.h>
#include <vc4/interface/khronos/include/GLES/glext.h>
#include <vc4/interface/khronos/include/GLES2/gl2ext.h>

class CH264Decoder
{
public:
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CONSTRUCTOR / DECONSTRUCTOR
//----------------------------------------------------------------------------------------------------------------------------------------------------
                CH264Decoder();
               ~CH264Decoder();
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              USER API
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool            MMALinitialize                              (   u32 InBufferHandle,        // my input buffer handle from smem
                                                                u32 InBufferSize,           // my allocated input buffer size 
                                                                u32 OutBufferHandleA,       // my output buffer handle a from smem 
                                                                u32 OutBufferHandleB,       // my output buffer handle b from smem
                                                                u32 OutBufferSize,          // my allocated output buffer size
                                                                u32 ResolutionX,            // 
                                                                u32 ResolutionY,
                                                                EGLDisplay eglDisplay,
                                                                EGLContext eglContext);
bool            MMALFramePoller                             (   u32 frame_offset, u32 frame_length);                                           
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
private:
static  void    MMALcallBack                                (   void *callback_param, VCHI_CALLBACK_REASON_T reason, void *msg_handle);
        u32     NextTransId                                 (   u32 &tid);
        bool    GetVCHIstate                                (   );
        bool    MMALinitEvents                              (   );
     // void    MMALstoreLog                                (   const char* label, u32 value);
        void    MMALstoreLog                                (   const char* label, 
                                                                u32 value1 = STOREDEBUG_WHITESPACE, 
                                                                u32 value2 = STOREDEBUG_WHITESPACE);
        void    MMALstoreMsg                                (   const void* tx_msg, u32 total_size, const char* label);
        bool    MMALsendAndWait                             (   const void *tx_msg, 
                                                                size_t msg_size, 
                                                                void *rx_msg, 
                                                                size_t max_reply_len, 
                                                                size_t *actual_reply_len );
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264 Decoder Setup Code
//----------------------------------------------------------------------------------------------------------------------------------------------------                                                                
        bool    MMALopenService                             (   );
        bool    MMALcreateComponent                         (   );                                                      // MMALsendAndWait mmal_msg_component_create
        bool    MMALgetPortInfo                             (   u32 port_type, 
                                                                u32 &port_handle, 
                                                                mmal_msg_port_info_get_reply &PortInfoReply);           // MMALsendAndWait mmal_msg_port_info_get
        bool    MMALsetInputPortFormat                      (   );                                                      // MMALsendAndWait mmal_msg_port_info_set Input
        bool    MMALsetOutputPortFormat                     (   );                                                      // MMALsendAndWait mmal_msg_port_info_set Output
        bool    MMALenableComponent                         (   );                                                      // MMALsendAndWait mmal_msg_component_enable
        bool    MMALenablePort                              (   u32 port_handle, 
                                                                const mmal_msg_port_info_get_reply &PortInfoReply);     // MMALsendAndWait mmal_msg_port_action_port
        bool    MMALsetZeroCopyMode                         (   u32 port_handle);                                       // MMALsendAndWait mmal_msg_port_parameter_set
        void    MMALinitialOutputBuffers                    (   );                                                      // MMALsendAndWait mmal_msg_buffer_from_host via MMALqueueOutputBuffer
        bool    CheckGLError                                (   );        
        bool    MMALcreateTextures                          (   );
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264 Decoder Runtime Code
//----------------------------------------------------------------------------------------------------------------------------------------------------        
        bool    MMALbufferReady                             (   ); 
        bool    MMALqueueOutputBuffer                       (   );                                                      // MMALsendAndWait mmal_msg_buffer_from_host
        bool    MMALqueueInputFrame                         (   u32 frame_offset, u32 frame_length);                    // MMALsendAndWait mmal_msg_buffer_from_host
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              MEMBER VARIABLES
//----------------------------------------------------------------------------------------------------------------------------------------------------
        VCHI_INSTANCE_T                                     m_VCHIInstance;             // global vchi instance from bcm_host.c via GetVCHIstate
        VCHI_CONNECTION_T*                                  m_Connection;               // global vchi instance from bcm_host.c via GetVCHIstate
        VCOS_EVENT_T                                        m_VCOSevent;                // vcos event from vcos for the callback control
        VCHI_SERVICE_HANDLE_T                               m_ServiceHandle;            // vchi service handle 

        u32                                                 m_TransactionId;            // to get unique message numbers

        EGLDisplay                                          m_eglDisplay;               // egl display handle 
        EGLContext                                          m_eglContext;               // egl contey handle
        EGLImageKHR                                         m_EGLimageA = nullptr;
        EGLImageKHR                                         m_EGLimageB = nullptr;

        mmal_msg_port_info_get_reply                        m_PortInfoReplyIn = {};
        mmal_msg_port_info_get_reply                        m_PortInfoReplyOut = {};
        
        u32                                                 m_ComponentHandle;          // Your private member variable
        u32                                                 m_NumInputs;
        u32                                                 m_NumOutputs;
        u32                                                 m_NumClock;
        
//      u32                                                 m_LastOutputBufferQueued;
        
        u32                                                 m_InputBufferHandle;        // the handles from vc sm "smem"
        u32                                                 m_OutputBufferHandleA;
        u32                                                 m_OutputBufferHandleB;

        u32                                                 m_InputBufferSize;
        u32                                                 m_OutputBufferSize;

        u32                                                 m_ResolutionX;
        u32                                                 m_ResolutionY;

        u32                                                 m_InputPortHandle;
        u32                                                 m_OutputPortHandle;

//      u32                                                 m_VCSMHandleIn = 0;

        u32                                                 m_VCSMHandleA = 0;
        u32                                                 m_VCSMHandleB = 0;

        u32                                                 m_CurrentVCSMHandle = 0;
        u32                                                 m_AltVCSMHandle = 0;
        u32                                                 m_CurrentBufferHandle = 0;
        u32                                                 m_AltBufferHandle = 0;
public:
        u32                                                 m_CharIndex = 0;
        char                                                m_DebugCharArray[MMAL_MAX_DEBUG_FILE_LENGTH] = { 0 }; // is exposed for 
        GLuint                                              m_TextureA = 0;
        GLuint                                              m_TextureB = 0;
};
#endif // _H264_DECODER_CLASS_H
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              END OF FILE
//----------------------------------------------------------------------------------------------------------------------------------------------------

// Members assumed:
// u32 m_OutputBufferHandleA, m_OutputBufferHandleB;
// u32 m_VCSMHandleA, m_VCSMHandleB;
// u32 m_CurrentBufferHandle, m_AltBufferHandle;
// u32 m_CurrentVCSMHandle, m_AltVCSMHandle;