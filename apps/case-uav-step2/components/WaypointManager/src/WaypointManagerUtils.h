/*
 * Author: Dan DaCosta
 * Company: Rockwell Collins
 * License: Air Force Open Source Agreement Version 1.0
 *
 * Terminology: 
 *
 *  waypoint - Largely self-explanatory. We are only really using the number
 *  and nextwaypoint field in this data structure.
 *
 *  mission command - An array of unordered Waypoint data
 *  structures.
 *
 *  mission command segment - An array of unordered Waypoint data
 *  structure which is a subset of a mission command.
 *   
 */
#ifndef __WAYPOINTMANAGERUTILS_H__
#define __WAYPOINTMANAGERUTILS_H__
#include <stdbool.h>
#include "./CMASI/Waypoint.h"
#include "./CMASI/AutomationResponse.h"

//#define WINDOW_SIZE 15
//#define INIT_CMD_ID 72
//#define HOME_WAYPOINT_LAT 4631577348376571884UL     // 45.3364
//#define HOME_WAYPOINT_LONG 13861587297017124409UL   // -121.0032
//#define HOME_WAYPOINT_ALT 1143930880U               // 700.0
//#define HOME_WAYPOINT_SPEED 1102053376U
//#define HOME_WAYPOINT_NUM 17554

//void initializeWaypointManager();
bool IsWaypointInWindow( Waypoint ** waypointList, uint16_t waypointListSize, uint16_t windowSize, int64_t startId, int64_t id);
bool AutoPilotMissionCommandSegment(Waypoint ** ws, uint16_t len_ws, int64_t id, Waypoint ** ws_win, uint16_t len_ws_win);

#endif /* __WAYPOINTMANAGERUTILS_H__ */
