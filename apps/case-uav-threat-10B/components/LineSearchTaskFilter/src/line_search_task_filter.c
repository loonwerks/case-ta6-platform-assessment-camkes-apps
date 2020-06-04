/*
 * Copyright 2020, Collins Aerospace
 */

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

#include "LineSearchTask.h"
#include "Location3D.h"
#include "Wedge.h"
#include "lmcp.h"

#define LATITUDE_MIN -90.0
#define LATITUDE_MAX 90.0
#define LONGITUDE_MIN -180.0
#define LONGITUDE_MAX 180.0
#define ALTITUDE_MIN 0.0
#define ALTITUDE_MAX 5000.0
#define TASK_ID_MIN 0
#define TASK_ID_MAX 2000
#define AZIMUTH_CENTERLINE_MIN -180.0
#define AZIMUTH_CENTERLINE_MAX 180.0
#define VERTICAL_CENTERLINE_MIN -180.0
#define VERTICAL_CENTERLINE_MAX 180.0

// Forward declarations
void line_search_task_out_event_data_send(data_t *data);

bool isValidLineSearchTaskMessage(data_t *data) {

    LineSearchTask *lineSearchTask = NULL;
    lmcp_init_LineSearchTask(&lineSearchTask);

    if (lineSearchTask != NULL) {
        uint8_t *payload = &(data->payload[0]);
        int msg_result = lmcp_process_msg(&payload, sizeof(data->payload), (lmcp_object**)&lineSearchTask);

        if (msg_result == 0) {
            printf("LineSearchTaskFilter: message received containing %u waypoints\n", lineSearchTask->pointlist_ai.length);
//            fflush(stdout);
//            hexdump("    ", 24, data->payload, compute_addr_attr_lmcp_message_size(data->payload, sizeof(data->payload)));

            for (size_t i = 0; i < lineSearchTask->pointlist_ai.length; i++) {
                Location3D * point = lineSearchTask->pointlist[i];
                double latitude = unpack754(point->latitude, 64, 11);
                double longitude = unpack754(point->longitude, 64, 11);
                float altitude = unpack754(point->altitude, 32, 8);
                if (latitude < LATITUDE_MIN || latitude > LATITUDE_MAX ||
                    longitude < LONGITUDE_MIN || longitude > LONGITUDE_MAX ||
                    altitude < ALTITUDE_MIN || altitude > ALTITUDE_MAX) {
                        return false;
                }
                
            }
            
            if (lineSearchTask->super.super.taskid < TASK_ID_MIN ||
                lineSearchTask->super.super.taskid > TASK_ID_MAX) {
                    return false;
            }

            for (size_t i = 0; i < lineSearchTask->viewanglelist_ai.length; i++) {
                Wedge * wedge = lineSearchTask->viewanglelist[i];
                if (unpack754(wedge->azimuthcenterline, 32, 8) < AZIMUTH_CENTERLINE_MIN || unpack754(wedge->azimuthcenterline, 32, 8) > AZIMUTH_CENTERLINE_MAX ||
                    unpack754(wedge->verticalcenterline, 32, 8) < VERTICAL_CENTERLINE_MIN || unpack754(wedge->verticalcenterline, 32, 8) > VERTICAL_CENTERLINE_MAX) {
                        return false;
                }
            }

        } else {
            printf("Unable to process LineSearchTask message\n"); fflush(stdout);
            return false;
        }
    } else {
        printf("Unable to initialize lineSearchTask\n"); fflush(stdout);
        return false;
    }
    return true;
}


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "p1_in".
void line_search_task_in_event_data_receive(counter_t numDropped, data_t *data) {
//    printf("%s: received line search task: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped); fflush(stdout);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));

    if (isValidLineSearchTaskMessage(data)) {
        printf("Line search task is valid!\n"); fflush(stdout);
        line_search_task_out_event_data_send(data);
    } else {
        printf("Line search task is not valid!\n"); fflush(stdout);
    }
}


recv_queue_t lineSearchTaskInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool line_search_task_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&lineSearchTaskInRecvQueue, numDropped, data);
}



void done_emit_underlying(void) WEAK;
static void done_emit(void) {
  /* If the interface is not connected, the 'underlying' function will
   * not exist.
   */
  if (done_emit_underlying) {
    done_emit_underlying();
  }
}


void line_search_task_out_event_data_send(data_t *data) {
    queue_enqueue(line_search_task_out_queue, data);
    line_search_task_out_SendEvent_emit();
    done_emit();
}


void run_poll(void) {
    counter_t numDropped;
    data_t data;

    while (true) {

        bool dataReceived = line_search_task_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            line_search_task_in_event_data_receive(numDropped, &data);
        }

        seL4_Yield();
    }

}



void post_init(void) {
    recv_queue_init(&lineSearchTaskInRecvQueue, line_search_task_in_queue);
    queue_init(line_search_task_out_queue);
}

int run(void) {

    run_poll();

}

