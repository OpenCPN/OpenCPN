#include "AdapterInfo.h"
#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

  AdapterInfo::AdapterInfo() {
    QueryAdapterInfo();
    }

  void AdapterInfo::QueryAdapterInfo() {
        PIP_ADAPTER_INFO pAdapterInfo;
        PIP_ADAPTER_INFO pAdapter = NULL;
        DWORD dwRetVal = 0;

        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
        if (pAdapterInfo == NULL) {
            std::cerr << "Error allocating memory needed to call GetAdaptersinfo\n";
            return;
        }

        // Make an initial call to GetAdaptersInfo to get the necessary size into ulOutBufLen
        if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
            if (pAdapterInfo == NULL) {
                std::cerr << "Error allocating memory needed to call GetAdaptersinfo\n";
                return;
            }
        }

        if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
            pAdapter = pAdapterInfo;
            while (pAdapter) {

                if (pAdapter->GatewayList.IpAddress.String[0] != '0') { // Check if gateway is defined
                    IPAddress = pAdapter->IpAddressList.IpAddress.String;
                    NetMask = pAdapter->IpAddressList.IpMask.String;
                    GateWay = pAdapter->GatewayList.IpAddress.String;
                    break;
                }

                pAdapter = pAdapter->Next;
            }
        }
        else {
            std::cerr << "GetAdaptersInfo failed with error: " << dwRetVal << "\n";
        }

        if (pAdapterInfo) {
            free(pAdapterInfo);
        }

   }

 std::string AdapterInfo::longToIp(unsigned long ip) {
      return std::to_string((ip >> 24) & 0xFF) + "." +
          std::to_string((ip >> 16) & 0xFF) + "." +
          std::to_string((ip >> 8) & 0xFF) + "." +
          std::to_string(ip & 0xFF);
  }

 unsigned long AdapterInfo::ipToLong(const std::string& ip) {
     unsigned long ipBytes[4] = { 0 };
     int count = sscanf_s(ip.c_str(), "%lu.%lu.%lu.%lu", &ipBytes[3], &ipBytes[2], &ipBytes[1], &ipBytes[0]);
     if (count != 4) {         
         std::cerr << "Error parsing IP address." << std::endl;
         return 0; 
     }
     return (ipBytes[3] << 24) | (ipBytes[2] << 16) | (ipBytes[1] << 8) | ipBytes[0];
 }

std::string AdapterInfo::CalculateBroadcastAddress(const std::string& ip, const std::string& netmask) {
     unsigned long ipLong = ipToLong(ip);
     unsigned long netmaskLong = ipToLong(netmask);
     unsigned long wildcardMask = ~netmaskLong;
     unsigned long broadcastAddress = ipLong | wildcardMask;
     return longToIp(broadcastAddress);
 }

std::string AdapterInfo::GetBroadcastAddress() {
    return CalculateBroadcastAddress(IPAddress, NetMask);

  }

  std::string AdapterInfo::GetIPAddress() {
      return IPAddress;
  }

  std::string AdapterInfo::GetNetMask() {
      return NetMask;
  }

  std::string AdapterInfo::GetGateWay() {
      return GateWay;
  }
