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
  { .domain =  3, .length = 1 },   // 076 ms : 2 ms conn01, conn02
  { .domain =  0, .length = 1 },   // 078 ms : 2 ms seL4, APSS
  { .domain =  4, .length = 1 },   // 080 ms : 2 ms conn03
  { .domain =  0, .length = 1 },   // 082 ms : 2 ms seL4, APSS
  { .domain =  5, .length = 2 },   // 084 ms : 4 ms AM Gate
  { .domain =  0, .length = 1 },   // 088 ms : 2 ms seL4, APSS
  { .domain =  6, .length = 1 },   // 090 ms : 2 ms conn04, conn05, conn06
  { .domain =  0, .length = 1 },   // 092 ms : 2 ms seL4, APSS
  { .domain =  7, .length = 4 },   // 094 ms : 8 ms LST filter
  { .domain =  0, .length = 1 },   // 102 ms : 2 ms seL4, APSS
  { .domain =  8, .length = 1 },   // 104 ms : 2 ms conn08, conn12, conn15
  { .domain =  0, .length = 1 },   // 106 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 108 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 116 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 118 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 126 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 128 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 136 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 138 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 146 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 148 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 156 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 158 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 166 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 168 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 176 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 178 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 186 ms : 2 ms seL4, APSS
  { .domain =  9, .length = 4 },   // 188 ms : 8 ms vmUxAS
  { .domain =  0, .length = 1 },   // 196 ms : 2 ms seL4, APSS
  { .domain = 10, .length = 1 },   // 198 ms : 2 ms conn10, conn11, conn13
  { .domain =  0, .length = 1 },   // 200 ms : 2 ms seL4, APSS
  { .domain = 11, .length = 4 },   // 202 ms : 8 ms Geo monitor
  { .domain =  0, .length = 1 },   // 210 ms : 2 ms seL4, APSS
  { .domain = 11, .length = 4 },   // 212 ms : 8 ms Geo monitor
  { .domain =  0, .length = 1 },   // 220 ms : 2 ms seL4, APSS
  { .domain = 12, .length = 1 },   // 222 ms : 2 ms conn14, conn 17
  { .domain =  0, .length = 1 },   // 224 ms : 2 ms seL4, APSS
  { .domain = 13, .length = 4 },   // 226 ms : 8 ms WPM
  { .domain =  0, .length = 1 },   // 234 ms : 2 ms seL4, APSS
  { .domain = 13, .length = 4 },   // 236 ms : 8 ms WPM
  { .domain =  0, .length = 1 },   // 244 ms : 2 ms seL4, APSS
  { .domain = 14, .length = 2 },   // 246 ms : 4 ms response monitor
};

const word_t ksDomScheduleLength = sizeof(ksDomSchedule) / sizeof(dschedule_t);
