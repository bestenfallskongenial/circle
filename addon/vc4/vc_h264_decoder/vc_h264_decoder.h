//----------------------------------------------------------------------------------------------------------------------------------------------------
//              h264_decoder_class.h 
//----------------------------------------------------------------------------------------------------------------------------------------------------
    #ifndef _H264_DECODER_CLASS_H
    #define _H264_DECODER_CLASS_H

    #include <circle/types.h> 

    #include "h264_decoder_defs.h"

    #include "interface/vcos/vcos.h"
//  #include "interface/vcos/vcos_event.h"
    #include <vc4/vchi/vchi.h>  

    #include <vc4/vchiq/vchiq.h>
    #include <circle/bcm2835.h>

    #include <vc4/interface/khronos/include/EGL/egl.h>
    #include <vc4/interface/khronos/include/GLES/gl.h>
    #include <vc4/interface/khronos/include/GLES2/gl2.h>
    #include <vc4/interface/khronos/include/EGL/eglext.h>
    #include <vc4/interface/khronos/include/GLES/glext.h>
    #include <vc4/interface/khronos/include/GLES2/gl2ext.h>

extern "C" void vc_host_get_vchi_state(VCHI_INSTANCE_T *inst, VCHI_CONNECTION_T **conn);

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
        bool    MMALinitialize                              (   u32 InBufferHandle,        // my input buffer handle from smem
                                                                u32 InBufferSize,           // my allocated input buffer size 
                                                                u32 OutBufferHandleA,       // my output buffer handle a from smem 
                                                                u32 OutBufferHandleB,       // my output buffer handle b from smem
                                                                u32 OutBufferSize,          // my allocated output buffer size
                                                                u32 ResolutionX,            // 
                                                                u32 ResolutionY,
                                                                EGLDisplay eglDisplay,
                                                                EGLContext eglContext   );
        bool    MMALFramePoller                             (   u32 frame_offset, u32 frame_length      );
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
private:
static  void    MMALcallBack                                (   void *callback_param, VCHI_CALLBACK_REASON_T reason, void *msg_handle   );
        u32     NextTransId                                 (   u32 &tid);
        bool    GetVCHIstate                                (   );
        bool    MMALinitEvents                              (   );

        void    MMALstoreLog                                (   const char* label,
                                                                u32  value1 = STOREDEBUG_WHITESPACE,
                                                                u32  value2 = STOREDEBUG_WHITESPACE,
                                                                u32  value3 = STOREDEBUG_WHITESPACE,
                                                                u32  value4 = STOREDEBUG_WHITESPACE );        // NEW
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
        void    MMALsetInputPortFormat                      (   const mmal_msg_port_info_get_reply &OriginalPortInfo,
                                                                mmal_msg_port_info_get_reply &WorkingCopy);             // MMALsendAndWait mmal_msg_port_info_set Input
        void    MMALsetOutputPortFormat                     (   const mmal_msg_port_info_get_reply &OriginalPortInfo,
                                                                mmal_msg_port_info_get_reply &WorkingCopy);             // MMALsendAndWait mmal_msg_port_info_set Output
        bool    SendPortWorkingCopy                         (   u32 port_type, const mmal_msg_port_info_get_reply &WorkingCopy);
        bool    MMALenableComponent                         (   );                                                      // MMALsendAndWait mmal_msg_component_enable
        bool    MMALenablePort                              (   u32 port_handle, const mmal_msg_port_info_get_reply &port_info);                                       // MMALsendAndWait mmal_msg_port_action_port
        bool    MMALsetZeroCopyMode                         (   u32 port_handle);                                       // MMALsendAndWait mmal_msg_port_parameter_set
        void    MMALinitialOutputBuffers                    (   );                                                      // MMALsendAndWait mmal_msg_buffer_from_host via MMALqueueOutputBuffer
        bool    CheckGLError                                (   );        
        void    InitBodies                                  (   );

