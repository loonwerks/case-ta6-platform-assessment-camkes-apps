/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * Copyright 2019 Adventium Labs
 * Modifications made to original
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_Adventium_BSD)
 */

// Single sender multiple receiver Queue implementation for AADL Event Data
// Ports. Every receiver receives the sent data (ie brodcast). The queue
// operations are all non-blocking. The sender enqueue always succeeds. A
// receiver dequeue can fail and drop data if the sender writes while the
// receiver is reading. This situation is detected unless the sender gets
// ahead of a receiver by more than COUNTER_MAX. Since COUNTER_MAX is typically
// 2^64 (see counter.h), this is extremely unlikely. If it does happen the
// only adverse effect is that the receiver will not detect all dropped
// elements.

#pragma once

#ifdef __cplusplus
#extern "C" {
#endif

#include <stdbool.h>

// The event data port type. Sender and receiver type must match. Many ports can use the same type.
// The data representation is independent of the queue representation.
//
// NOTE: data_t must define a type for which assignment will copy all the
// data. For example, pointers do not work.

#include <stdint.h>
#include <sys/types.h>


// Defs to allow easy changing of counter type. Keep these consistent!
typedef uintmax_t camkes_log_counter_t;
#define CAMKES_LOG_COUNTER_MAX UINTMAX_MAX
#define PRIcamkes_log_counter PRIuMAX


#define CAMKES_LOG_DATA_T_MAX_PAYLOAD (512 - sizeof(uint32_t) - sizeof(unsigned long long int))

typedef struct camkes_log_data {
  uint32_t payload_length;
  uint8_t payload[CAMKES_LOG_DATA_T_MAX_PAYLOAD];
} camkes_log_data_t;


// Queue size must be an integer factor of the size for counter_t (an unsigned
// integer type). Since we are using standard C unsigned integers for the
// counter, picking a queue size that is a power of 2 is a good choice. We
// could alternatively set the size of our counter to the largest possible
// multiple of queue size. But then we would need to do our own modulo
// operations on the counter rather than depending on c's unsigned integer
// operations.
//
// Note: One cell in the queue is always considered dirty. Its the next
// element to be written. Thus the queue can only contain QUEUE_SIZE-1
// elements.
#define QUEUE_SIZE 16

// This is the type of the seL4 dataport (shared memory) that is shared by the
// sender and all receivers. This type is referenced in the sender and receiver
// CAmkES component definition files. The seL4 CAmkES runtime creates an
// instance of this struct.
typedef struct camkes_log_queue {
  // Number of elements enqueued since the sender. The implementation depends
  // on C's standard module behaviour for unsigned integers. The counter never
  // overflows. It just wraps modulo the size of the counter type. The counter
  // is typically very large (see counter.h), so this should happen very
  // infrequently. Depending in C to initialize this to zero.
  _Atomic camkes_log_counter_t numSent;
  // Queue of elements of type data_t (see data.h) implemented as a ring buffer.
  // No initialization necessary.
  camkes_log_data_t elt[QUEUE_SIZE];
} camkes_log_queue_t;

//------------------------------------------------------------------------------
// Sender API
//
// Could split this into separate header and source file since only sender
// code needs this.

// Initialize the queue. Sender must call this exactly once before any calls to queue_enqueue();
void camkes_log_queue_init(camkes_log_queue_t *queue);

// Enqueue data. This always succeeds and never blocks. Data is copied.
void camkes_log_queue_enqueue(camkes_log_queue_t *queue, camkes_log_data_t *data);

//------------------------------------------------------------------------------
// Receiver API
//
// Could split this into separate header and source file since only receiver
// code needs this.

// Each receiver needs to create an instance of this.
typedef struct camkes_log_recv_queue {
  // Number of elements dequeued (or dropped) by a receiver. The implementation
  // depends on C's standard module behaviour for unsigned integers. The
  // counter never overflows. It just wraps modulo the size of the counter
  // type. The counter is typically very large (see counter.h), so this should
  // happen very infrequently.
  camkes_log_counter_t numRecv;
  // Pointer to the actual queue. This is the seL4 dataport (shared memory)
  // that is shared by the sender and all receivers.
  camkes_log_queue_t *queue;
} camkes_log_recv_queue_t;

// Each receiver must call this exactly once before any calls to other queue
// API functions.
void camkes_log_recv_queue_init(camkes_log_recv_queue_t *recvQueue, camkes_log_queue_t *queue);

// Dequeue data. Never blocks but can fail if the sender writes at same
// time. 

// When successful returns true. The dequeued data will be copied to
// *data. *numDropped will contain the number of elements that were dropped
// since the last call to queue_dequeue().
//
// When queue is empty, returns false and *numDropped is zero. *data is left in
// unspecified state.
//
// When dequeue fails due to possible write of data being read, returns false
// and *numDropped will be >= 1 specifying the number of elements that were
// dropped since the last call to queue_dequeue(). *data is left in
// unspecified state.
//
// If the sender ever gets ahead of a receiver by more than COUNTER_MAX,
// recv_dequeue will fail to count a multiple of COUNTER_MAX in
// numDropped. Since COUNTER_MAX is very large (typically on the order of 2^64,
// see counter.h), this is very unlikely.  If the sender is ever this far
// ahead of a receiver the system is probably in a very bad state.
bool camkes_log_queue_dequeue(camkes_log_recv_queue_t *recvQueue, camkes_log_counter_t *numDropped, camkes_log_data_t *data);

// Is queue empty? If the queue is not empty, it will stay that way until the
// receiver dequeues all data. If the queue is empty you can make no
// assumptions about how long it will stay empty.
bool camkes_log_queue_is_empty(camkes_log_recv_queue_t *recvQueue); 

#ifdef __cplusplus
}
#endif


