#include "sim/in.h"
#include <arpa/inet.h>

namespace sim {

// Global variable to store the IP address of the simulated NIC
static SimulatedNIC g_nic;

// Function to set the IP address of the simulated NIC
void set_ipaddr(const char* ip) {
    strncpy(g_nic.ip, ip, INET_ADDRSTRLEN);
    g_nic.ip[INET_ADDRSTRLEN - 1] = '\0'; // Ensure null termination
}

// Function to retrieve the IP address of the simulated NIC
const char* get_ipaddr() {
    return g_nic.ip;
}

// Simulated inet_ntop function 
const char* inet_ntop(int af, const void* src, char* dst, socklen_t size) {
    if (af != AF_INET || !src || !dst || size < INET_ADDRSTRLEN) {
        return nullptr;
    }
    
    // Convert network byte order to string
    const in_addr* addr = static_cast<const in_addr*>(src);
    uint32_t ip_addr = ntohl(addr->s_addr);
    
    snprintf(dst, size, "%d.%d.%d.%d",
        (ip_addr >> 24) & 0xFF,
        (ip_addr >> 16) & 0xFF,
        (ip_addr >> 8) & 0xFF,
        ip_addr & 0xFF);
    
    return dst;
}

// Simulated inet_pton function
int inet_pton(int af, const char* src, void* dst) {
    if (af != AF_INET || !src || !dst) {
        return -1;
    }
    
    in_addr* addr = static_cast<in_addr*>(dst);
    uint32_t a, b, c, d;
    
    if (sscanf(src, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        return 0;
    }
    
    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return 0;
    }
    
    addr->s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
    return 1;
}

}  // namespace sim