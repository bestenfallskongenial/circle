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

#define VCSMLOG

#define DEBUG_MAX_LENGTH (1024 * 16)

extern "C" void vc_host_get_vchi_state(VCHI_INSTANCE_T *inst, VCHI_CONNECTION_T **conn);
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CONSTRUCTOR / DECONSTRUCTOR
//----------------------------------------------------------------------------------------------------------------------------------------------------
                CVCSharedMemory::CVCSharedMemory    (   )
                : m_ServiceHandle(0)
                , m_TransactionId(0)
{
}
                CVCSharedMemory::~CVCSharedMemory   (   )
{
                if (m_ServiceHandle)
                    {
                    vchi_service_close(m_ServiceHandle);
                    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              USER API
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool            CVCSharedMemory::VCSMInitialize     (   )
{
                VCSMstoreLog ( SERVICENAMESTRING, SERVICEVERSIONSTRING);    

                VCSMgetVCHI();

                if(!VCSMinitEvents())
                    {
            #ifdef VCSMLOG
                    VCSMstoreLog ( "\nVCOS Init FAILED!");      
            #endif // VCSMLOG
                    return false;
                    }
                if(!VCSMopenService())
                    {
            #ifdef VCSMLOG            
                    VCSMstoreLog ( "VCHI Init FAILED!");      
            #endif // VCSMLOG                    
                    return false;
                    }
            #ifdef VCSMLOG  
                VCSMstoreLog ( "\nVCSM Successful Initialized");      
            #endif // VCSMLOG
                return true;
}
bool            CVCSharedMemory::VCSMimportMemory   (   void* buffer, 
                                                        size_t size, 
                                                        int slot )
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
            #ifdef VCSMLOG                        
                    VCSMstoreLog ( "\nRX to Short ", rlen);
            #endif // VCSMLOG                    
                    return false;
                    }

                const vc_sm_import_result *result = reinterpret_cast<const vc_sm_import_result *>(rx_msg);

                if (result->res_handle == 0)
                    {
            #ifdef VCSMLOG                        
                    VCSMstoreLog ( "\nRX No Valid Handle  ", result->res_handle);  
            #endif // VCSMLOG                      
                    return false;
                    }

                m_vc_handle[slot] = result->res_handle;
            #ifdef VCSMLOG    
                VCSMstoreLog ( "\nRX - Buffer Address", (u32)buffer);
                VCSMstoreLog ( "RX - Buffer Size   ", size);
                VCSMstoreLog ( "RX - Buffer Handle ", result->res_handle);
            #endif // VCSMLOG
                return true;
}
bool            CVCSharedMemory::VCSMLockMemory(int slot)
{
                vc_sm_msg_hdr_t tx_hdr = {};                     // 1. Compose header
                tx_hdr.type = VC_SM_MSG_TYPE_LOCK;
                tx_hdr.trans_id = VCSMnextId(m_TransactionId);

                vc_sm_lock_unlock_t lock_msg = {};               // 2. Compose body
                lock_msg.res_handle = m_vc_handle[slot];
                lock_msg.res_mem = 0;

                u8 tx_msg[sizeof(tx_hdr) + sizeof(lock_msg)];    // 3. Compose full message buffer
                memcpy(tx_msg, &tx_hdr, sizeof(tx_hdr));
                memcpy(tx_msg + sizeof(tx_hdr), &lock_msg, sizeof(lock_msg));

                u8 rx_msg[sizeof(vc_sm_lock_result_t)] = {};     // 4. Prepare RX buffer
                size_t rlen = 0;

                if (!VCSMSendAndWait(tx_msg, sizeof(tx_msg), rx_msg, sizeof(rx_msg), &rlen))
                    {
                    return false;
                    }

                if (rlen < sizeof(vc_sm_lock_result_t))
                    {
            #ifdef VCSMLOG                        
                    VCSMstoreLog("RX to Short!", rlen);
            #endif // VCSMLOG                    
                    return false;
                    }

                const vc_sm_lock_result_t* result = reinterpret_cast<const vc_sm_lock_result_t*>(rx_msg);
            #ifdef VCSMLOG
                VCSMstoreLog("LOCK - VC Handle", result->res_handle);
                VCSMstoreLog("LOCK - VC Address", result->res_mem);
            #endif // VCSMLOG

                m_vc_pointer[slot] = result->res_mem;
                return true;
}

