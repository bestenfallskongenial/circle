//----------------------------------------------------------------------------------------------------------------------------------------------------
// 										Based on vc_sm_defs.h from the vmcs_sm driver Copyright Broadcom Corporation.
// 										All IPC messages are copied across to this file, even if the vc-sm-cma
// 										driver is not currently using them.
//----------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef __VC_SM_DEFS_H__INCLUDED__
#define __VC_SM_DEFS_H__INCLUDED__


#define VC_SM_VER  1
#define VC_SM_MIN_VER 0

#define MAX_DEBUG_FILE_LENGTH (1024 * 16)

#define SERVICEVERSIONSTRING 0207202501 // !!! WE WILL UPDATE IT FROM NOW FORMAT DDMMYYYYVV
#define SERVICENAMESTRING "VC-SM ( SMEM ) SERVICE "
/* Maximum message length */
#define VC_SM_MAX_MSG_LEN (sizeof(union vc_sm_msg_union_t) + \
	sizeof(struct vc_sm_msg_hdr_t))
#define VC_SM_MAX_RSP_LEN (sizeof(union vc_sm_msg_union_t))

#define VC_SM_RESOURCE_NAME 32				// Resource name maximum size //

// Version to be reported to the VPU
// VPU assumes 0 (aka 1) which does not require the released callback, nor
// expect the client to handle VC_MEM_REQUESTS.
// Version 2 requires the released callback, and must support VC_MEM_REQUESTS.

#define VC_SM_PROTOCOL_VERSION	2

enum vc_sm_msg_type 						// Message types supported for HOST->VC direction //			
	{				
	VC_SM_MSG_TYPE_ALLOC,					// Allocate shared memory block //
	VC_SM_MSG_TYPE_LOCK,					// Lock allocated shared memory block //
	VC_SM_MSG_TYPE_UNLOCK,					// Unlock allocated shared memory block //
	VC_SM_MSG_TYPE_UNLOCK_NOANS,			// Unlock allocated shared memory block, do not answer command //
	VC_SM_MSG_TYPE_FREE,					// Free shared memory block //
	VC_SM_MSG_TYPE_RESIZE,					// Resize a shared memory block //
	VC_SM_MSG_TYPE_WALK_ALLOC,				// Walk the allocated shared memory block(s) //
	VC_SM_MSG_TYPE_ACTION_CLEAN,			// A previously applied action will need to be reverted //
	VC_SM_MSG_TYPE_IMPORT,					// Import a physical address and wrap into a MEM_HANDLE_T - Release with VC_SM_MSG_TYPE_FREE.
	VC_SM_MSG_TYPE_CLIENT_VERSION,			// Tells VC the protocol version supported by this client. 2 supports the async/cmd messages from the VPU for final release of memory, and for VC allocations.
	VC_SM_MSG_TYPE_VC_MEM_REQUEST_REPLY,	// Response to VC request for memory //
											// Asynchronous/cmd messages supported for VC->HOST direction.
											// Signalled by setting the top bit in vc_sm_result_t trans_id.
											// VC has finished with an imported memory allocation.
											// Release any Linux reference counts on the underlying block.
	VC_SM_MSG_TYPE_RELEASED,
	VC_SM_MSG_TYPE_VC_MEM_REQUEST,			// VC request for memory //

	VC_SM_MSG_TYPE_MAX
	};
enum vc_sm_alloc_type_t 					// Type of memory to be allocated //
	{
	VC_SM_ALLOC_CACHED,
	VC_SM_ALLOC_NON_CACHED,
	};
struct vc_sm_msg_hdr_t 						// Message header for all messages in HOST->VC direction //
	{
	u32 type;
	u32 trans_id;
	u8 body[0];
	};
struct vc_sm_alloc_t 						// Request to allocate memory (HOST->VC) //
	{
	enum vc_sm_alloc_type_t type;			// type of memory to allocate //
	u32 base_unit;							// byte amount of data to allocate per unit //
	u32 num_unit;							// number of unit to allocate //
	u32 alignment;							// alignment to be applied on allocation //
	u32 allocator;							// identity of who allocated this block //
	char name[VC_SM_RESOURCE_NAME];			// resource name (for easier tracking on vc side) //
	};
struct vc_sm_alloc_result_t 				// Result of a requested memory allocation (VC->HOST) //
	{
	u32 trans_id;							// Transaction identifier //
	u32 res_handle;							// Resource handle //
	u32 res_mem;							// Pointer to resource buffer //
	u32 res_base_size;						// Resource base size (bytes) //
	u32 res_num;							// Resource number //
	};
