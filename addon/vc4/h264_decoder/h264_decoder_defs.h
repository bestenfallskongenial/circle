//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264_DECODER_DEFS.H
//----------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef _H264_DECODER_DEFS_H
#define _H264_DECODER_DEFS_H

#define LOG_NAME "VCSM"

#define STOREDEBUG_WHITESPACE 255
#define STOREDEBUG_NEWLINE '\n'

#define MMAL_FOURCC(a, b, c, d) ((a) | (b << 8) | (c << 16) | (d << 24)) // two times!
#define MMAL_MAGIC 						MMAL_FOURCC('m', 'm', 'a', 'l')


#define MMAL_EVENT_ERROR				MMAL_FOURCC('E', 'R', 'R', 'O')	// really????
#define MMAL_EVENT_EOS					MMAL_FOURCC('E', 'E', 'O', 'S')
#define MMAL_EVENT_FORMAT_CHANGED		MMAL_FOURCC('E', 'F', 'C', 'H')
#define MMAL_EVENT_PARAMETER_CHANGED	MMAL_FOURCC('E', 'P', 'C', 'H')

#define VC_MMAL_VER 15
#define VC_MMAL_MIN_VER 10

// max total message size is 512 bytes //
#define MMAL_MSG_MAX_SIZE 512
// with six 32bit header elements max payload is therefore 488 bytes //
#define MMAL_MSG_MAX_PAYLOAD 488


// #define MMAL_TIME_UNKNOWN BIT_ULL(63)                               // Special value signalling that time is not known //
// #define MMAL_TIME_UNKNOWN (1ULL << 63)

#define BIT(n)          (1U << (n))
#define BIT_ULL(n)      (1ULL << (n))
#define MMAL_TIME_UNKNOWN BIT_ULL(63)

#define MIN_BUFFERS 2                                                 // from CKernel ??
#define FIXED_BUFFER_SIZE (1024 * 1024) // 1024 KB

#define MMAL_MAX_DEBUG_FILE_LENGTH (1024 * 64)
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              FROM MMAL-VCHIQ.H
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define MAX_PORT_COUNT 4

// Maximum size of the format extradata. //
#define MMAL_FORMAT_EXTRADATA_MAX_SIZE 128

enum vchiq_mmal_es_type 
{
	MMAL_ES_TYPE_UNKNOWN,     // Unknown elementary stream type //
	MMAL_ES_TYPE_CONTROL,     // Elementary stream of control commands //
	MMAL_ES_TYPE_AUDIO,       // Audio elementary stream //
	MMAL_ES_TYPE_VIDEO,       // Video elementary stream //
	MMAL_ES_TYPE_SUBPICTURE   // Sub-picture elementary stream //
};
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              FROM MMAL-MSG-COMMON.H
//----------------------------------------------------------------------------------------------------------------------------------------------------
enum mmal_msg_status 
{
	MMAL_MSG_STATUS_SUCCESS = 0, // Success //
	MMAL_MSG_STATUS_ENOMEM,      // Out of memory //
	MMAL_MSG_STATUS_ENOSPC,      // Out of resources other than memory //
	MMAL_MSG_STATUS_EINVAL,      // Argument is invalid //
	MMAL_MSG_STATUS_ENOSYS,      // Function not implemented //
	MMAL_MSG_STATUS_ENOENT,      // No such file or directory //
	MMAL_MSG_STATUS_ENXIO,       // No such device or address //
	MMAL_MSG_STATUS_EIO,         // I/O error //
	MMAL_MSG_STATUS_ESPIPE,      // Illegal seek //
	MMAL_MSG_STATUS_ECORRUPT,    // Data is corrupt \attention //
	MMAL_MSG_STATUS_ENOTREADY,   // Component is not ready //
	MMAL_MSG_STATUS_ECONFIG,     // Component is not configured //
	MMAL_MSG_STATUS_EISCONN,     // Port is already connected //
	MMAL_MSG_STATUS_ENOTCONN,    // Port is disconnected //
	MMAL_MSG_STATUS_EAGAIN,      // Resource temporarily unavailable. //
	MMAL_MSG_STATUS_EFAULT,      // Bad address //
};

// <------------------------------------------------------------ do i need you??

struct mmal_rect 
{
	s32 x;      // x coordinate (from left) //
	s32 y;      // y coordinate (from top) //
	s32 width;  // width //
	s32 height; // height //
};

