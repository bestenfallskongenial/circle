//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264 Decoder Runtime Code
//----------------------------------------------------------------------------------------------------------------------------------------------------
#include "vc_h264_decoder.h"

bool            CH264Decoder::MMALbufferReady(u32 handle)
{
                if (handle == m_VCSMHandleA)
                    {
                    if (m_EGLimageA)
                        {
                        eglDestroyImageKHR(m_eglDisplay, m_EGLimageA);
                        m_EGLimageA = EGL_NO_IMAGE_KHR;
                        }
                    struct egl_image_brcm_vcsm_info infoA = 
                        {
                        .width       = m_ResolutionX,
                        .height      = m_ResolutionY,
                        .vcsm_handle = m_VCSMHandleA
                        };
                    m_EGLimageA = eglCreateImageKHR(
                        m_eglDisplay, m_eglContext,
                        EGL_IMAGE_BRCM_VCSM, (EGLClientBuffer)&infoA, NULL);

                    if (m_EGLimageA == EGL_NO_IMAGE_KHR)
                        {
                        MMALstoreLog("\nm_EGLimageA Failed");
                        return false;
                        }
                    // Always bind the public texture
                    glBindTexture(GL_TEXTURE_2D, m_TextureExposed);
                    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_EGLimageA);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    }
                else if (handle == m_VCSMHandleB)
                    {
                    if (m_EGLimageB)
                        {
                        eglDestroyImageKHR(m_eglDisplay, m_EGLimageB);
                        m_EGLimageB = EGL_NO_IMAGE_KHR;
                        }
                    struct egl_image_brcm_vcsm_info infoB = 
                        {
                        .width       = m_ResolutionX,
                        .height      = m_ResolutionY,
                        .vcsm_handle = m_VCSMHandleB
                        };
                    m_EGLimageB = eglCreateImageKHR(
                        m_eglDisplay, m_eglContext,
                        EGL_IMAGE_BRCM_VCSM, (EGLClientBuffer)&infoB, NULL);

                    if (m_EGLimageB == EGL_NO_IMAGE_KHR)
                        {
                        MMALstoreLog("\nm_EGLimageB Failed");
                        return false;
                        }
                    // Always bind the public texture
                    glBindTexture(GL_TEXTURE_2D, m_TextureExposed);
                    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_EGLimageB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    }
                return true;
}

bool            CH264Decoder::MMALqueueOutputBuffer(mmal_msg_buffer_from_host_wire32& body, u32 vc_handle, u32 alloc_size)
{
                mmal_msg_header tx_hdr = {};

                tx_hdr.magic                               = MMAL_MAGIC;
                tx_hdr.type                                = MMAL_MSG_TYPE_BUFFER_FROM_HOST;
                tx_hdr.control_service                      = 0;         // Opaque handle to the control service  *** NEW TO MATCH THE DEFINITION! 
                tx_hdr.context                             = NextTransId(m_TransactionId);
                tx_hdr.status                               = 0;         // The status of the vchiq operation  *** NEW TO MATCH THE DEFINITION! 
                tx_hdr.padding                              = 0;         // *** NEW TO MATCH THE DEFINITION!

                mmal_msg_buffer_from_host tx_body           = {};
                
                tx_body.drvbuf.magic                        = MMAL_MAGIC; 
                tx_body.drvbuf.component_handle             = m_ComponentHandle;
                tx_body.drvbuf.port_handle                  = m_OutputPortHandle;
                tx_body.drvbuf.client_context               = tx_hdr.context;
                
                tx_body.drvbuf_rev.magic                    = MMAL_MAGIC;
                tx_body.drvbuf_rev.component_handle         = m_ComponentHandle;
                tx_body.drvbuf_rev.port_handle              = m_OutputPortHandle;
                tx_body.drvbuf_rev.client_context           = tx_hdr.context;

                tx_body.buffer_header.next                  =
                tx_body.buffer_header.priv                  =
                tx_body.buffer_header.cmd                   =
                tx_body.buffer_header.data                  =
                tx_body.buffer_header.alloc_size            = m_OutputBufferSize;
                tx_body.buffer_header.length                =
                tx_body.buffer_header.offset                =
                tx_body.buffer_header.flags                 =
                tx_body.buffer_header.pts                   =
                tx_body.buffer_header.dts                   =
                tx_body.buffer_header.type                  =
                tx_body.buffer_header.user_data             =

                tx_body.buffer_header_type_specific.planes  =
                tx_body.buffer_header_type_specific.offset  = u32 offset[4];
                tx_body.buffer_header_type_specific.pitch   = u32 pitch[4];
                tx_body.buffer_header_type_specific.flags   = 

                tx_body.is_zero_copy                        = 1;
                tx_body.has_reference                       = 0;
                tx_body.payload_in_message                  = 0;
                tx_body.short_data                          = u8 short_data[MMAL_VC_SHORT_DATA];

                u8 msg[sizeof(hdr) + sizeof(body)];
                memcpy(msg, &hdr, sizeof(hdr));
                memcpy(msg + sizeof(hdr), &body, sizeof(body));

                MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(msg));   // log the size of the message must be 268 + 24 = 292 dec / 124 hex
                MMALstoreMsg(msg, (u32)sizeof(msg), "QueueOutputBuffer");   // log the complete tx message

                return vchi_msg_queue(m_ServiceHandle, msg, (u32)sizeof(msg), VCHI_FLAGS_BLOCK_UNTIL_QUEUED, nullptr) == 0;
}

