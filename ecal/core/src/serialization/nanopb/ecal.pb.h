/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.8 */

#ifndef PB_ECAL_PB_ECAL_PB_H_INCLUDED
#define PB_ECAL_PB_ECAL_PB_H_INCLUDED
#include <pb.h>
#include "host.pb.h"
#include "process.pb.h"
#include "service.pb.h"
#include "topic.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _eCAL_pb_eCmdType { /* Reserved fields in enums are not supported in protobuf 3.0
 reserved 7 to 11; */
    eCAL_pb_eCmdType_bct_none = 0, /* undefined command */
    eCAL_pb_eCmdType_bct_set_sample = 1, /* set sample content */
    eCAL_pb_eCmdType_bct_reg_publisher = 2, /* register publisher */
    eCAL_pb_eCmdType_bct_reg_subscriber = 3, /* register subscriber */
    eCAL_pb_eCmdType_bct_reg_process = 4, /* register process */
    eCAL_pb_eCmdType_bct_reg_service = 5, /* register service */
    eCAL_pb_eCmdType_bct_reg_client = 6, /* register client */
    eCAL_pb_eCmdType_bct_unreg_publisher = 12, /* unregister publisher */
    eCAL_pb_eCmdType_bct_unreg_subscriber = 13, /* unregister subscriber */
    eCAL_pb_eCmdType_bct_unreg_process = 14, /* unregister process */
    eCAL_pb_eCmdType_bct_unreg_service = 15, /* unregister service */
    eCAL_pb_eCmdType_bct_unreg_client = 16 /* unregister client */
} eCAL_pb_eCmdType;

/* Struct definitions */
typedef struct _eCAL_pb_Content { /* Reserved fields in enums are not supported in protobuf 3.0
 reserved 5; */
    int64_t id; /* sample id */
    int64_t clock; /* internal used clock */
    int64_t time; /* time the content was updated */
    pb_callback_t payload; /* octet stream */
    int32_t size; /* size (redundant for compatibility) */
    int64_t hash; /* unique hash for that sample */
} eCAL_pb_Content;

typedef struct _eCAL_pb_Sample {
    eCAL_pb_eCmdType cmd_type; /* sample command type */
    bool has_host;
    eCAL_pb_Host host; /* host information */
    bool has_process;
    eCAL_pb_Process process; /* process information */
    bool has_service;
    eCAL_pb_Service service; /* service information */
    bool has_topic;
    eCAL_pb_Topic topic; /* topic information */
    bool has_content;
    eCAL_pb_Content content; /* topic content */
    bool has_client;
    eCAL_pb_Client client; /* client information */
    pb_callback_t padding; /* padding to artificially increase the size of the message. This is a workaround for TCP topics, to get the actual user-payload 8-byte-aligned. REMOVE ME IN ECAL6 */
} eCAL_pb_Sample;

typedef struct _eCAL_pb_SampleList {
    pb_callback_t samples; /* list of Samples used currently by SHM registration */
} eCAL_pb_SampleList;