struct mmal_rational 
{
	s32 num;    // Numerator //
	s32 den;    // Denominator //
};
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              FROM MMAL-PARAMETERS.H
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define MMAL_PARAMETER_GROUP_COMMON		(0 << 16)


enum mmal_parameter_common_type 	/* Common parameters */
{
	/**< Never a valid parameter ID */
	MMAL_PARAMETER_UNUSED = MMAL_PARAMETER_GROUP_COMMON,

		/**< MMAL_PARAMETER_ENCODING_T */
	MMAL_PARAMETER_SUPPORTED_ENCODINGS,
		/**< MMAL_PARAMETER_URI_T */
	MMAL_PARAMETER_URI,
		/** MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T */
	MMAL_PARAMETER_CHANGE_EVENT_REQUEST,
		/** MMAL_PARAMETER_BOOLEAN_T */
	MMAL_PARAMETER_ZERO_COPY,
		/**< MMAL_PARAMETER_BUFFER_REQUIREMENTS_T */
	MMAL_PARAMETER_BUFFER_REQUIREMENTS,
		/**< MMAL_PARAMETER_STATISTICS_T */
	MMAL_PARAMETER_STATISTICS,
		/**< MMAL_PARAMETER_CORE_STATISTICS_T */
	MMAL_PARAMETER_CORE_STATISTICS,
		/**< MMAL_PARAMETER_MEM_USAGE_T */
	MMAL_PARAMETER_MEM_USAGE,
		/**< MMAL_PARAMETER_UINT32_T */
	MMAL_PARAMETER_BUFFER_FLAG_FILTER,
		/**< MMAL_PARAMETER_SEEK_T */
	MMAL_PARAMETER_SEEK,
		/**< MMAL_PARAMETER_BOOLEAN_T */
	MMAL_PARAMETER_POWERMON_ENABLE,
		/**< MMAL_PARAMETER_LOGGING_T */
	MMAL_PARAMETER_LOGGING,
		/**< MMAL_PARAMETER_UINT64_T */
	MMAL_PARAMETER_SYSTEM_TIME,
		/**< MMAL_PARAMETER_BOOLEAN_T */
	MMAL_PARAMETER_NO_IMAGE_PADDING,
};
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              FROM MMAL-MSG-FORMAT.H
//----------------------------------------------------------------------------------------------------------------------------------------------------
// MMAL_ES_FORMAT_T //
struct mmal_audio_format 
{
	u32 channels;		// Number of audio channels //
	u32 sample_rate;	// Sample rate //
	u32 bits_per_sample;	// Bits per sample //
	u32 block_align;	// Size of a block of data //
};
struct mmal_video_format 
{
	u32 width;		// Width of frame in pixels //
	u32 height;		// Height of frame in rows of pixels //
	struct mmal_rect crop;	// Visible region of the frame //
	struct mmal_rational frame_rate;	// Frame rate //
	struct mmal_rational par;		// Pixel aspect ratio //
	u32 color_space;	// FourCC specifying the color space of the video stream. See the MmalColorSpace "pre-defined color spaces" for some examples.
};

struct mmal_subpicture_format 
{
	u32 x_offset;
	u32 y_offset;
};

union mmal_es_specific_format 
{
	struct mmal_audio_format audio;
	struct mmal_video_format video;
	struct mmal_subpicture_format subpicture;
};


struct mmal_es_format_local 	// Definition of an elementary stream format (MMAL_ES_FORMAT_T) //
{
	u32 type;	// enum mmal_es_type //
	u32 encoding;	// FourCC specifying encoding of the elementary stream.
	u32 encoding_variant;	// FourCC specifying the specific encoding variant of the elementary stream.
	union mmal_es_specific_format *es;	// Type specific information for the elementary stream
	u32 bitrate;	// Bitrate in bits per second //
	u32 flags;	// Flags describing properties of the elementary stream.
	u32 extradata_size;	// Size of the codec specific data //
	u8  *extradata;		// Codec specific data //
};


struct mmal_es_format 	// Remote definition of an elementary stream format (MMAL_ES_FORMAT_T) //
{
	u32 type;	// enum mmal_es_type //
	u32 encoding;	// FourCC specifying encoding of the elementary stream.
	u32 encoding_variant;	// FourCC specifying the specific encoding variant of the elementary stream.
	u32 es;	// Type specific information for the elementary stream
	u32 bitrate;	// Bitrate in bits per second //
	u32 flags;	// Flags describing properties of the elementary stream.
	u32 extradata_size;	// Size of the codec specific data //
	u32 extradata;		// Codec specific data //
};
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              FROM MMAL-MSG-PORT.H
//----------------------------------------------------------------------------------------------------------------------------------------------------

