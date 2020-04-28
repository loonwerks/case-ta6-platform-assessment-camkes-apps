#pragma once

// The event data port type. Sender and receiver type must match. Many ports can use the same type.
// The data representation is independent of the queue representation.
//
// NOTE: data_t must define a type for which assignment will copy all the
// data. For example, pointers do not work.

#include <stdint.h>
#include <sys/types.h>

//#define DATA_T_MAX_PAYLOAD (4096 - 2 * sizeof(size_t))
#define DATA_T_MAX_PAYLOAD (1024 - 2 * sizeof(size_t))

typedef struct data {
  size_t len;
  uint8_t payload[DATA_T_MAX_PAYLOAD];
} data_t;