bool            CH264Decoder::MMALqueueInputBuffer(mmal_msg_buffer_from_host_wire32& body, u32 frame_offset, u32 frame_length)
{
            //  if (frame_offset + frame_length > m_InputBufferSize) return false;

                mmal_msg_header tx_hdr = {};

                tx_hdr.magic                               = MMAL_MAGIC;
                tx_hdr.type                                = MMAL_MSG_TYPE_BUFFER_FROM_HOST;
                tx_hdr.control_service                      = 0;         // Opaque handle to the control service  *** NEW TO MATCH THE DEFINITION! 
                tx_hdr.context                             = NextTransId(m_TransactionId);
                tx_hdr.status                               = 0;         // The status of the vchiq operation  *** NEW TO MATCH THE DEFINITION! 
                tx_hdr.padding                              = 0;         // *** NEW TO MATCH THE DEFINITION!

                mmal_msg_buffer_from_host tx_body           = {};
                
                tx_body.drvbuf.magic                        = MMAL_MAGIC; 
                tx_body.drvbuf.component_handle             = m_ComponentHandle;
                tx_body.drvbuf.port_handle                  = m_InputPortHandle;
                tx_body.drvbuf.client_context               = tx_hdr.context;
                
                tx_body.drvbuf_rev.magic                    = MMAL_MAGIC;
                tx_body.drvbuf_rev.component_handle         = m_ComponentHandle;
                tx_body.drvbuf_rev.port_handle              = m_InputPortHandle;
                tx_body.drvbuf_rev.client_context           = tx_hdr.context;

                tx_body.buffer_header.next                  =
                tx_body.buffer_header.priv                  =
                tx_body.buffer_header.cmd                   =
                tx_body.buffer_header.data                  =
                tx_body.buffer_header.alloc_size            = m_InputBufferSize;
                tx_body.buffer_header.length                = frame_length;
                tx_body.buffer_header.offset                = frame_offset;
                tx_body.buffer_header.flags                 = MMAL_BUFFER_HEADER_FLAG_FRAME | MMAL_BUFFER_HEADER_FLAG_KEYFRAME;
                tx_body.buffer_header.pts                   =
                tx_body.buffer_header.dts                   =
                tx_body.buffer_header.type                  =
                tx_body.buffer_header.user_data             =

                tx_body.buffer_header_type_specific.planes  =
                tx_body.buffer_header_type_specific.offset  = u32 offset[4];
                tx_body.buffer_header_type_specific.pitch   = u32 pitch[4];
                tx_body.buffer_header_type_specific.flags   = 

                tx_body.is_zero_copy                        = 1;
                tx_body.has_reference                       = 0;
                tx_body.payload_in_message                  = 0;
                tx_body.short_data                          = u8 short_data[MMAL_VC_SHORT_DATA];

                u32 flags = MMAL_BUFFER_HEADER_FLAG_FRAME | MMAL_BUFFER_HEADER_FLAG_KEYFRAME;
            //  if (!m_FirstFrameQueued) flags |= MMAL_BUFFER_HEADER_FLAG_DISCONTINUITY;
            
                u8 msg[sizeof(hdr) + sizeof(body)];
                memcpy(msg, &hdr, sizeof(hdr));
                memcpy(msg + sizeof(hdr), &body, sizeof(body));

                MMALstoreLog("\nBUFFER FROM HOST MSG", (u32)sizeof(msg));   // log the size of the message must be 268 + 24 = 292 dec / 124 hex
                MMALstoreMsg(msg, (u32)sizeof(msg), "QueueInputBuffer");    // log the complete tx message

                return vchi_msg_queue(m_ServiceHandle, msg, (u32)sizeof(msg), VCHI_FLAGS_BLOCK_UNTIL_QUEUED, nullptr) == 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              END OF FILE
//----------------------------------------------------------------------------------------------------------------------------------------------------