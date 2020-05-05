#pragma once

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


/**
 * Assuming the following strings are never present within the combined string
 * containing payload size, payload and payload checksum:
 *
 *     "+="
 *     "=+"
 *     "#@"
 *     "@#"
 *     "!%"
 *     "%!"
 *     "?^"
 *     "^?"
 *
 *  8-character markers:
 *
 *     "+=+=+=+=123#@#@#@#@"          lmcpObjPayloadSize
 *     "#@#@#@#@abcxyz123!%!%!%!%"    rawPayload
 *     "!%!%!%!%789?^?^?^?^"          lmcpObjPayloadChksum
 *     "+=+=+=+=123#@#@#@#@abcxyz123!%!%!%!%789?^?^?^?^" lmcpObjPayloadSize, rawPayload and lmcpObjPayloadChksum (combined int
 */


#define SENTINEL_SERIAL_BUFFER_RING_SIZE (x10000)


typedef struct sentinel_serial_buffer {
  /**
   * multi-thread safety not implemented
   * @param data
   * @return 
   */
  uint8_t data[SENTINEL_SERIAL_BUFFER_RING_SIZE];
  size_t write_offset;
  size_t read_offset;
  uint32_t valid_deserialize_count;
  uint32_t invalid_deserialize_count;
  uint32_t disregarded_data_count;
} sentinel_serial_buffer_t;


struct sentinel_serial_buffer *sentinel_serial_buffer_alloc();


void sentinel_serial_buffer_free(struct sentinel_serial_buffer *ctx);


uint32_t calculate_checksum(const uint8_t *buffer, size_t length);


uint32_t calculate_checksum_in_ctx(const struct sentinel_serial_buffer *ctx, size_t start_offset, size_t length);


bool append_sentinelized_string(struct sentinel_serial_buffer *ctx, const uint8_t buffer, size_t length);


bool append_char(struct sentinel_serial_buffer *ctx, uint8_t c);


ssize_t get_next_payload_string(struct sentinel_serial_buffer *ctx, uint8_t *buffer, size_t buffer_size);