struct vc_sm_free_t 						// Request to free a previously allocated memory (HOST->VC) //
	{
	u32 res_handle;							// Resource handle (returned from alloc) //
	u32 res_mem;							// Resource buffer (returned from alloc) //
	};
struct vc_sm_lock_unlock_t 					// Request to lock a previously allocated memory (HOST->VC) //
	{
	u32 res_handle;							// Resource handle (returned from alloc) //
	u32 res_mem;							// Resource buffer (returned from alloc) //
	};
struct vc_sm_resize_t 						// Request to resize a previously allocated memory (HOST->VC) //
	{
	u32 res_handle;							// Resource handle (returned from alloc) //
	u32 res_mem;							// Resource buffer (returned from alloc) //
	u32 res_new_size;						// Resource *new* size requested (bytes) //
	};
struct vc_sm_lock_result_t 					// Result of a requested memory lock (VC->HOST) //
	{
	u32 trans_id;							// Transaction identifier //
	u32 res_handle;							// Resource handle //
	u32 res_mem;							// Pointer to resource buffer //
	u32 res_old_mem;						// Pointer to former resource buffer if the memory was reallocated
	};
struct vc_sm_result_t 						// Generic result for a request (VC->HOST) //
	{
	u32 trans_id;							// Transaction identifier //
	s32 success;
	};
struct vc_sm_action_clean_t 				// Request to revert a previously applied action (HOST->VC) //
	{
	enum vc_sm_msg_type res_action;			// Action of interest //
	u32 action_trans_id;					// Transaction identifier for the action of interest //
	};
struct vc_sm_free_all_t 					// Request to remove all data associated with a given allocator (HOST->VC) //
	{
	u32 allocator;							// Allocator identifier //
	};
struct vc_sm_import 						// Request to import memory (HOST->VC) //	
	{
	enum vc_sm_alloc_type_t type;			// type of memory to allocate //
	u32 addr;								// pointer to the VC (ie physical) address of the allocated memory //
	u32 size;								// size of buffer //
	u32 kernel_id;							// opaque handle returned in RELEASED messages //
	u32 allocator;							// Allocator identifier //
	char     name[VC_SM_RESOURCE_NAME];		// resource name (for easier tracking on vc side) //
	};
struct vc_sm_import_result 					// Result of a requested memory import (VC->HOST) //
	{
	u32 trans_id;							// Transaction identifier //
	u32 res_handle;							// Resource handle //
	};
struct vc_sm_released 						// Notification that VC has finished with an allocation (VC->HOST) //
	{
	u32 cmd;								// cmd type / trans_id //
	u32 addr;								// pointer to the VC (ie physical) address of the allocated memory //
	u32 size;								// size of buffer //
	u32 kernel_id;							// opaque handle returned in RELEASED messages //
	u32 vc_handle;
	};							
struct vc_sm_version 						// Client informing VC as to the protocol version it supports.
	{										// 
	u32 version;							// >=2 requires the released callback, and supports VC asking for memory. Failure means that the firmware doesn't support this call, and therefore the client should either fail, or NOT rely on getting the released callback.
	};										// 
struct vc_sm_vc_mem_request 				// Request FROM VideoCore for some memory //
	{
	u32 cmd;								// cmd type //
	u32 trans_id;							// trans_id (from VPU) //
	u32 size;								// size of buffer //
	u32 align;								// alignment of buffer //
	char     name[VC_SM_RESOURCE_NAME];		// resource name (for easier tracking) //
	u32 vc_handle;							// VPU handle for the resource //
	};
struct vc_sm_vc_mem_request_result 			// Response from the kernel to provide the VPU with some memory //
	{
	u32 trans_id;							// Transaction identifier for the VPU //
	u32 addr;								// pointer to the physical address of the allocated memory //
	u32 kernel_id;							// opaque handle returned in RELEASED messages //
	};
union vc_sm_msg_union_t 					// Union of ALL messages //
	{
	struct vc_sm_alloc_t alloc;
	struct vc_sm_alloc_result_t alloc_result;
	struct vc_sm_free_t free;
	struct vc_sm_lock_unlock_t lock_unlock;
	struct vc_sm_action_clean_t action_clean;
	struct vc_sm_resize_t resize;
	struct vc_sm_lock_result_t lock_result;
	struct vc_sm_result_t result;
	struct vc_sm_free_all_t free_all;
	struct vc_sm_import import;
	struct vc_sm_import_result import_result;
	struct vc_sm_version version;
	struct vc_sm_released released;
	struct vc_sm_vc_mem_request vc_request;
	struct vc_sm_vc_mem_request_result vc_request_result;
	};

#endif // __VC_SM_DEFS_H__INCLUDED__ //
