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
#include "./CMASI/AirVehicleState.h"
#include "./CMASI/EntityState.h"
#include "./CMASI/Waypoint.h"
#include "./CMASI/AutomationResponse.h"
#include "./CMASI/AddressAttributedMessage.h"


#define WINDOW_SIZE 15
#define WINDOW_OVERLAP 5
#define INIT_CMD_ID 101
#define HOME_WAYPOINT_LAT 4631577348376571884UL     // 45.3364
#define HOME_WAYPOINT_LONG 13861587297017124409UL   // -121.0032
#define HOME_WAYPOINT_ALT 1143930880U               // 700.0
#define HOME_WAYPOINT_SPEED 1102053376U
#define HOME_WAYPOINT_NUM 17554

int64_t currentWaypoint;
int64_t currentCommand;
bool returnHome;
AutomationResponse * automationResponse;
Waypoint * homeWaypoint;


// Forward declarations
void mission_command_out_event_data_send(data_t *data);
void sendMissionCommand();

void initializeWaypointManager() {
  currentWaypoint = 0;
  currentCommand = INIT_CMD_ID;
  returnHome = false;
  automationResponse = NULL;
  homeWaypoint = NULL;

  lmcp_init_Waypoint(&homeWaypoint);
  homeWaypoint->super.latitude = HOME_WAYPOINT_LAT;
  homeWaypoint->super.longitude = HOME_WAYPOINT_LONG;
  homeWaypoint->super.altitude = HOME_WAYPOINT_ALT;
  homeWaypoint->super.altitudetype = 1;
  homeWaypoint->number = HOME_WAYPOINT_NUM;
  homeWaypoint->nextwaypoint = HOME_WAYPOINT_NUM;
  homeWaypoint->speed = HOME_WAYPOINT_SPEED;
  homeWaypoint->speedtype = 0;
  homeWaypoint->climbrate = 0;
  homeWaypoint->turntype = 0;
  homeWaypoint->vehicleactionlist_ai.length = 0;
  homeWaypoint->contingencywaypointa = 0;
  homeWaypoint->contingencywaypointb = 0;
  homeWaypoint->associatedtasks_ai.length = 0;

}

Waypoint * FindWaypoint(Waypoint ** ws,
                        uint16_t len,
                        int64_t id) {

  for (uint16_t i = 0 ; i < len; i++) {
    if (ws[i]->number == id) {
      return ws[i];
    }
  }
  return NULL;
}


bool IsWaypointInWindow( Waypoint ** waypointList,
                      uint16_t waypointListSize,
                      uint16_t windowSize,
                      int64_t startId,
                      int64_t id) {
  int64_t nid = startId;
  Waypoint * wp = NULL;

  for (int i = 0; i < windowSize; i++) {
    wp = FindWaypoint(waypointList, waypointListSize, nid);
    if (wp != NULL) {
      if (wp->number == id) {
        return true;
      }
      nid = wp->nextwaypoint;
    } else {
      return false;
    }
  }
  return false;
}


/* NB: Cycles in ws will be unrolled into win. */
bool FillWindow(  Waypoint ** ws,
                  uint16_t len_ws,
                  int64_t id,
                  Waypoint ** ws_win, /* out */
                  uint16_t len_ws_win) {
  uint16_t i;
  int64_t nid = id;
  Waypoint * wp = NULL;
  bool success = true;

  for(i=0; i < len_ws_win && success == true; i++) {
    success = false;
    wp = FindWaypoint(ws, len_ws, nid);
    if(wp != NULL) {
      success = true;
      ws_win[i] = wp;
      nid = ws_win[i]->nextwaypoint;
    }
  }
  return success;
}


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "p1_in".
void air_vehicle_state_in_event_data_receive_handler(counter_t numDropped, data_t *data) {

  printf("\n%s: received air vehicle state: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped); fflush(stdout);
  
  if (automationResponse == NULL) {
    return;
  }

  AirVehicleState *airVehicleState = NULL;
  lmcp_init_AirVehicleState(&airVehicleState);

  if (airVehicleState != NULL) {
    uint8_t *payload = &(data->payload[0]);
    int msg_result = lmcp_process_msg(&payload, sizeof(data->payload), (lmcp_object**)&airVehicleState);

    if (msg_result == 0) {

      printf("AirVehicleState waypoint = %llu, currentWaypoint = %llu\n", airVehicleState->super.currentwaypoint, currentWaypoint);
      fflush(stdout);
//      hexdump_raw(24, data->payload, compute_addr_attr_lmcp_message_size(data->payload, sizeof(data->payload)));

      if (airVehicleState->super.currentwaypoint == 0) {
        lmcp_free_AirVehicleState(airVehicleState, 1);
        return;
      }

      // Check to see if we need to return home
      if (returnHome) {
        if (airVehicleState->super.currentwaypoint != HOME_WAYPOINT_NUM) {
          currentWaypoint = HOME_WAYPOINT_NUM;
          sendMissionCommand();
        }
      } else {

        bool waypointInWindow = IsWaypointInWindow(automationResponse->missioncommandlist[0]->waypointlist,
                                              automationResponse->missioncommandlist[0]->waypointlist_ai.length,
                                              WINDOW_SIZE - WINDOW_OVERLAP,
                                              currentWaypoint,
                                              airVehicleState->super.currentwaypoint);

        if (!waypointInWindow) {
          currentWaypoint = airVehicleState->super.currentwaypoint;
          sendMissionCommand();
        }
      }

    } else {
      printf("%s: air vehicle state rx handler: failed processing message into structure\n", get_instance_name()); fflush(stdout);
    }

    lmcp_free_AirVehicleState(airVehicleState, 1);

  } else {
    printf("%s: air vehicle state rx handler: couldn't allocate structure\n", get_instance_name()); fflush(stdout);
  }

}


recv_queue_t airVehicleStateInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool air_vehicle_state_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&airVehicleStateInRecvQueue, numDropped, data);
}



