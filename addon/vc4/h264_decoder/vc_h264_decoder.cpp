// h264_decoder_class.cpp

#include "vc_h264_decoder.h"

#include <circle/types.h>
#include <circle/logger.h>
#include "interface/vcos/vcos.h"
#include "interface/vcos/vcos_event.h"
#include <vc4/vchi/vchi.h>
#include <vc4/vchiq/vchiq.h>
#include <circle/bcm2835.h>

#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/EGL/egl.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES/gl.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES2/gl2.h"

#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/EGL/eglext.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES/glext.h"
#include "/home/bestenfalls/circle/addon/vc4/interface/khronos/include/GLES2/gl2ext.h"
/*
#include <vc4/interface/khronos/include/EGL/egl.h>
#include <vc4/interface/khronos/include/GLES/gl.h>
#include <vc4/interface/khronos/include/GLES2/gl2.h>

#include <vc4/interface/khronos/include/EGL/eglext.h>
#include <vc4/interface/khronos/include/GLES/glext.h>
#include <vc4/interface/khronos/include/GLES2/gl2ext.h>
*/
extern "C" void vc_host_get_vchi_state(VCHI_INSTANCE_T *inst, VCHI_CONNECTION_T **conn);

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
                                                            u32 InBufferSize,           // my allocated input buffer size 
                                                            u32 OutBufferHandleA,       // my output buffer handle a from smem 
                                                            u32 OutBufferHandleB,       // my output buffer handle b from smem
                                                            u32 OutBufferSize,          // my allocated output buffer size
                                                            u32 ResolutionX,            // 
                                                            u32 ResolutionY, 
                                                            EGLDisplay eglDisplay,      // <-- add this
                                                            EGLContext eglContext)       // <-- and this)
{
                m_InputBufferHandle                 = InBufferHandle;
                m_OutputBufferHandleA               = OutBufferHandleA;
                m_OutputBufferHandleB               = OutBufferHandleB;

                m_InputBufferSize                   = InBufferSize;
                m_OutputBufferSize                  = OutBufferSize;

                m_ResolutionX                       = ResolutionX;
                m_ResolutionY                       = ResolutionY;

                m_eglDisplay                        = eglDisplay;
                m_eglContext                        = eglContext;
            MMALstoreLog ( "\n------------------------------------------------");     
                GetVCHIstate                ();
                MMALinitEvents              ();
                MMALopenService             ();
            MALstoreLog ( "\n------------------------------------------------");     
                MMALcreateComponent         ();
                MMALenableComponent         ();

                MMALgetPortInfo             (   MMAL_PORT_TYPE_INPUT , m_InputPortHandle , m_PortInfoReplyIn);
                MMALgetPortInfo             (   MMAL_PORT_TYPE_OUTPUT, m_OutputPortHandle, m_PortInfoReplyOut);
                
                return true; // <- early exit we are debugging          

                MMALsetInputPortFormat      ();
                MMALsetOutputPortFormat     ();
                
                MMALgetPortInfo             (   MMAL_PORT_TYPE_INPUT , m_InputPortHandle , m_PortInfoReplyIn);
                MMALgetPortInfo             (   MMAL_PORT_TYPE_OUTPUT, m_OutputPortHandle, m_PortInfoReplyOut);
                MMALsetZeroCopyMode         (   m_InputPortHandle );
                MMALsetZeroCopyMode         (   m_OutputPortHandle );

                MMALenablePort              (   m_InputPortHandle , m_PortInfoReplyIn );
                MMALenablePort              (   m_OutputPortHandle, m_PortInfoReplyOut );


                MMALinitialOutputBuffers    ();
                MMALcreateTextures          ();

                return true;
}
bool            CH264Decoder::MMALFramePoller          (    u32 frame_offset, u32 frame_length)
{
                CString message = "";

                if (vcos_event_wait(&m_VCOSevent) != VCOS_SUCCESS)
                    {
                    return false;
                    }
                mmal_msg rx_msg = {};
                uint32_t msg_len = 0;
                if (vchi_msg_dequeue(m_ServiceHandle, &rx_msg, sizeof(rx_msg), &msg_len, VCHI_FLAGS_NONE) != 0)
                    {
                    return false;
                    }
                switch (rx_msg.h.type)
                    {
                    case MMAL_MSG_STATUS_SUCCESS:           //  0 Success //
                        {
                        uint32_t ready_vcsm_handle = rx_msg.u.buffer_from_host.buffer_header.data;

                        // Alternating mechanism using if/else
                        if (ready_vcsm_handle == m_VCSMHandleA) 
                            {
                            m_CurrentVCSMHandle   = m_VCSMHandleA;
                            m_CurrentBufferHandle = m_OutputBufferHandleA;
                            m_AltVCSMHandle       = m_VCSMHandleB;
                            m_AltBufferHandle     = m_OutputBufferHandleB;
                            } 
                        else 
                            {
                            m_CurrentVCSMHandle   = m_VCSMHandleB;
                            m_CurrentBufferHandle = m_OutputBufferHandleB;
                            m_AltVCSMHandle       = m_VCSMHandleA;
                            m_AltBufferHandle     = m_OutputBufferHandleA;
                            }

                        MMALbufferReady();

                        MMALqueueOutputBuffer();

                        MMALqueueInputFrame(frame_offset, frame_length);
                        message                             = "MMAL_MSG_STATUS_SUCCESS      - All is Fine                           ";        
                        MMALstoreLog ( message, frame_offset, frame_length);     
                        return true;
                        }
                    case MMAL_MSG_STATUS_ENOMEM: message    = "MMAL_MSG_STATUS_ENOMEM       - Out of memory                         "; break;
                    case MMAL_MSG_STATUS_ENOSPC: message    = "MMAL_MSG_STATUS_ENOSPC       - Out of resources other than memory    "; break;
                    case MMAL_MSG_STATUS_EINVAL: message    = "MMAL_MSG_STATUS_EINVAL       - Argument is invalid                   "; break;
                    case MMAL_MSG_STATUS_ENOSYS: message    = "MMAL_MSG_STATUS_ENOSYS       - Function not implemented              "; break;
                    case MMAL_MSG_STATUS_ENOENT: message    = "MMAL_MSG_STATUS_ENOENT       - No such file or directory             "; break;
                    case MMAL_MSG_STATUS_ENXIO: message     = "MMAL_MSG_STATUS_ENXIO        - No such device or address             "; break;
                    case MMAL_MSG_STATUS_EIO: message       = "MMAL_MSG_STATUS_EIO          - I/O error                             "; break;
                    case MMAL_MSG_STATUS_ESPIPE: message    = "MMAL_MSG_STATUS_ESPIPE       - Illegal seek                          "; break;
                    case MMAL_MSG_STATUS_ECORRUPT: message  = "MMAL_MSG_STATUS_ECORRUPT     - Data is corrupt                       "; break;
                    case MMAL_MSG_STATUS_ENOTREADY: message = "MMAL_MSG_STATUS_ENOTREADY    - Component is not ready                "; break;
                    case MMAL_MSG_STATUS_ECONFIG: message   = "MMAL_MSG_STATUS_ECONFIG      - Component is not configured           "; break;
                    case MMAL_MSG_STATUS_EISCONN: message   = "MMAL_MSG_STATUS_EISCONN      - Port is already connected             "; break;
                    case MMAL_MSG_STATUS_ENOTCONN: message  = "MMAL_MSG_STATUS_ENOTCONN     - Port is disconnected                  "; break;
                    case MMAL_MSG_STATUS_EAGAIN: message    = "MMAL_MSG_STATUS_EAGAIN       - Resource temporarily unavailable      "; break;
                    case MMAL_MSG_STATUS_EFAULT: message    = "MMAL_MSG_STATUS_EFAULT       - Bad address                           "; break;
                    default: message                        = "Unknown MMAL status                                                  "; break;
                }
                MMALstoreLog ( message, frame_offset, frame_length);            
                    
                MMALstoreMsg(&rx_msg, msg_len, "Poller ERROR");  

                return false;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
void            CH264Decoder::MMALcallBack              (   void *callback_param, VCHI_CALLBACK_REASON_T reason, void *msg_handle)
{
                VCOS_EVENT_T *event = (VCOS_EVENT_T *)callback_param;
                if (reason == VCHI_CALLBACK_MSG_AVAILABLE && event)
                    {  
                    vcos_event_signal(event); 
                    }
}
u32             CH264Decoder::NextTransId               (   u32 &tid)
{
                tid = ( tid+1 ) & ~0x80000000u;                                             // mask for async messages really needed ?!                        
                return tid;
}
bool            CH264Decoder::GetVCHIstate              (   )
{
                vc_host_get_vchi_state(&m_VCHIInstance, &m_Connection);                         //1. get the VCHI instance and the connection handle from bcm_host.h

                MMALstoreLog ( "\nVCHI State Instance & Connection  ", (u32)m_VCHIInstance, (u32)m_Connection);                 

                return true;
}
bool            CH264Decoder::MMALinitEvents            (   )
{
                if (vcos_event_create(&m_VCOSevent, "MMAL") != VCOS_SUCCESS)
                    {
                    MMALstoreLog ( "\nVCOS Event Init Failed!   ");                        
                    return false;
                    }
                MMALstoreLog ( "\nVCOS Event Init Success!  ", (u32)&m_VCOSevent);                    
                return true;    
}
void            CH264Decoder::MMALstoreLog              (   const char* label, u32 value1, u32 value2)
{
    // Always write the label
    for (const char* p = label; *p; ++p)
        m_DebugCharArray[m_CharIndex++] = *p;

    // If both values are placeholders, stop here
    if (value1 == STOREDEBUG_WHITESPACE && value2 == STOREDEBUG_WHITESPACE) {
        m_DebugCharArray[m_CharIndex++] = '\n';
        m_DebugCharArray[m_CharIndex]   = '\0';
        return;
    }

    // If value is valid, write it
    if (value1 != STOREDEBUG_WHITESPACE) {
        m_DebugCharArray[m_CharIndex++] = ' ';
        m_DebugCharArray[m_CharIndex++] = '0';
        m_DebugCharArray[m_CharIndex++] = 'x';
        for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) {
            char hex = "0123456789ABCDEF"[(value1 >> (i * 4)) & 0xF];
            m_DebugCharArray[m_CharIndex++] = hex;
        }
    }

    // If second value is valid, write it
    if (value2 != STOREDEBUG_WHITESPACE) {
        m_DebugCharArray[m_CharIndex++] = ' ';
        m_DebugCharArray[m_CharIndex++] = '0';
        m_DebugCharArray[m_CharIndex++] = 'x';
        for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) {
            char hex = "0123456789ABCDEF"[(value2 >> (i * 4)) & 0xF];
            m_DebugCharArray[m_CharIndex++] = hex;
        }
    }

    // Terminate
    m_DebugCharArray[m_CharIndex++] = '\n';
    m_DebugCharArray[m_CharIndex]   = '\0';
}
/*
void            CH264Decoder::MMALstoreLog              (   const char* label, u32 value)
{
    // copy label
    for (const char* p = label; *p; ++p) 
        {
        m_DebugCharArray[m_CharIndex] = *p;
        m_CharIndex++;
        }
    if(value == STOREDEBUG_WHITESPACE ) return;    
    // Write " 0x" prefix
    m_DebugCharArray[m_CharIndex] = ' ';
    m_CharIndex++;
    m_DebugCharArray[m_CharIndex] = '0';
    m_CharIndex++;
    m_DebugCharArray[m_CharIndex] = 'x';
    m_CharIndex++;

    // Write value in hexadecimal (big-endian, no leading zero suppression)
    for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) 
        {
        char hex = "0123456789ABCDEF"[(value >> (i * 4)) & 0xF];
        m_DebugCharArray[m_CharIndex] = hex;
        m_CharIndex++;
        }
    // newline + terminator
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;    
    m_DebugCharArray[m_CharIndex] = '\0';
}
*/
void            CH264Decoder::MMALstoreMsg              (   const void* tx_msg, u32 total_size, const char* label)
{   
    // insert leading newline
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;
    // copy label
    for (const char* p = label; *p; ++p) 
        {
        m_DebugCharArray[m_CharIndex] = *p;
        m_CharIndex++;
        }
    // next line please
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;
    // hex dump, 16 bytes per line
    const unsigned char* b = (const unsigned char*)tx_msg;
    for (u32 i = 0; i < total_size; ++i) {
        if (i && (i % 16) == 0) 
            {
            m_DebugCharArray[m_CharIndex] = '\n';
            m_CharIndex++;
            }
        unsigned char v = b[i];

        char hi = "0123456789ABCDEF"[v >> 4];
        m_DebugCharArray[m_CharIndex] = hi;
        m_CharIndex++;

        char lo = "0123456789ABCDEF"[v & 0xF];
        m_DebugCharArray[m_CharIndex] = lo;
        m_CharIndex++;

        m_DebugCharArray[m_CharIndex] = ' ';
        m_CharIndex++;
    }
    // newline + terminator
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;
    m_DebugCharArray[m_CharIndex] = '\n';
    m_CharIndex++;    
    m_DebugCharArray[m_CharIndex] = '\0';
}
bool            CH264Decoder::MMALsendAndWait           (   const void *msg, 
                                                            size_t msg_size, 
                                                            void *rx_msg, 
                                                            size_t max_reply_len, 
                                                            size_t *actual_reply_len )
{
                MMALstoreMsg(msg, msg_size, "Raw TX");

                if (vchi_msg_queue(m_ServiceHandle, msg, msg_size, VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL) != 0)
                    {
                    return false;
                    }
                uint32_t ReplyLength = 0;
                do {
                    if (vchi_msg_dequeue(m_ServiceHandle, rx_msg, max_reply_len, &ReplyLength, VCHI_FLAGS_NONE) == 0)
                    {
                        MMALstoreMsg(rx_msg, ReplyLength, "Raw RX");
                        break;
                    }
                } while (vcos_event_wait(&m_VCOSevent) == VCOS_SUCCESS);

                if (actual_reply_len)
                    *actual_reply_len = ReplyLength;

                return (ReplyLength != 0);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264 Decoder Setup Code
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool            CH264Decoder::MMALopenService          (   )
{
                SERVICE_CREATION_T params           = {};
                params.version.version              = VC_MMAL_VER;
                params.version.version_min          = VC_MMAL_MIN_VER;
                params.service_id                   = VCHIQ_MAKE_FOURCC('m','m','a','l');
                params.connection                   = m_Connection;                             // VCHI connection ( returned from bcmhost´s vc_host_get_vchi_state )
                params.rx_fifo_size                 = 0;
                params.tx_fifo_size                 = 0;
                params.callback                     = MMALcallBack;
                params.callback_param               = &m_VCOSevent;
                params.want_unaligned_bulk_rx       = 0;
                params.want_unaligned_bulk_tx       = 0;
                params.want_crc                     = 0;

                MMALstoreLog("\nversion                       ", params.version.version);
                MMALstoreLog("version_min                   ", params.version.version_min);
                MMALstoreLog("service_id                    ", params.service_id);
                MMALstoreLog("connection                    ", (u32)(uintptr_t)params.connection);
                MMALstoreLog("rx_fifo_size                  ", params.rx_fifo_size);
                MMALstoreLog("tx_fifo_size                  ", params.tx_fifo_size);
                MMALstoreLog("callback                      ", (u32)(uintptr_t)params.callback);
                MMALstoreLog("callback_param                ", (u32)(uintptr_t)params.callback_param);
                MMALstoreLog("want_unaligned_bulk_rx        ", params.want_unaligned_bulk_rx);
                MMALstoreLog("want_unaligned_bulk_tx        ", params.want_unaligned_bulk_tx);
                MMALstoreLog("want_crc                      ", params.want_crc);
/*
                if(!vchi_service_open(m_VCHIInstance, &params, &m_ServiceHandle))
                    {
                    MMALstoreLog ( "\nOpen MMAL Service Failed!");
                    return false;
                    }
                MMALstoreLog ( "\nOpen MMAL Service Success!", (u32)m_ServiceHandle); 
                return true; // (rc == 0);
*/

                int rc = vchi_service_open(m_VCHIInstance, &params, &m_ServiceHandle);
                if ( rc != 0)
                    {
                    MMALstoreLog ( "\nFailed to open MMAL service!", (u32)m_ServiceHandle);
                    }
                if ( rc == 0)
                    {
                    MMALstoreLog ( "\nMMALservice opened!", (u32)m_ServiceHandle);
                    }
                return (rc == 0);
}
bool            CH264Decoder::MMALcreateComponent      (   )                                                    // mmal_msg_component_create    // expects a pointer therefore CreateComponent(&m_My_private_Member);
{
                mmal_msg_header tx_hdr = {};                                                    // 1. MMAL header: all fields shown

                tx_hdr.magic                           = MMAL_MAGIC;
                tx_hdr.type                            = MMAL_MSG_TYPE_COMPONENT_CREATE;
                tx_hdr.context                         = NextTransId(m_TransactionId);          // If you want to track transactions, set it here.
                tx_hdr.status                          = 0;
                tx_hdr.padding                         = 0;                                     // If your struct has this field (show all!)

                mmal_msg_component_create tx_body = {};                                         // 2. MMAL tx_body: all fields shown

                tx_body.client_component                = 0;
                memset(tx_body.name, 0, sizeof(tx_body.name));
                strncpy(tx_body.name, "ril.video_decode", sizeof(tx_body.name) - 1);         // or "ril.video_decode"? sure about the -1 here?
                tx_body.pid                             = 0;
            //    tx_body.reserved0                      = 0;                                     // If present
            //    tx_body.reserved1                      = 0;                                     // If present

                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)];                                    // 3. Compose full message buffer
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));
                
                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};                             // 4. Prepare RX buffer for reply (show all fields) shall i use sizeof(message name) or better MMAL_MSG_MAX_SIZE?
                size_t rx_len = 0;

                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))  // 5. Send and wait
                    {
                    MMALstoreLog ( "\nCreate Component Failed!");
                    return false;
                    }
                if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_component_create_reply))
                    {
                    MMALstoreLog ( "\nCreate Component Failed!");
                    return false;
                    }

                const mmal_msg_component_create_reply* reply = 
                    reinterpret_cast<const mmal_msg_component_create_reply*>(rx_msg + sizeof(mmal_msg_header)); // 6. Parse reply (show all fields)
                
                if (reply->status != MMAL_MSG_STATUS_SUCCESS)                                   // 7. Check all fields for status and handle
                    {
                    MMALstoreLog ( "\nCreate Component Failed!", (u32)m_ComponentHandle);
                    return false;
                    }

                m_ComponentHandle = reply->component_handle;   // Direct member assignment
                m_NumInputs     = reply->input_num;
                m_NumOutputs    = reply->output_num;
                m_NumClock      = reply->clock_num;

                MMALstoreLog ( "\nCreate Component Success!     ", (u32)m_ComponentHandle);
                MMALstoreLog ( "\nNumber of In / Output Ports   ", (u32)m_NumInputs, (u32)m_NumOutputs);
                MMALstoreLog ( "\nNumber of Clock Ports         ", (u32)m_NumClock);
                
                return /*true;*/ (reply->status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALgetPortInfo          (    u32 port_type,
                                                            u32 &port_handle, 
                                                            mmal_msg_port_info_get_reply &PortInfoReply)        // mmal_msg_port_info_get
{
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic                           = MMAL_MAGIC;
                tx_hdr.type                            = MMAL_MSG_TYPE_PORT_INFO_GET;
                tx_hdr.context                         = NextTransId(m_TransactionId);
                tx_hdr.status                          = 0;
                tx_hdr.padding                         = 0;

                mmal_msg_port_info_get tx_body = {};
                tx_body.component_handle               = m_ComponentHandle;
                tx_body.port_type                      = port_type;
                tx_body.index                          = 0; //port_index;

                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)] = {};
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};                                              //  u8 rx_msg[sizeof(mmal_msg_port_info_get_reply)] = {};

                size_t rx_len = 0;

                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    MMALstoreLog ( "\nGet Port Info Failed!");                        
                    return false;
                    }
                if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_port_info_get_reply))
                    {
                    MMALstoreLog ( "\nGet Port Info Failed!");                        
                    return false;
                    }
                mmal_msg_header *rx_hdr = reinterpret_cast<mmal_msg_header *>(rx_msg);
                if (rx_hdr->type != MMAL_MSG_TYPE_PORT_INFO_GET)
                    {
                    MMALstoreLog ( "\nGet Port Info Failed!");                        
                    return false;
                    }                            
                // usually we parse the result via pointer magic but we need to store the port results globally for later use
                memcpy(&PortInfoReply, rx_msg + sizeof(mmal_msg_header), sizeof(mmal_msg_port_info_get_reply));

                MMALstoreLog("status                        ", PortInfoReply.status);
                MMALstoreLog("component_handle              ", PortInfoReply.component_handle);
                MMALstoreLog("port_type                     ", PortInfoReply.port_type);
                MMALstoreLog("port_index                    ", PortInfoReply.port_index);
                MMALstoreLog("found                         ", PortInfoReply.found);
                MMALstoreLog("port_handle                   ", PortInfoReply.port_handle);
                // mmal_port is a struct; log each field individually:
                MMALstoreLog("port.priv                     ", PortInfoReply.port.priv);
                MMALstoreLog("port.name                     ", PortInfoReply.port.name);
                MMALstoreLog("port.type                     ", PortInfoReply.port.type);
                MMALstoreLog("port.index                    ", PortInfoReply.port.index);
                MMALstoreLog("port.index_all                ", PortInfoReply.port.index_all);
                MMALstoreLog("port.is_enabled               ", PortInfoReply.port.is_enabled);
                MMALstoreLog("port.format                   ", PortInfoReply.port.format);
                MMALstoreLog("port.buffer_num_min           ", PortInfoReply.port.buffer_num_min);
                MMALstoreLog("port.buffer_size_min          ", PortInfoReply.port.buffer_size_min);
                MMALstoreLog("port.buffer_alignment_min     ", PortInfoReply.port.buffer_alignment_min);
                MMALstoreLog("port.buffer_num_recommended   ", PortInfoReply.port.buffer_num_recommended);
                MMALstoreLog("port.buffer_size_recommended  ", PortInfoReply.port.buffer_size_recommended);
                MMALstoreLog("port.buffer_num               ", PortInfoReply.port.buffer_num);
                MMALstoreLog("port.buffer_size              ", PortInfoReply.port.buffer_size);
                MMALstoreLog("port.component                ", PortInfoReply.port.component);
                MMALstoreLog("port.userdata                 ", PortInfoReply.port.userdata);
                MMALstoreLog("port.capabilities             ", PortInfoReply.port.capabilities);
                // mmal_es_format is a struct; log each field individually:
                MMALstoreLog("format.type                   ", PortInfoReply.format.type);
                MMALstoreLog("format.encoding               ", PortInfoReply.format.encoding);
                MMALstoreLog("format.encoding_variant       ", PortInfoReply.format.encoding_variant);
                MMALstoreLog("format.es                     ", PortInfoReply.format.es);
                MMALstoreLog("format.bitrate                ", PortInfoReply.format.bitrate);
                MMALstoreLog("format.flags                  ", PortInfoReply.format.flags);
                MMALstoreLog("format.extradata_size         ", PortInfoReply.format.extradata_size);
                MMALstoreLog("format.extradata              ", PortInfoReply.format.extradata);
                // mmal_es_specific_format MMAL_ES_TYPE_VIDEO:
                MMALstoreLog("es.video.width                ", PortInfoReply.es.video.width);
                MMALstoreLog("es.video.height               ", PortInfoReply.es.video.height);
                MMALstoreLog("es.video.crop.x               ", PortInfoReply.es.video.crop.x);
                MMALstoreLog("es.video.crop.y               ", PortInfoReply.es.video.crop.y);
                MMALstoreLog("es.video.crop.width           ", PortInfoReply.es.video.crop.width);
                MMALstoreLog("es.video.crop.height          ", PortInfoReply.es.video.crop.height);
                MMALstoreLog("es.video.frame_rate.num       ", PortInfoReply.es.video.frame_rate.num);
                MMALstoreLog("es.video.frame_rate.den       ", PortInfoReply.es.video.frame_rate.den);
                MMALstoreLog("es.video.par.num              ", PortInfoReply.es.video.par.num);
                MMALstoreLog("es.video.par.den              ", PortInfoReply.es.video.par.den);
                MMALstoreLog("es.video.color_space          ", PortInfoReply.es.video.color_space);
                // extradata is a byte array
                MMALstoreMsg(PortInfoReply.extradata, PortInfoReply.format.extradata_size, "extradata");
                // Assign port handle to out reference
                port_handle = PortInfoReply.port_handle;
                   
                // Optionally also store in your private member here if needed:
                // m_PortHandleIn = PortInfoReply.port_handle; // or similar
                MMALstoreLog ( "\nGet Port Info Success!", (u32)PortInfoReply.port_handle);      // & really?
                
                return /*true;*/(PortInfoReply.status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALsetInputPortFormat   (   )                                                    // mmal_msg_port_info_set
{
                mmal_msg_header tx_hdr = {};                                                    // 1. MMAL header
                tx_hdr.magic                           = MMAL_MAGIC;
                tx_hdr.type                            = MMAL_MSG_TYPE_PORT_INFO_SET;
                tx_hdr.context                         = NextTransId(m_TransactionId);          // If you want to track transactions, set it here.
                tx_hdr.status                          = 0;
                tx_hdr.padding                         = 0;                                     // If your struct has this field (show all!)

                
                mmal_msg_port_info_set tx_body = {};                                            // 2. MMAL tx_body (all fields shown)
                tx_body.component_handle               = m_ComponentHandle;
                tx_body.port_type                      = MMAL_PORT_TYPE_INPUT;
                tx_body.port_index                     = 0;                                     // input[0]
                memset(&tx_body.port, 0, sizeof(tx_body.port));                                 // i like to know how i can / must set up the nested structs here
                tx_body.port.type                      = MMAL_PORT_TYPE_INPUT;
                tx_body.port.index                     = 0;
                tx_body.port.is_enabled                = 1;
                tx_body.port.buffer_num_min            = MIN_BUFFERS;
                tx_body.port.buffer_size_min           = FIXED_BUFFER_SIZE;
                tx_body.port.buffer_alignment_min      = 0;
                tx_body.port.buffer_num_recommended    = MIN_BUFFERS;
                tx_body.port.buffer_size_recommended   = FIXED_BUFFER_SIZE;
                tx_body.port.buffer_num                = MIN_BUFFERS;
                tx_body.port.buffer_size               = FIXED_BUFFER_SIZE;
                tx_body.port.userdata                  = 0;

                tx_body.format.type                    = MMAL_ES_TYPE_VIDEO;                    // format fields (all explicit)
                tx_body.format.encoding                = MMAL_ENCODING_H264;
                tx_body.format.encoding_variant        = MMAL_ENCODING_VARIANT_H264_DEFAULT;
                tx_body.format.bitrate                 = 0;
                tx_body.format.flags                   = 0;
                tx_body.format.extradata_size          = 0;
                memset(&tx_body.format.extradata, 0, sizeof(tx_body.format.extradata));
                memset(&tx_body.es, 0, sizeof(tx_body.es));

                
                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)];                                    // 3. Compose and send
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};
                size_t rx_len = 0;

                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    MMALstoreLog ( "\nSet Input Port Failed");                        
                    return false;
                    }

                if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_port_info_set_reply))
                    {
                    MMALstoreLog ( "\nSet Input Port Failed");                        
                    return false;
                    }

                const mmal_msg_port_info_set_reply* reply = 
                    reinterpret_cast<const mmal_msg_port_info_set_reply*>(rx_msg + sizeof(mmal_msg_header));

                MMALstoreLog("\ntype                      ", reply->format.type);
                MMALstoreLog("encoding                  ", reply->format.encoding);
                MMALstoreLog("encoding_variant          ", reply->format.encoding_variant);
                MMALstoreLog("es                        ", reply->format.es);
                MMALstoreLog("bitrate                   ", reply->format.bitrate);
                MMALstoreLog("flags                     ", reply->format.flags);
                MMALstoreLog("extradata_size            ", reply->format.extradata_size);
                MMALstoreLog("extradata                 ", reply->format.extradata);

                MMALstoreLog("video.width               ", reply->es.video.width);
                MMALstoreLog("video.height              ", reply->es.video.height);
                MMALstoreLog("video.crop.x              ", reply->es.video.crop.x);
                MMALstoreLog("video.crop.y              ", reply->es.video.crop.y);
                MMALstoreLog("video.crop.width          ", reply->es.video.crop.width);
                MMALstoreLog("video.crop.height         ", reply->es.video.crop.height);
                MMALstoreLog("video.frame_rate.num      ", reply->es.video.frame_rate.num);
                MMALstoreLog("video.frame_rate.den      ", reply->es.video.frame_rate.den);
                MMALstoreLog("video.par.num             ", reply->es.video.par.num);
                MMALstoreLog("video.par.den             ", reply->es.video.par.den);
                MMALstoreLog("video.color_space         ", reply->es.video.color_space);

                    MMALstoreLog ( "\nSet Input Port Success", reply->format.type);

                return true; //(reply->status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALsetOutputPortFormat  (   )                                                    // mmal_msg_port_info_set
{
                // 1. MMAL header
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic                           = MMAL_MAGIC;
                tx_hdr.type                            = MMAL_MSG_TYPE_PORT_INFO_SET;
                tx_hdr.context                         = NextTransId(m_TransactionId);   // If you want to track transactions, set it here.
                tx_hdr.status                          = 0;
                tx_hdr.padding                         = 0;   // If your struct has this field (show all!)

                // 2. MMAL tx_body (all fields shown)
                mmal_msg_port_info_set tx_body = {};
                tx_body.component_handle               = m_ComponentHandle;
                tx_body.port_type                      = MMAL_PORT_TYPE_OUTPUT;
                tx_body.port_index                     = 0; // output[0]
                memset(&tx_body.port, 0, sizeof(tx_body.port));
                tx_body.port.type                      = MMAL_PORT_TYPE_OUTPUT;
                tx_body.port.index                     = 0;
                tx_body.port.is_enabled                = 1;
                tx_body.port.buffer_num_min            = MIN_BUFFERS;
                tx_body.port.buffer_size_min           = FIXED_BUFFER_SIZE;
                tx_body.port.buffer_alignment_min      = 0;
                tx_body.port.buffer_num_recommended    = MIN_BUFFERS;
                tx_body.port.buffer_size_recommended   = FIXED_BUFFER_SIZE;
                tx_body.port.buffer_num                = MIN_BUFFERS;
                tx_body.port.buffer_size               = FIXED_BUFFER_SIZE;
                tx_body.port.userdata                  = 0;
                // format fields (all explicit)
                tx_body.format.type                    = MMAL_ES_TYPE_VIDEO;
                tx_body.format.encoding                = MMAL_ENCODING_I420; // or NV12 if preferred
                tx_body.format.encoding_variant        = 0;
                tx_body.format.bitrate                 = 0;
                tx_body.format.flags                   = 0;
                tx_body.format.extradata_size          = 0;

                memset(&tx_body.format.extradata, 0, sizeof(tx_body.format.extradata));

                memset(&tx_body.es, 0, sizeof(tx_body.es));

                tx_body.es.video.width                 = m_ResolutionX;
                tx_body.es.video.height                = m_ResolutionY;
                tx_body.es.video.crop.x                = 0;
                tx_body.es.video.crop.y                = 0;
                tx_body.es.video.crop.width            = m_ResolutionX;
                tx_body.es.video.crop.height           = m_ResolutionY;
                tx_body.es.video.frame_rate.num        = 0; // or set as needed
                tx_body.es.video.frame_rate.den        = 1;
                tx_body.es.video.par.num               = 1;
                tx_body.es.video.par.den               = 1;

                // 3. Compose and send
                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)];

                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};

                size_t rx_len = 0;
                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    MMALstoreLog ( "\nSet Output Port Failed");                        
                    return false;
                    }

                if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_port_info_set_reply))
                    {
                    MMALstoreLog ( "\nSet Output Port Failed");                        
                    return false;
                    }

                const mmal_msg_port_info_set_reply* reply = 
                    reinterpret_cast<const mmal_msg_port_info_set_reply*>(rx_msg + sizeof(mmal_msg_header));

                MMALstoreLog("\ntype                      ", reply->format.type);
                MMALstoreLog("encoding                  ", reply->format.encoding);
                MMALstoreLog("encoding_variant          ", reply->format.encoding_variant);
                MMALstoreLog("es                        ", reply->format.es);
                MMALstoreLog("bitrate                   ", reply->format.bitrate);
                MMALstoreLog("flags                     ", reply->format.flags);
                MMALstoreLog("extradata_size            ", reply->format.extradata_size);
                MMALstoreLog("extradata                 ", reply->format.extradata);

                MMALstoreLog("video.width               ", reply->es.video.width);
                MMALstoreLog("video.height              ", reply->es.video.height);
                MMALstoreLog("video.crop.x              ", reply->es.video.crop.x);
                MMALstoreLog("video.crop.y              ", reply->es.video.crop.y);
                MMALstoreLog("video.crop.width          ", reply->es.video.crop.width);
                MMALstoreLog("video.crop.height         ", reply->es.video.crop.height);
                MMALstoreLog("video.frame_rate.num      ", reply->es.video.frame_rate.num);
                MMALstoreLog("video.frame_rate.den      ", reply->es.video.frame_rate.den);
                MMALstoreLog("video.par.num             ", reply->es.video.par.num);
                MMALstoreLog("video.par.den             ", reply->es.video.par.den);
                MMALstoreLog("video.color_space         ", reply->es.video.color_space);

                MMALstoreLog ( "\nSet Output Port Success", reply->format.type);

                return true; //(reply->status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALenableComponent      (   )                                                    // mmal_msg_component_enable
{
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic                           = MMAL_MAGIC;
                tx_hdr.type                            = MMAL_MSG_TYPE_COMPONENT_ENABLE;
                tx_hdr.context                         = NextTransId(m_TransactionId);
                tx_hdr.status                          = 0;
                tx_hdr.padding                         = 0;

                mmal_msg_component_enable tx_body = {};
                tx_body.component_handle             = m_ComponentHandle;

                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)] = {};
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};                                              // u8 rx_msg[sizeof(mmal_msg_component_enable_reply) + sizeof(mmal_msg_header)] = {};

                size_t rx_len = 0;

                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    return false;
                    MMALstoreLog ( "\nEnable Component Failed");                    
                    }
                if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_component_enable_reply))
                    {
                    return false;
                    MMALstoreLog ( "\nEnable Component Failed", m_ComponentHandle);                    
                    }
                const mmal_msg_component_enable_reply *reply =
                    reinterpret_cast<const mmal_msg_component_enable_reply *>(rx_msg + sizeof(mmal_msg_header));
                    MMALstoreLog ( "\nEnable Component Success", m_ComponentHandle);
                
                return true; // (reply->status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALenablePort           (   u32 port_handle, 
                                                            const mmal_msg_port_info_get_reply &PortInfoReply)  // mmal_msg_port_action_port
{
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic   = MMAL_MAGIC;
                tx_hdr.type    = MMAL_MSG_TYPE_PORT_ACTION;
                tx_hdr.context = NextTransId(m_TransactionId);
                tx_hdr.status  = 0;
                tx_hdr.padding = 0;

                mmal_msg_port_action_port tx_body = {};

                tx_body.component_handle = m_ComponentHandle;
                tx_body.port_handle = port_handle;
                tx_body.action = MMAL_MSG_PORT_ACTION_TYPE_ENABLE;

                tx_body.port = PortInfoReply.port;
                tx_body.port.is_enabled = 1;

                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)] = {};
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};

                size_t rx_len = 0;

                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    MMALstoreLog ( "\nEnable Input Port Failed");                        
                    return false;
                    }

                if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_port_action_reply))
                    {
                    MMALstoreLog ( "\nEnable Input Port Failed");                        
                    return false;
                    }

                const mmal_msg_port_action_reply* reply =
                    reinterpret_cast<const mmal_msg_port_action_reply*>(rx_msg + sizeof(mmal_msg_header));

                    MMALstoreLog ( "\nEnable Output Port Success", (u32)port_handle);

                return true; // (reply->status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALsetZeroCopyMode      (   u32 port_handle)                                     // mmal_msg_port_parameter_set
{
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic   = MMAL_MAGIC;
                tx_hdr.type    = MMAL_MSG_TYPE_PORT_PARAMETER_SET;
                tx_hdr.context = NextTransId(m_TransactionId);
                tx_hdr.status  = 0;

                mmal_msg_port_parameter_set tx_body = {};
                tx_body.component_handle = m_ComponentHandle;
                tx_body.port_handle      = port_handle; // 0; // Match by type+index OR the handle!?
            //  tx_body.port_type        = port_type;
            //  tx_body.port_index       = 0; // port_index;
                tx_body.id               = MMAL_PARAMETER_ZERO_COPY;
                tx_body.size             = sizeof(u32); // Size of boolean value only

                memset(tx_body.value, 0, sizeof(tx_body.value));
                tx_body.value[0] = 1; // Only ever set ON

                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)] = {};
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};
                size_t rx_len = 0;
                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    MMALstoreLog ( "\nEnable Zero Copy Input Port Failed");                        
                    return false;
                    }
                if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_port_parameter_set_reply))
                    {
                    MMALstoreLog ( "\nEnable Zero Copy Input Port Failed");                            
                    return false;
                    }

                const mmal_msg_port_parameter_set_reply* reply =
                    reinterpret_cast<const mmal_msg_port_parameter_set_reply*>(rx_msg + sizeof(mmal_msg_header));

                MMALstoreLog ( "\nEnable Zero Copy Input Port Success", (u32)port_handle);

                return true; //(reply->status == MMAL_MSG_STATUS_SUCCESS);
}
void            CH264Decoder::MMALinitialOutputBuffers (   )
{
                // Set up roles so both buffers get queued
                m_AltVCSMHandle = m_VCSMHandleA;
                m_AltBufferHandle = m_OutputBufferHandleA;
                if(!MMALqueueOutputBuffer())
                    {
                    MMALstoreLog ( "\nInitial Output Port Queue Failed", (u32)m_OutputBufferHandleA);    
                    }
                else
                    {
                    MMALstoreLog ( "\nInitial Output Port Queue Success", (u32)m_OutputBufferHandleA);       
                    }
                m_AltVCSMHandle = m_VCSMHandleB;
                m_AltBufferHandle = m_OutputBufferHandleB;
                if(!MMALqueueOutputBuffer())
                    {
                    MMALstoreLog ( "\nInitial Output Port Queue Failed", (u32)m_OutputBufferHandleB);    
                    }
                else
                    {
                    MMALstoreLog ( "\nInitial Output Port Queue Success", (u32)m_OutputBufferHandleB);       
                    }
}

