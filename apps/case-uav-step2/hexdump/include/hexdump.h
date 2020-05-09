/*
 * Copyright 2020, Collins Aerospace
 */

#pragma once

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>


void fhexdump(int fd, const char *prefix, size_t max_line_len, const uint8_t* data, size_t datalen);


void hexdump(const char *prefix, size_t max_line_len, const uint8_t* data, size_t datalen);