enum mmal_port_type 	// MMAL_PORT_TYPE_T //
{
	MMAL_PORT_TYPE_UNKNOWN = 0,	// Unknown port type //
	MMAL_PORT_TYPE_CONTROL,		// Control port //
	MMAL_PORT_TYPE_INPUT,		// Input port //
	MMAL_PORT_TYPE_OUTPUT,		// Output port //
	MMAL_PORT_TYPE_CLOCK,		// Clock port //
};

// The port is pass-through and doesn't need buffer headers allocated //
#define MMAL_PORT_CAPABILITY_PASSTHROUGH                       0x01

 //The port wants to allocate the buffer payloads.
 // This signals a preference that payload allocation should be done
 // on this port for efficiency reasons.

#define MMAL_PORT_CAPABILITY_ALLOCATION                        0x02

 // The port supports format change events.
 // This applies to input ports and is used to let the client know
 // whether the port supports being reconfigured via a format
 // change event (i.e. without having to disable the port).

#define MMAL_PORT_CAPABILITY_SUPPORTS_EVENT_FORMAT_CHANGE      0x04

 // mmal port structure (MMAL_PORT_T)
 //
 // most elements are informational only, the pointer values for
 // interogation messages are generally provided as additional
 // structures within the message. When used to set values only the
 // buffer_num, buffer_size and userdata parameters are writable.

struct mmal_port 
{
	u32 priv;	// Private member used by the framework //
	u32 name;	// Port name. Used for debugging purposes (RO) //
	u32 type;	// Type of the port (RO) enum mmal_port_type //
	u16 index;	// Index of the port in its type list (RO) //
	u16 index_all;	// Index of the port in the list of all ports (RO) //
	u32 is_enabled;	// Indicates whether the port is enabled or not (RO) //
	u32 format;	// Format of the elementary stream //
	u32 buffer_num_min;	// Minimum number of buffers the port requires (RO).  This is set by the component.
	u32 buffer_size_min;	// Minimum size of buffers the port requires (RO).  This is set by the component.
	u32 buffer_alignment_min;// Minimum alignment requirement for the buffers (RO).  A value of zero means no special alignment requirements.  This is set by the component.
	u32 buffer_num_recommended;	// Number of buffers the port recommends for optimal performance (RO).  A value of zero means no special recommendation.  This is set by the component.
	u32 buffer_size_recommended;	// Size of buffers the port recommends for optimal performance (RO).  A value of zero means no special recommendation.  This is set by the component.
	u32 buffer_num;	// Actual number of buffers the port will use. This is set by the client.
	u32 buffer_size; // Actual maximum size of the buffers that will be sent to the port. This is set by the client.
	u32 component;	// Component this port belongs to (Read Only) //
	u32 userdata;	// Field reserved for use by the client //
	u32 capabilities;	// Flags describing the capabilities of a port (RO).  Bitwise combination of \ref portcapabilities "Port capabilities" values.
};
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              FROM MMAL-MSG.H
//----------------------------------------------------------------------------------------------------------------------------------------------------
enum mmal_msg_type 
{
	MMAL_MSG_TYPE_QUIT = 1,
	MMAL_MSG_TYPE_SERVICE_CLOSED,
	MMAL_MSG_TYPE_GET_VERSION,
	MMAL_MSG_TYPE_COMPONENT_CREATE,
	MMAL_MSG_TYPE_COMPONENT_DESTROY,	// 5 //
	MMAL_MSG_TYPE_COMPONENT_ENABLE,
	MMAL_MSG_TYPE_COMPONENT_DISABLE,
	MMAL_MSG_TYPE_PORT_INFO_GET,
	MMAL_MSG_TYPE_PORT_INFO_SET,
	MMAL_MSG_TYPE_PORT_ACTION,		// 10 //
	MMAL_MSG_TYPE_BUFFER_FROM_HOST,
	MMAL_MSG_TYPE_BUFFER_TO_HOST,
	MMAL_MSG_TYPE_GET_STATS,
	MMAL_MSG_TYPE_PORT_PARAMETER_SET,
	MMAL_MSG_TYPE_PORT_PARAMETER_GET,	// 15 //
	MMAL_MSG_TYPE_EVENT_TO_HOST,
	MMAL_MSG_TYPE_GET_CORE_STATS_FOR_PORT,
	MMAL_MSG_TYPE_OPAQUE_ALLOCATOR,
	MMAL_MSG_TYPE_CONSUME_MEM,
	MMAL_MSG_TYPE_LMK,			// 20 //
	MMAL_MSG_TYPE_OPAQUE_ALLOCATOR_DESC,
	MMAL_MSG_TYPE_DRM_GET_LHS32,
	MMAL_MSG_TYPE_DRM_GET_TIME,
	MMAL_MSG_TYPE_BUFFER_FROM_HOST_ZEROLEN,
	MMAL_MSG_TYPE_PORT_FLUSH,		// 25 //
	MMAL_MSG_TYPE_HOST_LOG,
	MMAL_MSG_TYPE_MSG_LAST
};


