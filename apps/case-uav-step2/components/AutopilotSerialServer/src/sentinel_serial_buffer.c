#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "sentinel_serial_buffer.h"


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


#define PAYLOAD_SIZE_BUFFER_SIZE 32
#define CHECKSUM_BUFFER_SIZE 32


static const uint8_t serial_sentinel_before_payload_size[] = "+=+=+=+=";


static const uint8_t serial_sentinel_after_payload_size[] = "#@#@#@#@";


static const uint8_t serial_sentinel_before_checksum[] = "!%!%!%!%";


static const uint8_t serial_sentinel_after_checksum[] = "?^?^?^?^";


struct sentinel_serial_buffer *sentinel_serial_buffer_alloc() {
  return calloc(1, sizeof(struct sentinel_serial_buffer));
}


void sentinel_serial_buffer_free(struct sentinel_serial_buffer *ctx) {
  free(ctx);
}


uint32_t calculate_checksum(const uint8_t *buffer, size_t length) {
  uint32_t checksum = 0;
  for (size_t index = 0; index < length; ++index) {
    checksum += (uint32_t) buffer[index];
  }
  return checksum;
}


uint32_t calculate_checksum_in_ctx(const struct sentinel_serial_buffer *ctx, size_t start_offset, size_t length) {
  uint32_t checksum = 0;
  for (size_t index = 0; index < length; ++index) {
    checksum += (uint32_t) ctx->data[(start_offset + index) % SENTINEL_SERIAL_BUFFER_RING_SIZE];
  }
  return checksum;
}


