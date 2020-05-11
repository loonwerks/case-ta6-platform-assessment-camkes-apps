/*
 * Copyright 2020, Collins Aerospace
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>


void fhexdump(FILE *stream, const char *prefix, size_t max_line_len, const uint8_t* data, size_t datalen) {
  static const char empty[] = "";
  char *printables = malloc(max_line_len + 1);
  fprintf(stream, "%s     |", prefix);
  for (int index = 0; index < max_line_len; ++index) {
    fprintf(stream," %02x", (uint8_t) index);
  }
  fprintf(stream, "\n%s-----|", prefix);
  for (int index = 0; index < max_line_len; ++index) {
    fprintf(stream, "---");
  }
  size_t offset = 0, line_offset = 0;
  for (; line_offset < datalen; line_offset += max_line_len) {
    fprintf(stream, "\n%s%04x |", prefix, (uint16_t) line_offset);
    if (printables != NULL) memset(printables, 0, max_line_len + 1);
    for (; offset < datalen && offset < line_offset + max_line_len; ++offset) {
      fprintf(stream, " %02x", data[offset]);
      if (printables != NULL) printables[offset - line_offset] = ((isprint(data[offset])) ? data[offset] : '.');
    }
    if (printables != NULL) fprintf(stream, "  %s", printables);
  }
  fprintf(stream, "\n");
  if (printables != NULL) free(printables);
}


void hexdump(const char *prefix, size_t max_line_len, const uint8_t* data, size_t datalen) {
  fhexdump(stdout, prefix, max_line_len, data, datalen);
}
