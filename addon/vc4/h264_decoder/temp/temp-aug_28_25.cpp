struct mmal_msg_buffer_from_host_wire32 {
    mmal_driver_buffer drvbuf;     // you set these fields → keep as struct (16)

    u8  drvbuf_ref[16];            // was mmal_driver_buffer (unused → pad as bytes)

    mmal_buffer_header_wire32 buffer_header; // you patch these → keep (56)

    u8  type_specific[40];         // was mmal_buffer_header_type_specific (unused)

    s32 is_zero_copy;              // 4
    s32 has_reference;             // 4
    u32 payload_in_message;        // 4

    u8  short_data[128];           // unchanged, stays zero
}; // total: 16 + 16 + 56 + 40 + 4 + 4 + 4 + 128 = 268 bytes

class CH264Decoder {
    // ...
private:
    mmal_msg_buffer_from_host_wire32 mBodyOut{};
    mmal_msg_buffer_from_host_wire32 mBodyIn{};

    void InitBodies();

public:
    bool MMALqueueOutputBuffer(mmal_msg_buffer_from_host_wire32& body, u32 vc_handle, u32 alloc_size);
    bool MMALqueueInputBuffer (mmal_msg_buffer_from_host_wire32& body, u32 frame_offset, u32 frame_length);
    // ...
};


void CH264Decoder::InitBodies()
{
    // OUTPUT constants
    mBodyOut = {};
    mBodyOut.drvbuf.magic            = MMAL_MAGIC;
    mBodyOut.drvbuf.component_handle = m_ComponentHandle;
    mBodyOut.drvbuf.port_handle      = m_OutputPortHandle;
    mBodyOut.buffer_header.pts_lo    = 0;
    mBodyOut.buffer_header.pts_hi    = 0x80000000;   // MMAL_TIME_UNKNOWN
    mBodyOut.buffer_header.dts_lo    = 0;
    mBodyOut.buffer_header.dts_hi    = 0x80000000;
    mBodyOut.is_zero_copy            = 1;
    mBodyOut.has_reference           = 0;
    mBodyOut.payload_in_message      = 0;

    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(mBodyOut));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(drvbuf));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(drvbuf_ref));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(buffer_header));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(type_specific));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(short_data));

    // INPUT constants
    mBodyIn = {};
    mBodyIn.drvbuf.magic             = MMAL_MAGIC;
    mBodyIn.drvbuf.component_handle  = m_ComponentHandle;
    mBodyIn.drvbuf.port_handle       = m_InputPortHandle;
    mBodyIn.buffer_header.data       = m_InputBufferHandle;
    mBodyIn.buffer_header.alloc_size = m_InputBufferSize;
    mBodyIn.buffer_header.pts_lo     = 0;
    mBodyIn.buffer_header.pts_hi     = 0x80000000;
    mBodyIn.buffer_header.dts_lo     = 0;
    mBodyIn.buffer_header.dts_hi     = 0x80000000;
    mBodyIn.is_zero_copy             = 1;
    mBodyIn.has_reference            = 0;
    mBodyIn.payload_in_message       = 0;

    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(mBodyIn));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(drvbuf));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(drvbuf_ref));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(buffer_header));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(type_specific));
    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(short_data));    

    if ( (sizeof(drvbuf)+sizeof(drvbuf_rev)+sizeof(buffer_header)+sizeof(type_specific)+sizeof(short_data)) != 268 ) 
        {
        MMALstoreLog("\nBUFFER FROM HOST MSG false formated", (u32)sizeof(short_data));    
        return false;
        }
    return true;
}

