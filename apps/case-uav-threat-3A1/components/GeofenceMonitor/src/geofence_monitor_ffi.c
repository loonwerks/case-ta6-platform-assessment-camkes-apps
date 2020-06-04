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

char geoFenceMsgBuffer[256];
data_t _geoFenceData;
data_t *geoFenceData = &_geoFenceData;
size_t geoFenceDataSizeBytes = sizeof(geoFenceData->payload);
size_t zoneSizeBytes = 48;

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
    sprintf(geoFenceMsgBuffer, "\n\tERROR writing %ld bytes to buffer of size %ld", srcSizeBytes, dstSizeBytes);
    api_logInfo(geoFenceMsgBuffer);
  }
}

/**
 * API for get and send
 */

void dumpBuffer(size_t numBytes, uint8_t* buffer) {
  hexdump("    ", 32, buffer, numBytes);
  fflush(stdout);
} 

void clearGeoFenceData() {
  for (int i = 0 ; i < geoFenceDataSizeBytes ; ++i) {
    geoFenceData->payload[i] = 0;
  }
}

uint8_t isaacKeepInZone[] = {0x40, 0x46, 0xA6, 0x73, 0x7F, 0x91, 0x58, 0x22, 
                             0xC0, 0x5E, 0x40, 0xF1, 0x55, 0xC9, 0x5C, 0x81, 
                             0x44, 0x7A, 0x00, 0x00, 
                             0x00, 0x00, 0x00, 0x01,
                             0x40, 0x46, 0xAC, 0x33, 0x4C, 0x34, 0xCA, 0x58, 
                             0xC0, 0x5E, 0x3A, 0x66, 0xB8, 0x6D, 0xFA, 0x7D, 
                             0x44, 0x7A, 0x00, 0x00, 
                             0x00, 0x00, 0x00, 0x01};

void ffiapi_get_keep_in_zone(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  checkBufferOverrun(outputSizeBytes, zoneSizeBytes);
  for (size_t i = 0 ; i < zoneSizeBytes ; ++i) {
    output[i] = isaacKeepInZone[i];
  }
}

uint8_t isaacKeepOutZone[] = {0x40, 0x46, 0xAA, 0xA1, 0xB1, 0xAD, 0xC6, 0xD5, 
                              0xC0, 0x5E, 0x3C, 0x09, 0xC2, 0xEB, 0xA6, 0x0C, 
                              0x44, 0x7A, 0x00, 0x00, 
                              0x00, 0x00, 0x00, 0x01,
                              0x40, 0x46, 0xAA, 0xFA, 0x00, 0x85, 0xEC, 0xB9, 
                              0xC0, 0x5E, 0x3B, 0xCA, 0xF3, 0x58, 0x81, 0xEB, 
                              0x44, 0x7A, 0x00, 0x00, 
                              0x00, 0x00, 0x00, 0x01};

void ffiapi_get_keep_out_zone(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  checkBufferOverrun(outputSizeBytes, zoneSizeBytes);
  for (size_t i = 0 ; i < zoneSizeBytes ; ++i) {
    output[i] = isaacKeepOutZone[i];
  }
}

extern bool automation_response_in_event_data_poll(counter_t *numDropped, data_t *data);

void ffiapi_get_observed(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  counter_t numRcvd = 0;

  checkBufferOverrun(outputSizeBytes, geoFenceDataSizeBytes);
  clearGeoFenceData();

  output[0] = automation_response_in_event_data_poll(&numRcvd, geoFenceData);
  if (output[0]) {
    memcpy(output+1, geoFenceData->payload, geoFenceDataSizeBytes);
  }
  if (numRcvd > 0) {
    sprintf(geoFenceMsgBuffer, "\n\treceived AutomationRequest (%ld)", numRcvd);
    api_logInfo(geoFenceMsgBuffer);
  }
}

void ffiapi_send_output(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  checkBufferOverrun(geoFenceDataSizeBytes, parameterSizeBytes);
  clearGeoFenceData();
  memcpy(geoFenceData->payload, parameter, parameterSizeBytes);
  // TODO: output_event_data_send(geoFenceData); <-- not in case-uav-step4
}

extern void alert_out_event_data_send(data_t *data);

void ffiapi_send_alert(unsigned char *parameter, long parameterSizeBytes, unsigned char *output, long outputSizeBytes) {
  checkBufferOverrun(geoFenceDataSizeBytes, parameterSizeBytes);
  clearGeoFenceData();
  alert_out_event_data_send(geoFenceData);
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