#ifdef __cplusplus
extern "C" {
#endif

/* Helper constants for enums */
#define _eCAL_pb_eCmdType_MIN eCAL_pb_eCmdType_bct_none
#define _eCAL_pb_eCmdType_MAX eCAL_pb_eCmdType_bct_unreg_client
#define _eCAL_pb_eCmdType_ARRAYSIZE ((eCAL_pb_eCmdType)(eCAL_pb_eCmdType_bct_unreg_client+1))


#define eCAL_pb_Sample_cmd_type_ENUMTYPE eCAL_pb_eCmdType



/* Initializer values for message structs */
#define eCAL_pb_Content_init_default             {0, 0, 0, {{NULL}, NULL}, 0, 0}
#define eCAL_pb_Sample_init_default              {_eCAL_pb_eCmdType_MIN, false, eCAL_pb_Host_init_default, false, eCAL_pb_Process_init_default, false, eCAL_pb_Service_init_default, false, eCAL_pb_Topic_init_default, false, eCAL_pb_Content_init_default, false, eCAL_pb_Client_init_default, {{NULL}, NULL}}
#define eCAL_pb_SampleList_init_default          {{{NULL}, NULL}}
#define eCAL_pb_Content_init_zero                {0, 0, 0, {{NULL}, NULL}, 0, 0}
#define eCAL_pb_Sample_init_zero                 {_eCAL_pb_eCmdType_MIN, false, eCAL_pb_Host_init_zero, false, eCAL_pb_Process_init_zero, false, eCAL_pb_Service_init_zero, false, eCAL_pb_Topic_init_zero, false, eCAL_pb_Content_init_zero, false, eCAL_pb_Client_init_zero, {{NULL}, NULL}}
#define eCAL_pb_SampleList_init_zero             {{{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define eCAL_pb_Content_id_tag                   1
#define eCAL_pb_Content_clock_tag                2
#define eCAL_pb_Content_time_tag                 3
#define eCAL_pb_Content_payload_tag              4
#define eCAL_pb_Content_size_tag                 6
#define eCAL_pb_Content_hash_tag                 7
#define eCAL_pb_Sample_cmd_type_tag              1
#define eCAL_pb_Sample_host_tag                  2
#define eCAL_pb_Sample_process_tag               3
#define eCAL_pb_Sample_service_tag               4
#define eCAL_pb_Sample_topic_tag                 5
#define eCAL_pb_Sample_content_tag               6
#define eCAL_pb_Sample_client_tag                7
#define eCAL_pb_Sample_padding_tag               8
#define eCAL_pb_SampleList_samples_tag           1

/* Struct field encoding specification for nanopb */
#define eCAL_pb_Content_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT64,    id,                1) \
X(a, STATIC,   SINGULAR, INT64,    clock,             2) \
X(a, STATIC,   SINGULAR, INT64,    time,              3) \
X(a, CALLBACK, SINGULAR, BYTES,    payload,           4) \
X(a, STATIC,   SINGULAR, INT32,    size,              6) \
X(a, STATIC,   SINGULAR, INT64,    hash,              7)
#define eCAL_pb_Content_CALLBACK pb_default_field_callback
#define eCAL_pb_Content_DEFAULT NULL

#define eCAL_pb_Sample_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    cmd_type,          1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  host,              2) \
X(a, STATIC,   OPTIONAL, MESSAGE,  process,           3) \
X(a, STATIC,   OPTIONAL, MESSAGE,  service,           4) \
X(a, STATIC,   OPTIONAL, MESSAGE,  topic,             5) \
X(a, STATIC,   OPTIONAL, MESSAGE,  content,           6) \
X(a, STATIC,   OPTIONAL, MESSAGE,  client,            7) \
X(a, CALLBACK, SINGULAR, BYTES,    padding,           8)
#define eCAL_pb_Sample_CALLBACK pb_default_field_callback
#define eCAL_pb_Sample_DEFAULT NULL
#define eCAL_pb_Sample_host_MSGTYPE eCAL_pb_Host
#define eCAL_pb_Sample_process_MSGTYPE eCAL_pb_Process
#define eCAL_pb_Sample_service_MSGTYPE eCAL_pb_Service
#define eCAL_pb_Sample_topic_MSGTYPE eCAL_pb_Topic
#define eCAL_pb_Sample_content_MSGTYPE eCAL_pb_Content
#define eCAL_pb_Sample_client_MSGTYPE eCAL_pb_Client

#define eCAL_pb_SampleList_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, MESSAGE,  samples,           1)
#define eCAL_pb_SampleList_CALLBACK pb_default_field_callback
#define eCAL_pb_SampleList_DEFAULT NULL
#define eCAL_pb_SampleList_samples_MSGTYPE eCAL_pb_Sample

extern const pb_msgdesc_t eCAL_pb_Content_msg;
extern const pb_msgdesc_t eCAL_pb_Sample_msg;
extern const pb_msgdesc_t eCAL_pb_SampleList_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define eCAL_pb_Content_fields &eCAL_pb_Content_msg
#define eCAL_pb_Sample_fields &eCAL_pb_Sample_msg
#define eCAL_pb_SampleList_fields &eCAL_pb_SampleList_msg

/* Maximum encoded size of messages (where known) */
/* eCAL_pb_Content_size depends on runtime parameters */
/* eCAL_pb_Sample_size depends on runtime parameters */
/* eCAL_pb_SampleList_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