enum mmal_msg_port_action_type 	// port action request messages differ depending on the action type //
{
	MMAL_MSG_PORT_ACTION_TYPE_UNKNOWN = 0,	// Unknown action //
	MMAL_MSG_PORT_ACTION_TYPE_ENABLE,	// Enable a port //
	MMAL_MSG_PORT_ACTION_TYPE_DISABLE,	// Disable a port //
	MMAL_MSG_PORT_ACTION_TYPE_FLUSH,	// Flush a port //
	MMAL_MSG_PORT_ACTION_TYPE_CONNECT,	// Connect ports //
	MMAL_MSG_PORT_ACTION_TYPE_DISCONNECT,	// Disconnect ports //
	MMAL_MSG_PORT_ACTION_TYPE_SET_REQUIREMENTS, // Set buffer requirements//
};

struct mmal_msg_header 
{
	u32 magic;
	u32 type;	// enum mmal_msg_type //
	u32 control_service; // Opaque handle to the control service //
	u32 context;	// a u32 per message context //
	u32 status;	// The status of the vchiq operation //
	u32 padding;
};


struct mmal_msg_version 	// Send from VC to host to report version //
{
	u32 flags;
	u32 major;
	u32 minor;
	u32 minimum;
};


struct mmal_msg_component_create 	// request to VC to create component //
{
	u32 client_component;	// component context //
	char name[128];
	u32 pid;		// For debug //
};


struct mmal_msg_component_create_reply 	// reply from VC to component creation request //
{
	u32 status;	// enum mmal_msg_status - how does this differ to the one in the header?
	u32 component_handle; // VideoCore handle for component //
	u32 input_num;        // Number of input ports //
	u32 output_num;       // Number of output ports //
	u32 clock_num;        // Number of clock ports //
};

// request to VC to destroy a component //
struct mmal_msg_component_destroy 
{
	u32 component_handle;
};

struct mmal_msg_component_destroy_reply 
{
	u32 status; // The component destruction status //
};

// request and reply to VC to enable a component //
struct mmal_msg_component_enable 
{
	u32 component_handle;
};

struct mmal_msg_component_enable_reply 
{
	u32 status; // The component enable status //
};


struct mmal_msg_component_disable 	// request and reply to VC to disable a component //
{
	u32 component_handle;
};

struct mmal_msg_component_disable_reply 
{
	u32 status; // The component disable status //
};


struct mmal_msg_port_info_get 	// request to VC to get port information //
{
	u32 component_handle;  // component handle port is associated with //
	u32 port_type;         // enum mmal_msg_port_type //
	u32 index;             // port index to query //
};


struct mmal_msg_port_info_get_reply 	// reply from VC to get port info request //
{
	u32 status;		// enum mmal_msg_status //
	u32 component_handle;	// component handle port is associated with //
	u32 port_type;		// enum mmal_msg_port_type //
	u32 port_index;		// port indexed in query //
	s32 found;		// unused //
	u32 port_handle;	// Handle to use for this port //
	struct mmal_port port;
	struct mmal_es_format format; // elementary stream format //
	union mmal_es_specific_format es; // es type specific data //
	u8 extradata[MMAL_FORMAT_EXTRADATA_MAX_SIZE]; // es extra data //
};


