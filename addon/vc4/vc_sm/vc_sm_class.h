//----------------------------------------------------------------------------------------------------------------------------------------------------
//      vc_sm_class.h
//----------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef _VC_SM_CLASS_H
#define _VC_SM_CLASS_H

#include <circle/types.h>                                                           // for u32, size_t, etc.
#include "vc_sm_defs.h"
#include "interface/vcos/vcos.h"
#include <vc4/interface/vcinclude/common.h>
#include <vc4/vchi/vchi.h>                                                          // for VCHI_INSTANCE_T, VCHI_CONNECTION_T, VCHI_SERVICE_HANDLE_T, VCHI_CALLBACK_REASON_T

class CVCSharedMemory
{
public:
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CONSTRUCTOR / DECONSTRUCTOR
//----------------------------------------------------------------------------------------------------------------------------------------------------
                CVCSharedMemory     (   );
               ~CVCSharedMemory     (   );
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              USER API
//----------------------------------------------------------------------------------------------------------------------------------------------------
        bool    VCSMInitialize      (   );                                              // initialise the vc4 shared memory service

        bool    VCSMimportMemory    (   void* buffer,                                   // buffer: Memory allocated with HEAP_DMA30
                                        size_t size,                                    // size: Size of buffer in bytes
                                        int slot);                                      // handle_out: Receives VideoCore handle on success   
        bool    VCSMLockMemory      (   int slot);                                      // lock buffer and get VPU address
        bool    VCSMFreeMemory      (   int slot);                                      // handle of the buffer to deregister
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              CALLBACK / HELPERS / UTILITY / WRAPPER
//----------------------------------------------------------------------------------------------------------------------------------------------------
private:
static  void    VCSMcallback        (   void *callback_param,                       // vchi service callback
                                        VCHI_CALLBACK_REASON_T reason, 
                                        void *msg_handle);                          
        bool    VCSMopenService     (   );                                          // open the SMEM service                                        
        u32     VCSMnextId          (   u32 &tid);                                  // increments the transition ID
        void    VCSMgetVCHI         (   );                                          // gets the VCHI Instance from BCM_HOST
        bool    VCSMinitEvents      (   );                                          // guess we need this too? 
        u32     VCSMconvertAddress  (   void* buffer, size_t size);                 // convert the arm address to an uncached vpu address
        void    VCSMstoreMsg        (   const void* msg,                            // DEBUG stores the rx/tx vpu messages in the log_buffer
                                        u32 total_size, 
                                        const char* label);
        void    VCSMstoreLog        (   const char* label,                          // DEBUG stores one "userstring" and one variable in the log_buffer
                                        u32 value);    

        bool    VCSMSendAndWait     (   const void *msg,                            // synchronous massaging 
                                        size_t msg_size, 
                                        void *raw_reply, 
                                        size_t max_reply_len, 
                                        size_t *actual_reply_len );
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              MEMBER VARIABLES
//----------------------------------------------------------------------------------------------------------------------------------------------------
        VCHI_INSTANCE_T                 m_VCHIInstance;
        VCOS_EVENT_T                    m_VCOSevent;
        VCHI_CONNECTION_T*              m_Connection;
        VCHI_SERVICE_HANDLE_T           m_ServiceHandle;
        u32                             m_TransactionId;
public:
        u32                             m_vc_handle[MAX_BUFFER] = {0};
        u32                             m_vc_pointer[MAX_BUFFER] = {0};

        u32                             m_CharIndex = 0;
        char                            m_DebugCharArray[MAX_DEBUG_FILE_LENGTH] = { 0 }; // log_buffer is exposed for external use ( like storing ) 
};
#endif // _VC_SM_CLASS_H
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              END OF FILE
//----------------------------------------------------------------------------------------------------------------------------------------------------