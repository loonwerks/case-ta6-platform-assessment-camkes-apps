/*
 * Copyright 2020, Collins Aerospace
 */

#include <camkes.h>
#include <stdio.h>
#include <string.h>
#include <sel4/sel4.h>
#include <stdlib.h>
#include <counter.h>
#include <data.h>
#include <queue.h>

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
// Implementation of AADL Input Event Data Port (out) named "p1_out"
//
// NOTE: If we only need polling style receivers, we can get rid of the SendEvent

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
void p1_out_aadl_event_data_send(data_t *data) {
    queue_enqueue(p1_out_queue, data);
    p1_out_SendEvent_emit();
    done_emit();
}

//------------------------------------------------------------------------------
// Testing

void post_init(void) {
    queue_init(p1_out_queue);
}

int run(void) {

    int i = 0;
    int err = 0;
    data_t data;

    const char message[] = {
      '4', '0', '0'
      '$',
         'a', 't', 't', 'r', 'i', 'b', 'u', 't', 'e', 's',
      '$',
         'p', 'a', 'y', 'l', 'o', 'a', 'd',
      '$'
    };

    while (1) {

        // Busy loop to slow things down
        for(unsigned int j = 0; j < 100000; ++j){
            seL4_Yield();
        }

        // Stage data
        data.len = sizeof(message);
        memcpy((void *) &data.payload[0], (const void *) &message[0], sizeof(message));
        printf("%s: sending: %d\n", get_instance_name(), data.x);

        // Send the data
        p1_out_aadl_event_data_send(&data);          
    }
}



