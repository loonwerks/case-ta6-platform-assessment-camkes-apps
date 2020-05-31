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

// TODO: Put real values in here
uint64_t keepInLat[2] = {4,16};
uint64_t keepInLong[2] = {4,16};
uint64_t keepInAlt = 0;
uint64_t keepOutLat[2] = {8,12};
uint64_t keepOutLong[2] = {8,12};
uint64_t keepOutAlt = 0;

bool inKeepInZone(Waypoint * waypoint) {

    return (waypoint->super.latitude >= keepInLat[0] &&
            waypoint->super.latitude <= keepInLat[1] &&
            waypoint->super.longitude >= keepInLong[0] &&
            waypoint->super.longitude <= keepInLong[1] &&
            waypoint->super.altitude >= keepInAlt);

}

bool inKeepOutZone(Waypoint * waypoint) {

    return (waypoint->super.latitude >= keepOutLat[0] &&
            waypoint->super.latitude <= keepOutLat[1] &&
            waypoint->super.longitude >= keepOutLong[0] &&
            waypoint->super.longitude <= keepOutLong[1] &&
            waypoint->super.altitude >= keepOutAlt);

}


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
void automation_response_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received automation response: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));

    AutomationResponse * automationResponse = NULL;
    lmcp_init_AutomationResponse(&automationResponse);
    
    uint8_t *payload = &(data->payload[0]);

    int msg_result = lmcp_process_msg(&payload, sizeof(data->payload), (lmcp_object**)&automationResponse);

    if (msg_result == 0 && automationResponse->missioncommandlist_ai.length > 0) {

        hexdump_raw(24, data->payload, compute_addr_attr_lmcp_message_size(data->payload, sizeof(data->payload)));

        // check that each waypoint is in the keep-in zones and not in the keep-out zones
        for (size_t i = 0; i < automationResponse->missioncommandlist[0]->waypointlist_ai.length; i++) {
            Waypoint * waypoint = automationResponse->missioncommandlist[0]->waypointlist[i];
            if (!inKeepInZone(waypoint) || inKeepOutZone(waypoint)) {
                alert_out_event_data_send(data);
                return;
            }
        }

        // check if there are any duplicate waypoints
        for (size_t m = 0; m < automationResponse->missioncommandlist[0]->waypointlist_ai.length; m++) {
            Location3D point_m = automationResponse->missioncommandlist[0]->waypointlist[m]->super;
            for (size_t n = 0; n < automationResponse->missioncommandlist[0]->waypointlist_ai.length; n++) {
                Location3D point_n = automationResponse->missioncommandlist[0]->waypointlist[n]->super;
                if (m != n &&
                    point_m.latitude == point_n.latitude && 
                    point_m.longitude == point_n.longitude &&
                    point_m.altitude == point_n.altitude) {
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


void initializeFlyZones() {


}


void run_poll(void) {
    counter_t numDropped;
    data_t data;

    initializeFlyZones();

    while (true) {

        bool dataReceived = automation_response_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            automation_response_in_event_data_receive(numDropped, &data);
        }

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

