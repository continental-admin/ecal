/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.8 */

#ifndef PB_ECAL_PB_PROCESS_PB_H_INCLUDED
#define PB_ECAL_PB_PROCESS_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _eCAL_pb_eProcessSeverity {
    eCAL_pb_eProcessSeverity_proc_sev_unknown = 0, /* condition unknown */
    eCAL_pb_eProcessSeverity_proc_sev_healthy = 1, /* process healthy */
    eCAL_pb_eProcessSeverity_proc_sev_warning = 2, /* process warning level */
    eCAL_pb_eProcessSeverity_proc_sev_critical = 3, /* process critical */
    eCAL_pb_eProcessSeverity_proc_sev_failed = 4 /* process failed */
} eCAL_pb_eProcessSeverity;

typedef enum _eCAL_pb_eProcessSeverityLevel {
    eCAL_pb_eProcessSeverityLevel_proc_sev_level_unknown = 0, /* condition unknown */
    eCAL_pb_eProcessSeverityLevel_proc_sev_level1 = 1, /* default severity level 1 */
    eCAL_pb_eProcessSeverityLevel_proc_sev_level2 = 2, /* severity level 2 */
    eCAL_pb_eProcessSeverityLevel_proc_sev_level3 = 3, /* severity level 3 */
    eCAL_pb_eProcessSeverityLevel_proc_sev_level4 = 4, /* severity level 4 */
    eCAL_pb_eProcessSeverityLevel_proc_sev_level5 = 5 /* severity level 5 */
} eCAL_pb_eProcessSeverityLevel;

typedef enum _eCAL_pb_eTSyncState {
    eCAL_pb_eTSyncState_tsync_none = 0, /* not synchronized */
    eCAL_pb_eTSyncState_tsync_realtime = 1, /* real time sync mode */
    eCAL_pb_eTSyncState_tsync_replay = 2 /* replay time sync mode */
} eCAL_pb_eTSyncState;

/* Struct definitions */
typedef struct _eCAL_pb_ProcessState {
    eCAL_pb_eProcessSeverity severity; /* severity */
    pb_callback_t info; /* info string */
    eCAL_pb_eProcessSeverityLevel severity_level; /* severity level */
} eCAL_pb_ProcessState;

typedef struct _eCAL_pb_Process { /* Reserved fields in enums are not supported in protobuf 3.0
 reserved 7 to 11; */
    int32_t rclock; /* registration clock */
    pb_callback_t hname; /* host name */
    int32_t pid; /* process id */
    pb_callback_t pname; /* process name */
    pb_callback_t uname; /* unit name */
    pb_callback_t pparam; /* process parameter */
    /* 10 = data write bytes per sec
 11 = data read bytes per sec */
    bool has_state;
    eCAL_pb_ProcessState state; /* process state info */
    eCAL_pb_eTSyncState tsync_state; /* time synchronization state */
    pb_callback_t tsync_mod_name; /* time synchronization module name */
    int32_t component_init_state; /* eCAL component initialization state (eCAL::Initialize(..)) */
    pb_callback_t component_init_info; /* like comp_init_state as human readable string (pub|sub|srv|mon|log|time|proc) */
    pb_callback_t ecal_runtime_version; /* loaded / runtime eCAL version of a component */
    pb_callback_t hgname; /* host group name */
} eCAL_pb_Process;