//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
void automation_response_in_event_data_receive_handler(counter_t numDropped, data_t *data) {

    printf("\n%s: received automation response: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped); fflush(stdout);
    
    if (automationResponse != NULL) {
        lmcp_free_AutomationResponse(automationResponse, 1);
        automationResponse = NULL;
    }
    lmcp_init_AutomationResponse(&automationResponse);
    
    uint8_t *payload = &(data->payload[0]);

    int msg_result = lmcp_process_msg(&payload, sizeof(data->payload), (lmcp_object**)&automationResponse);

    if (msg_result == 0 && automationResponse->missioncommandlist_ai.length > 0) {

//        hexdump_raw(24, data->payload, compute_addr_attr_lmcp_message_size(data->payload, sizeof(data->payload)));

        currentWaypoint = automationResponse->missioncommandlist[0]->firstwaypoint;
        sendMissionCommand();

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



void return_home_in_event_data_receive_handler(counter_t numDropped, data_t *data) {

    printf("\n%s: received return home: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped); fflush(stdout);

}


recv_queue_t returnHomeInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool return_home_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&returnHomeInRecvQueue, numDropped, data);
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



void mission_command_out_event_data_send(data_t *data) {
    queue_enqueue(mission_command_out_queue, data);
    mission_command_out_SendEvent_emit();
    done_emit();
}

const char mission_command_attributes[] = "afrl.cmasi.MissionCommand$lmcp|afrl.cmasi.MissionCommand||400|63$";

void sendMissionCommand() {

    // Don't do anything if current waypoint is 0.
    // Something is wrong.  This method should not have been called.
    if (currentWaypoint == 0) {
        printf("%s: sendMissionCommand(): currentWaypoint == 0\n", get_instance_name()); fflush(stdout);
        return;
    }
  
    // Don't do anything until an AutomationRequest is recevied
    if (automationResponse == NULL) {
        printf("%s: sendMissionCommand(): automationResponse == NULL\n", get_instance_name()); fflush(stdout);
        return;
    }

    printf("%s: sendMissionCommand()\n", get_instance_name()); fflush(stdout);

    // Construct mission command message
    MissionCommand * missionCommand = NULL;
    lmcp_init_MissionCommand(&missionCommand);
    missionCommand->super = automationResponse->missioncommandlist[0]->super;
    missionCommand->super.commandid = currentCommand++;
    missionCommand->super.status = 1;
    missionCommand->waypointlist_ai.length = WINDOW_SIZE;
    missionCommand->waypointlist = malloc(sizeof(Waypoint*) * WINDOW_SIZE);

    if (returnHome) {
      currentWaypoint = HOME_WAYPOINT_NUM;
      // Set mission window waypoints to home
      for (int i = 0; i < WINDOW_SIZE; i++) {
          missionCommand->waypointlist[i] = homeWaypoint;
      }
    } else {
      // Construct mission window
      FillWindow( automationResponse->missioncommandlist[0]->waypointlist,
                  automationResponse->missioncommandlist[0]->waypointlist_ai.length,
                  currentWaypoint,
                  missionCommand->waypointlist,
                  WINDOW_SIZE);
    }

    missionCommand->firstwaypoint = currentWaypoint;
    AddressAttributedMessage * addressAttributedMessage = NULL;

    lmcp_init_AddressAttributedMessage(&addressAttributedMessage);
    addressAttributedMessage->attributes = mission_command_attributes;
    addressAttributedMessage->lmcp_obj = (lmcp_object*)missionCommand;

    data_t* data = calloc(1, sizeof(data_t));
    if (data != NULL) {
      lmcp_pack_AddressAttributedMessage(data->payload, addressAttributedMessage);

//      hexdump_raw(24, data->payload, compute_addr_attr_lmcp_message_size(data->payload, sizeof(data->payload)));

      // Send it
      mission_command_out_event_data_send(data);

      free(data);
    } else {
      printf("%s: sendMissionCommand(): could not allocate data buffer\n", get_instance_name()); fflush(stdout);
    }

    lmcp_free_AddressAttributedMessage(addressAttributedMessage, 1);

}



void run_poll(void) {
    counter_t numDropped;
    data_t data;

    while (true) {

        bool dataReceived = false;

        dataReceived = return_home_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            returnHome = true;
        }
        
        if (automationResponse != NULL) {
          dataReceived = air_vehicle_state_in_event_data_poll(&numDropped, &data);
          if (dataReceived) {
              air_vehicle_state_in_event_data_receive_handler(numDropped, &data);
          }
        }

        dataReceived = automation_response_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            automation_response_in_event_data_receive_handler(numDropped, &data);
        }

        seL4_Yield();
    }

}



void post_init(void) {
    recv_queue_init(&airVehicleStateInRecvQueue, air_vehicle_state_in_queue);
    recv_queue_init(&automationResponseInRecvQueue, automation_response_in_queue);
    recv_queue_init(&returnHomeInRecvQueue, return_home_in_queue);
    queue_init(mission_command_out_queue);
}

int run(void) {

    // Initialization
    initializeWaypointManager();

    // Run
    run_poll();

}

