/*
 * Copyright 2020, Collins Aerospace
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv3.txt" for details.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <camkes_log_queue.h>


camkes_log_recv_queue_t recvQueue;
int dataport_fd;


int camkes_dataport_read_wait(camkes_log_counter_t *numDropped, camkes_log_data_t *data) {
    while (!camkes_log_queue_dequeue(&recvQueue, numDropped, data)) {
    	int val;
    	/* Blocking read */
    	int result = read(dataport_fd, &val, sizeof(val));
		if (result < 0) {
		    printf("Error: %s\n", strerror(errno));
		    return -1;
		} 
    }
}


ssize_t output_to_udp(int fd, struct sockaddr_in *daddr, const void* buffer, size_t length) {
    size_t current_offset = 0;
    const size_t mtu = 1440;  // TODO: ask the interface via getsockopt()
    while (current_offset < length) {
        size_t length_remaining = length - current_offset;
        size_t chunk_size = (length_remaining <= mtu) ? length_remaining : mtu;
        if (sendto(fd, buffer + current_offset, chunk_size, 0, (struct sockaddr *) daddr, sizeof(struct sockaddr_in)) < 0)
        {
            printf("camkes_log_relay: sendto failed: %s\n", strerror(errno));
            current_offset = (size_t) -1;
	        break;
        }
        current_offset += chunk_size;
    }
    return (ssize_t) current_offset;
}


int main(int argc, char *argv[])
{

    if (argc != 3) {
        printf("Usage: %s <camkes_device> <udp_destination>\n\n"
               "Relays the specified dataport file to the specified UDP destination (x.x.x.x:y)",
               argv[0]);
        return 1;
    }

    char *dataport_name = argv[1];
    dataport_fd = open(dataport_name, O_RDWR);
    assert(dataport_fd >= 0);

    camkes_log_queue_t *dataport;
    if ((dataport = (camkes_log_queue_t *) mmap(NULL, sizeof(camkes_log_queue_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                                dataport_fd, 1 * getpagesize())) == (void *) -1) {
        printf("mmap failed\n");
        close(dataport_fd);
    }
    camkes_log_recv_queue_init(&recvQueue, dataport);

    struct sockaddr_in client_sockaddr, destination_sockaddr;

    memset((void *) &client_sockaddr, 0, sizeof(struct sockaddr_in));
    client_sockaddr.sin_family = AF_INET;
    client_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_sockaddr.sin_port = htons(0);

    // TODO: accept FQDN in addition to IP addresses as with following code:
    // look up the address of the server given its name
    // struct hostent* hp = gethostbyname(host);
    // if (!hp)
    // {
    //     fprintf(stderr, "could not obtain address of %s\n", host);
    //     return 0;
    // }
    // put the host's address into the server address structure
    // memcpy((void *)&m_destinationSockaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

    char *destination_port = strtok(argv[2], ":");
    memset((void *) &destination_sockaddr, 0, sizeof(struct sockaddr_in));
    destination_sockaddr.sin_family = AF_INET;
    destination_sockaddr.sin_addr.s_addr = inet_addr(argv[2]);
    destination_sockaddr.sin_port = htons((destination_port != NULL) ? atoi(destination_port) : 5577);
    printf("camkes_log_relay: setting UDP destination to %s:%u\n",
        inet_ntoa(destination_sockaddr.sin_addr), (unsigned) ntohs(destination_sockaddr.sin_port));

    int destination_fd = -1;
    if ((destination_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("camkes_log_relay: could not create destination socket: %s\n", strerror(errno));
        munmap(dataport, sizeof(camkes_log_queue_t));
        close(dataport_fd);
        exit(1);
    }

    if (bind(destination_fd, (struct sockaddr *) &client_sockaddr, sizeof(struct sockaddr_in)) < 0)
    {
        printf("camkes_log_relay: could not bind to client address: %s\n", strerror(errno));
        close(destination_fd);
        munmap(dataport, sizeof(camkes_log_queue_t));
        close(dataport_fd);
        exit(1);
    }

    camkes_log_counter_t numDropped;
    camkes_log_data_t data;

    while (true) {
        if (camkes_dataport_read_wait(&numDropped, &data) < 0) {
            printf("camkes_log_relay: error reading CAmkES dataport\n");
            goto bailout;
        }
        if (output_to_udp(destination_fd, &destination_sockaddr, &data.payload[0], data.payload_length) < 0) {
            printf("camkes_log_relay: error sending to UDP destination\n");
            goto bailout;
        }
    }

    close(destination_fd);
    munmap(dataport, sizeof(camkes_log_queue_t));
    close(dataport_fd);
    exit(0);

bailout:

    close(destination_fd);
    munmap(dataport, sizeof(camkes_log_queue_t));
    close(dataport_fd);
    exit(1);

}

