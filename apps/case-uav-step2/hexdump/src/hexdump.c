/*
 * Copyright 2020, Collins Aerospace
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>


void fhexdump(int fd, const char *prefix, size_t max_line_len, const uint8_t* data, size_t datalen) {
  static const char empty[] = "";
  char *printables = malloc(max_line_len + 1);
  fprintf(fd, "%s     |", prefix);
  for (int index = 0; index < max_line_len; ++index) {
    fprintf(fd," %02x", (uint8_t) index);
  }
  fprintf(fd, "\n%s-----|", prefix);
  for (int index = 0; index < max_line_len; ++index) {
    fprintf(fd, "---");
  }
  size_t offset = 0, line_offset = 0;
  for (; line_offset < datalen; line_offset += max_line_len) {
    fprintf(fd, "\n%s%04x |", prefix, (uint16_t) line_offset);
    if (printables != NULL) memset(printables, 0, max_line_len + 1);
    for (; offset < datalen && offset < line_offset + max_line_len; ++offset) {
      fprintf(fd, " %02x", data[offset]);
      if (printables != NULL) printables[offset - line_offset] = ((isprint(data[offset])) ? data[offset] : '.');
    }
    if (printables != NULL) fprintf(fd, "  %s", printables);
  }
  fprintf(fd, "\n");
  if (printables != NULL) free(printables);
}


void hexdump(const char *prefix, size_t max_line_len, const uint8_t* data, size_t datalen) {
  fhexdump(stdout, prefix, max_line_len, data, datalen);
}
