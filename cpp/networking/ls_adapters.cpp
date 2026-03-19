#include <iostream>
#include <cstdlib>
#include <cstring>

#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>

int main() {
    
    // Populate the linked list of network interface structs
    ifaddrs* addresses = nullptr;
    if (getifaddrs(&addresses) == -1) {
        std::cout << "getifaddrs call failed\n";
        return -1;
    }

    // Loop through each interface
    ifaddrs* address = addresses;
    while (address != nullptr) {
        if (address->ifa_addr == nullptr) {
            address = address->ifa_next;
            continue;
        }

        int family = address->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) {
            std::cout << '\n' << address->ifa_name << '\t';
            std::cout << ((family == AF_INET) ? "IPv4" : "IPv6") << '\n';

            // Get the network address for each interface
            char addrStr[NI_MAXHOST];
            const int family_size = family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
            if (getnameinfo(address->ifa_addr, family_size, addrStr, sizeof(addrStr), 0, 0, NI_NUMERICHOST) != 0) {
                snprintf(addrStr, NI_MAXHOST, "N/A");
            }
            std::cout << "\tUnicast   : " << addrStr << "\n";

            // Get the netmask;
            if (address->ifa_netmask != nullptr) {
                if (getnameinfo(address->ifa_netmask, family_size, addrStr, sizeof(addrStr), 0, 0, NI_NUMERICHOST) != 0) {
                    snprintf(addrStr, NI_MAXHOST, "N/A");
                }
                std::cout << "\tNetmask   : " << addrStr << "\n";
            }


            // Get the broadcast / point-to-point destination address for iPv4 addresses
            if (family == AF_INET) { 
                sockaddr* bdAddr = nullptr;
                if (address->ifa_flags & IFF_BROADCAST) {
                    bdAddr = address->ifa_ifu.ifu_broadaddr;
                    std::cout << "\tBroadcast : ";
                }
                else if (address->ifa_flags & IFF_POINTOPOINT) {
                    bdAddr = address->ifa_ifu.ifu_dstaddr;
                    std::cout << "\tPoint-to-point destination : ";
                }

                if (bdAddr != nullptr) {
                    if (getnameinfo(bdAddr, sizeof(sockaddr_in), addrStr, sizeof(addrStr), 0, 0, NI_NUMERICHOST) != 0) {
                        std::cout << "getnameinfo failed";
                        snprintf(addrStr, NI_MAXHOST, "N/A");
                    }
                    std::cout << addrStr << '\n';
                }
                else {
                    std::cout << "\tNo broadcast or ptp address\n";
                }
            
            }
        }

        address = address->ifa_next;
    }
    std::cout << '\n';

    // free the allocate space
    freeifaddrs(addresses);

    return 0;
}
