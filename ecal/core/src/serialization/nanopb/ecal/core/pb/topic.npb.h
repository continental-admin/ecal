/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.9.1 */

#ifndef PB_ECAL_PB_TOPIC_NPB_H_INCLUDED
#define PB_ECAL_PB_TOPIC_NPB_H_INCLUDED
#include <pb.h>
#include "ecal/core/pb/layer.npb.h"
#include "ecal/core/pb/datatype.npb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _eCAL_pb_Topic { /* Reserved fields in enums are not supported in protobuf 3.0
 reserved 9, 10, 11, 14, 15, 22 to 26, 29; */
    int32_t registration_clock; /* registration clock (heart beat) */
    pb_callback_t host_name; /* host name */
    int32_t process_id; /* process id */
    pb_callback_t process_name; /* process name */
    pb_callback_t unit_name; /* unit name */
    pb_callback_t topic_id; /* topic id */
    pb_callback_t topic_name; /* topic name */
    pb_callback_t direction; /* direction (publisher, subscriber) */
    pb_callback_t transport_layer; /* active topic transport layers and it's specific parameter */
    int32_t topic_size; /* topic size */
    int32_t connections_loc; /* number of local connected entities */
    int32_t connections_ext; /* number of external connected entities */
    int32_t message_drops; /* dropped messages */
    int64_t did; /* data send id (publisher setid) */
    int64_t dclock; /* data clock (send / receive action) */
    int32_t dfreq; /* data frequency (send / receive samples per second) [mHz] */
    pb_callback_t attr; /* generic topic description */
    pb_callback_t shm_transport_domain; /* shm_transport_domain */
    /* 9 = topic type + topic encoding (deprecated)
 10 = topic description (protocol descriptor) (deprecated) */
    bool has_datatype_information;
    eCAL_pb_DataTypeInformation datatype_information; /* topic datatype information (encoding & type & description) */
} eCAL_pb_Topic;

typedef struct _eCAL_pb_Topic_AttrEntry {
    pb_callback_t key;
    pb_callback_t value;
} eCAL_pb_Topic_AttrEntry;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define eCAL_pb_Topic_init_default               {0, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, 0, 0, 0, 0, 0, 0, 0, {{NULL}, NULL}, {{NULL}, NULL}, false, eCAL_pb_DataTypeInformation_init_default}
#define eCAL_pb_Topic_AttrEntry_init_default     {{{NULL}, NULL}, {{NULL}, NULL}}
#define eCAL_pb_Topic_init_zero                  {0, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, 0, 0, 0, 0, 0, 0, 0, {{NULL}, NULL}, {{NULL}, NULL}, false, eCAL_pb_DataTypeInformation_init_zero}
#define eCAL_pb_Topic_AttrEntry_init_zero        {{{NULL}, NULL}, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define eCAL_pb_Topic_registration_clock_tag     1
#define eCAL_pb_Topic_host_name_tag              2
#define eCAL_pb_Topic_process_id_tag             3
#define eCAL_pb_Topic_process_name_tag           4
#define eCAL_pb_Topic_unit_name_tag              5
#define eCAL_pb_Topic_topic_id_tag               6
#define eCAL_pb_Topic_topic_name_tag             7
#define eCAL_pb_Topic_direction_tag              8
#define eCAL_pb_Topic_transport_layer_tag        12
#define eCAL_pb_Topic_topic_size_tag             13
#define eCAL_pb_Topic_connections_loc_tag        16
#define eCAL_pb_Topic_connections_ext_tag        17
#define eCAL_pb_Topic_message_drops_tag          18
#define eCAL_pb_Topic_did_tag                    19
#define eCAL_pb_Topic_dclock_tag                 20
#define eCAL_pb_Topic_dfreq_tag                  21
#define eCAL_pb_Topic_attr_tag                   27
#define eCAL_pb_Topic_shm_transport_domain_tag   28
#define eCAL_pb_Topic_datatype_information_tag   30
#define eCAL_pb_Topic_AttrEntry_key_tag          1
#define eCAL_pb_Topic_AttrEntry_value_tag        2

/* Struct field encoding specification for nanopb */
#define eCAL_pb_Topic_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    registration_clock,   1) \
X(a, CALLBACK, SINGULAR, STRING,   host_name,         2) \
X(a, STATIC,   SINGULAR, INT32,    process_id,        3) \
X(a, CALLBACK, SINGULAR, STRING,   process_name,      4) \
X(a, CALLBACK, SINGULAR, STRING,   unit_name,         5) \
X(a, CALLBACK, SINGULAR, STRING,   topic_id,          6) \
X(a, CALLBACK, SINGULAR, STRING,   topic_name,        7) \
X(a, CALLBACK, SINGULAR, STRING,   direction,         8) \
X(a, CALLBACK, REPEATED, MESSAGE,  transport_layer,  12) \
X(a, STATIC,   SINGULAR, INT32,    topic_size,       13) \
X(a, STATIC,   SINGULAR, INT32,    connections_loc,  16) \
X(a, STATIC,   SINGULAR, INT32,    connections_ext,  17) \
X(a, STATIC,   SINGULAR, INT32,    message_drops,    18) \
X(a, STATIC,   SINGULAR, INT64,    did,              19) \
X(a, STATIC,   SINGULAR, INT64,    dclock,           20) \
X(a, STATIC,   SINGULAR, INT32,    dfreq,            21) \
X(a, CALLBACK, REPEATED, MESSAGE,  attr,             27) \
X(a, CALLBACK, SINGULAR, STRING,   shm_transport_domain,  28) \
X(a, STATIC,   OPTIONAL, MESSAGE,  datatype_information,  30)
#define eCAL_pb_Topic_CALLBACK pb_default_field_callback
#define eCAL_pb_Topic_DEFAULT NULL
#define eCAL_pb_Topic_transport_layer_MSGTYPE eCAL_pb_TransportLayer
#define eCAL_pb_Topic_attr_MSGTYPE eCAL_pb_Topic_AttrEntry
#define eCAL_pb_Topic_datatype_information_MSGTYPE eCAL_pb_DataTypeInformation

#define eCAL_pb_Topic_AttrEntry_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   key,               1) \
X(a, CALLBACK, SINGULAR, STRING,   value,             2)
#define eCAL_pb_Topic_AttrEntry_CALLBACK pb_default_field_callback
#define eCAL_pb_Topic_AttrEntry_DEFAULT NULL

extern const pb_msgdesc_t eCAL_pb_Topic_msg;
extern const pb_msgdesc_t eCAL_pb_Topic_AttrEntry_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define eCAL_pb_Topic_fields &eCAL_pb_Topic_msg
#define eCAL_pb_Topic_AttrEntry_fields &eCAL_pb_Topic_AttrEntry_msg

/* Maximum encoded size of messages (where known) */
/* eCAL_pb_Topic_size depends on runtime parameters */
/* eCAL_pb_Topic_AttrEntry_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
