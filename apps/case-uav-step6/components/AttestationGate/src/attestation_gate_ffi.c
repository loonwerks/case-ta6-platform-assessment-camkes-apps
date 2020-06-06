#include <camkes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <counter.h>
#include <am_data.h>
#include <am_queue.h>
#include <data.h>
#include <queue.h>

#include <stdint.h>
#include <sys/types.h>

#include "hexdump.h"


char attestationMsgBuffer[256];

am_data_t _attestationIds;
am_data_t *attestationIds = &_attestationIds;
size_t attestationIdsSizeBytes = sizeof(attestationIds->payload);

data_t _attestationData;
data_t *attestationData = &_attestationData;
size_t attestationDataSizeBytes = sizeof(attestationData->payload);

/**
 * API for Logging
 */

void api_logInfo(char* str) {
  printf("%s: %s\n", get_instance_name(), str);
  fflush(stdout);
}

void ffiapi_logInfo(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  api_logInfo((char *)parameter);
}

void checkBufferOverrun(size_t dstSizeBytes, size_t srcSizeBytes) {
  if (srcSizeBytes > dstSizeBytes) {
    sprintf(attestationMsgBuffer, "\n\tERROR writing %ld bytes to buffer of size %ld", srcSizeBytes, dstSizeBytes);
    api_logInfo(attestationMsgBuffer);
  }
}

/**
 * API for get and send
 */

void dumpBuffer(size_t numBytes, uint8_t* buffer) {
  hexdump("    ", 32, buffer, numBytes);
  fflush(stdout);
} 

void clearattestationData() {
  for (int i = 0 ; i < attestationDataSizeBytes ; ++i) {
    attestationData->payload[i] = 0;
  }
}

void clearattestationIds() {
  for (int i = 0 ; i < attestationIdsSizeBytes ; ++i) {
    attestationIds->payload[i] = 0;
  }
}

// uint8_t trusted_ids_out[] = {0x30, 0x35, 0x30, 0x30, 0x30, 0x34, 0x30, 0x30, 0x30, 0x36, 0x30, 0x30};
// size_t trusted_ids_outSizeBytes = sizeof(trusted_ids_out);
// 
// void ffiapi_get_trusted_ids(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
//   checkBufferOverrun(outputSizeBytes, trusted_ids_outSizeBytes);
//   for (size_t i = 0 ; i < trusted_ids_outSizeBytes ; ++i) {
//     output[i] = trusted_ids_out[i];
//   }
// }

extern bool trusted_ids_in_event_data_poll(am_counter_t *, am_data_t *);

void ffiapi_get_trusted_ids(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  am_counter_t numRcvd = 0;

  checkBufferOverrun(outputSizeBytes, attestationIdsSizeBytes);
  clearattestationIds();

  output[0] = trusted_ids_in_event_data_poll(&numRcvd, attestationIds);
  if (output[0]) {
    memcpy(output+1, attestationIds->payload, attestationIdsSizeBytes);
  }
  if (numRcvd > 0) {
    sprintf(attestationMsgBuffer, "\n\treceived Trusted Ids (%ld)", numRcvd);
    api_logInfo(attestationMsgBuffer);
  }
  
}

extern bool automation_request_in_event_data_poll(counter_t *, data_t *);

void ffiapi_get_AutomationRequest_in(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  counter_t numRcvd = 0;

  checkBufferOverrun(outputSizeBytes, attestationDataSizeBytes);
  clearattestationData();

  output[0] = automation_request_in_event_data_poll(&numRcvd, attestationData);
  if (output[0]) {
    memcpy(output+1, attestationData->payload, attestationDataSizeBytes);
  }
  if (numRcvd > 0) {
    sprintf(attestationMsgBuffer, "\n\treceived AutomationRequest (%ld)", numRcvd);
    api_logInfo(attestationMsgBuffer);
  }
  
}

extern void automation_request_out_event_data_send(data_t*);

void ffiapi_send_AutomationRequest_out(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  checkBufferOverrun(attestationDataSizeBytes, parameterSizeBytes);
  clearattestationData();
  memcpy(attestationData->payload, parameter, parameterSizeBytes);
  automation_request_out_event_data_send(attestationData);
}

extern bool operating_region_in_event_data_poll(counter_t *, data_t *);

void ffiapi_get_OperatingRegion_in(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  counter_t numRcvd = 0;

  checkBufferOverrun(outputSizeBytes, attestationDataSizeBytes);
  clearattestationData();

  output[0] = operating_region_in_event_data_poll(&numRcvd, attestationData);
  if (output[0]) {
    memcpy(output+1, attestationData->payload, attestationDataSizeBytes);
  }
  if (numRcvd > 0) {
    sprintf(attestationMsgBuffer, "\n\treceived OperatingRegion (%ld)", numRcvd);
    api_logInfo(attestationMsgBuffer);
  }
  
}

extern void operating_region_out_event_data_send(data_t*);

void ffiapi_send_OperatingRegion_out(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  checkBufferOverrun(attestationDataSizeBytes, parameterSizeBytes);
  clearattestationData();
  memcpy(attestationData->payload, parameter, parameterSizeBytes);
  operating_region_out_event_data_send(attestationData);
}

extern bool line_search_task_in_event_data_poll(counter_t *, data_t *);

void ffiapi_get_LineSearchTask_in(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  counter_t numRcvd = 0;

  checkBufferOverrun(outputSizeBytes, attestationDataSizeBytes);
  clearattestationData();

  output[0] = line_search_task_in_event_data_poll(&numRcvd, attestationData);
  if (output[0]) {
    memcpy(output+1, attestationData->payload, attestationDataSizeBytes);
  }
  if (numRcvd > 0) {
    sprintf(attestationMsgBuffer, "\n\treceived LineSearchTask (%ld)", numRcvd);
    api_logInfo(attestationMsgBuffer);
  }
  
}

extern void line_search_task_out_event_data_send(data_t*);

void ffiapi_send_LineSearchTask_out(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  checkBufferOverrun(attestationDataSizeBytes, parameterSizeBytes);
  clearattestationData();
  memcpy(attestationData->payload, parameter, parameterSizeBytes);
  line_search_task_out_event_data_send(attestationData);
}

/**
 * PACER
 */

extern void seL4_Yield();

void ffiseL4_yield(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  seL4_Yield();;
}

/**
 * Required by the FFI framework
 */
 
void ffiwrite (unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes){
}

void cml_exit(int arg) {
  #ifdef DEBUG_FFI
  {
    fprintf(stderr,"GCNum: %d, GCTime(us): %ld\n",numGC,microsecs);
  }
  #endif
  exit(arg);
}
