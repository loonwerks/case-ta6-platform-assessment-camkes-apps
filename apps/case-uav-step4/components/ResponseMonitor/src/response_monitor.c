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
#include "AutomationResponse.h"
#include "lmcp.h"


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
void automation_request_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received automation request\n", get_instance_name()); fflush(stdout);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));

}


recv_queue_t automationRequestInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool automation_request_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&automationRequestInRecvQueue, numDropped, data);
}



//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
bool automation_response_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received automation response\n", get_instance_name()); fflush(stdout);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));

    AutomationResponse * automationResponse;
    lmcp_init_AutomationResponse(&automationResponse);
    
    uint8_t *payload = &(data->payload[0]);

    int msg_result = lmcp_process_msg(&payload, sizeof(data->payload), (lmcp_object**)&automationResponse);

    bool result = (msg_result == 0 && automationResponse->missioncommandlist_ai.length > 0);

    lmcp_free_AutomationResponse(automationResponse, 1);

    return result;

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



void run_poll(void) {
    counter_t numDropped;
    data_t data;
    uint32_t invocations = 0;

    while (true) {

        if (invocations > 0) {
            invocations++;
        }

        bool dataReceived = automation_request_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            automation_request_in_event_data_receive(numDropped, &data);
            invocations = 1;
        }

        dataReceived = automation_response_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            if (automation_response_in_event_data_receive(numDropped, &data)) {
                invocations = 0;
            }
        }

        if (invocations > 2000) {
            printf("\n************************************\n");
            printf("** Response Monitor:              **\n");
            printf("** Expected a response from UxAS, **\n");
            printf("** but did not receive one!       **\n");
            printf("** Consider aborting mission.     **\n");
            printf("************************************\n\n");
            fflush(stdout);
            invocations = 0;
        }

        seL4_Yield();
    }

}


void post_init(void) {
    recv_queue_init(&automationRequestInRecvQueue, automation_request_in_queue);
    recv_queue_init(&automationResponseInRecvQueue, automation_response_in_queue);
}

int run(void) {

    run_poll();

}

