/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#include <autoconf.h>
#include <camkes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sel4/sel4.h>
/* #include <utils/attribute.h> */
/* #include <utils/ansi.h> */
#include <camkes.h>
#include <camkes/io.h>
#include <camkes/irq.h>
#include <platsupport/chardev.h>
#include <platsupport/irq.h>
#include "serial.h"
#include "plat.h"

#define CLIENT_OUTPUT_BUFFER_SIZE 4096

/* TODO: have the MultiSharedData template generate a header with these */
void getchar_emit(unsigned int id) WEAK;
seL4_Word getchar_enumerate_badge(unsigned int id) WEAK;
unsigned int getchar_num_badges() WEAK;
void *getchar_buf(unsigned int id) WEAK;
int getchar_largest_badge(void) WEAK;

typedef struct getchar_buffer {
    uint32_t head;
    uint32_t tail;
    char buf[4096 - 8];
} getchar_buffer_t;

compile_time_assert(getchar_buf_sized, sizeof(getchar_buffer_t) == sizeof(Buf));

typedef struct getchar_client {
    unsigned int client_id;
    volatile getchar_buffer_t *buf;
    uint32_t last_head;
} getchar_client_t;

static ps_io_ops_t io_ops;

static uint8_t output_buffer[CLIENT_OUTPUT_BUFFER_SIZE];
static int output_buffer_used= 0;
static uint16_t output_buffer_bitmask = 0;

static int done_output = 0;

static int has_data = 0;

static int num_getchar_clients = 0;
static getchar_client_t *getchar_clients = NULL;

static void flush_buffer(void)
{
    if (output_buffer_used == 0) {
        return;
    }
    for (int i = 0; i < output_buffer_used; i++) {
        printf("%c", output_buffer[i]);
    }
    done_output = 1;
    output_buffer_used = 0;
    output_buffer_bitmask &= ~BIT(0);
    fflush(stdout);
}

static int debug = 0;

/* Try to flush up to the end of the line. */
static bool flush_buffer_line(void)
{
    if (output_buffer_used == 0) {
        return 0;
    }
    uint8_t *nlptr = memchr(output_buffer, '\r', output_buffer_used);
    if (nlptr == NULL) {
        nlptr = memchr(output_buffer, '\n', output_buffer_used);
    }
    if (nlptr == NULL) {
        if (debug == 2) {
            ZF_LOGD("newline not found!\r\n");
        }
        return 0;
    }
    size_t length = (nlptr - &output_buffer[0]) + 1;
    if (length < output_buffers_used && (output_buffer[length] == '\n' || output_buffer[length] == '\r')) {
        length++;               /* Include \n after \r if present */
    }
    if (length == 0) {
        if (debug == 2) {
            ZF_LOGD("0-length!\r\n");
        }
        return 0;
    }
    int i;
    for (i = 0; i < length; i++) {
        printf("%c", output_buffer[i]);
    }
    for (i = length; i < output_buffer_used; i++) {
        output_buffer[i - length] = output_buffer[i];
    }
    output_buffer_used -= length;
    if (output_buffer_used == 0) {
        output_buffer_bitmask &= ~BIT(0);
    }
    return 1;
}

static int is_newline(const uint8_t *c)
{
    return (c[0] == '\r' && c[1] == '\n') || (c[0] == '\n' && c[1] == '\r');
}

static void internal_putchar(int c)
{
    int UNUSED error;
    error = serial_lock();
    /* Add to buffer */
    int index = output_buffer_used;
    uint8_t *buffer = output_buffer;
    buffer[index] = (uint8_t)c;
    output_buffer_used++;

    if (output_buffer_used == CLIENT_OUTPUT_BUFFER_SIZE) {
        /* Since we're violating contract anyway (flushing in the
         * middle of someone else's line), flush all buffers, so the
         * fastpath can be used again. */
        char is_done;
        int i;
        flush_buffer_line();
        while (!is_done) {
            is_done = 1;
                if (flush_buffer_line()) {
                    is_done = 0;
                }
        }
    } else if ((index >= 1 && is_newline(buffer + index - 1) && coalesce_status == -1)
               || (last_out == b && output_buffer_bitmask == 0 && coalesce_status == -1)) {
        /* Allow fast output (newline or same-as-last-client) if
         * multi-input is not enabled OR last coalescing attempt
         * failed due to a mismatch. This is important as client output
         * may be delayed; coalescing failure due to empty buffer
         * should lead to further buffering rather than early flush,
         * in case we can coalesce later. */
        flush_buffer();
    } else {
        output_buffer_bitmask |= BIT(0);
    }
    has_data = 1;
    error = serial_unlock();
}

