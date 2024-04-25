#pragma once

#include <string>

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
	std::string CalculateBroadcastAddress(const std::string& ip, const std::string& netmask);
	std::string GetBroadcastAddress();
};


