/*
 * Copyright 2020, Collins Aerospace
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 3-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD3.txt" for details.
 */

/* remember that this is compiled as part of the kernel, and so is referencing kernel headers */

#include <config.h>
#include <object/structures.h>
#include <model/statedata.h>

/*
 *
 */

const dschedule_t ksDomSchedule[] = {
  { .domain =  0, .length = 1 },   // 000 ms : 2 ms seL4, APSS
  { .domain =  1, .length = 1 },   // 002 ms : 2 ms Pacer
  { .domain =  0, .length = 1 },   // 004 ms : 2 ms seL4, APSS
  { .domain =  2, .length = 4 },   // 006 ms : 8 ms vmRadio
  { .domain =  0, .length = 1 },   // 014 ms : 2 ms seL4, APSS
  { .domain =  2, .length = 4 },   // 016 ms : 8 ms vmRadio
  { .domain =  0, .length = 1 },   // 024 ms : 2 ms seL4, APSS
  { .domain =  2, .length = 4 },   // 026 ms : 8 ms vmRadio
  { .domain =  0, .length = 1 },   // 034 ms : 2 ms seL4, APSS
  { .domain =  2, .length = 4 },   // 036 ms : 8 ms vmRadio
  { .domain =  0, .length = 1 },   // 044 ms : 2 ms seL4, APSS
  { .domain =  2, .length = 4 },   // 046 ms : 8 ms vmRadio
  { .domain =  0, .length = 1 },   // 054 ms : 2 ms seL4, APSS
  { .domain =  2, .length = 4 },   // 056 ms : 8 ms vmRadio
  { .domain =  0, .length = 1 },   // 064 ms : 2 ms seL4, APSS
  { .domain =  2, .length = 4 },   // 066 ms : 8 ms vmRadio
  { .domain =  0, .length = 1 },   // 074 ms : 2 ms seL4, APSS
  { .domain =  2, .length = 4 },   // 076 ms : 8 ms vmRadio
  { .domain =  0, .length = 1 },   // 084 ms : 2 ms seL4, APSS
  { .domain =  3, .length = 1 },   // 086 ms : 2 ms conn01, conn02
  { .domain =  0, .length = 1 },   // 088 ms : 2 ms seL4, APSS
  { .domain =  4, .length = 1 },   // 090 ms : 2 ms conn03
  { .domain =  0, .length = 1 },   // 092 ms : 2 ms seL4, APSS
  { .domain =  5, .length = 2 },   // 094 ms : 4 ms AM Gate
  { .domain =  0, .length = 1 },   // 098 ms : 2 ms seL4, APSS
  { .domain =  6, .length = 1 },   // 100 ms : 2 ms conn04, conn05, conn06
  { .domain =  0, .length = 1 },   // 102 ms : 2 ms seL4, APSS
  { .domain =  7, .length = 2 },   // 104 ms : 4 ms LST filter
  { .domain =  0, .length = 1 },   // 108 ms : 2 ms seL4, APSS
  { .domain =  8, .length = 1 },   // 110 ms : 2 ms conn08, conn12, conn15
  { .domain =  0, .length = 1 },   // 112 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 114 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 122 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 124 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 132 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 134 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 142 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 144 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 152 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 154 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 162 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 164 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 172 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 174 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 182 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 184 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 192 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 194 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 202 ms : 2 ms seL4, APSS
  { .domain = 10, .length = 1 },   // 204 ms : 2 ms conn10, conn11, conn13
  { .domain =  0, .length = 1 },   // 206 ms : 2 ms seL4, APSS
  { .domain = 11, .length = 2 },   // 208 ms : 4 ms Geo monitor
  { .domain =  0, .length = 1 },   // 212 ms : 2 ms seL4, APSS
  { .domain = 12, .length = 1 },   // 214 ms : 2 ms conn14, conn 17
  { .domain =  0, .length = 1 },   // 216 ms : 2 ms seL4, APSS
  { .domain = 13, .length = 4 },   // 218 ms : 8 ms WPM
  { .domain =  0, .length = 1 },   // 220 ms : 2 ms seL4, APSS
  { .domain = 13, .length = 4 },   // 222 ms : 8 ms WPM
  { .domain =  0, .length = 1 },   // 230 ms : 2 ms seL4, APSS
  { .domain = 14, .length = 2 },   // 232 ms : 4 ms response monitor
  { .domain =  0, .length = 7 },   // 236 ms : 14 ms, seL4, APSS
};

const word_t ksDomScheduleLength = sizeof(ksDomSchedule) / sizeof(dschedule_t);
