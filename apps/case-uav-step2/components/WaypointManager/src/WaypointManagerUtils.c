/*
 * Author: Dan DaCosta
 * Company: Rockwell Collins
 * License: Air Force Open Source Agreement Version 1.0
 *   
 */

#include "WaypointManagerUtils.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "./CMASI/lmcp.h"
#include "./CMASI/common/conv.h"
#include "./CMASI/MissionCommand.h"
#include "./CMASI/AirVehicleState.h"
#include "./CMASI/EntityState.h"
#include "./CMASI/AutomationResponse.h"
#include "./CMASI/AddressAttributedMessage.h"

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


/* NB: Cycles in ws will be unrolled into win. */
bool FillWindow(  Waypoint ** ws,
                  uint16_t len_ws,
                  int64_t id,
                  uint16_t len_ws_win,
                  Waypoint ** ws_win /* out */) {
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


/* ASM: ws != null */
/* ASM: len_ws > 0. */
/* ASM: ws_win != null */
/* ASM: len_ws_win > 0 */
/* ASM: len_ws_win is less than the number of waypoints that can be
   stored in ws_win. */
/* ASM: Last waypoint starts a cycle. */
bool AutoPilotMissionCommandSegment(  Waypoint ** ws,
                                      uint16_t len_ws,
                                      int64_t id,
                                      Waypoint ** ws_win, /* out */
                                      uint16_t len_ws_win) {

  bool success = false;
  success = FillWindow(ws, len_ws, id, len_ws_win, ws_win);

  return success;
}

#ifdef __WAYPOINTMANAGERUTILS_TEST__

void sendMissionCommand() {

printf("\n************************\n");
printf("** sendMissionCommand **\n");
printf("************************\n\n");
printf("currentWaypoint = %llu\n", currentWaypoint);
  // Don't do anything if current waypoint is 0.
  // Something is wrong.  This method should not have been called.
  if (currentWaypoint == 0) {
    return;
  }
  
  // Don't do anything until an AutomationRequest is recevied
  if (automationResponse == NULL) {
    return;
  }

  MissionCommand * missionCommand = NULL;
  lmcp_init_MissionCommand(&missionCommand);
  missionCommand->super = automationResponse->missioncommandlist[0]->super;
  missionCommand->super.commandid = currentCommand++;
  missionCommand->super.status = 1;
  missionCommand->waypointlist_ai.length = WINDOW_SIZE;
  missionCommand->waypointlist = malloc(sizeof(Waypoint*) * WINDOW_SIZE);

  if (returnHome) {
    // Set mission window waypoints to home
    for (int i = 0; i < WINDOW_SIZE; i++) {
      missionCommand->waypointlist[i] = homeWaypoint;
    }
  } else {
    AutoPilotMissionCommandSegment( automationResponse->missioncommandlist[0]->waypointlist,
                                    automationResponse->missioncommandlist[0]->waypointlist_ai.length,
                                    currentWaypoint,
                                    missionCommand->waypointlist,
                                    WINDOW_SIZE);
  }

  missionCommand->firstwaypoint = missionCommand->waypointlist[0]->nextwaypoint;
  char * attributes = "afrl.cmasi.MissionCommand$lmcp|afrl.cmasi.MissionCommand||400|63$";
  AddressAttributedMessage * addressAttributedMessage = NULL;
  lmcp_init_AddressAttributedMessage(&addressAttributedMessage);
  addressAttributedMessage->attributes = attributes;
  lmcp_init_MissionCommand((MissionCommand**)&(addressAttributedMessage->lmcp_obj));
  addressAttributedMessage->lmcp_obj = (lmcp_object*)missionCommand;

  // Send it
  size_t msgSize = lmcp_packsize_AddressAttributedMessage(addressAttributedMessage);
  uint8_t * msg = calloc(1, msgSize);
  lmcp_pack_AddressAttributedMessage(msg, addressAttributedMessage);

printf("afrl.cmasi.MissionCommand (%lu bytes) :\n", msgSize);
for (size_t i = 0; i < msgSize; i++) {
  printf("%02X ", msg[i]);
}
printf("\n");

  lmcp_free_AddressAttributedMessage(addressAttributedMessage, 1);

  return;
}



/* lmcpObjectFromFile: Deserialize an AddressAttributedMessage reading from a file.

     f - The file to read the message from.

     lmcpObj - The LMCP Object to be populated from the serialized data.

*/
void lmcpObjectFromFile(FILE * f, lmcp_object ** lmcpObj) {

    uint8_t * source = NULL;
    uint8_t * orig_source = NULL;
    size_t size = 0;

    /* assumption: avs is not NULL. */
    assert(lmcpObj != NULL);
    /* assumption: f is not NULL. */
    assert(f != NULL);

    if (fseek(f, 0L, SEEK_END) == 0) {
        /* Get the size of the file. */
        size = ftell(f);
        if (size == -1) {
            /* Error */
        }

        /* Allocate our buffer to that size. */
        source = malloc((sizeof(char) * (size + 1))/5);
        orig_source = source;
        /* Go back to the start of the file. */
        if (fseek(f, 0L, SEEK_SET) != 0) {
            /* Error */
        }

        /* Read the entire file into memory. */
        size_t newlen = 0;
        int data;
        while (fscanf(f, "%*c%*c%x,", &data) == 1) {
          source[newlen++] = (unsigned char)data;
        }
        assert(lmcp_process_msg(&source,size,lmcpObj) != -1);
        free(orig_source);
        source = NULL;
        orig_source = NULL;
    }

    return;
}
void getMessagesFromFile(FILE * f, uint8_t ** msgs, size_t msgLens[NUM_AVS_MSGS]) {

  size_t nread;
  size_t len = 0;
  char * line;
  for (size_t i = 0; i < NUM_AVS_MSGS; i++) {

    line = NULL;

    if ((nread = getline(&line, &len, f)) == -1) {
      break;
    }
    int data;
    size_t newlen = 0;
    size_t size = nread/5;
    msgs[i] = malloc(size);

    while (sscanf(line, "%*c%*c%x,", &data) == 1) {
      msgs[i][newlen++] = (unsigned char)data;
      line += 5;
    }

    msgLens[i] = size;

  }

  return;

}



bool checkAutomationResponse() {

printf("\n*****************************\n");
printf("** checkAutomationResponse **\n");
printf("*****************************\n\n");

  FILE * f = NULL;

  if (automationResponse != NULL) {
    lmcp_free_AutomationResponse(automationResponse, 1);
    automationResponse = NULL;
  }
  lmcp_init_AutomationResponse(&automationResponse);
  /* Load waterways AutomationResponse message. */
  f = fopen("./testdata/waterways.ares", "r");
  if (f == NULL) {
    return false;
  }
  lmcpObjectFromFile(f, (lmcp_object**)&automationResponse);
  fclose(f);

  if (currentWaypoint > 0) {
    sendMissionCommand();
  }

  return true;

}


void checkReturnHome() {

  printf("\n*********************\n");
  printf("** checkReturnHome **\n");
  printf("*********************\n");

//  returnHome = getReturnHome();

  returnHome = true;

  return;

}


bool loadAirVehicleStateMessages() {

  FILE * f = NULL;

  /* Load waterways AirVehicleState data. */
  f = fopen("./testdata/waterways.avs", "r");
  if (f == NULL) {
    return false;
  }

  avsMsgs = malloc(sizeof(uint8_t*) * NUM_AVS_MSGS);

  getMessagesFromFile(f, avsMsgs, avsMsgLens);
  fclose(f);

  currentAvsMsg = 0;

  return true;
}

bool checkAirVehicleState() {

printf("\n**************************\n");
printf("** checkAirVehicleState **\n");
printf("**************************\n\n");

for (int j = 0; j < avsMsgLens[currentAvsMsg]; j++) {
  printf("%02X ", avsMsgs[currentAvsMsg][j]);
}
printf("\n\n");

  AirVehicleState * airVehicleState = NULL;
  lmcp_init_AirVehicleState(&airVehicleState);
  assert(lmcp_process_msg(&(avsMsgs[currentAvsMsg]), avsMsgLens[currentAvsMsg], (lmcp_object**)&airVehicleState) != -1);

printf("AirVehicleState waypoint = %llu\n", airVehicleState->super.currentwaypoint);

  // Get the current waypoint
  if (currentWaypoint != airVehicleState->super.currentwaypoint) {
    currentWaypoint = airVehicleState->super.currentwaypoint;
    if (automationResponse != NULL) {
      sendMissionCommand();
    }
  }

  lmcp_free_AirVehicleState(airVehicleState, 1);
  
  currentAvsMsg++;

  return;

}


int main(void) {

  // Initialize
  initializeWaypointManager();

  loadAirVehicleStateMessages();
  // initial air vehicle state message
  checkAirVehicleState();

  // Check for Automation Response
  checkAutomationResponse();

  // Check if we need to return home
  checkReturnHome();

  // Update current waypoint
  while (currentAvsMsg < NUM_AVS_MSGS) {
printf("\nAVS msg # %lu\n", currentAvsMsg + 1);
    checkAirVehicleState();
  }

  if (automationResponse != NULL) {
    lmcp_free_AutomationResponse(automationResponse, 1);
    automationResponse = NULL;
  }
  if (homeWaypoint != NULL) {
    lmcp_free_Waypoint(homeWaypoint, 1);
    homeWaypoint = NULL;
  }
  free(avsMsgs);

  return 0;
}

#endif /* __WAYPOINTMANAGERUTILS_TEST__ */