inline bool CH264Decoder::CheckGLError()
{
                GLenum error = glGetError();
                if (error != GL_NO_ERROR)
                    {
                    const char* error_str;
                    
                    switch(error) 
                        {
                        case GL_INVALID_ENUM:               
                            error_str = "GL_INVALID_ENUM"; break;
                        case GL_INVALID_VALUE:              
                            error_str = "GL_INVALID_VALUE"; break;
                        case GL_INVALID_OPERATION:          
                            error_str = "GL_INVALID_OPERATION"; break;
                        case GL_OUT_OF_MEMORY:              
                            error_str = "GL_OUT_OF_MEMORY"; break;
                        case GL_INVALID_FRAMEBUFFER_OPERATION: 
                            error_str = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
                        default:                            
                            error_str = "UNKNOWN_ERROR"; break;
                        }
                    MMALstoreLog (error_str);     
                    return false;
                    }
                return true;    
}

bool            CH264Decoder::MMALcreateTextures       (   )
{
                glGenTextures(1, &m_TextureA);
                // CheckGLError();
                glBindTexture(GL_TEXTURE_2D, m_TextureA);
                // CheckGLError();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                // CheckGLError();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                // CheckGLError();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                // CheckGLError();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                // CheckGLError();
                glBindTexture(GL_TEXTURE_2D, 0);
                // CheckGLError();

                glGenTextures(1, &m_TextureB);
                // CheckGLError();
                glBindTexture(GL_TEXTURE_2D, m_TextureB);
                // CheckGLError();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                // CheckGLError();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                // CheckGLError();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                // CheckGLError();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                // CheckGLError();
                glBindTexture(GL_TEXTURE_2D, 0);
                // CheckGLError();

                return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264 Decoder Runtime Code
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool            CH264Decoder::MMALbufferReady          (   )
{
                // For buffer A:
                if (m_CurrentVCSMHandle == m_VCSMHandleA) 
                    {
                    if (m_EGLimageA) 
                        {
                        eglDestroyImageKHR(m_eglDisplay, m_EGLimageA);
                        m_EGLimageA = EGL_NO_IMAGE_KHR;
                        }
                    struct egl_image_brcm_vcsm_info info = 
                        {
                        .width = m_ResolutionX,
                        .height = m_ResolutionY,
                        .vcsm_handle = m_VCSMHandleA,
                        };
                    m_EGLimageA = eglCreateImageKHR(  m_eglDisplay,
                                                    m_eglContext,
                                                    EGL_IMAGE_BRCM_VCSM,
                                                    (EGLClientBuffer)&info,
                                                    NULL);
                    if (m_EGLimageA == EGL_NO_IMAGE_KHR) 
                        {
                        MMALstoreLog ( "\nm_EGLimageA Failed");                                 
                        return false;
                        }
                    glBindTexture(GL_TEXTURE_2D, m_TextureA);
                    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_EGLimageA);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    }
                // For buffer B:
                else 
                    {
                    if (m_EGLimageB) 
                        {
                        eglDestroyImageKHR(m_eglDisplay, m_EGLimageB);
                        m_EGLimageB = EGL_NO_IMAGE_KHR;
                        }
                    struct egl_image_brcm_vcsm_info info = 
                        {
                        .width = m_ResolutionX,
                        .height = m_ResolutionY,
                        .vcsm_handle = m_VCSMHandleB,
                        };
                    m_EGLimageB = eglCreateImageKHR(  m_eglDisplay,
                                                    m_eglContext,
                                                    EGL_IMAGE_BRCM_VCSM,
                                                    (EGLClientBuffer)&info,
                                                    NULL);
                    if (m_EGLimageB == EGL_NO_IMAGE_KHR) 
                        {
                        MMALstoreLog ( "\nm_EGLimageB Failed");                                 
                        return false;
                        }
                    glBindTexture(GL_TEXTURE_2D, m_TextureB);
                    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_EGLimageB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    }
                MMALstoreLog ( "\neglImage Success");     

                return true;    // please the compiler!!!!
}
bool            CH264Decoder::MMALqueueOutputBuffer    (   )    // mmal_msg_buffer_from_host
{
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic      = MMAL_MAGIC;
                tx_hdr.type       = MMAL_MSG_TYPE_BUFFER_FROM_HOST;
                tx_hdr.context    = NextTransId(m_TransactionId);
                tx_hdr.status     = 0;

                mmal_msg_buffer_from_host tx_body = {};
                tx_body.drvbuf.magic             = MMAL_MAGIC;
                tx_body.drvbuf.component_handle  = m_ComponentHandle;
                tx_body.drvbuf.port_handle       = m_OutputPortHandle;
                tx_body.drvbuf.client_context    = 0;

                tx_body.buffer_header.cmd        = 0;
                tx_body.buffer_header.data       = m_AltVCSMHandle;
                tx_body.buffer_header.alloc_size = m_OutputBufferSize;
                tx_body.buffer_header.length     = 0;
                tx_body.buffer_header.offset     = 0;
                tx_body.buffer_header.flags      = 0;
                tx_body.buffer_header.pts        = MMAL_TIME_UNKNOWN;
                tx_body.buffer_header.dts        = MMAL_TIME_UNKNOWN;

                memset(&tx_body.buffer_header_type_specific, 0, sizeof(tx_body.buffer_header_type_specific));
                tx_body.payload_in_message = 0;
                memset(tx_body.short_data, 0, sizeof(tx_body.short_data));

                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)] = {};
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};
                size_t rx_len = 0;

                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    MMALstoreLog ( "\nOutput Port Queue Failed", (u32)m_OutputPortHandle);                           
                    return false;
                    }
                    MMALstoreLog ( "\nOutput Port Queue Success", (u32)m_OutputPortHandle);                     
                return true;
}
bool            CH264Decoder::MMALqueueInputFrame      (   u32 frame_offset, u32 frame_length)  // mmal_msg_buffer_from_host
{
            // 1. MMAL header
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic   = MMAL_MAGIC;
                tx_hdr.type    = MMAL_MSG_TYPE_BUFFER_FROM_HOST;
                tx_hdr.context = 0;
                tx_hdr.status  = 0;
            // 2. MMAL buffer message tx_body (all fields shown)
                mmal_msg_buffer_from_host tx_body = {};
                // drvbuf
                tx_body.drvbuf.magic            = MMAL_MAGIC;
                tx_body.drvbuf.component_handle = m_ComponentHandle;
                tx_body.drvbuf.port_handle      = m_InputPortHandle;
                tx_body.drvbuf.client_context   = 0;
                // buffer_header
                tx_body.buffer_header.cmd         = 0;
                tx_body.buffer_header.data        = m_InputBufferHandle;         // This is your input VCSM/VC handle
                tx_body.buffer_header.alloc_size  = m_InputBufferSize;         // Full input buffer size (e.g. 80 MB)
                tx_body.buffer_header.length      = frame_length;        // Bytes of this frame
                tx_body.buffer_header.offset      = frame_offset;        // Offset within the buffer to the current frame
                tx_body.buffer_header.flags       = MMAL_BUFFER_HEADER_FLAG_KEYFRAME | MMAL_BUFFER_HEADER_FLAG_FRAME;         // MMAL_BUFFER_HEADER_FLAG_KEYFRAME | MMAL_BUFFER_HEADER_FLAG_FRAME
                tx_body.buffer_header.pts         = MMAL_TIME_UNKNOWN;
                tx_body.buffer_header.dts         = MMAL_TIME_UNKNOWN;

                memset(&tx_body.buffer_header_type_specific, 0, sizeof(tx_body.buffer_header_type_specific));
                tx_body.payload_in_message = 0;
                memset(tx_body.short_data, 0, sizeof(tx_body.short_data));
            // 3. Compose message
                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)];
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));
            // 4. Prepare RX buffer for reply/event (rarely used, but needed for protocol)
                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};
                size_t rx_len = 0;
            // 5. Send (no need to wait for explicit reply, VPU will callback on completion)
                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    MMALstoreLog ( "\nInput Frame Queue Failed", (u32)frame_offset, (u32)frame_length);                           
                    return false;
                    }
                    MMALstoreLog ( "\nInput Frame Queue Success");                         
                return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              END OF FILE
//----------------------------------------------------------------------------------------------------------------------------------------------------