int append_sentinelized_string(struct sentinel_serial_buffer *ctx, const uint8_t *buffer, size_t length) {
  size_t capacity_remaining = (ctx->write_offset + SENTINEL_SERIAL_BUFFER_RING_SIZE - ctx->read_offset)
    % SENTINEL_SERIAL_BUFFER_RING_SIZE;
  
  uint8_t payload_size_buffer[32] = { 0 };
  snprintf((char *) payload_size_buffer, sizeof(payload_size_buffer), "%z", length);
  size_t payload_size_length = strnlen(payload_size_buffer, sizeof(payload_size_buffer));

  uint8_t payload_checksum_buffer[32] = { 0 };
  uint32_t payload_checksum  = calculate_checksum(buffer, length);
  snprintf((char *) payload_checksum_buffer, sizeof(payload_checksum_buffer), "%u", payload_checksum);
  size_t payload_checksum_length = strnlen(payload_checksum_buffer, sizeof(payload_checksum_buffer));

  size_t sentinelized_length = length
    + sizeof(serial_sentinel_before_payload_size)
    + payload_size_length
    + sizeof(serial_sentinel_after_payload_size)
    + sizeof(serial_sentinel_before_checksum)
    + payload_checksum_length
    + sizeof(serial_sentinel_after_checksum);

  if (sentinelized_length <= capacity_remaining) {

    // Sentinel before payload size
    for (size_t index = 0; index < sizeof(serial_sentinel_before_payload_size);
	 ++index, ctx->write_offset = (ctx->write_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE) {
      ctx->data[ctx->write_offset] = serial_sentinel_before_payload_size[index];
    }

    // Payload size
    for (size_t index = 0; index < payload_size_length;
	 ++index, ctx->write_offset = (ctx->write_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE) {
      ctx->data[ctx->write_offset] = payload_size_buffer[index];
    }

    // Sentinel after payload size
    for (size_t index = 0; index < sizeof(serial_sentinel_after_payload_size);
	 ++index, ctx->write_offset = (ctx->write_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE) {
      ctx->data[ctx->write_offset] = serial_sentinel_after_payload_size[index];
    }

      // Data payload
    for (size_t index = 0; index < length;
	 ++index, ctx->write_offset = (ctx->write_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE) {
      ctx->data[ctx->write_offset] = buffer[index];
    }

    // Sentinel before checksum
    for (size_t index = 0; index < sizeof(serial_sentinel_before_checksum);
	 ++index, ctx->write_offset = (ctx->write_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE) {
      ctx->data[ctx->write_offset] = serial_sentinel_before_checksum[index];
    }

    // Checksum
    for (size_t index = 0; index < payload_checksum_length;
	 ++index, ctx->write_offset = (ctx->write_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE) {
      ctx->data[ctx->write_offset] = payload_checksum_buffer[index];      
    }

    // Sentinel after checksum
    for (size_t index = 0; index < sizeof(serial_sentinel_after_checksum);
	 ++index, ctx->write_offset = (ctx->write_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE) {
      ctx->data[ctx->write_offset] = serial_sentinel_after_checksum[index];
    }

    return 1;
  }

  return 0;
}


int append_char(struct sentinel_serial_buffer *ctx, uint8_t c) {
  size_t capacity_remaining = (ctx->write_offset + SENTINEL_SERIAL_BUFFER_RING_SIZE - ctx->read_offset)
    % SENTINEL_SERIAL_BUFFER_RING_SIZE;
  if (1 <= capacity_remaining) {
    ctx->data[ctx->write_offset];
    ctx->write_offset = (ctx->write_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE;
    return 1;
  }
  return 0;
}


int find_sentinel_in_ring(struct sentinel_serial_buffer *ctx, const uint8_t *sentinel,
			   size_t sentinel_length, size_t *offset) {
  int found = 0;
  size_t search_offset = (ctx->read_offset + *offset) % SENTINEL_SERIAL_BUFFER_RING_SIZE;
  while (!found) {
    // if we've traversed the whole buffer without finding the sentinel, quit
    if ((search_offset + SENTINEL_SERIAL_BUFFER_RING_SIZE) % SENTINEL_SERIAL_BUFFER_RING_SIZE == ctx->write_offset) {
      break;
    }

    for (int symbols_found = 0;
	 symbols_found < sentinel_length
	   && sentinel[symbols_found] == ctx->data[(search_offset + symbols_found) % SENTINEL_SERIAL_BUFFER_RING_SIZE];
	 ++symbols_found) {
      if (symbols_found == sentinel_length) {
	*offset = (search_offset + SENTINEL_SERIAL_BUFFER_RING_SIZE - ctx->read_offset) % SENTINEL_SERIAL_BUFFER_RING_SIZE;
	found = 1;
	break;
      }
    }
    
    // advance the search offset
    search_offset = (search_offset + 1) % SENTINEL_SERIAL_BUFFER_RING_SIZE;
  }

  return found;
}


unsigned long ring_strtoul(const uint8_t *buffer, size_t buffer_size, size_t start_offset, size_t end_offset) {
  unsigned long base = 10;
  size_t decode_length = end_offset - start_offset;
  // Allow leading spaces
  while (decode_length > 0 && buffer[start_offset % buffer_size] == ' ') {
    ++start_offset;
    --decode_length;
  }
  // Ignore leading minus sign
  if (decode_length > 0 && buffer[start_offset % buffer_size] == '-') {
    ++start_offset;
    --decode_length;
  }
  // Detect base
  if (decode_length > 1 && buffer[start_offset % buffer_size] == '0') {
    if (decode_length > 2 && buffer[(start_offset + 1) % buffer_size] == 'x') {
      base = 16;
      start_offset += 2;
      decode_length -= 2;
    } else {
      base = 8;
      start_offset += 1;
      decode_length -= 1;
    }
  }
  // Decode digits
  unsigned long result = 0;
  errno = 0;
  for (size_t index = 0; index < decode_length; ++index) {
    if (result > (UINT_MAX / base)) {
      errno = ERANGE;
      break;
    }
    uint8_t val = buffer[(start_offset + index) % buffer_size];
    if (base == 8) {
      if ('0' <= val && val <= '7') {
	result = (result * base) + (unsigned long) (val - '0');
      } else {
	errno = EINVAL;
	break;
      }
    } else if (base == 16) {
      if ('0' <= val && val <= '9') {
	result = (result * base) + (unsigned long) (val - '0');
      } else if ('a' <= val && val <= 'f') {
	result = (result * base) + (unsigned long) (val - 'a' + 10);
      } else if ('A' <= val && val <= 'F') {
	result = (result * base) + (unsigned long) (val - 'A' + 10);
      } else {
	errno = EINVAL;
	break;
      }
    } else /* base == 10) */ {
      if ('0' <= val && val <= '9') {
	result = (result * base) + (unsigned long) (val - '0');
      } else {
	errno = EINVAL;
	break;
      }
    }
  }
  return result;
}


ssize_t get_next_payload_string(struct sentinel_serial_buffer *ctx, uint8_t *buffer, size_t buffer_size) {
  ssize_t result = 0;

  size_t before_payload_size_offset = 0;
  if (!find_sentinel_in_ring(ctx, serial_sentinel_before_payload_size, sizeof(serial_sentinel_before_payload_size),
			     &before_payload_size_offset)) {
    errno = EAGAIN;
    return -1;
  }

  size_t after_payload_size_offset = before_payload_size_offset + sizeof(serial_sentinel_before_payload_size);
  if (!find_sentinel_in_ring(ctx, serial_sentinel_after_payload_size, sizeof(serial_sentinel_after_payload_size),
			     &after_payload_size_offset)) {
    errno = EAGAIN;
    return -1;
  }

  size_t before_checksum_offset = after_payload_size_offset + sizeof(serial_sentinel_after_payload_size);;
  if (!find_sentinel_in_ring(ctx, serial_sentinel_before_checksum, sizeof(serial_sentinel_before_checksum),
			     &before_checksum_offset)) {
    errno = EAGAIN;
    return -1;
  }

  size_t after_checksum_offset = before_checksum_offset + sizeof(serial_sentinel_before_checksum);
  if (!find_sentinel_in_ring(ctx, serial_sentinel_after_checksum, sizeof(serial_sentinel_after_checksum),
			     &after_checksum_offset)) {
    errno = EAGAIN;
    return -1;
  }

  size_t original_read_offset = ctx->read_offset;
  ctx->read_offset = (ctx->read_offset + after_checksum_offset + sizeof(serial_sentinel_after_checksum))
    % SENTINEL_SERIAL_BUFFER_RING_SIZE;

  size_t payload_size =
    (size_t) ring_strtoul(ctx->data, SENTINEL_SERIAL_BUFFER_RING_SIZE,
			  original_read_offset + before_payload_size_offset + sizeof(serial_sentinel_before_payload_size),
			  after_payload_size_offset);
  if (errno) {
    errno = EINVAL;
    return -1;
  }

  uint32_t expected_checksum =
    (size_t) ring_strtoul(ctx->data, SENTINEL_SERIAL_BUFFER_RING_SIZE,
			  original_read_offset + before_checksum_offset + sizeof(serial_sentinel_before_checksum),
			  after_checksum_offset);
  if (errno) {
    errno = EIO;
    return -1;
  }

  if (payload_size > buffer_size) {
    errno = EFAULT;
    return -1;
  }

  for (int index = 0; index < payload_size; ++index) {
    buffer[index] = ctx->data[(original_read_offset + after_payload_size_offset + sizeof(serial_sentinel_after_payload_size))
			      % SENTINEL_SERIAL_BUFFER_RING_SIZE];
  }

  uint32_t computed_checksum = calculate_checksum(buffer, payload_size);
  if (expected_checksum != computed_checksum) {
    errno = EIO;
    return -1;
  }

  return payload_size;
}