struct mmal_msg_port_info_set 		// request to VC to set port information //
{
	u32 component_handle;
	u32 port_type;		// enum mmal_msg_port_type //
	u32 port_index;		// port indexed in query //
	struct mmal_port port;
	struct mmal_es_format format;
	union mmal_es_specific_format es;
	u8 extradata[MMAL_FORMAT_EXTRADATA_MAX_SIZE];
};


struct mmal_msg_port_info_set_reply 	// reply from VC to port info set request //
{
	u32 status;
	u32 component_handle;	// component handle port is associated with //
	u32 port_type;		// enum mmal_msg_port_type //
	u32 index;		// port indexed in query //
	s32 found;		// unused //
	u32 port_handle;	// Handle to use for this port //
	struct mmal_port port;
	struct mmal_es_format format;
	union mmal_es_specific_format es;
	u8 extradata[MMAL_FORMAT_EXTRADATA_MAX_SIZE];
};


struct mmal_msg_port_action_port 	// port action requests that take a mmal_port as a parameter //
{
	u32 component_handle;
	u32 port_handle;
	u32 action;		// enum mmal_msg_port_action_type //
	struct mmal_port port;
};


struct mmal_msg_port_action_handle 	// port action requests that take handles as a parameter //
{
	u32 component_handle;
	u32 port_handle;
	u32 action;		// enum mmal_msg_port_action_type //
	u32 connect_component_handle;
	u32 connect_port_handle;
};

struct mmal_msg_port_action_reply 
{
	u32 status;	// The port action operation status //
};

// MMAL buffer transfer //

// Size of space reserved in a buffer message for short messages. //
#define MMAL_VC_SHORT_DATA 128
// Signals that the current payload is the end of the stream of data //
#define MMAL_BUFFER_HEADER_FLAG_EOS                    BIT(0)
// Signals that the start of the current payload starts a frame //
#define MMAL_BUFFER_HEADER_FLAG_FRAME_START            BIT(1)
// Signals that the end of the current payload ends a frame //
#define MMAL_BUFFER_HEADER_FLAG_FRAME_END              BIT(2)
// Signals that the current payload contains only complete frames (>1) //
#define MMAL_BUFFER_HEADER_FLAG_FRAME                  \
	(MMAL_BUFFER_HEADER_FLAG_FRAME_START | \
	 MMAL_BUFFER_HEADER_FLAG_FRAME_END)
// Signals that the current payload is a keyframe (i.e. self decodable) //
#define MMAL_BUFFER_HEADER_FLAG_KEYFRAME               BIT(3)
// Signals a discontinuity in the stream of data (e.g. after a seek). Can be used for instance by a decoder to reset its state
#define MMAL_BUFFER_HEADER_FLAG_DISCONTINUITY          BIT(4)
// Signals a buffer containing some kind of config data for the component (e.g. codec config data)
#define MMAL_BUFFER_HEADER_FLAG_CONFIG                 BIT(5)
// Signals an encrypted payload //
#define MMAL_BUFFER_HEADER_FLAG_ENCRYPTED              BIT(6)
// Signals a buffer containing side information //
#define MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO          BIT(7)
// Signals a buffer which is the snapshot/postview image from a stills capture
#define MMAL_BUFFER_HEADER_FLAGS_SNAPSHOT              BIT(8)
// Signals a buffer which contains data known to be corrupted //
#define MMAL_BUFFER_HEADER_FLAG_CORRUPTED              BIT(9)
// Signals that a buffer failed to be transmitted //
#define MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED    BIT(10)
// Video buffer header flags
// videobufferheaderflags
// The following flags describe properties of a video buffer header.
// As there is no collision with the MMAL_BUFFER_HEADER_FLAGS_ defines, these
// flags will also be present in the MMAL_BUFFER_HEADER_T flags field.
#define MMAL_BUFFER_HEADER_FLAG_FORMAT_SPECIFIC_START_BIT 16
#define MMAL_BUFFER_HEADER_FLAG_FORMAT_SPECIFIC_START \
			(1 << MMAL_BUFFER_HEADER_FLAG_FORMAT_SPECIFIC_START_BIT)
// Signals an interlaced video frame //
#define MMAL_BUFFER_HEADER_VIDEO_FLAG_INTERLACED \
			(MMAL_BUFFER_HEADER_FLAG_FORMAT_SPECIFIC_START << 0)
// Signals that the top field of the current interlaced frame should be
// displayed first
#define MMAL_BUFFER_HEADER_VIDEO_FLAG_TOP_FIELD_FIRST \
			(MMAL_BUFFER_HEADER_FLAG_FORMAT_SPECIFIC_START << 1)

