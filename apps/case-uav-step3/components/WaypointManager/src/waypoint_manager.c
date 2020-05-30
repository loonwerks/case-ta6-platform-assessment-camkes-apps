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

// Forward declarations
/* int send_mission_command(void); */
void mission_command_out_event_data_send(data_t *data);


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "p1_in".
void air_vehicle_state_in_event_data_receive(counter_t numDropped, data_t *data) {
    // printf("%s: received air vehicle state: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));
}

//------------------------------------------------------------------------------
// Implementation of AADL Input Event Data Port (in) named "p1_in"
// Three styles: poll, wait and callback.
//
// Callback would typically be avoid for safety critical systems. It is harder
// to analyze since the callback handler is run on some arbitrary thread.
//
// NOTE: If we only need polling style receivers, we can get rid of SendEvent

recv_queue_t airVehicleStateInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool air_vehicle_state_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&airVehicleStateInRecvQueue, numDropped, data);
}

// void air_vehicle_state_in_event_data_wait(counter_t *numDropped, data_t *data) {
//     while (!air_vehicle_state_in_event_data_poll(numDropped, data)) {
//         air_vehicle_state_in_SendEvent_wait();
//     }
// }

// static void air_vehicle_state_in_handler(void *v) {
//     counter_t numDropped;
//     data_t data;
// 
//     // Handle ALL events that have been queued up
//     while (air_vehicle_state_in_event_data_poll(&numDropped, &data)) {
//         air_vehicle_state_in_event_data_receive(numDropped, &data);
//     }
//     while (! air_vehicle_state_in_SendEvent_reg_callback(&air_vehicle_state_in_handler, NULL));
// }

//---

//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
void automation_response_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received automation response: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));
    // For testing, whenever we receive an automation response, send it out on the mission command out port
    mission_command_out_event_data_send(data);
}

//------------------------------------------------------------------------------
// Implementation of AADL Input Event Data Port (in) named "p1_in"
// Three styles: poll, wait and callback.
//
// Callback would typically be avoid for safety critical systems. It is harder
// to analyze since the callback handler is run on some arbitrary thread.
//
// NOTE: If we only need polling style receivers, we can get rid of SendEvent

recv_queue_t automationResponseInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool automation_response_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&automationResponseInRecvQueue, numDropped, data);
}

// void automation_response_in_event_data_wait(counter_t *numDropped, data_t *data) {
//     while (!automation_response_in_event_data_poll(numDropped, data)) {
//         automation_response_in_SendEvent_wait();
//     }
// }

// static void automation_response_in_handler(void *v) {
//     counter_t numDropped;
//     data_t data;
// 
//     // Handle ALL events that have been queued up
//     while (automation_response_in_event_data_poll(&numDropped, &data)) {
//         automation_response_in_event_data_receive(numDropped, &data);
//     }
//     while (! automation_response_in_SendEvent_reg_callback(&automation_response_in_handler, NULL));
// }

//--

void done_emit_underlying(void) WEAK;
static void done_emit(void) {
  /* If the interface is not connected, the 'underlying' function will
   * not exist.
   */
  if (done_emit_underlying) {
    done_emit_underlying();
  }
}


//------------------------------------------------------------------------------
// Implementation of AADL Input Event Data Port (out) named "mission_command_out"
//
// NOTE: If we only need polling style receivers, we can get rid of the SendEvent

// Assumption: only one thread is calling this and/or reading mission_command_out_recv_counter.
void mission_command_out_event_data_send(data_t *data) {
    queue_enqueue(mission_command_out_queue, data);
    mission_command_out_SendEvent_emit();
    done_emit();
}


static const char message[] = {
// TODO: This message is BOGUS... Fix it.
  0x61,0x66,0x72,0x6C,0x2E,0x63,0x6D,0x61,0x73,0x69,0x2E,0x4D,0x69,0x73,0x73,0x69,  /* afrl:cmasi:Missi */
  0x6F,0x6E,0x43,0x6F,0x6D,0x6D,0x61,0x6E,0x6D,0x24,0x6C,0x6D,0x63,0x70,0x7C,0x61,  /* onCommand$lmcp|a */
  0x66,0x72,0x6C,0x2E,0x63,0x6D,0x61,0x73,0x69,0x2E,0x4D,0x69,0x73,0x73,0x69,0x6F,  /* frl:cmasi:Missio */
  0x6E,0x43,0x6F,0x6D,0x6D,0x61,0x6E,0x64,0x7C,0x54,0x63,0x70,0x42,0x72,0x69,0x64,  /* nCommand|6?p*rid */
  0x67,0x65,0x7C,0x34,0x30,0x30,0x7C,0x36,0x38,0x24,0x4C,0x4D,0x43,0x50,0x00,0x00,  /* ge|400|68$LMCP.. */
  0x00,0x2B,0x01,0x43,0x4D,0x41,0x53,0x49,0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x00,  /* .+.CMASI......'. */
  0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0x00,0x01,0x00,0x00,0x00,0x00,0x00,  /* ........P....... */
  0x00,0x01,0x4E,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x4F,0x00,0x00,0x03,  /* ..N.........O... */
  0xE1                                                                              /* .                */
};


/*
int send_mission_command(void) {
    data_t data;

    // Stage data
    memcpy((void *) &data.payload[0], (const void *) &message[0], sizeof(message));
    printf("%s: sending mission command: %d\n", get_instance_name(), sizeof(message));

    // Send the data
    mission_command_out_event_data_send(&data);          
}
*/

//---

//------------------------------------------------------------------------------
// Testing - Three tests for the different styles: poll, wait and callback.
//
// NOTE: The constants in the tests were chosen to cause a variety of
// situations at runtime including dropped packets and no data
// available. These numbers may not cause the same variety of behaviour in
// different test environments.

void run_poll(void) {
    counter_t numDropped;
    data_t data;

    while (true) {

        bool dataReceived = air_vehicle_state_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            air_vehicle_state_in_event_data_receive(numDropped, &data);
        }

        dataReceived = automation_response_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            automation_response_in_event_data_receive(numDropped, &data);
        }

        seL4_Yield();
    }

}

// void run_wait(void) {
//     counter_t numDropped;
//     data_t data;
// 
//     while (true) {
//         // TODO: how to wait on either of two send events?
//         air_vehicle_state_in_event_data_wait(&numDropped, &data);
//         air_vehicle_state_in_event_data_receive(numDropped, &data);
//         automation_response_in_event_data_wait(&numDropped, &data);
//         automation_response_in_event_data_receive(numDropped, &data);
//     }
// }

// int run_callback(void) {
//      return p1_in_SendEvent_reg_callback(&p1_in_handler, NULL);
// }

void post_init(void) {
    recv_queue_init(&airVehicleStateInRecvQueue, air_vehicle_state_in_queue);
    recv_queue_init(&automationResponseInRecvQueue, automation_response_in_queue);
    queue_init(mission_command_out_queue);
}

int run(void) {
    // Pick one receive style to test.
    run_poll();
    //run_wait();
    //return run_callback();
}

