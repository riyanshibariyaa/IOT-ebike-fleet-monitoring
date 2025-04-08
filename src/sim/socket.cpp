#include "sim/socket.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <cstring>
#include <thread>
#include <algorithm>

namespace sim {

// Initialize the static IP map
std::unordered_map<std::string, std::string> socket::ip_map;

// Constructor
socket::socket(int domain, int type, int protocol) {
    if (domain != AF_INET || type != SOCK_DGRAM) {
        throw std::runtime_error("Only AF_INET/SOCK_DGRAM is supported");
    }
    
    // Generate a unique identifier for this socket
    std::stringstream ss;
    ss << std::this_thread::get_id();
    uid = ss.str();
    
    // Store the IP address for this thread
    ip_map[uid] = get_ipaddr();
    
    // Create a UNIX domain socket
    sockfd = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }
}

// Destructor
socket::~socket() {
    if (sockfd >= 0) {
        close(sockfd);
    }
    
    // Remove the socket file if we created one
    if (!unixPath.empty()) {
        unlink(unixPath.c_str());
    }
    
    // Remove the IP mapping
    ip_map.erase(uid);
}

// Bind the socket to an address
void socket::bind(const struct ::sockaddr_in& addr) {
    // Create a path for the UNIX socket based on the IP and port
    unixPath = ipPortToPath(addr);
    
    // Remove the socket file if it already exists
    unlink(unixPath.c_str());
    
    // Bind the socket to the UNIX path
    struct sockaddr_un unixAddr;
    unixAddr.sun_family = AF_UNIX;
    strncpy(unixAddr.sun_path, unixPath.c_str(), sizeof(unixAddr.sun_path) - 1);
    
    if (::bind(sockfd, (struct sockaddr*)&unixAddr, sizeof(unixAddr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }
}

// Send data to a destination
ssize_t socket::sendto(const void* data, size_t size, int flags, const struct ::sockaddr_in& destAddr) {
    // Convert the destination address to a UNIX path
    std::string destPath = ipPortToPath(destAddr);
    
    // Create the destination address structure
    struct sockaddr_un unixDestAddr;
    unixDestAddr.sun_family = AF_UNIX;
    strncpy(unixDestAddr.sun_path, destPath.c_str(), sizeof(unixDestAddr.sun_path) - 1);
    
    // Send the data
    return ::sendto(sockfd, data, size, flags, (struct sockaddr*)&unixDestAddr, sizeof(unixDestAddr));
}

// Receive data from a source
ssize_t socket::recvfrom(void* buffer, size_t size, int flags, struct ::sockaddr_in& srcAddr) {
    // Create a buffer for the source address
    struct sockaddr_un unixSrcAddr;
    socklen_t addrLen = sizeof(unixSrcAddr);
    
    // Receive the data
    ssize_t received = ::recvfrom(sockfd, buffer, size, flags, (struct sockaddr*)&unixSrcAddr, &addrLen);
    
    if (received >= 0) {
        // Convert the UNIX path back to an IP and port
        pathToIpPort(unixSrcAddr.sun_path, srcAddr);
    }
    
    return received;
}

// Convert IP and port to a UNIX socket path
std::string socket::ipPortToPath(const struct ::sockaddr_in& addr) {
    char ipStr[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &(addr.sin_addr), ipStr, INET_ADDRSTRLEN);
    
    int port = ntohs(addr.sin_port);
    
    std::stringstream ss;
    ss << "/tmp/sim_socket_" << ipStr << "_" << port;
    std::string path = ss.str();
    
    // Replace dots with underscores for valid path
    std::replace(path.begin(), path.end(), '.', '_');
    
    return path;
}

// Convert a UNIX socket path back to IP and port
void socket::pathToIpPort(const std::string& path, struct ::sockaddr_in& addr) {
    // Extract the IP and port from the path
    std::string pathCopy = path;
    
    // Extract the IP part
    size_t start = pathCopy.find("sim_socket_") + 11;
    size_t end = pathCopy.rfind('_');
    
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        // Invalid path format
        addr.sin_family = AF_INET;
        addr.sin_port = 0;
        addr.sin_addr.s_addr = 0;
        return;
    }
    
    std::string ipPart = pathCopy.substr(start, end - start);
    std::string portPart = pathCopy.substr(end + 1);
    
    // Replace underscores with dots in the IP
    std::replace(ipPart.begin(), ipPart.end(), '_', '.');
    
    // Set the address structure
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::stoi(portPart));
    ::inet_pton(AF_INET, ipPart.c_str(), &(addr.sin_addr));
}

// Get the IP address for the current thread
std::string socket::get_ipaddr() {
    // Get the thread ID as a string
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string threadId = ss.str();
    
    // Look up the IP address in the map
    auto it = ip_map.find(threadId);
    if (it != ip_map.end()) {
        return it->second;
    }
    
    // Default IP if not found
    return "127.0.0.1";
}

}  // namespace sim