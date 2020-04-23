/*
 * Copyright 2020, Collins Aerospace
 */

#include <camkes.h>
#include <stdio.h>
#include <stdlib.h>
#include <counter.h>
#include <data.h>
#include <queue.h>

#include <stdint.h>
#include <sys/types.h>

//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "p1_in".

void hexdump(const char *prefix, size_t max_line_len, const uint8_t* data, size_t datalen) {
    printf("%s     |", prefix);
    for (int index = 0; index < max_line_len; ++index) {
        printf(" %02x", (uint8_t) index);
    }
    printf("%s-----|", prefix);
    for (int index = 0; index < max_line_len; ++index) {
        printf("---");
    }
    size_t offset = 0, line_offset = 0;
    for (; line_offset < datalen; line_offset += max_line_len) {
        printf("%s%04x |", prefix, (uint16_t) line_offset);
        for (; offset < datalen && offset < line_offset + max_line_len; ++offset) {
            printf(" %02x", data[offset]);
        }
    }
}

void p1_in_aadl_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received: %d  numDropped: %" PRIcounter "\n", get_instance_name(), data->len, numDropped);
    hexdump("    ", 8, data->payload, data->len);
}

//------------------------------------------------------------------------------
// Implementation of AADL Input Event Data Port (in) named "p1_in"
// Three styles: poll, wait and callback.
//
// Callback would typically be avoid for safety critical systems. It is harder
// to analyze since the callback handler is run on some arbitrary thread.
//
// NOTE: If we only need polling style receivers, we can get rid of SendEvent

recv_queue_t recvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool p1_in_aadl_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&recvQueue, numDropped, data);
}

void p1_in_aadl_event_data_wait(counter_t *numDropped, data_t *data) {
    while (!p1_in_aadl_event_data_poll(numDropped, data)) {
        p1_in_SendEvent_wait();
    }
}

static void p1_in_handler(void *v) {
    counter_t numDropped;
    data_t data;

    // Handle ALL events that have been queued up
    while (p1_in_aadl_event_data_poll(&numDropped, &data)) {
    	p1_in_aadl_event_data_receive(numDropped, &data);
    }
    while (! p1_in_SendEvent_reg_callback(&p1_in_handler, NULL));
}

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

        // Random number of polls for testing
        int n = (random() % 10);
        for(unsigned int j = 0; j < n; ++j){
            bool dataReceived = p1_in_aadl_event_data_poll(&numDropped, &data);
            if (dataReceived) {
                p1_in_aadl_event_data_receive(numDropped, &data);
            } else {
                /* printf("%s: received nothing\n", get_instance_name()); */
            }
        }

    }

}

void run_wait(void) {
    counter_t numDropped;
    data_t data;

    while (true) {
        p1_in_aadl_event_data_wait(&numDropped, &data);
        p1_in_aadl_event_data_receive(numDropped, &data);

        // Busy loop to wait a bit and slow things down randomly
        int n = (random() % 10) * 5000;
        for(unsigned int j = 0; j < n; ++j){
            seL4_Yield();
        }

    }
}

int run_callback(void) {
     return p1_in_SendEvent_reg_callback(&p1_in_handler, NULL);
}

void post_init(void) {
   recv_queue_init(&recvQueue, p1_in_queue);
}

int run(void) {
    // Pick one receive style to test.
    run_poll();
    //run_wait();
    //return run_callback();
}