bool            CVCSharedMemory::VCSMFreeMemory     (   int slot )
{
                vc_sm_msg_hdr_t tx_hdr = {};                                                   // 1. Compose header
                tx_hdr.type                         = VC_SM_MSG_TYPE_FREE;
                tx_hdr.trans_id                     = VCSMnextId(m_TransactionId);
                vc_sm_free_t free_msg = {};                                                 // 2. Compose body
                free_msg.res_handle                 = m_vc_handle[slot];
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
            #ifdef VCSMLOG                        
                    VCSMstoreLog ( "RX to Short ", rlen);
            #endif // VCSMLOG                    
                    return false;
                    }

                const vc_sm_result_t *result = reinterpret_cast<const vc_sm_result_t *>(rx_msg);

                if (result->success == 0)
                    {
                    m_vc_handle[slot]  = 0;
                    m_vc_pointer[slot] = 0;
                    return true;
                    }
                return false;
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
            #ifdef VCSMLOG
                VCSMstoreLog("\nversion               ", params.version.version);
                VCSMstoreLog("version_min           ", params.version.version_min);
                VCSMstoreLog("service_id            ", params.service_id);
                VCSMstoreLog("connection            ", (u32)(uintptr_t)params.connection);
                VCSMstoreLog("rx_fifo_size          ", params.rx_fifo_size);
                VCSMstoreLog("tx_fifo_size          ", params.tx_fifo_size);
                VCSMstoreLog("callback              ", (u32)(uintptr_t)params.callback);
                VCSMstoreLog("callback_param        ", (u32)(uintptr_t)params.callback_param);
                VCSMstoreLog("want_unaligned_bulk_rx", params.want_unaligned_bulk_rx);
                VCSMstoreLog("want_unaligned_bulk_tx", params.want_unaligned_bulk_tx);
                VCSMstoreLog("want_crc              ", params.want_crc);
            #endif // VCSMLOG
                int rc = vchi_service_open(m_VCHIInstance, &params, &m_ServiceHandle);
                if ( rc != 0)
                    {
            #ifdef VCSMLOG                        
                    VCSMstoreLog ( "\nFailed to open SMEM service!", (u32)m_ServiceHandle);
            #endif // VCSMLOG                    
                    }
                if ( rc == 0)
                    {
            #ifdef VCSMLOG                        
                    VCSMstoreLog ( "\nSMEM service opened!", (u32)m_ServiceHandle);
            #endif // VCSMLOG                    
                    }
                return (rc == 0);
}
u32             CVCSharedMemory::VCSMnextId         (   u32 &tid )
{
                tid = ( tid+1 ) & ~0x80000000u;                                             // mask for async messages really needed ?!                        
                return tid;
}
void            CVCSharedMemory::VCSMgetVCHI        (   )
{
                vc_host_get_vchi_state(&m_VCHIInstance, &m_Connection);                         //1. get the VCHI instance and the connection handle from bcm_host.h
            #ifdef VCSMLOG
                VCSMstoreLog ( "\nVCHI State Instance     ", (u32)m_VCHIInstance);   
                VCSMstoreLog ( "VCHI State Connection   ", (u32)m_Connection);   
            #endif // VCSMLOG            
}
bool            CVCSharedMemory::VCSMinitEvents     (   )
{
                if (vcos_event_create(&m_VCOSevent, "SMEM") != VCOS_SUCCESS)
                    {
            #ifdef VCSMLOG                        
                    VCSMstoreLog ( "\nVCOS Event Init Failed!", (u32)&m_VCOSevent);  
            #endif // VCSMLOG                    
                    return false;                
                    }
                return true;    
}
u32             CVCSharedMemory::VCSMconvertAddress (   void* buffer, size_t size )
{
                u32 bus_addr = BUS_ADDRESS(reinterpret_cast<uintptr_t>(buffer));
                u32 vcsm_addr = (bus_addr & ~0xC0000000) | 0xC0000000;
            #ifdef VCSMLOG
                VCSMstoreLog ( "\nBuffer Address USR", (u32)buffer); 
                VCSMstoreLog ( "Buffer Address ARM", (u32)bus_addr); 
                VCSMstoreLog ( "Buffer Address VPU", (u32)vcsm_addr); 
            #endif // VCSMLOG
                CleanAndInvalidateDataCacheRange((uintptr_t)(buffer), size);

                return vcsm_addr;
}
void            CVCSharedMemory::MMALstoreLog              (   const char* label, u32 value1, u32 value2, u32 value3, u32 value4 )
{
                /* always write the label */
                for (const char* p = label; *p; ++p)
                    m_DebugCharArray[m_CharIndex++] = *p;

                /* if all values are placeholders, finish */
                if ( value1 == STOREDEBUG_WHITESPACE &&
                    value2 == STOREDEBUG_WHITESPACE &&
                    value3 == STOREDEBUG_WHITESPACE &&
                    value4 == STOREDEBUG_WHITESPACE )
                {
                    m_DebugCharArray[m_CharIndex++] = '\n';
                    m_DebugCharArray[m_CharIndex]   = '\0';
                    return;
                }
                /* write first value if valid */
                if (value1 != STOREDEBUG_WHITESPACE) {
                    m_DebugCharArray[m_CharIndex++] = ' ';
                    m_DebugCharArray[m_CharIndex++] = '0';
                    m_DebugCharArray[m_CharIndex++] = 'x';
                    for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) {
                        char hex = "0123456789ABCDEF"[(value1 >> (i * 4)) & 0xF];
                        m_DebugCharArray[m_CharIndex++] = hex;
                    }
                }
                /* write second value if valid */
                if (value2 != STOREDEBUG_WHITESPACE) {
                    m_DebugCharArray[m_CharIndex++] = ' ';
                    m_DebugCharArray[m_CharIndex++] = '0';
                    m_DebugCharArray[m_CharIndex++] = 'x';
                    for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) {
                        char hex = "0123456789ABCDEF"[(value2 >> (i * 4)) & 0xF];
                        m_DebugCharArray[m_CharIndex++] = hex;
                    }
                }
                /* write third value if valid */
                if (value3 != STOREDEBUG_WHITESPACE) {
                    m_DebugCharArray[m_CharIndex++] = ' ';
                    m_DebugCharArray[m_CharIndex++] = '0';
                    m_DebugCharArray[m_CharIndex++] = 'x';
                    for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) {
                        char hex = "0123456789ABCDEF"[(value3 >> (i * 4)) & 0xF];
                        m_DebugCharArray[m_CharIndex++] = hex;
                    }
                }
                /* write fourth value if valid */
                if (value4 != STOREDEBUG_WHITESPACE) {
                    m_DebugCharArray[m_CharIndex++] = ' ';
                    m_DebugCharArray[m_CharIndex++] = '0';
                    m_DebugCharArray[m_CharIndex++] = 'x';
                    for (int i = (sizeof(u32) * 2) - 1; i >= 0; --i) {
                        char hex = "0123456789ABCDEF"[(value4 >> (i * 4)) & 0xF];
                        m_DebugCharArray[m_CharIndex++] = hex;
                    }
                }
                /* terminate line */
                m_DebugCharArray[m_CharIndex++] = '\n';
                m_DebugCharArray[m_CharIndex]   = '\0';
}

