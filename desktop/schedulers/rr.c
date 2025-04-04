#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_UNIQUE_IPS 4

// Structure to hold packet information
typedef struct {
    char sender_ip[16]; // IPv4 max length is 15 + null terminator
    int serial;
    int size;
} Packet;

// Function to check if an IP exists in the list of unique IPs
int find_ip_index(char unique_ips[MAX_UNIQUE_IPS][16], int ip_count, char *ip) {
    for (int i = 0; i < ip_count; i++) {
        if (strcmp(unique_ips[i], ip) == 0) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s SENTIP_...\n", argv[0]);
        return 1;
    }

    int MAX_PACKETS = argc - 1;
    
    Packet packets[MAX_PACKETS];
    char unique_ips[MAX_UNIQUE_IPS][16];
    int packet_count = 0, ip_count = 0;

    for (int i = 1; i < argc && packet_count < MAX_PACKETS; i++) {
        // Parse the input format: SENTIP_<ip>_PACKETSL_<serial>_<size>
        char *token;
        token = strtok(argv[i], "_");
        if (!token || strcmp(token, "SENTIP") != 0) continue;
        
        token = strtok(NULL, "_"); // Extract IP
        if (!token) continue;
        strcpy(packets[packet_count].sender_ip, token);
        
        // Check if this IP is new and store it in unique IP list
        if (find_ip_index(unique_ips, ip_count, token) == -1) {
            strcpy(unique_ips[ip_count++], token);
        }
        
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
    
    // Round-robin scheduling by IP
    int printed[MAX_PACKETS];
    for (int i = 0; i < MAX_PACKETS; i++) {
        printed[i] = 0;
    }
    int printed_count = 0;
    while (printed_count < packet_count) {
        for (int i = 0; i < ip_count; i++) {
            for (int j = 0; j < packet_count; j++) {
                if (!printed[j] && strcmp(packets[j].sender_ip, unique_ips[i]) == 0) {
                    printf("SENTIP_%s_PACKETSL_%d_%d\n", packets[j].sender_ip, packets[j].serial, packets[j].size);
                    printed[j] = 1;
                    printed_count++;
                    break;
                }
            }
        }
    }
    
    return 0;
}