struct mmal_driver_buffer 
{
	u32 magic;
	u32 component_handle;
	u32 port_handle;
	u32 client_context;
};


struct mmal_buffer_header 	// buffer header //
{
	u32 next;	// next header //
	u32 priv;	// framework private data //
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

struct mmal_buffer_header_type_specific 
{
	union {
		struct {
		u32 planes;
		u32 offset[4];
		u32 pitch[4];
		u32 flags;
		} video;
	} u;
};

struct mmal_msg_buffer_from_host 
{
	 //The front 32 bytes of the buffer header are copied
	 // back to us in the reply to allow for context. This
	 // area is used to store two mmal_driver_buffer structures to
	 // allow for multiple concurrent service users.

	// control data //
	struct mmal_driver_buffer drvbuf;
	// referenced control data for passthrough buffer management //
	struct mmal_driver_buffer drvbuf_ref;
	struct mmal_buffer_header buffer_header; // buffer header itself //
	struct mmal_buffer_header_type_specific buffer_header_type_specific;
	s32 is_zero_copy;
	s32 has_reference;
	// allows short data to be xfered in control message //
	u32 payload_in_message;
	u8 short_data[MMAL_VC_SHORT_DATA];
};

// port parameter setting //

#define MMAL_WORKER_PORT_PARAMETER_SPACE      96

struct mmal_msg_port_parameter_set 
{
	u32 component_handle;	// component //
	u32 port_handle;	// port //
	u32 id;			// Parameter ID  //
	u32 size;		// Parameter size //
	u32 value[MMAL_WORKER_PORT_PARAMETER_SPACE];
};

struct mmal_msg_port_parameter_set_reply 
{
	u32 status;	// enum mmal_msg_status todo: how does this differ to the one in the header?
};

// port parameter getting //

struct mmal_msg_port_parameter_get 
{
	u32 component_handle;	// component //
	u32 port_handle;	// port //
	u32 id;			// Parameter ID  //
	u32 size;		// Parameter size //
};

struct mmal_msg_port_parameter_get_reply 
{
	u32 status;		// Status of mmal_port_parameter_get call //
	u32 id;			// Parameter ID  //
	u32 size;		// Parameter size //
	u32 value[MMAL_WORKER_PORT_PARAMETER_SPACE];
};

// event messages //
#define MMAL_WORKER_EVENT_SPACE 256

// Four CC's for events //
#define MMAL_FOURCC(a, b, c, d) ((a) | (b << 8) | (c << 16) | (d << 24))

#define MMAL_EVENT_ERROR		MMAL_FOURCC('E', 'R', 'R', 'O')
#define MMAL_EVENT_EOS			MMAL_FOURCC('E', 'E', 'O', 'S')
#define MMAL_EVENT_FORMAT_CHANGED	MMAL_FOURCC('E', 'F', 'C', 'H')
#define MMAL_EVENT_PARAMETER_CHANGED	MMAL_FOURCC('E', 'P', 'C', 'H')


struct mmal_msg_event_eos 		// Structs for each of the event message payloads //
{
	u32 port_type;	// Type of port that received the end of stream //
	u32 port_index;	// Index of port that received the end of stream //
};


struct mmal_msg_event_format_changed 	// Format changed event data. //
{
	u32 buffer_size_min;	// Minimum size of buffers the port requires //
	u32 buffer_num_min;		// Minimum number of buffers the port requires //
	u32 buffer_size_recommended;	// Size of buffers the port recommends for optimal performance. A value of zero means no special recommendation.
	u32 buffer_num_recommended;		// Number of buffers the port recommends for optimal performance. A value of zero means no special recommendation.

	u32 es_ptr;
	struct mmal_es_format format;
	union mmal_es_specific_format es;
	u8 extradata[MMAL_FORMAT_EXTRADATA_MAX_SIZE];
};

struct mmal_msg_event_to_host 
{
	u32 client_component;	// component context //
	u32 port_type;
	u32 port_num;
	u32 cmd;
	u32 length;
	u8 data[MMAL_WORKER_EVENT_SPACE];
	u32 delayed_buffer;
};

struct mmal_msg 	// all mmal messages are serialised through this structure //
{
	// header //
	struct mmal_msg_header h;
	// payload //
	union 
	{
		struct mmal_msg_version version;

