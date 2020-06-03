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

#include "./CMASI/lmcp.h"
#include "./CMASI/common/conv.h"
#include "./CMASI/MissionCommand.h"
#include "./CMASI/Waypoint.h"
#include "./CMASI/AutomationResponse.h"

// Forward declarations
void alert_out_event_data_send(data_t *data);

double keepInLat[2] = {45.30039972874535, 45.34531548097283};
double keepInLong[2] = {-121.01472992576784, -120.91251955738149};
double keepInAlt = 1000.0;
double keepOutLat[2] = {45.33305951104345, 45.3357544568948};
double keepOutLong[2] = {-120.93809578907548, -120.93426211970625};
double keepOutAlt = 1000.0;

bool inKeepInZone(Waypoint * waypoint) {

    return unpack754(waypoint->super.latitude, 64, 11) >= keepInLat[0] &&
           unpack754(waypoint->super.latitude, 64, 11) <= keepInLat[1] &&
           unpack754(waypoint->super.longitude, 64, 11) >= keepInLong[0] &&
           unpack754(waypoint->super.longitude, 64, 11) <= keepInLong[1] &&
           unpack754(waypoint->super.altitude, 32, 8) <= keepInAlt;

}

bool inKeepOutZone(Waypoint * waypoint) {

    return unpack754(waypoint->super.latitude, 64, 11) >= keepOutLat[0] &&
            unpack754(waypoint->super.latitude, 64, 11) <= keepOutLat[1] &&
            unpack754(waypoint->super.longitude, 64, 11) >= keepOutLong[0] &&
            unpack754(waypoint->super.longitude, 64, 11) <= keepOutLong[1] &&
            unpack754(waypoint->super.altitude, 32, 8) <= keepOutAlt;

}


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
void automation_response_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received automation response: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped); fflush(stdout);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));

    AutomationResponse * automationResponse = NULL;
    lmcp_init_AutomationResponse(&automationResponse);
    
    uint8_t *payload = &(data->payload[0]);

    int msg_result = lmcp_process_msg(&payload, sizeof(data->payload), (lmcp_object**)&automationResponse);

    if (msg_result == 0 && automationResponse->missioncommandlist_ai.length > 0) {

//        hexdump_raw(24, data->payload, compute_addr_attr_lmcp_message_size(data->payload, sizeof(data->payload)));

        // check that each waypoint is in the keep-in zones and not in the keep-out zones
        for (size_t i = 0; i < automationResponse->missioncommandlist[0]->waypointlist_ai.length; i++) {
            Waypoint * waypoint = automationResponse->missioncommandlist[0]->waypointlist[i];
            if (!inKeepInZone(waypoint)) {
                printf("\n********************************************\n");
                printf("** Geofence Monitor:                      **\n");
                printf("** UxAS generated a flight plan that is   **\n");
                printf("** not contained in the specified keep-in **\n");
                printf("** zone. This is likely due to an attack. **\n");
                printf("** Aborting mission and returning home.   **\n");
                printf("********************************************\n\n");
                fflush(stdout);
                alert_out_event_data_send(data);
                return;
            } else if (inKeepOutZone(waypoint)) {
                printf("\n**********************************************\n");
                printf("** Geofence Monitor:                        **\n");
                printf("** UxAS generated a flight plan that passes **\n");
                printf("** through a specified keep-out zone. This  **\n");
                printf("** is likely due to an attack.              **\n");
                printf("** Aborting mission and returning home.     **\n");
                printf("**********************************************\n\n");
                fflush(stdout);
                alert_out_event_data_send(data);
                return;
            }
        }

        // check if there are any duplicate waypoints
        for (size_t m = 0; m < automationResponse->missioncommandlist[0]->waypointlist_ai.length; m++) {
            Waypoint * waypoint_m = automationResponse->missioncommandlist[0]->waypointlist[m];
            if (waypoint_m->nextwaypoint == waypoint_m->number) {
                continue;
            }
            for (size_t n = 0; n < automationResponse->missioncommandlist[0]->waypointlist_ai.length; n++) {
                Waypoint * waypoint_n = automationResponse->missioncommandlist[0]->waypointlist[n];
                if (waypoint_n->nextwaypoint == waypoint_n->number) {
                    continue;
                }
                if (waypoint_m->nextwaypoint == waypoint_n->number &&
                    waypoint_m->super.latitude == waypoint_n->super.latitude && 
                    waypoint_m->super.longitude == waypoint_n->super.longitude &&
                    waypoint_m->super.altitude == waypoint_n->super.altitude) {
                        printf("\n******************************************\n");
                        printf("** Geofence Monitor:                    **\n");
                        printf("** UxAS generated a flight plan with a  **\n");
                        printf("** suspicious sequence of waypoints!    **\n");
                        printf("** This is likely due to an attack.     **\n");
                        printf("** Aborting mission and returning home. **\n");
                        printf("******************************************\n\n");
                        fflush(stdout);
                        alert_out_event_data_send(data);
                        return;
                }
            }
        }

    } else {
      printf("%s: automation response rx handler: failed processing message into structure\n", get_instance_name()); fflush(stdout);
      lmcp_free_AutomationResponse(automationResponse, 1);
      automationResponse = NULL;
    }

}


recv_queue_t automationResponseInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool automation_response_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&automationResponseInRecvQueue, numDropped, data);
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



void alert_out_event_data_send(data_t *data) {
    queue_enqueue(alert_out_queue, data);
    alert_out_SendEvent_emit();
    done_emit();
}


void run_poll(void) {
    counter_t numDropped;
    data_t data;

    while (true) {

#ifndef PASS_THRU

        bool dataReceived = automation_response_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            automation_response_in_event_data_receive(numDropped, &data);
        }

#endif

        seL4_Yield();
    }

}


void post_init(void) {
    recv_queue_init(&automationResponseInRecvQueue, automation_response_in_queue);
    queue_init(alert_out_queue);
}

int run(void) {

    run_poll();

}