void CVCSharedMemory::VCSMstoreMsg(const void* tx_msg, u32 total_size, const char* label)
{
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

bool            CVCSharedMemory::VCSMSendAndWait    (   const void *msg, 
                                                        size_t msg_size, 
                                                        void *rx_msg, 
                                                        size_t max_reply_len, 
                                                        size_t *actual_reply_len )
{
            #ifdef VCSMLOG    
                MMALstoreLog("\nTX MSG", (u32)msg_size);
                VCSMstoreMsg(msg, msg_size, "Raw TX");
            #endif // VCSMLOG
                if (vchi_msg_queue(m_ServiceHandle, msg, msg_size, VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL) != 0)
                    {
                    return false;
                    }
                uint32_t ReplyLength = 0;
                do {
                    if (vchi_msg_dequeue(m_ServiceHandle, rx_msg, max_reply_len, &ReplyLength, VCHI_FLAGS_NONE) == 0)
                        {
            #ifdef VCSMLOG
                        MMALstoreLog("\nRX MSG", ReplyLength);                        
                        VCSMstoreMsg(rx_msg, ReplyLength, "Raw RX");
            #endif // VCSMLOG                        
                        break;
                        }
                    } 
                while (vcos_event_wait(&m_VCOSevent) == VCOS_SUCCESS);

                if (actual_reply_len)
                    {
                    *actual_reply_len = ReplyLength;
                    }
                return (ReplyLength != 0);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              END OF FILE
//----------------------------------------------------------------------------------------------------------------------------------------------------
