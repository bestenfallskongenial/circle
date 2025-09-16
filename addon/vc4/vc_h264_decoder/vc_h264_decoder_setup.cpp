//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264 Decoder Setup Code
//----------------------------------------------------------------------------------------------------------------------------------------------------
#include "vc_h264_decoder.h"
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool            CH264Decoder::MMALopenService          (   )
{
                SERVICE_CREATION_T params                   = {};
                params.version.version                      = VC_MMAL_VER;
                params.version.version_min                  = VC_MMAL_MIN_VER;
                params.service_id                           = VCHIQ_MAKE_FOURCC('m','m','a','l');
                params.connection                           = m_Connection;                             // VCHI connection ( returned from bcmhost´s vc_host_get_vchi_state )
                params.rx_fifo_size                         = 0;
                params.tx_fifo_size                         = 0;
                params.callback                             = MMALcallBack;
                params.callback_param                       = &m_VCOSevent;
                params.want_unaligned_bulk_rx               = 0;
                params.want_unaligned_bulk_tx               = 0;
                params.want_crc                             = 0;

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

                int rc = vchi_service_open(m_VCHIInstance, &params, &m_ServiceHandle);
                if ( rc != 0)
                    {
                    MMALstoreLog ("\nFailed to open MMAL service!", (u32)m_ServiceHandle);
                    }
                if ( rc == 0)
                    {
                    MMALstoreLog ("\nMMALservice opened!", (u32)m_ServiceHandle);
                    }
                return (rc == 0);
}
bool            CH264Decoder::MMALcreateComponent      (   )                                                    // mmal_msg_component_create    // expects a pointer therefore CreateComponent(&m_My_private_Member);
{
                mmal_msg_header tx_hdr                  = {};                                                    // 1. MMAL header: all fields shown

                tx_hdr.magic                            = MMAL_MAGIC;
                tx_hdr.type                             = MMAL_MSG_TYPE_COMPONENT_CREATE;
                tx_hdr.control_service                  = 0;         // *** NEW TO MATCH THE DEFINITION!
                tx_hdr.context                          = NextTransId(m_TransactionId);          // If you want to track transactions, set it here.
                tx_hdr.status                           = 0;
                tx_hdr.padding                          = 0;                                     // If your struct has this field (show all!)

                mmal_msg_component_create tx_body       = {};                                         // 2. MMAL tx_body: all fields shown

                tx_body.client_component                = 0;
                memset(tx_body.name, 0, sizeof(tx_body.name));
                strncpy(tx_body.name, "ril.video_decode", sizeof(tx_body.name) - 1);         // or "ril.video_decode"? sure about the -1 here?
                tx_body.pid                             = 0;
            //  tx_body.reserved0                       = 0;                                     // If present
            //  tx_body.reserved1                       = 0;                                     // If present

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

                m_ComponentHandle                   = reply->component_handle;   // Direct member assignment
                m_NumInputs                         = reply->input_num;
                m_NumOutputs                        = reply->output_num;
                m_NumClock                          = reply->clock_num;

                MMALstoreLog ( "\nCreate Component Success!     ", (u32)m_ComponentHandle);
                MMALstoreLog ( "\nNumber of In / Output Ports   ", (u32)m_NumInputs, (u32)m_NumOutputs);
                MMALstoreLog ( "\nNumber of Clock Ports         ", (u32)m_NumClock);
                
                return /*true;*/ (reply->status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALgetPortInfo          (    u32 port_type, u32 &port_handle, mmal_msg_port_info_get_reply &PortInfoReply)        // mmal_msg_port_info_get
{
                mmal_msg_header tx_hdr              = {};
                tx_hdr.magic                        = MMAL_MAGIC;
                tx_hdr.type                         = MMAL_MSG_TYPE_PORT_INFO_GET;
                tx_hdr.control_service              = 0;         // *** NEW TO MATCH THE DEFINITION!                
                tx_hdr.context                      = NextTransId(m_TransactionId);
                tx_hdr.status                       = 0;
                tx_hdr.padding                      = 0;

                mmal_msg_port_info_get tx_body      = {};
                tx_body.component_handle            = m_ComponentHandle;
                tx_body.port_type                   = port_type;
                tx_body.index                       = 0; //port_index;

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

                MMALstoreLog("status                        ", PortInfoReply.status);                           /* enum mmal_msg_status */
                MMALstoreLog("component_handle             >", PortInfoReply.component_handle);                 /* component handle port is associated with */
                MMALstoreLog("port_type                    >", PortInfoReply.port_type);                        /* enum mmal_msg_port_type */
                MMALstoreLog("port_index                   >", PortInfoReply.port_index);                       /* port indexed in query */
                MMALstoreLog("found                         ", PortInfoReply.found);                            /* unused */
                MMALstoreLog("port_handle                  >", PortInfoReply.port_handle);                      /* Handle to use for this port */
                // mmal_port is a struct; log each field individually:
                MMALstoreLog("port.priv                     ", PortInfoReply.port.priv);                        /* Private member used by the framework */
                MMALstoreLog("port.name                     ", PortInfoReply.port.name);                        /* Port name. Used for debugging purposes (RO) */
                MMALstoreLog("port.type                     ", PortInfoReply.port.type);                        /* Type of the port (RO) enum mmal_port_type */
                MMALstoreLog("port.index                    ", PortInfoReply.port.index);                       /* Index of the port in its type list (RO) */
                MMALstoreLog("port.index_all                ", PortInfoReply.port.index_all);                   /* Index of the port in the list of all ports (RO) */
                MMALstoreLog("port.is_enabled              >", PortInfoReply.port.is_enabled);                  /* Indicates whether the port is enabled or not (RO) */
                MMALstoreLog("port.format                   ", PortInfoReply.port.format);                      /* Format of the elementary stream */
                MMALstoreLog("port.buffer_num_min          !", PortInfoReply.port.buffer_num_min);              /* Minimum number of buffers the port requires (RO).  This is set by the component. */
                MMALstoreLog("port.buffer_size_min         !", PortInfoReply.port.buffer_size_min);             /* Minimum size of buffers the port requires (RO).  This is set by the component. */
                MMALstoreLog("port.buffer_alignment_min    !", PortInfoReply.port.buffer_alignment_min);        /* Minimum alignment requirement for the buffers (RO). A value of zero means no special alignment requirements. This is set by the component. */
                MMALstoreLog("port.buffer_num_recommended  !", PortInfoReply.port.buffer_num_recommended);      /* Number of buffers the port recommends for optimal performance (RO).  A value of zero means no special recommendation.  This is set by the component. */
                MMALstoreLog("port.buffer_size_recommended !", PortInfoReply.port.buffer_size_recommended);     /* Size of buffers the port recommends for optimal performance (RO).  A value of zero means no special recommendation.  This is set by the component. */
                MMALstoreLog("port.buffer_num           *I/O", PortInfoReply.port.buffer_num);                  /* Actual number of buffers the port will use.  This is set by the client. */
                MMALstoreLog("port.buffer_size          *I/O", PortInfoReply.port.buffer_size);                 /* Actual maximum size of the buffers that will be sent to the port. This is set by the client. */
                MMALstoreLog("port.component                ", PortInfoReply.port.component);                   /* Component this port belongs to (Read Only) */
                MMALstoreLog("port.userdata                *", PortInfoReply.port.userdata);                    /* Field reserved for use by the client */
                MMALstoreLog("port.capabilities             ", PortInfoReply.port.capabilities);                /* Flags describing the capabilities of a  port (RO).  Bitwise combination of \ref portcapabilities "Port capabilities"  values. */
                                                                                                            /* Remote definition of an elementary stream format (MMAL_ES_FORMAT_T) */
                MMALstoreLog("format.type                  *", PortInfoReply.format.type);                      /* enum mmal_es_type */
                MMALstoreLog("format.encoding             *I", PortInfoReply.format.encoding);                  /* FourCC specifying encoding of the elementary stream. */
                MMALstoreLog("format.encoding_variant     *I", PortInfoReply.format.encoding_variant);          /* FourCC specifying the specific encoding variant of the elementary stream.  */
                MMALstoreLog("format.es                     ", PortInfoReply.format.es);                        /* Type specific information for the elementary stream */
                MMALstoreLog("format.bitrate               *", PortInfoReply.format.bitrate);                   /* Bitrate in bits per second */
                MMALstoreLog("format.flags                 *", PortInfoReply.format.flags);                     /* Flags describing properties of the elementary stream. */
                MMALstoreLog("format.extradata_size       **", PortInfoReply.format.extradata_size);            /* Size of the codec specific data */
                MMALstoreLog("format.extradata            **", PortInfoReply.format.extradata);                 /* Codec specific data */
                // mmal_es_specific_format MMAL_ES_TYPE_VIDEO -> struct mmal_video_format:
                MMALstoreLog("es.video.width              *O", PortInfoReply.es.video.width);                   /* Width of frame in pixels */
                MMALstoreLog("es.video.height             *O", PortInfoReply.es.video.height);                  /* Height of frame in rows of pixels */
                MMALstoreLog("es.video.crop.x             *O", PortInfoReply.es.video.crop.x);                  /* Visible region of the frame */ /**< x coordinate (from left) */
                MMALstoreLog("es.video.crop.y             *O", PortInfoReply.es.video.crop.y);                  /* Visible region of the frame */ /**< y coordinate (from top) */
                MMALstoreLog("es.video.crop.width         *O", PortInfoReply.es.video.crop.width);              /* Visible region of the frame */ /**< width */
                MMALstoreLog("es.video.crop.height        *O", PortInfoReply.es.video.crop.height);             /* Visible region of the frame */ /**< height */
                MMALstoreLog("es.video.frame_rate.num      *", PortInfoReply.es.video.frame_rate.num);          /* Frame rate */ /**< Numerator */
                MMALstoreLog("es.video.frame_rate.den      *", PortInfoReply.es.video.frame_rate.den);          /* Frame rate */ /**< Denominator */
                MMALstoreLog("es.video.par.num             *", PortInfoReply.es.video.par.num);                 /* Pixel aspect ratio */ /**< Numerator */
                MMALstoreLog("es.video.par.den             *", PortInfoReply.es.video.par.den);                 /* Pixel aspect ratio */ /**< Denominator */
                MMALstoreLog("es.video.color_space          ", PortInfoReply.es.video.color_space);             /* FourCC specifying the color space of the video stream. See the * MmalColorSpace "pre-defined color spaces" for some examples. */
                // extradata is a byte array
                
                MMALstoreMsg(PortInfoReply.extradata, PortInfoReply.format.extradata_size, "extradata");
                // Assign port handle to out reference
                port_handle                         = PortInfoReply.port_handle;
                   
                // Optionally also store in your private member here if needed:
                // m_PortHandleIn = PortInfoReply.port_handle; // or similar
                MMALstoreLog ( "\nGet Port Info Success!", (u32)PortInfoReply.port_handle);      // & really?
                 
                return /*true;*/(PortInfoReply.status == MMAL_MSG_STATUS_SUCCESS);
}
void            CH264Decoder::MMALsetInputPortFormat( const mmal_msg_port_info_get_reply &OriginalPortInfo, mmal_msg_port_info_get_reply &WorkingCopy)
{
                // 1. Copy full original struct (includes all nested fields)
                WorkingCopy = OriginalPortInfo;

                // 2. Modify only writable fields for the input port
                WorkingCopy.port.buffer_num                     = NUMBER_INPUTBUFFER;      // ≥ OriginalPortInfo.port.buffer_num_min
                WorkingCopy.port.buffer_size                    = m_InputBufferSize;   // ≥ OriginalPortInfo.port.buffer_size_min

                // INPUT bitstream needs only codec type/variant
                WorkingCopy.format.encoding                     = MMAL_ENCODING_H264;  
                WorkingCopy.format.encoding_variant             = MMAL_ENCODING_VARIANT_H264_DEFAULT;

                // Width/height/crop remain untouched (decoder extracts from SPS/PPS)
                WorkingCopy.es.video.width                      = m_ResolutionX;
                WorkingCopy.es.video.height                     = m_ResolutionY;
                WorkingCopy.es.video.crop.x                     = 0;
                WorkingCopy.es.video.crop.y                     = 0;
                WorkingCopy.es.video.crop.width                 = m_ResolutionX;
                WorkingCopy.es.video.crop.height                = m_ResolutionY;    
}
void            CH264Decoder::MMALsetOutputPortFormat( const mmal_msg_port_info_get_reply &OriginalPortInfo, mmal_msg_port_info_get_reply &WorkingCopy)
{
                // 1. Copy full original struct (includes all nested fields)
                WorkingCopy = OriginalPortInfo;

                // 2. Modify only writable fields for the output port
                WorkingCopy.port.buffer_num                     = NUMBER_OUTPUTBUFFER;
                WorkingCopy.port.buffer_size                    = m_OutputBufferSize;

                WorkingCopy.format.encoding                     = MMAL_ENCODING_I420;

                WorkingCopy.es.video.width                      = m_ResolutionX;
                WorkingCopy.es.video.height                     = m_ResolutionY;
                WorkingCopy.es.video.crop.x                     = 0;
                WorkingCopy.es.video.crop.y                     = 0;
                WorkingCopy.es.video.crop.width                 = m_ResolutionX;
                WorkingCopy.es.video.crop.height                = m_ResolutionY;

                // leave all other fields from GET untouched
}

bool            CH264Decoder::SendPortWorkingCopy(u32 port_type, const mmal_msg_port_info_get_reply &WorkingCopy)
{
                // 1) Header
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic           = MMAL_MAGIC;
                tx_hdr.type            = MMAL_MSG_TYPE_PORT_INFO_SET;
                tx_hdr.control_service = 0;
                tx_hdr.context         = NextTransId(m_TransactionId);
                tx_hdr.status          = 0;
                tx_hdr.padding         = 0;

                // 2) Body from working copy
                mmal_msg_port_info_set tx_body = {};
                tx_body.component_handle       = m_ComponentHandle;
                tx_body.port_type              = port_type; // INPUT or OUTPUT
                tx_body.port_index             = 0;

                memcpy(&tx_body.port,   &WorkingCopy.port,   sizeof(WorkingCopy.port));
                memcpy(&tx_body.format, &WorkingCopy.format, sizeof(WorkingCopy.format));
                memcpy(&tx_body.es,     &WorkingCopy.es,     sizeof(WorkingCopy.es));

                // 3) TX buffer
                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)] = {};
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                // 4) Send + wait
                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};
                size_t rx_len = 0;

                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                {
                    MMALstoreLog("\nSend Port Copy Failed");
                    return false;
                }

                if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_port_info_set_reply))
                {
                    MMALstoreLog("\nSend Port Copy Failed");
                    return false;
                }

                mmal_msg_header *rx_hdr = reinterpret_cast<mmal_msg_header *>(rx_msg);
                if (rx_hdr->type != MMAL_MSG_TYPE_PORT_INFO_SET)
                {
                    MMALstoreLog("\nSend Port Copy Failed");
                    return false;
                }

                mmal_msg_port_info_set_reply reply = {};
                memcpy(&reply, rx_msg + sizeof(mmal_msg_header), sizeof(reply));

                // 5) Log ALL fields from reply
                MMALstoreLog ( "\n----------------------------------------------------------------");                 
                MMALstoreLog ( "mmal_msg_port_info_set_reply - port", port_type);
                MMALstoreLog ( "\n----------------------------------------------------------------");  

                MMALstoreLog("status                        ", reply.status);
                MMALstoreLog("component_handle             >", reply.component_handle);
                MMALstoreLog("port_type                    >", reply.port_type);
                MMALstoreLog("index                        >", reply.index);
                MMALstoreLog("found                         ", reply.found);
                MMALstoreLog("port_handle                  >", reply.port_handle);

                MMALstoreLog("port.priv                     ", reply.port.priv);
                MMALstoreLog("port.name                     ", reply.port.name);
                MMALstoreLog("port.type                     ", reply.port.type);
                MMALstoreLog("port.index                    ", reply.port.index);
                MMALstoreLog("port.index_all                ", reply.port.index_all);
                MMALstoreLog("port.is_enabled              >", reply.port.is_enabled);
                MMALstoreLog("port.format                   ", reply.port.format);
                MMALstoreLog("port.buffer_num_min          !", reply.port.buffer_num_min);
                MMALstoreLog("port.buffer_size_min         !", reply.port.buffer_size_min);
                MMALstoreLog("port.buffer_alignment_min    !", reply.port.buffer_alignment_min);
                MMALstoreLog("port.buffer_num_recommended  !", reply.port.buffer_num_recommended);
                MMALstoreLog("port.buffer_size_recommended !", reply.port.buffer_size_recommended);
                MMALstoreLog("port.buffer_num           *I/O", reply.port.buffer_num);
                MMALstoreLog("port.buffer_size          *I/O", reply.port.buffer_size);
                MMALstoreLog("port.component                ", reply.port.component);
                MMALstoreLog("port.userdata                *", reply.port.userdata);
                MMALstoreLog("port.capabilities             ", reply.port.capabilities);

                MMALstoreLog("format.type                  *", reply.format.type);
                MMALstoreLog("format.encoding             *I", reply.format.encoding);
                MMALstoreLog("format.encoding_variant     *I", reply.format.encoding_variant);
                MMALstoreLog("format.es                     ", reply.format.es);
                MMALstoreLog("format.bitrate               *", reply.format.bitrate);
                MMALstoreLog("format.flags                 *", reply.format.flags);
                MMALstoreLog("format.extradata_size       **", reply.format.extradata_size);
                MMALstoreLog("format.extradata            **", reply.format.extradata);

                MMALstoreLog("es.video.width              *O", reply.es.video.width);
                MMALstoreLog("es.video.height             *O", reply.es.video.height);
                MMALstoreLog("es.video.crop.x             *O", reply.es.video.crop.x);
                MMALstoreLog("es.video.crop.y             *O", reply.es.video.crop.y);
                MMALstoreLog("es.video.crop.width         *O", reply.es.video.crop.width);
                MMALstoreLog("es.video.crop.height        *O", reply.es.video.crop.height);
                MMALstoreLog("es.video.frame_rate.num      *", reply.es.video.frame_rate.num);
                MMALstoreLog("es.video.frame_rate.den      *", reply.es.video.frame_rate.den);
                MMALstoreLog("es.video.par.num             *", reply.es.video.par.num);
                MMALstoreLog("es.video.par.den             *", reply.es.video.par.den);
                MMALstoreLog("es.video.color_space          ", reply.es.video.color_space);

                // dump extradata bytes according to size in reply.format.extradata_size
                MMALstoreMsg(reply.extradata, reply.format.extradata_size, "extradata");

                MMALstoreLog("\nSend Port Copy Success", reply.port_handle);
                return (reply.status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALenableComponent      (   )                                                    // mmal_msg_component_enable
{
                mmal_msg_header tx_hdr              = {};
                tx_hdr.magic                        = MMAL_MAGIC;
                tx_hdr.type                         = MMAL_MSG_TYPE_COMPONENT_ENABLE;
                tx_hdr.control_service              = 0;         // *** NEW TO MATCH THE DEFINITION!                
                tx_hdr.context                      = NextTransId(m_TransactionId);
                tx_hdr.status                       = 0;
                tx_hdr.padding                      = 0;

                mmal_msg_component_enable tx_body   = {};
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
//              const mmal_msg_component_enable_reply *reply =
//                  reinterpret_cast<const mmal_msg_component_enable_reply *>(rx_msg + sizeof(mmal_msg_header));
                    MMALstoreLog ( "\nEnable Component Success", m_ComponentHandle);
                
                return true; // (reply->status == MMAL_MSG_STATUS_SUCCESS);
}
bool            CH264Decoder::MMALenablePort(u32 port_handle, const mmal_msg_port_info_get_reply &port_info)
{
                // Prepare MMAL header
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic   = MMAL_MAGIC;
                tx_hdr.type    = MMAL_MSG_TYPE_PORT_ACTION;
                tx_hdr.control_service                 = 0;         // *** NEW TO MATCH THE DEFINITION!                
                tx_hdr.context = NextTransId(m_TransactionId);
                tx_hdr.status                          = 0;         // *** NEW TO MATCH THE DEFINITION!
                tx_hdr.padding                         = 0;         // *** NEW TO MATCH THE DEFINITION!

                // Use full port struct for enable
                mmal_msg_port_action_port tx_body = {};
                tx_body.component_handle         = m_ComponentHandle;
                tx_body.port_handle              = port_handle;
                tx_body.action                   = MMAL_MSG_PORT_ACTION_TYPE_ENABLE;
                tx_body.port                     = port_info.port;

                // Combine header + body
                u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)];
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &tx_body, sizeof(tx_body));

                // RX buffer
                u8 rx_msg[MMAL_MSG_MAX_SIZE] = {};
                size_t rx_len = 0;

                // Send + wait
                if (!MMALsendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rx_len))
                    {
                    MMALstoreLog("Enable Port Failed", port_handle);
                    return false;
                    }

                // Parse reply
                const mmal_msg_header *rx_hdr = reinterpret_cast<const mmal_msg_header *>(rx_msg);
                if (rx_hdr->type != MMAL_MSG_TYPE_PORT_ACTION)
                    {
                    MMALstoreLog("Enable Port Failed", port_handle);
                    return false;
                    }

                const mmal_msg_port_action_reply *reply =
                    reinterpret_cast<const mmal_msg_port_action_reply *>(rx_msg + sizeof(mmal_msg_header));

                if (reply->status != MMAL_MSG_STATUS_SUCCESS)
                    {
                    MMALstoreLog("Enable Port Failed", port_handle);
                    return false;
                    }

                MMALstoreLog("Enable Port Success", port_handle);
                return true;
}
bool            CH264Decoder::MMALsetZeroCopyMode      (   u32 port_handle)                                     // mmal_msg_port_parameter_set
{
                mmal_msg_header tx_hdr = {};
                tx_hdr.magic   = MMAL_MAGIC;
                tx_hdr.type    = MMAL_MSG_TYPE_PORT_PARAMETER_SET;
                tx_hdr.control_service                 = 0;         // *** NEW TO MATCH THE DEFINITION!                    
                tx_hdr.context = NextTransId(m_TransactionId);
                tx_hdr.status  = 0;
                tx_hdr.padding                         = 0;         // *** NEW TO MATCH THE DEFINITION!                

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

//              const mmal_msg_port_parameter_set_reply* reply =
//              reinterpret_cast<const mmal_msg_port_parameter_set_reply*>(rx_msg + sizeof(mmal_msg_header));

                MMALstoreLog ( "\nEnable Zero Copy Input Port Success", (u32)port_handle);

                return true; //(reply->status == MMAL_MSG_STATUS_SUCCESS);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------