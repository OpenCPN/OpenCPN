#pragma once

#include <string>

/**
 * Represents network adapter information and provides related functionality.
 * Encapsulates IP address, netmask, and gateway, with methods to calculate and retrieve the broadcast address.
 */
class AdapterInfo {
  std::string IPAddress;
  std::string NetMask;
  std::string GateWay;
  void QueryAdapterInfo();
  unsigned long ipToLong(const std::string& ip);
  std::string longToIp(unsigned long ip);

public:
  AdapterInfo();
  std::string GetIPAddress();
  std::string GetNetMask();
  std::string GetGateWay();
  std::string CalculateBroadcastAddress(const std::string& ip,
                                        const std::string& netmask);
  std::string GetBroadcastAddress();
};
