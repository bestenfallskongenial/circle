//----------------------------------------------------------------------------------------------------------------------------------------------------
//              h264_decoder_class.cpp
//----------------------------------------------------------------------------------------------------------------------------------------------------
#include "vc_h264_decoder.h"
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CONSTRUCTOR / DECONSTRUCTOR
//----------------------------------------------------------------------------------------------------------------------------------------------------
                CH264Decoder::CH264Decoder              (   )
                    : m_ServiceHandle(0)
                    , m_TransactionId(0)
                    {
                    }
                CH264Decoder::~CH264Decoder             (   )
{
                if (m_ServiceHandle)
                    {
                    vchi_service_close(m_ServiceHandle);
                    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              USER API
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool            CH264Decoder::MMALinitialize           (    u32 InBufferHandle,         // my input buffer handle from smem
                                                            u32 InBufferPointer,        // my input buffer pointer from smem
                                                            u32 InBufferSize,           // my allocated input buffer size 
                                                            u32 OutBufferHandleA,       // my output buffer handle a from smem
                                                            u32 OutBufferPointerA,      // myoutput buffer pointer from smem 
                                                            u32 OutBufferHandleB,       // my output buffer handle b from smem
                                                            u32 OutBufferPointerA,      // my output buffer pointer from smem
                                                            u32 OutBufferSize,          // my allocated output buffer size
                                                            u32 ResolutionX,            // 
                                                            u32 ResolutionY, 
                                                            EGLDisplay eglDisplay,      // EGL display connection
                                                            EGLContext eglContext)      // EGL rendering context
{
                m_InputBufferHandle                 = InBufferHandle;
                m_InputBufferPointer                = InBufferPointer;
                m_OutputBufferHandleA               = OutBufferHandleA;
                m_OutputBufferPointerA              = OutBufferPointerA;        
                m_OutputBufferHandleB               = OutBufferHandleB;
                m_OutputBufferPointerB              = OutBufferPointerB; 
                m_InputBufferSize                   = InBufferSize; // 81920; <- for testing the recomented buffer size - works fie!
                m_OutputBufferSize                  = OutBufferSize;

                m_ResolutionX                       = ResolutionX;
                m_ResolutionY                       = ResolutionY;

                m_eglDisplay                        = eglDisplay;
                m_eglContext                        = eglContext;

                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "Input Port      Handle / Size ",m_InputBufferHandle, m_InputBufferSize);
                MMALstoreLog ( "Output A Port   Handle / Size ",m_OutputBufferHandleA, m_OutputBufferSize);
                MMALstoreLog ( "Output B Port   Handle / Size ",m_OutputBufferHandleB, m_OutputBufferSize);        
                MMALstoreLog ( "Resolution      Height / Width",m_ResolutionX, m_ResolutionY);
                MMALstoreLog ( "EGL Display / Contex          ", (u32)m_eglDisplay, (u32)m_eglContext);
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "init/setup service");
                MMALstoreLog ( "----------------------------------------------------------------");
                    GetVCHIstate                ();      // initialize and bind to VCHI instance
                    MMALinitEvents              ();      // prepare MMAL internal event slots
                    MMALopenService             ();      // open MMAL VCHI service - LOG THE MESSAGE
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_component_create - create component");
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALcreateComponent         ();      // create decoder component (vc.ril.video_decode)
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_port_info_get - query initial port states");
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_INPUT , m_InputPortHandle , m_InputPortInfoReply);   // LOG THE MESSAGE
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_OUTPUT, m_OutputPortHandle, m_OutputPortInfoReply);  // LOG THE MESSAGE
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_port_info_set - setup / send port formats");
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALsetInputPortFormat      (m_InputPortInfoReply, m_InputPortWorkingSet);    // set input codec + buffer size 
                    MMALsetOutputPortFormat     (m_OutputPortInfoReply, m_OutputPortWorkingSet); // set output resolution + format
        
                    SendPortWorkingCopy         (MMAL_PORT_TYPE_INPUT,  m_InputPortWorkingSet);  // apply config to VC - LOG THE MESSAGE
                    SendPortWorkingCopy         (MMAL_PORT_TYPE_OUTPUT, m_OutputPortWorkingSet); // apply config to VC - LOG THE MESSAGE
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_component_enable - enable decoder component"); 
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALenableComponent         ();      // enable component AFTER format setup
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_port_info_get - re-query port states");
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_INPUT , m_InputPortHandle , m_InputPortInfoReply);   // LOG THE MESSAGE
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_OUTPUT, m_OutputPortHandle, m_OutputPortInfoReply);  // LOG THE MESSAGE
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_port_parameter_set ? set zero-copy mode");
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALsetZeroCopyMode         (   m_InputPortHandle );
                    MMALsetZeroCopyMode         (   m_OutputPortHandle );
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_port_info_get - verify final port states");
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_INPUT , m_InputPortHandle , m_InputPortInfoReply);   // LOG THE MESSAGE
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_OUTPUT, m_OutputPortHandle, m_OutputPortInfoReply);  // LOG THE MESSAGE
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_port_action_port - enable input/output ports");
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALenablePort              (   m_InputPortHandle,  m_InputPortInfoReply);
                    MMALenablePort              (   m_OutputPortHandle, m_OutputPortInfoReply);
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_port_info_get - verify post-enable port state");
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_INPUT , m_InputPortHandle , m_InputPortInfoReply);   // LOG THE MESSAGE
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_OUTPUT, m_OutputPortHandle, m_OutputPortInfoReply);  // LOG THE MESSAGE
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "mmal_msg_buffer_from_host - finalize Output Ports");
                MMALstoreLog ( "----------------------------------------------------------------");
                    InitBodies();             
                    MMALinitialOutputBuffers    ();      // queue initial output buffers to VC
                MMALstoreLog ( "----------------------------------------------------------------");
                MMALstoreLog ( "init Output Textures");  
                MMALstoreLog ( "----------------------------------------------------------------");
                    MMALcreateTextures          ();      // allocate and bind EGLImage textures
                MMALstoreLog ( "----------------------------------------------------------------");
        

                    return true; // <- early exit we are debugging         
}
bool            CH264Decoder::MMALFramePoller(u32 frame_offset, u32 frame_length)
{
                if (!m_FirstFrameQueued) 
                    {
                    if (!MMALqueueInputBuffer(mBodyIn, frame_offset, frame_length))
                        {
                        MMALstoreLog("very first frame queue error!", frame_offset, frame_length);
                        return false;
                        }
                    MMALstoreLog("very first frame queue success", frame_offset, frame_length); 
                    // debug 

                    MMALgetPortInfo             (   MMAL_PORT_TYPE_INPUT , m_InputPortHandle , m_InputPortInfoReply);
                    MMALgetPortInfo             (   MMAL_PORT_TYPE_OUTPUT, m_OutputPortHandle, m_OutputPortInfoReply);
                    
                    m_FirstFrameQueued = true;
                    return true;  // exit after priming input buffer
                    }

                CString message = "";

                // Skip if no event
                int st = vcos_event_try(&m_VCOSevent);
                if ( st == VCOS_EAGAIN)
            //  if (vcos_event_try(&m_VCOSevent) == VCOS_EAGAIN)
                    {
                    MMALstoreLog("tell me why!", frame_offset, frame_length, st);              
                    return true; // nothing to process, not an error
                    }
                mmal_msg rx_msg = {};
                uint32_t msg_len = 0;

                // Drain all pending messages
                while (vchi_msg_dequeue(m_ServiceHandle, &rx_msg, sizeof(rx_msg), &msg_len, VCHI_FLAGS_NONE) == 0)
                    {
                    switch (rx_msg.h.status)
                        {
                        case MMAL_MSG_STATUS_SUCCESS:
                            {
                            uint32_t ready_vcsm_handle = rx_msg.u.buffer_from_host.buffer_header.data;
                                
                            MMALstoreLog("offset / length / status / data", frame_offset, frame_length, rx_msg.h.status, rx_msg.u.buffer_from_host.buffer_header.data);

                            if ( ready_vcsm_handle != m_VCSMHandleA && ready_vcsm_handle != m_VCSMHandleB )
                                {
                                continue; // nothing to do here drain further    
                                }
                            else if ( ready_vcsm_handle  == m_VCSMHandleA )
                                {
                                if (!MMALqueueOutputBuffer(mBodyOut, m_OutputBufferHandleB, m_OutputBufferSize)) return false;

                                MMALbufferReady(ready_vcsm_handle);
                                MMALqueueInputBuffer(mBodyIn, frame_offset, frame_length);
                                message = "MMAL_MSG_STATUS_SUCCESS      - All is Fine";
                                MMALstoreLog(message, frame_offset, frame_length);     
                                return true;
                                }                
                            else if ( ready_vcsm_handle == m_VCSMHandleB )
                                {
                                if (!MMALqueueOutputBuffer(mBodyOut, m_OutputBufferHandleA, m_OutputBufferSize)) return false;

                                MMALbufferReady(ready_vcsm_handle);
                                MMALqueueInputBuffer(mBodyIn, frame_offset, frame_length);
                                message = "MMAL_MSG_STATUS_SUCCESS      - All is Fine";
                                MMALstoreLog(message, frame_offset, frame_length);     
                                return true;
                                }
                            break;
                            }
                        case MMAL_MSG_STATUS_ENOMEM:   message = "MMAL_MSG_STATUS_ENOMEM       - Out of memory                      "; break;
                        case MMAL_MSG_STATUS_ENOSPC:   message = "MMAL_MSG_STATUS_ENOSPC       - Out of resources other than memory "; break;
                        case MMAL_MSG_STATUS_EINVAL:   message = "MMAL_MSG_STATUS_EINVAL       - Argument is invalid                "; break;
                        case MMAL_MSG_STATUS_ENOSYS:   message = "MMAL_MSG_STATUS_ENOSYS       - Function not implemented           "; break;
                        case MMAL_MSG_STATUS_ENOENT:   message = "MMAL_MSG_STATUS_ENOENT       - No such file or directory          "; break;
                        case MMAL_MSG_STATUS_ENXIO:    message = "MMAL_MSG_STATUS_ENXIO        - No such device or address          "; break;
                        case MMAL_MSG_STATUS_EIO:      message = "MMAL_MSG_STATUS_EIO          - I/O error                          "; break;
                        case MMAL_MSG_STATUS_ESPIPE:   message = "MMAL_MSG_STATUS_ESPIPE       - Illegal seek                       "; break;
                        case MMAL_MSG_STATUS_ECORRUPT: message = "MMAL_MSG_STATUS_ECORRUPT     - Data is corrupt                    "; break;
                        case MMAL_MSG_STATUS_ENOTREADY:message = "MMAL_MSG_STATUS_ENOTREADY    - Component is not ready             "; break;
                        case MMAL_MSG_STATUS_ECONFIG:  message = "MMAL_MSG_STATUS_ECONFIG      - Component is not configured        "; break;
                        case MMAL_MSG_STATUS_EISCONN:  message = "MMAL_MSG_STATUS_EISCONN      - Port is already connected          "; break;
                        case MMAL_MSG_STATUS_ENOTCONN: message = "MMAL_MSG_STATUS_ENOTCONN     - Port is disconnected               "; break;
                        case MMAL_MSG_STATUS_EAGAIN:   message = "MMAL_MSG_STATUS_EAGAIN       - Resource temporarily unavailable   "; break;
                        case MMAL_MSG_STATUS_EFAULT:   message = "MMAL_MSG_STATUS_EFAULT       - Bad address                        "; break;
                        default:                       message = "Unknown MMAL status          - WTF!!!                             "; break;
                        }
                    MMALstoreLog(message, frame_offset, frame_length);
                    MMALstoreMsg(&rx_msg, msg_len, "Poller ERROR");
                    return false;
                    }
                return true; // queue drained, nothing relevant processed
}
//----------------------------------------------------------------------------------------------------------------------------------------------------