static void give_client_char(uint8_t c)
{
    getchar_client_t *client = &getchar_client;
    uint32_t next_tail = (client->buf->tail + 1) % sizeof(client->buf->buf);
    if (next_tail == client->buf->head) {
        /* full */
        return;
    }
    uint32_t last_read_head = client->buf->head;
    client->buf->buf[client->buf->tail] = (uint8_t)c;
    /* no synchronize in here as we assume TSO */
    client->buf->tail = next_tail;
    __sync_synchronize();
    if (last_read_head != client->last_head) {
        getchar_emit(client->client_id);
        client->last_head = last_read_head;
    }
}

static void handle_char(uint8_t c)
{
    /* If there are no getchar clients, then we return immediately */
    if (!getchar_num_badges) {
        return;
    }

    give_client_char(c);
}

static void timer_callback(void *data)
{
    int UNUSED error;
    error = serial_lock();
    if (done_output) {
        done_output = 0;
    } else if (has_data) {
        /* flush everything if no writes since last callback */
        int i;
        char is_done = 0;
        char succeeded = 0;
        while (!is_done) {
            is_done = 1;
            if (flush_buffer_line()) {
                succeeded = 1;
                is_done = 0;
            }
        }
        if (!succeeded) {
            flush_buffer();
        }
    }
    error = serial_unlock();
}

seL4_CPtr timeout_notification(void);

int run(void)
{
    seL4_CPtr notification = timeout_notification();
    while (1) {
        seL4_Wait(notification, NULL);
        timer_callback(NULL);
    }
    return 0;
}

void autopilot_serial_server_irq_handle(void *data, ps_irq_acknowledge_fn_t acknowledge_fn, void *ack_data)
{
    int error = serial_lock();
    ZF_LOGF_IF(error, "Failed to lock serial server");

    plat_serial_interrupt(handle_char);

    error = acknowledge_fn(ack_data);
    ZF_LOGF_IF(error, "Failed to acknowledge IRQ");

    error = serial_unlock();
    ZF_LOGF_IF(error, "Failed to unlock serial server");
}

void serial_putchar(int c)
{
    plat_serial_putchar(c);
}

void pre_init(void)
{
    int error;
    error = serial_lock();

    error = camkes_io_ops(&io_ops);
    ZF_LOGF_IF(error, "Failed to initialise IO ops");

    // Initialize the serial port
    plat_pre_init(&io_ops);
    set_putchar(serial_putchar);
    /* query what getchar clients exist */
    if (getchar_num_badges) {
        num_getchar_clients = getchar_num_badges();
        getchar_clients = calloc(num_getchar_clients, sizeof(getchar_client_t));
        for (int i = 0; i < num_getchar_clients; i++) {
            unsigned int badge = getchar_enumerate_badge(i);
            assert(badge <= num_getchar_clients);
            getchar_clients[i].client_id = badge;
            getchar_clients[i].buf = getchar_buf(badge);
            getchar_clients[i].last_head = -1;
        }
    }
    plat_post_init(&(io_ops.irq_ops));
    /* Start regular heartbeat of 500ms */
    timeout_periodic(0, 500000000);
    error = serial_unlock();
}

void post_init(void)
{
}

seL4_Word raw_putchar_get_sender_id(void) WEAK;
void raw_putchar_putchar(int c)
{
    internal_putchar(c);
}

/* We had to define at least one function in the getchar RPC procedure
 * so now we need to implement it even though it is not used */
void getchar_foo(void)
{
    assert(!"should not be reached");
}