//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264 Decoder Runtime Code
//----------------------------------------------------------------------------------------------------------------------------------------------------
        bool    MMALbufferReady                             (   u32 handle);     
        bool    MMALqueueOutputBuffer                       (   mmal_msg_buffer_from_host_wire32& body, u32 vc_handle, u32 alloc_size);
        bool    MMALqueueInputBuffer                        (   mmal_msg_buffer_from_host_wire32& body, u32 frame_offset, u32 frame_length);    
//      bool    MMALqueueOutputBuffer                       (   u32 vc_handle, u32 alloc_size);                         // MMALsendAndWait mmal_msg_buffer_from_host
//      bool    MMALqueueInputBuffer                        (   u32 frame_offset, u32 frame_length);   
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              MEMBER VARIABLES
//----------------------------------------------------------------------------------------------------------------------------------------------------
        VCHI_INSTANCE_T                                     m_VCHIInstance;                                             // global vchi instance from bcm_host.c via GetVCHIstate
        VCHI_CONNECTION_T*                                  m_Connection;                                               // global vchi instance from bcm_host.c via GetVCHIstate
        VCOS_EVENT_T                                        m_VCOSevent;                                                // vcos event from vcos for the callback control
        VCHI_SERVICE_HANDLE_T                               m_ServiceHandle;                                            // vchi service handle 

        u32                                                 m_TransactionId;                                            // to get unique message numbers

        EGLDisplay                                          m_eglDisplay;                                               // egl display handle 
        EGLContext                                          m_eglContext;                                               // egl contey handle
        EGLImageKHR                                         m_EGLimageA                 = nullptr;
        EGLImageKHR                                         m_EGLimageB                 = nullptr;

        mmal_msg_port_info_get_reply                        m_PortInfoReplyIn           = {0};
        mmal_msg_port_info_get_reply                        m_PortInfoReplyOut          = {0};

        mmal_msg_port_info_get_reply                        m_InputPortInfoReply        = {0};
        mmal_msg_port_info_get_reply                        m_InputPortWorkingSet       = {0};
        mmal_msg_port_info_get_reply                        m_OutputPortInfoReply       = {0};
        mmal_msg_port_info_get_reply                        m_OutputPortWorkingSet      = {0};
        
        u32                                                 m_ComponentHandle;                                          // Your private member variable
        u32                                                 m_NumInputs;
        u32                                                 m_NumOutputs;
        u32                                                 m_NumClock;
        
//      u32                                                 m_LastOutputBufferQueued;
        
        u32                                                 m_InputBufferHandle;                                        // the handles from vc sm "smem"

        u32                                                 m_InputBufferPointer;

        u32                                                 m_OutputBufferHandleA;
        u32                                                 m_OutputBufferHandleB;

        u32                                                 m_OutputBufferPointerA;
        u32                                                 m_OutputBufferPointerB;

        u32                                                 m_ResolutionX;
        u32                                                 m_ResolutionY;

        u32                                                 m_InputPortHandle;
        u32                                                 m_OutputPortHandle;

        u32                                                 m_InputBufferSize;
        u32                                                 m_OutputBufferSize;

        u32                                                 m_VCSMHandleA = 0;      // really?? lets think over it
        u32                                                 m_VCSMHandleB = 0;

    mmal_msg_buffer_from_host_wire32 mBodyOut{};
    mmal_msg_buffer_from_host_wire32 mBodyIn{};
public:
        u32                                                 m_CharIndex = 0;
        char                                                m_DebugCharArray[MMAL_MAX_DEBUG_FILE_LENGTH] = { 0 };       // is exposed for 
        GLuint                                              m_TextureExposed = 0;                                             // must be exposed
        GLuint                                              m_TextureHidden = 0;                                             // kept local

        bool                                                m_FirstFrameQueued = false;    
}
#endif // _H264_DECODER_CLASS_H
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              END OF FILE
//----------------------------------------------------------------------------------------------------------------------------------------------------
