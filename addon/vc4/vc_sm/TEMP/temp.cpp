//----------------------------------------------------------------------------------------------------------------------------------------------------
//      vc_sm_class.cpp
//----------------------------------------------------------------------------------------------------------------------------------------------------
#include "vc_sm_class.h"

#include <circle/types.h>
#include <circle/logger.h>
#include "interface/vcos/vcos.h"
#include "interface/vcos/vcos_event.h"
#include <vc4/vchi/vchi.h>
#include <vc4/vchiq/vchiq.h>
#include <circle/bcm2835.h>

extern "C" void vc_host_get_vchi_state(VCHI_INSTANCE_T *inst, VCHI_CONNECTION_T **conn);
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CONSTRUCTOR / DECONSTRUCTOR
//----------------------------------------------------------------------------------------------------------------------------------------------------
                CVCSharedMemory::CVCSharedMemory    ()
                : m_ServiceHandle(0)
                , m_TransactionId(0)
{
}
                CVCSharedMemory::~CVCSharedMemory   ()
{
                if (m_ServiceHandle)
                    {
                    vchi_service_close(m_ServiceHandle);
                    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              USER API
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool            CVCSharedMemory::VCSMImportMemory   (   void* buffer, 
                                                        size_t size, 
                                                        u32 *handle_out )
{
                vc_sm_msg_hdr_t tx_hdr = {};                                                   // 1. Compose header
                tx_hdr.type = VC_SM_MSG_TYPE_IMPORT;
                tx_hdr.trans_id = VCSMnextId(m_TransactionId);
                vc_sm_import import = {};                                                   // 2. Compose body
                import.type                         = VC_SM_ALLOC_NON_CACHED;
                import.addr                         = VCSMconvertAddress(buffer, size);
                import.size                         = static_cast<u32>(size);
                import.kernel_id                    = 0;
                import.allocator                    = 0;
                strncpy(import.name, "SMEM", sizeof(import.name));                          // will i really use the name? do i need it?!
                
                u8 tx_msg[sizeof(tx_hdr) + sizeof(import)];                                       // 3. Compose full message buffer
                
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &import, sizeof(import));

                u8 rx_msg[sizeof(vc_sm_import_result)];                                      // 4. Prepare RX buffer for reply
                size_t rlen = 0;
                
                if (!VCSMSendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rlen))
                    {
                    return false;
                    }
                if (rlen < sizeof(vc_sm_import_result))
                    {
                    VCSMstoreLog ( "VPU RX to Short ", rlen);
                //  CLogger::Get()->Write("SMEM-aloc", LogDebug, "VPU Reply to short"); 
                    return false;
                    }

                const vc_sm_import_result *result = reinterpret_cast<const vc_sm_import_result *>(rx_msg);

                if (result->res_handle == 0)
                    {
                    VCSMstoreLog ( "VPU RX No Valid Handle  ", result->res_handle);    
                //  CLogger::Get()->Write("SMEM-Init", LogError, "no valid handle receive", result->res_handle);    
                    return false;
                    }

                *handle_out = result->res_handle;
                
                VCSMstoreLog ( "VCP RX - Buffer ", buffer);
                VCSMstoreLog ( "VCP RX - Buffer ", size);
                VCSMstoreLog ( "VCP RX - Buffer ", result->res_handle);
            //  CLogger::Get()->Write("SMEM-Import", LogDebug, "Succeeded Buffer 0X%p - %u Bytes - Handle 0x%08X,", buffer, size, result->res_handle);

                return true;
}
bool            CVCSharedMemory::VCSMFreeMemory     (   u32 handle )
{
                vc_sm_msg_hdr_t tx_hdr = {};                                                   // 1. Compose header
                tx_hdr.type                         = VC_SM_MSG_TYPE_FREE;
                tx_hdr.trans_id                     = VCSMnextId(m_TransactionId);
                vc_sm_free_t free_msg = {};                                                 // 2. Compose body
                free_msg.res_handle                 = handle;
                free_msg.res_mem                    = 0;
                u8 tx_msg[sizeof(tx_hdr) + sizeof(free_msg)];                                     // 3. Compose full message buffer
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &free_msg, sizeof(free_msg));
                u8 rx_msg[sizeof(vc_sm_result_t)];                                           // 4. Prepare RX buffer for reply
                size_t rlen = 0;
                if (!VCSMSendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rlen))        // 5. Send and wait (with full message)
                    {
                    return false;
                    }

                if (rlen < sizeof(vc_sm_result_t))
                    {
                    VCSMstoreLog ( "VPU RX to Short ", rlen);
                //  CLogger::Get()->Write("SMEM-free", LogDebug, "VPU Reply to short");      
                    return false;
                    }

                const vc_sm_result_t *result = reinterpret_cast<const vc_sm_result_t *>(rx_msg);

                return (result->success == 0);
}
bool            CVCSharedMemory::VCMSInitialize     (   void )
{
                VCSMgetVCHI();

                if(!VCSMinitEvents())
                    {
                    VCSMstoreLog ( "VCOS Init FAILED!", STOREDEBUG_WHITESPACE);      
                //  CLogger::Get()->Write("SMEM-Init", LogDebug, "VCOS Init FAILED!");    
                    return false;
                    }
                if(!VCSMopenService())
                    {
                    VCSMstoreLog ( "VCHI Init FAILED!", STOREDEBUG_WHITESPACE);      
                //  CLogger::Get()->Write("SMEM-Init", LogDebug, "VCHI Init FAILED");    
                    return false;
                    }
                /*
                if(!NegotiateVersion(VC_SM_PROTOCOL_VERSION))
                    {
                    CLogger::Get()->Write("SMEM-Init", LogDebug, "Version Negotiation FAILED");    
                    return false;
                    }
                */    
                VCSMstoreLog ( "VCSM Successful lInitialized", STOREDEBUG_WHITESPACE);      
            //  CLogger::Get()->Write("SMEM-Init", LogDebug, "seemingly we have a working SMEM service");  
                return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
void            CVCSharedMemory::VCSMcallback       (   void *callback_param, 
                                                        VCHI_CALLBACK_REASON_T reason, 
                                                        void *msg_handle )
{
                VCOS_EVENT_T *event = (VCOS_EVENT_T *)callback_param;
                if (reason == VCHI_CALLBACK_MSG_AVAILABLE && event)
                    {
                    VCSMstoreLog ( "Callback", reason);        
                //  CLogger::Get()->Write("Callback", LogDebug, "....ring ring");
                    vcos_event_signal(event);
                    }
}
bool            CVCSharedMemory::VCSMopenService    (   void )
{
                SERVICE_CREATION_T params           = {};
                params.version.version              = VC_SM_VER;
                params.version.version_min          = VC_SM_MIN_VER;
                params.service_id                   = VCHIQ_MAKE_FOURCC('S','M','E','M');
                params.connection                   = m_Connection;
                params.rx_fifo_size                 = 0;
                params.tx_fifo_size                 = 0;
                params.callback                     = VCSMcallback;
                params.callback_param               = &m_VCOSevent;
                params.want_unaligned_bulk_rx       = 0;
                params.want_unaligned_bulk_tx       = 0;
                params.want_crc                     = 0;

                int rc = vchi_service_open(m_VCHIInstance, &params, &m_ServiceHandle);
                if ( rc != 0)
                    {
                    VCSMstoreLog ( "Failed to open SMEM service!", STOREDEBUG_WHITESPACE);   
                //  CLogger::Get()->Write("SMEM-Open", LogError, "Failed to open SMEM service!");    
                    }
                if ( rc == 0)
                    {
                    VCSMstoreLog ( "SMEM service opened!", STOREDEBUG_WHITESPACE);   
                //  CLogger::Get()->Write("SMEM-Open", LogDebug, "SMEM service opened. Handle=%p", m_ServiceHandle);
                    }
                return (rc == 0);
}
u32             CVCSharedMemory::VCSMnextId         (   u32 &tid )
{
                tid = ( tid+1 ) & ~0x80000000u;                                             // mask for async messages really needed ?!                        
                return tid;
}
void            CVCSharedMemory::VCSMgetVCHI        (   void )
{
                vc_host_get_vchi_state(&m_VCHIInstance, &m_Connection);                         //1. get the VCHI instance and the connection handle from bcm_host.h
                VCSMstoreLog ( "VCHI State Instance     ", m_VCHIInstance);   
                VCSMstoreLog ( "VCHI State Connection   ", m_Connection);   
            //  CLogger::Get()->Write("SMEM-Init", LogNotice, "VCHI State Instance=0x%08p Connection=0x%08p", m_VCHIInstance, m_Connection);
}
bool            CVCSharedMemory::VCSMinitEvents     (   void )
{
                if (vcos_event_create(&m_VCOSevent, "SMEM") != VCOS_SUCCESS)
                    {
                    VCSMstoreLog ( "VCOS Event Init Failed!", m_VCOSevent);  
                //  CLogger::Get()->Write("SMEM-Init", LogError, "VCOS Event Init Failed");
                    return false;
                    }
                return true;    
}
u32             CVCSharedMemory::VCSMconvertAddress (   void* buffer, size_t size )
{
                u32 bus_addr = BUS_ADDRESS(reinterpret_cast<uintptr_t>(buffer));
                u32 vcsm_addr = (bus_addr & ~0xC0000000) | 0xC0000000;

                VCSMstoreLog ( "Buffer Address USR", buffer); 
                VCSMstoreLog ( "Buffer Address ARM", bus_addr); 
                VCSMstoreLog ( "Buffer Address VPU", vcsm_addr); 
            //  CLogger::Get()->Write("SMEM-Import", LogDebug, "Address USR=%08p ARM=0x%08X VPU=0x%08X", buffer, bus_addr, vcsm_addr);

                CleanAndInvalidateDataCacheRange((uintptr_t)(buffer), size);

                return vcsm_addr;
}
void            CVCSharedMemory::VCSMstoreMsg       (   const void* tx_msg, 
                                                        uint32_t total_size, 
                                                        const char* label )
{
                const char* p = label;
                while (*p)
                {
                    if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                    m_DebugCharArray[m_CharIndex++] = *p;
                    p++;
                }
                if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                m_DebugCharArray[m_CharIndex++] = '\n';
                if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                m_DebugCharArray[m_CharIndex++] = '\n';

                const uint8_t* tx = (const uint8_t*)tx_msg;
                static const char hex[] = "0123456789ABCDEF";
                uint32_t i = 0;
                while (i < total_size)
                {
                    uint32_t j = 0;
                    while (j < 16 && i < total_size)
                    {
                        uint8_t v = tx[i];
                        if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                        m_DebugCharArray[m_CharIndex++] = hex[v >> 4];
                        if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                        m_DebugCharArray[m_CharIndex++] = hex[v & 0xF];
                        if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                        m_DebugCharArray[m_CharIndex++] = ' ';
                        i++;
                        j++;
                    }
                    if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                    m_DebugCharArray[m_CharIndex++] = '\n';
                }
                if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                m_DebugCharArray[m_CharIndex++] = '\n';
                if (m_CharIndex >= (sizeof(m_DebugCharArray) - 1)) return;
                m_DebugCharArray[m_CharIndex++] = '\n';
                if (m_CharIndex < (sizeof(m_DebugCharArray) - 1)) m_DebugCharArray[m_CharIndex] = 0;
}
void            CVCSharedMemory::VCSMstoreLog       (   const char* label, 
                                                        unsigned long value )
{
                int i = 0;
                // Write label
                while (label[i] && m_CharIndex < DEBUG_MAX_LENGTH - 1)
                    m_DebugCharArray[m_CharIndex++] = label[i++];
                if (m_CharIndex < DEBUG_MAX_LENGTH - 1)
                    m_DebugCharArray[m_CharIndex++] = ' ';

                // Write '0x'
                if (m_CharIndex < DEBUG_MAX_LENGTH - 1)
                    m_DebugCharArray[m_CharIndex++] = '0';
                if (m_CharIndex < DEBUG_MAX_LENGTH - 1)
                    m_DebugCharArray[m_CharIndex++] = 'x';

                // Write hex digits
                int nibbles = sizeof(unsigned long) * 2;
                for (int j = nibbles - 1; j >= 0; --j) {
                    unsigned char v = (value >> (j * 4)) & 0xF;
                    m_DebugCharArray[m_CharIndex++] = "0123456789ABCDEF"[v];
                }

                // Write newline
                if (m_CharIndex < DEBUG_MAX_LENGTH - 1)
                    m_DebugCharArray[m_CharIndex++] = '\n';
}
bool            CVCSharedMemory::VCSMSendAndWait    (   const void *msg, 
                                                        size_t msg_size, 
                                                        void *raw_reply, 
                                                        size_t max_reply_len, 
                                                        size_t *actual_reply_len )
{
                VCSMstoreMsg(msg, msg_size, "Raw TX");

                if (vchi_msg_queue(m_ServiceHandle, msg, msg_size, VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL) != 0)
                    {
                    return false;
                    }
                uint32_t ReplyLength = 0;
                do {
                    if (vchi_msg_dequeue(m_ServiceHandle, raw_reply, max_reply_len, &ReplyLength, VCHI_FLAGS_NONE) == 0)
                    {
                        VCSMstoreMsg(raw_reply, ReplyLength, "Raw RX");
                        break;
                    }
                } while (vcos_event_wait(&m_VCOSevent) == VCOS_SUCCESS);

                if (actual_reply_len)
                    *actual_reply_len = ReplyLength;

                return (ReplyLength != 0);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              END OF FILE
//----------------------------------------------------------------------------------------------------------------------------------------------------