		struct mmal_msg_component_create component_create;
		struct mmal_msg_component_create_reply component_create_reply;

		struct mmal_msg_component_destroy component_destroy;
		struct mmal_msg_component_destroy_reply component_destroy_reply;

		struct mmal_msg_component_enable component_enable;
		struct mmal_msg_component_enable_reply component_enable_reply;

		struct mmal_msg_component_disable component_disable;
		struct mmal_msg_component_disable_reply component_disable_reply;

		struct mmal_msg_port_info_get port_info_get;
		struct mmal_msg_port_info_get_reply port_info_get_reply;

		struct mmal_msg_port_info_set port_info_set;
		struct mmal_msg_port_info_set_reply port_info_set_reply;

		struct mmal_msg_port_action_port port_action_port;
		struct mmal_msg_port_action_handle port_action_handle;
		struct mmal_msg_port_action_reply port_action_reply;

		struct mmal_msg_buffer_from_host buffer_from_host;

		struct mmal_msg_port_parameter_set port_parameter_set;
		struct mmal_msg_port_parameter_set_reply
			port_parameter_set_reply;
		struct mmal_msg_port_parameter_get
			port_parameter_get;
		struct mmal_msg_port_parameter_get_reply
			port_parameter_get_reply;

		struct mmal_msg_event_to_host event_to_host;

		u8 payload[MMAL_MSG_MAX_PAYLOAD];
	} u;
};
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              FROM MMAL-ENCODINGS.H
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define MMAL_ENCODING_H264             MMAL_FOURCC('H', '2', '6', '4')

#define MMAL_ENCODING_I420             MMAL_FOURCC('I', '4', '2', '0')

// An EGL image handle
#define MMAL_ENCODING_EGL_IMAGE        MMAL_FOURCC('E', 'G', 'L', 'I')

// Pre-defined H264 encoding variants //

// ISO 14496-10 Annex B byte stream format //
#define MMAL_ENCODING_VARIANT_H264_DEFAULT   0
// ISO 14496-15 AVC stream format //
#define MMAL_ENCODING_VARIANT_H264_AVC1      MMAL_FOURCC('A', 'V', 'C', '1')
// Implicitly delineated NAL units without emulation prevention //
#define MMAL_ENCODING_VARIANT_H264_RAW       MMAL_FOURCC('R', 'A', 'W', ' ')

// \defgroup MmalColorSpace List of pre-defined video color spaces
// This defines a list of common color spaces. This list isn't exhaustive and
// is only provided as a convenience to avoid clients having to use FourCC
// codes directly. However components are allowed to define and use their own
// FourCC codes.

// Unknown color space //
#define MMAL_COLOR_SPACE_UNKNOWN       0
// ITU-R BT.601-5 [SDTV] //
#define MMAL_COLOR_SPACE_ITUR_BT601    MMAL_FOURCC('Y', '6', '0', '1')
// ITU-R BT.709-3 [HDTV] //
#define MMAL_COLOR_SPACE_ITUR_BT709    MMAL_FOURCC('Y', '7', '0', '9')
// JPEG JFIF //
#define MMAL_COLOR_SPACE_JPEG_JFIF     MMAL_FOURCC('Y', 'J', 'F', 'I')
// Title 47 Code of Federal Regulations (2003) 73.682 (a) (20) //
#define MMAL_COLOR_SPACE_FCC           MMAL_FOURCC('Y', 'F', 'C', 'C')
// Society of Motion Picture and Television Engineers 240M (1999) //
#define MMAL_COLOR_SPACE_SMPTE240M     MMAL_FOURCC('Y', '2', '4', '0')
// ITU-R BT.470-2 System M //
#define MMAL_COLOR_SPACE_BT470_2_M     MMAL_FOURCC('Y', '_', '_', 'M')
// ITU-R BT.470-2 System BG //
#define MMAL_COLOR_SPACE_BT470_2_BG    MMAL_FOURCC('Y', '_', 'B', 'G')
// JPEG JFIF, but with 16..255 luma //
#define MMAL_COLOR_SPACE_JFIF_Y16_255  MMAL_FOURCC('Y', 'Y', '1', '6')
// @} MmalColorSpace List //

#endif // _H264_DECODER_DEFS_H
//----------------------------------------------------------------------------------------------------------------------------------------------------
//              H264 Decoder Setup Code
//----------------------------------------------------------------------------------------------------------------------------------------------------