#ifdef __cplusplus
extern "C" {
#endif

/* Helper constants for enums */
#define _eCAL_pb_eProcessSeverity_MIN eCAL_pb_eProcessSeverity_proc_sev_unknown
#define _eCAL_pb_eProcessSeverity_MAX eCAL_pb_eProcessSeverity_proc_sev_failed
#define _eCAL_pb_eProcessSeverity_ARRAYSIZE ((eCAL_pb_eProcessSeverity)(eCAL_pb_eProcessSeverity_proc_sev_failed+1))

#define _eCAL_pb_eProcessSeverityLevel_MIN eCAL_pb_eProcessSeverityLevel_proc_sev_level_unknown
#define _eCAL_pb_eProcessSeverityLevel_MAX eCAL_pb_eProcessSeverityLevel_proc_sev_level5
#define _eCAL_pb_eProcessSeverityLevel_ARRAYSIZE ((eCAL_pb_eProcessSeverityLevel)(eCAL_pb_eProcessSeverityLevel_proc_sev_level5+1))

#define _eCAL_pb_eTSyncState_MIN eCAL_pb_eTSyncState_tsync_none
#define _eCAL_pb_eTSyncState_MAX eCAL_pb_eTSyncState_tsync_replay
#define _eCAL_pb_eTSyncState_ARRAYSIZE ((eCAL_pb_eTSyncState)(eCAL_pb_eTSyncState_tsync_replay+1))

#define eCAL_pb_ProcessState_severity_ENUMTYPE eCAL_pb_eProcessSeverity
#define eCAL_pb_ProcessState_severity_level_ENUMTYPE eCAL_pb_eProcessSeverityLevel

#define eCAL_pb_Process_tsync_state_ENUMTYPE eCAL_pb_eTSyncState


/* Initializer values for message structs */
#define eCAL_pb_ProcessState_init_default        {_eCAL_pb_eProcessSeverity_MIN, {{NULL}, NULL}, _eCAL_pb_eProcessSeverityLevel_MIN}
#define eCAL_pb_Process_init_default             {0, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, false, eCAL_pb_ProcessState_init_default, _eCAL_pb_eTSyncState_MIN, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}
#define eCAL_pb_ProcessState_init_zero           {_eCAL_pb_eProcessSeverity_MIN, {{NULL}, NULL}, _eCAL_pb_eProcessSeverityLevel_MIN}
#define eCAL_pb_Process_init_zero                {0, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, false, eCAL_pb_ProcessState_init_zero, _eCAL_pb_eTSyncState_MIN, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define eCAL_pb_ProcessState_severity_tag        1
#define eCAL_pb_ProcessState_info_tag            2
#define eCAL_pb_ProcessState_severity_level_tag  3
#define eCAL_pb_Process_rclock_tag               1
#define eCAL_pb_Process_hname_tag                2
#define eCAL_pb_Process_pid_tag                  3
#define eCAL_pb_Process_pname_tag                4
#define eCAL_pb_Process_uname_tag                5
#define eCAL_pb_Process_pparam_tag               6
#define eCAL_pb_Process_state_tag                12
#define eCAL_pb_Process_tsync_state_tag          13
#define eCAL_pb_Process_tsync_mod_name_tag       14
#define eCAL_pb_Process_component_init_state_tag 15
#define eCAL_pb_Process_component_init_info_tag  16
#define eCAL_pb_Process_ecal_runtime_version_tag 17
#define eCAL_pb_Process_hgname_tag               18

/* Struct field encoding specification for nanopb */
#define eCAL_pb_ProcessState_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    severity,          1) \
X(a, CALLBACK, SINGULAR, STRING,   info,              2) \
X(a, STATIC,   SINGULAR, UENUM,    severity_level,    3)
#define eCAL_pb_ProcessState_CALLBACK pb_default_field_callback
#define eCAL_pb_ProcessState_DEFAULT NULL

#define eCAL_pb_Process_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    rclock,            1) \
X(a, CALLBACK, SINGULAR, STRING,   hname,             2) \
X(a, STATIC,   SINGULAR, INT32,    pid,               3) \
X(a, CALLBACK, SINGULAR, STRING,   pname,             4) \
X(a, CALLBACK, SINGULAR, STRING,   uname,             5) \
X(a, CALLBACK, SINGULAR, STRING,   pparam,            6) \
X(a, STATIC,   OPTIONAL, MESSAGE,  state,            12) \
X(a, STATIC,   SINGULAR, UENUM,    tsync_state,      13) \
X(a, CALLBACK, SINGULAR, STRING,   tsync_mod_name,   14) \
X(a, STATIC,   SINGULAR, INT32,    component_init_state,  15) \
X(a, CALLBACK, SINGULAR, STRING,   component_init_info,  16) \
X(a, CALLBACK, SINGULAR, STRING,   ecal_runtime_version,  17) \
X(a, CALLBACK, SINGULAR, STRING,   hgname,           18)
#define eCAL_pb_Process_CALLBACK pb_default_field_callback
#define eCAL_pb_Process_DEFAULT NULL
#define eCAL_pb_Process_state_MSGTYPE eCAL_pb_ProcessState

extern const pb_msgdesc_t eCAL_pb_ProcessState_msg;
extern const pb_msgdesc_t eCAL_pb_Process_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define eCAL_pb_ProcessState_fields &eCAL_pb_ProcessState_msg
#define eCAL_pb_Process_fields &eCAL_pb_Process_msg

/* Maximum encoded size of messages (where known) */
/* eCAL_pb_ProcessState_size depends on runtime parameters */
/* eCAL_pb_Process_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
