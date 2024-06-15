#ifndef BoxStatus_h
#define BoxStatus_h

typedef enum {
  CONFIG_STATUS_PENDING,
  CONFIG_STATUS_PRESENT,
  CONFIG_STATUS_MISSING,
  CONFIG_STATUS__LOADED,
  CONFIG_STATUS__PARSED
} config_status_t;

#endif