bool CH264Decoder::MMALqueueOutputBuffer(mmal_msg_buffer_from_host_wire32& body, u32 vc_handle, u32 alloc_size)
{
    mmal_msg_header hdr = {};
    hdr.magic   = MMAL_MAGIC;
    hdr.type    = MMAL_MSG_TYPE_BUFFER_FROM_HOST;
    hdr.context = NextTransId(m_TransactionId);

    body.drvbuf.client_context    = hdr.context;
    body.buffer_header.data       = vc_handle;
    body.buffer_header.alloc_size = alloc_size;
    body.buffer_header.length     = 0;
    body.buffer_header.offset     = 0;
    body.buffer_header.flags      = 0;

    u8 msg[sizeof(hdr) + sizeof(body)];
    memcpy(msg, &hdr, sizeof(hdr));
    memcpy(msg + sizeof(hdr), &body, sizeof(body));

    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(msg));   // log the size of the message must be 268 + 24 = 292 dec / 124 hex
    MMALstoreMsg(msg, (u32)sizeof(msg), "QueueOutputBuffer");   // log the complete tx message

    return vchi_msg_queue(m_ServiceHandle, msg, (u32)sizeof(msg), VCHI_FLAGS_BLOCK_UNTIL_QUEUED, nullptr) == 0;
}

