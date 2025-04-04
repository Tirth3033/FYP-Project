#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to hold packet information
typedef struct {
    char sender_ip[16]; // IPv4 max length is 15 + null terminator
    int serial;
    int size;
} Packet;

// Comparator function for sorting by size (SJF)
int compare_size(const void *a, const void *b) {
    return ((Packet *)a)->size - ((Packet *)b)->size;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s SENTIP_...\n", argv[0]);
        return 1;
    }

    int MAX_PACKETS = argc - 1;
    
    Packet packets[MAX_PACKETS];
    int packet_count = 0;

    for (int i = 1; i < argc && packet_count < MAX_PACKETS; i++) {
        // Parse the input format: SENTIP_<ip>_PACKETSL_<serial>_<size>
        char *token;
        token = strtok(argv[i], "_");
        if (!token || strcmp(token, "SENTIP") != 0) continue;
        
        token = strtok(NULL, "_"); // Extract IP
        if (!token) continue;
        strcpy(packets[packet_count].sender_ip, token);
        
        token = strtok(NULL, "_"); // Should be "PACKETSL"
        if (!token || strcmp(token, "PACKETSL") != 0) continue;
        
        token = strtok(NULL, "_"); // Extract serial
        if (!token) continue;
        packets[packet_count].serial = atoi(token);
        
        token = strtok(NULL, "_"); // Extract size
        if (!token) continue;
        packets[packet_count].size = atoi(token);
        
        packet_count++;
    }
    
    // Sort packets by size (SJF)
    qsort(packets, packet_count, sizeof(Packet), compare_size);
    
    // Print packets in SJF order
    for (int i = 0; i < packet_count; i++) {
        printf("SENTIP_%s_PACKETSL_%d_%d\n", packets[i].sender_ip, packets[i].serial, packets[i].size);
    }
    
    return 0;
}