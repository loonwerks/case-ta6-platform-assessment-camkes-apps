#include <camkes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <counter.h>
#include <data.h>
#include <queue.h>

#include <stdint.h>
#include <sys/types.h>

#include "hexdump.h"


char lineSearchTaskFilterMsgBuffer[256];
data_t _lineSearchTaskFilterData;
data_t *lineSearchTaskFilterData = &_lineSearchTaskFilterData;
size_t lineSearchTaskFilterDataSizeBytes = sizeof(lineSearchTaskFilterData->payload);

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
    sprintf(lineSearchTaskFilterMsgBuffer, "\n\tERROR writing %ld bytes to buffer of size %ld", srcSizeBytes, dstSizeBytes);
    api_logInfo(lineSearchTaskFilterMsgBuffer);
  }
}

/**
 * API for get and send
 */

void dumpBuffer(size_t numBytes, uint8_t* buffer) {
  hexdump("    ", 32, buffer, numBytes);
  fflush(stdout);
} 

void clearlineSearchTaskFilterData() {
  for (int i = 0 ; i < lineSearchTaskFilterDataSizeBytes ; ++i) {
    lineSearchTaskFilterData->payload[i] = 0;
  }
}

extern bool line_search_task_in_event_data_poll(counter_t *, data_t *);

void ffiapi_get_filter_in(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  counter_t numRcvd = 0;

  checkBufferOverrun(outputSizeBytes, lineSearchTaskFilterDataSizeBytes);
  clearlineSearchTaskFilterData();

  output[0] = line_search_task_in_event_data_poll(&numRcvd, lineSearchTaskFilterData);
  if (output[0]) {
    memcpy(output+1, lineSearchTaskFilterData->payload, lineSearchTaskFilterDataSizeBytes);
  }
  if (numRcvd > 0) {
    sprintf(lineSearchTaskFilterMsgBuffer, "\n\treceived LineSearchTask (%ld)", numRcvd);
  }
  api_logInfo(lineSearchTaskFilterMsgBuffer);
}

extern void line_search_task_out_event_data_send(data_t*);

void ffiapi_send_filter_out(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  checkBufferOverrun(lineSearchTaskFilterDataSizeBytes, parameterSizeBytes);
  clearlineSearchTaskFilterData();
  memcpy(lineSearchTaskFilterData->payload, parameter, parameterSizeBytes);
  line_search_task_out_event_data_send(lineSearchTaskFilterData);
}

void ffiapi_float2double(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes)
{
  double result = *((float*)parameter);
  memcpy(output, (unsigned char*) &result, sizeof(double));
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