bool CH264Decoder::MMALqueueInputBuffer(mmal_msg_buffer_from_host_wire32& body, u32 frame_offset, u32 frame_length)
{
//  if (frame_offset + frame_length > m_InputBufferSize) return false;

    mmal_msg_header hdr = {};
    hdr.magic   = MMAL_MAGIC;
    hdr.type    = MMAL_MSG_TYPE_BUFFER_FROM_HOST;
    hdr.context = NextTransId(m_TransactionId);

    body.drvbuf.client_context  = hdr.context;
    body.buffer_header.offset   = frame_offset;
    body.buffer_header.length   = frame_length;
    u32 flags = MMAL_BUFFER_HEADER_FLAG_FRAME | ;
//  if (!m_FirstFrameQueued) flags |= MMAL_BUFFER_HEADER_FLAG_DISCONTINUITY;
    body.buffer_header.flags    = flags;

    u8 msg[sizeof(hdr) + sizeof(body)];
    memcpy(msg, &hdr, sizeof(hdr));
    memcpy(msg + sizeof(hdr), &body, sizeof(body));

    MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(msg));   // log the size of the message must be 268 + 24 = 292 dec / 124 hex
    MMALstoreMsg(msg, (u32)sizeof(msg), "QueueInputBuffer");    // log the complete tx message

    return vchi_msg_queue(m_ServiceHandle, msg, (u32)sizeof(msg), VCHI_FLAGS_BLOCK_UNTIL_QUEUED, nullptr) == 0;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool CH264Decoder::MMALqueueOutputBuffer(u32 vc_handle, u32 alloc_size)
{
    mmal_msg_header tx_hdr = {};
    tx_hdr.magic           = MMAL_MAGIC;
    tx_hdr.type            = MMAL_MSG_TYPE_BUFFER_FROM_HOST;
    tx_hdr.control_service = 0;
    tx_hdr.context         = NextTransId(m_TransactionId);
    tx_hdr.status          = 0;
    tx_hdr.padding         = 0;

    mmal_msg_buffer_from_host tx_body = {};
    tx_body.drvbuf.magic              = MMAL_MAGIC;
    tx_body.drvbuf.component_handle   = m_ComponentHandle;
    tx_body.drvbuf.port_handle        = m_OutputPortHandle;
    tx_body.drvbuf.client_context     = tx_hdr.context;

    // buffer header
    tx_body.buffer_header.cmd         = 0;
    tx_body.buffer_header.data        = vc_handle;        // VC/VCSM handle
    tx_body.buffer_header.alloc_size  = alloc_size;       // capacity (bytes)
    tx_body.buffer_header.length      = 0;                // output queue: 0
    tx_body.buffer_header.offset      = 0;
    tx_body.buffer_header.flags       = 0;
    tx_body.buffer_header.pts         = MMAL_TIME_UNKNOWN;
    tx_body.buffer_header.dts         = MMAL_TIME_UNKNOWN;

    // type-specific zeroed
    tx_body.is_zero_copy              = 1;
    tx_body.has_reference             = 0;
    tx_body.payload_in_message        = 0;
    // short_data[128] already zeroed

    // ---- exact on‑wire sizes ----
    const u32 body_wire =
        (u32)offsetof(mmal_msg_buffer_from_host, short_data) + MMAL_VC_SHORT_DATA; // 268
    const u32 msg_wire  = (u32)sizeof(mmal_msg_header) + body_wire;                // 292

    // build TX buffer (copy only the wire length of the body)
    u8 tx_msg[sizeof(tx_hdr) + sizeof(tx_body)];
    memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
    memcpy(tx_msg + sizeof(tx_hdr), &tx_body, body_wire);

    MMALstoreMsg(tx_msg, msg_wire, "QueueOutputBuffer");

    if (vchi_msg_queue(m_ServiceHandle, tx_msg, msg_wire,
                       VCHI_FLAGS_BLOCK_UNTIL_QUEUED, nullptr) != 0)
    {
        MMALstoreLog("\nOutput Port Queue Failed", (u32)vc_handle);
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool CH264Decoder::MMALsetPortParameterBufferAvailable(u32 component_handle, u32 port_handle)
{
    mmal_msg tx_msg = {};
    mmal_msg rx_msg = {};
    size_t   rx_len = 0;

    // --- MMAL header ---
    tx_msg.h.magic           = MMAL_MAGIC;
    tx_msg.h.type            = MMAL_MSG_TYPE_PORT_PARAMETER_SET; // 0x0B
    tx_msg.h.context         = NextTransId(m_TransactionId);
    tx_msg.h.status          = 0;
    tx_msg.h.control_service = 0;
    tx_msg.h.padding         = 0;

    // --- Payload: mmal_msg_port_parameter_set ---
    tx_msg.u.port_parameter_set.component_handle = component_handle;
    tx_msg.u.port_parameter_set.port_handle      = port_handle;

    // Fill parameter header
    tx_msg.u.port_parameter_set.param.hdr.id     = MMAL_PARAMETER_BUFFER_AVAILABLE;
    tx_msg.u.port_parameter_set.param.hdr.size   = sizeof(mmal_parameter_buffer_available);

    // Set the parameter struct (enable = 1)
    tx_msg.u.port_parameter_set.param.enable     = 1;

    // --- Send + wait for reply ---
    if (!MMALsendAndWait(&tx_msg,
                         sizeof(tx_msg.h) + sizeof(tx_msg.u.port_parameter_set),
                         &rx_msg, sizeof(rx_msg), &rx_len))
    {
        MMALstoreLog("MMALsetPortParameterBufferAvailable: send failed");
        return false;
    }

    if (rx_len < sizeof(mmal_msg_header) + sizeof(mmal_msg_port_parameter_set_reply))
    {
        MMALstoreLog("MMALsetPortParameterBufferAvailable: short reply");
        return false;
    }

    if (rx_msg.h.status != MMAL_MSG_STATUS_SUCCESS)
    {
        MMALstoreLog("MMALsetPortParameterBufferAvailable: VPU error", rx_msg.h.status);
        return false;
    }

    return true;
}

VCHI_SERVICE_HANDLE_T m_InputPortServiceHandle = 0;
VCHI_SERVICE_HANDLE_T m_OutputPortServiceHandle = 0;
VCOS_EVENT_T          m_InputPortEvent;
VCOS_EVENT_T          m_OutputPortEvent;


bool            CH264Decoder::MMALinitCommandEvents            (   )
{
                if (vcos_event_create(&m_VCOSevent, "MMALCMD") != VCOS_SUCCESS)
                    {
                    MMALstoreLog ( "\nVCOS Command Event Init Failed!   ");                        
                    return false;
                    }
                MMALstoreLog ( "\nVCOS Command Event Init Success!  ", (u32)&m_VCOSevent);                    
                return true;    
}

bool            CH264Decoder::MMALinitInPortEvents            (   )
{
                if (vcos_event_create(&m_InputPortEvent, "MMALIN") != VCOS_SUCCESS)
                    {
                    MMALstoreLog ( "\nVCOS Input Port Event Init Failed!   ");                        
                    return false;
                    }
                MMALstoreLog ( "\nVCOS Input Port Event Init Success!  ", (u32)&m_VCOSevent);                    
                return true;    
}

bool            CH264Decoder::MMALinitPortOutEvents            (   )
{
                if (vcos_event_create(&m_OutputPortEvent, "MMALOUT") != VCOS_SUCCESS)
                    {
                    MMALstoreLog ( "\nVCOS Output Port Event Init Failed!   ");                        
                    return false;
                    }
                MMALstoreLog ( "\nVCOS Output Port Event Init Success!  ", (u32)&m_VCOSevent);                    
                return true;    
}

bool CH264Decoder::MMALopenPortService(VCHI_SERVICE_HANDLE_T* handle, VCHI_CALLBACK_T callback, void* param)
{
    SERVICE_CREATION_T params = {};
    params.version.version              = VC_MMAL_VER;
    params.version.version_min         = VC_MMAL_MIN_VER;
    params.service_id                  = VCHIQ_MAKE_FOURCC('m','m','a','l');
    params.connection                  = m_Connection;
    params.rx_fifo_size                = 0;
    params.tx_fifo_size                = 0;
    params.callback                    = callback;
    params.callback_param              = param;
    params.want_unaligned_bulk_rx     = 0;
    params.want_unaligned_bulk_tx     = 0;
    params.want_crc                   = 0;

    return (vchi_service_open(m_VCHIInstance, &params, handle) == 0);
}

void CH264Decoder::Input_callback(void* param, const VCHI_CALLBACK_REASON_T reason, void* msg)
{
    if (reason == VCHI_CALLBACK_MSG_AVAILABLE && param)
        vcos_event_signal((VCOS_EVENT_T*)param);
}
void CH264Decoder::Output_callback(void* param, const VCHI_CALLBACK_REASON_T reason, void* msg)
{
    if (reason == VCHI_CALLBACK_MSG_AVAILABLE && param)
        vcos_event_signal((VCOS_EVENT_T*)param);
}


how to call:


MMALopenPortService(&m_InputPortServiceHandle, Input_callback, &m_InputPortEvent);
MMALopenPortService(&m_OutputPortServiceHandle, Output_callback, &m_OutputPortEvent);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool CH264Decoder::SendPortWorkingCopy(u32 port_type, const mmal_msg_port_info_get_reply &WorkingCopy)
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


void CH264Decoder::InitBodies()
{
    // OUTPUT constants
    mBodyOut = {};
    mBodyOut.drvbuf.magic            = MMAL_MAGIC;
    mBodyOut.drvbuf.component_handle = m_ComponentHandle;
    mBodyOut.drvbuf.port_handle      = m_OutputPortHandle;
    mBodyOut.buffer_header.pts_lo    = 0;
    mBodyOut.buffer_header.pts_hi    = 0x80000000;   // MMAL_TIME_UNKNOWN
    mBodyOut.buffer_header.dts_lo    = 0;
    mBodyOut.buffer_header.dts_hi    = 0x80000000;
    mBodyOut.is_zero_copy            = 1;
    mBodyOut.has_reference           = 0;
    mBodyOut.payload_in_message      = 0;

    MMALstoreLog("\n--- mBodyOut ---");
    MMALstoreLog("sizeof(mBodyOut)              ", (u32)sizeof(mBodyOut));
    MMALstoreLog("\ndrvbuf.magic                  ", mBodyOut.drvbuf.magic);
    MMALstoreLog("drvbuf.component_handle       ", mBodyOut.drvbuf.component_handle);
    MMALstoreLog("drvbuf.port_handle            ", mBodyOut.drvbuf.port_handle);
    MMALstoreLog("drvbuf.client_context         ", mBodyOut.drvbuf.client_context);

    MMALstoreMsg(mBodyOut.drvbuf_ref, sizeof(mBodyOut.drvbuf_ref), "drvbuf_ref");

    MMALstoreLog("buffer_header.next            ", mBodyOut.buffer_header.next);
    MMALstoreLog("buffer_header.priv            ", mBodyOut.buffer_header.priv);
    MMALstoreLog("buffer_header.cmd             ", mBodyOut.buffer_header.cmd);
    MMALstoreLog("buffer_header.data            ", mBodyOut.buffer_header.data);
    MMALstoreLog("buffer_header.alloc_size      ", mBodyOut.buffer_header.alloc_size);
    MMALstoreLog("buffer_header.length          ", mBodyOut.buffer_header.length);
    MMALstoreLog("buffer_header.offset          ", mBodyOut.buffer_header.offset);
    MMALstoreLog("buffer_header.flags           ", mBodyOut.buffer_header.flags);
    MMALstoreLog("buffer_header.pts_lo          ", mBodyOut.buffer_header.pts_lo);
    MMALstoreLog("buffer_header.pts_hi          ", mBodyOut.buffer_header.pts_hi);
    MMALstoreLog("buffer_header.dts_lo          ", mBodyOut.buffer_header.dts_lo);
    MMALstoreLog("buffer_header.dts_hi          ", mBodyOut.buffer_header.dts_hi);
    MMALstoreLog("buffer_header.type            ", mBodyOut.buffer_header.type);
    MMALstoreLog("buffer_header.user_data       ", mBodyOut.buffer_header.user_data);

    MMALstoreMsg(mBodyOut.type_specific, sizeof(mBodyOut.type_specific), "type_specific");

    MMALstoreLog("is_zero_copy                  ", (u32)mBodyOut.is_zero_copy);
    MMALstoreLog("has_reference                 ", (u32)mBodyOut.has_reference);
    MMALstoreLog("payload_in_message            ", (u32)mBodyOut.payload_in_message);
    MMALstoreMsg(mBodyOut.short_data, sizeof(mBodyOut.short_data), "short_data");

    // INPUT constants
    mBodyIn = {};
    mBodyIn.drvbuf.magic             = MMAL_MAGIC;
    mBodyIn.drvbuf.component_handle  = m_ComponentHandle;
    mBodyIn.drvbuf.port_handle       = m_InputPortHandle;
    mBodyIn.buffer_header.data       = m_InputBufferHandle;
    mBodyIn.buffer_header.alloc_size = m_InputBufferSize;
    mBodyIn.buffer_header.pts_lo     = 0;
    mBodyIn.buffer_header.pts_hi     = 0x80000000;
    mBodyIn.buffer_header.dts_lo     = 0;
    mBodyIn.buffer_header.dts_hi     = 0x80000000;
    mBodyIn.is_zero_copy             = 1;
    mBodyIn.has_reference            = 0;
    mBodyIn.payload_in_message       = 0;

    MMALstoreLog("\n--- mBodyIn ---");
    MMALstoreLog("sizeof(mBodyIn)               ", (u32)sizeof(mBodyIn));
    MMALstoreLog("\ndrvbuf.magic                  ", mBodyIn.drvbuf.magic);
    MMALstoreLog("drvbuf.component_handle       ", mBodyIn.drvbuf.component_handle);
    MMALstoreLog("drvbuf.port_handle            ", mBodyIn.drvbuf.port_handle);
    MMALstoreLog("drvbuf.client_context         ", mBodyIn.drvbuf.client_context);

    MMALstoreMsg(mBodyIn.drvbuf_ref, sizeof(mBodyIn.drvbuf_ref), "drvbuf_ref");

    MMALstoreLog("buffer_header.next            ", mBodyIn.buffer_header.next);
    MMALstoreLog("buffer_header.priv            ", mBodyIn.buffer_header.priv);
    MMALstoreLog("buffer_header.cmd             ", mBodyIn.buffer_header.cmd);
    MMALstoreLog("buffer_header.data            ", mBodyIn.buffer_header.data);
    MMALstoreLog("buffer_header.alloc_size      ", mBodyIn.buffer_header.alloc_size);
    MMALstoreLog("buffer_header.length          ", mBodyIn.buffer_header.length);
    MMALstoreLog("buffer_header.offset          ", mBodyIn.buffer_header.offset);
    MMALstoreLog("buffer_header.flags           ", mBodyIn.buffer_header.flags);
    MMALstoreLog("buffer_header.pts_lo          ", mBodyIn.buffer_header.pts_lo);
    MMALstoreLog("buffer_header.pts_hi          ", mBodyIn.buffer_header.pts_hi);
    MMALstoreLog("buffer_header.dts_lo          ", mBodyIn.buffer_header.dts_lo);
    MMALstoreLog("buffer_header.dts_hi          ", mBodyIn.buffer_header.dts_hi);
    MMALstoreLog("buffer_header.type            ", mBodyIn.buffer_header.type);
    MMALstoreLog("buffer_header.user_data       ", mBodyIn.buffer_header.user_data);

    MMALstoreMsg(mBodyIn.type_specific, sizeof(mBodyIn.type_specific), "type_specific");

    MMALstoreLog("is_zero_copy                  ", (u32)mBodyIn.is_zero_copy);
    MMALstoreLog("has_reference                 ", (u32)mBodyIn.has_reference);
    MMALstoreLog("payload_in_message            ", (u32)mBodyIn.payload_in_message);

    MMALstoreMsg(mBodyIn.short_data, sizeof(mBodyIn.short_data), "short_data");
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
struct mmal_msg_buffer_from_host {
	/*
	 *The front 32 bytes of the buffer header are copied
	 * back to us in the reply to allow for context. This
	 * area is used to store two mmal_driver_buffer structures to
	 * allow for multiple concurrent service users.
	 */
	/* control data */
	struct mmal_driver_buffer drvbuf;

	/* referenced control data for passthrough buffer management */
	struct mmal_driver_buffer drvbuf_ref;
	struct mmal_buffer_header buffer_header; /* buffer header itself */
	struct mmal_buffer_header_type_specific buffer_header_type_specific;
	s32 is_zero_copy;
	s32 has_reference;

	/* allows short data to be xfered in control message */
	u32 payload_in_message;
	u8 short_data[MMAL_VC_SHORT_DATA];
};

struct mmal_driver_buffer {
	u32 magic;
	u32 component_handle;
	u32 port_handle;
	u32 client_context;
};

struct mmal_buffer_header {
	u32 next;	/* next header */
	u32 priv;	/* framework private data */
	u32 cmd;
	u32 data;
	u32 alloc_size;
	u32 length;
	u32 offset;
	u32 flags;
	s64 pts;
	s64 dts;
	u32 type;
	u32 user_data;
};

struct mmal_buffer_header_type_specific {
	union {
		struct {
		u32 planes;
		u32 offset[4];
		u32 pitch[4];
		u32 flags;
		} video;
	} u;
};

/* request to VC to get port information */
struct mmal_msg_port_info_get {
	u32 component_handle;  /* component handle port is associated with */
	u32 port_type;         /* enum mmal_msg_port_type */
	u32 index;             /* port index to query */
};

/* reply from VC to get port info request */
struct mmal_msg_port_info_get_reply {
	u32 status;		/* enum mmal_msg_status */
	u32 component_handle;	/* component handle port is associated with */
	u32 port_type;		/* enum mmal_msg_port_type */
	u32 port_index;		/* port indexed in query */
	s32 found;		/* unused */
	u32 port_handle;	/* Handle to use for this port */
	struct mmal_port port;
	struct mmal_es_format format; /* elementary stream format */
	union mmal_es_specific_format es; /* es type specific data */
	u8 extradata[MMAL_FORMAT_EXTRADATA_MAX_SIZE]; /* es extra data */
};

/* request to VC to set port information */
struct mmal_msg_port_info_set {
	u32 component_handle;
	u32 port_type;		/* enum mmal_msg_port_type */
	u32 port_index;		/* port indexed in query */
	struct mmal_port port;
	struct mmal_es_format format;
	union mmal_es_specific_format es;
	u8 extradata[MMAL_FORMAT_EXTRADATA_MAX_SIZE];
};

/* reply from VC to port info set request */
struct mmal_msg_port_info_set_reply {
	u32 status;
	u32 component_handle;	/* component handle port is associated with */
	u32 port_type;		/* enum mmal_msg_port_type */
	u32 index;		/* port indexed in query */
	s32 found;		/* unused */
	u32 port_handle;	/* Handle to use for this port */
	struct mmal_port port;
	struct mmal_es_format format;
	union mmal_es_specific_format es;
	u8 extradata[MMAL_FORMAT_EXTRADATA_MAX_SIZE];
};

/* port action requests that take a mmal_port as a parameter */
struct mmal_msg_port_action_port {
	u32 component_handle;
	u32 port_handle;
	u32 action;		/* enum mmal_msg_port_action_type */
	struct mmal_port port;
};

/* port action requests that take handles as a parameter */
struct mmal_msg_port_action_handle {
	u32 component_handle;
	u32 port_handle;
	u32 action;		/* enum mmal_msg_port_action_type */
	u32 connect_component_handle;
	u32 connect_port_handle;
};

struct mmal_msg_port_action_reply {
	u32 status;	/* The port action operation status */
};

struct mmal_port {
	u32 priv;	/* Private member used by the framework */
	u32 name;	/* Port name. Used for debugging purposes (RO) */
	u32 type;	/* Type of the port (RO) enum mmal_port_type */
	u16 index;	/* Index of the port in its type list (RO) */
	u16 index_all;	/* Index of the port in the list of all ports (RO) */
	u32 is_enabled;	/* Indicates whether the port is enabled or not (RO) */
	u32 format;	/* Format of the elementary stream */
	u32 buffer_num_min;	// Minimum number of buffers the port requires (RO).  This is set by the component.
	u32 buffer_size_min;	// Minimum size of buffers the port requires (RO).  This is set by the component.
	u32 buffer_alignment_min;/* Minimum alignment requirement for the buffers (RO).  A value of zero means no special alignment requirements.  This is set by the component. */
	u32 buffer_num_recommended;	/* Number of buffers the port recommends for optimal performance (RO).  A value of zero means no special recommendation.  This is set by the component. */
	u32 buffer_size_recommended;	/* Size of buffers the port recommends for optimal performance (RO).  A value of zero means no special recommendation.  This is set by the component. */
	u32 buffer_num;	/* Actual number of buffers the port will use.  This is set by the client. */
	u32 buffer_size; /* Actual maximum size of the buffers that will be sent to the port. This is set by the client. */
	u32 component;	/* Component this port belongs to (Read Only) */
	u32 userdata;	/* Field reserved for use by the client */
	u32 capabilities;	/* Flags describing the capabilities of a  port (RO).  Bitwise combination of \ref portcapabilities "Port capabilities"  values. */
};

/* Remote definition of an elementary stream format (MMAL_ES_FORMAT_T) */
struct mmal_es_format {
	u32 type;	/* enum mmal_es_type */
	u32 encoding;	/* FourCC specifying encoding of the elementary stream. */
	u32 encoding_variant;	/* FourCC specifying the specific encoding variant of the elementary stream.  */
	u32 es;	/* Type specific information for the elementary stream */
	u32 bitrate;	/* Bitrate in bits per second */
	u32 flags;	/* Flags describing properties of the elementary stream. */
	u32 extradata_size;	/* Size of the codec specific data */
	u32 extradata;		/* Codec specific data */
};

/* MMAL_ES_FORMAT_T */

union mmal_es_specific_format {
	struct mmal_audio_format audio;
	struct mmal_video_format video;
	struct mmal_subpicture_format subpicture;
};

struct mmal_audio_format {
	u32 channels;		/* Number of audio channels */
	u32 sample_rate;	/* Sample rate */
	u32 bits_per_sample;	/* Bits per sample */
	u32 block_align;	/* Size of a block of data */
};

struct mmal_video_format {
	u32 width;		/* Width of frame in pixels */
	u32 height;		/* Height of frame in rows of pixels */
	struct mmal_rect crop;	/* Visible region of the frame */
	struct mmal_rational frame_rate;	/* Frame rate */
	struct mmal_rational par;		/* Pixel aspect ratio */
	u32 color_space; /* FourCC specifying the color space of the video stream. See the * MmalColorSpace "pre-defined color spaces" for some examples. */
};

struct mmal_subpicture_format {
	u32 x_offset;
	u32 y_offset;
};
struct mmal_rect {
	s32 x;      /**< x coordinate (from left) */
	s32 y;      /**< y coordinate (from top) */
	s32 width;  /**< width */
	s32 height; /**< height */
};

struct mmal_rational {
	s32 num;    /**< Numerator */
	s32 den;    /**< Denominator */
};