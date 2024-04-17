import netifaces
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(("8.8.8.8", 80))
local_ip =s.getsockname()[0]
s.close()
interfaces =[]
if local_ip == None:
    print("Connect with internet")
    exit(1)

loopback_intf = None
local_intf = None

for i in netifaces.interfaces():
    if netifaces.AF_INET in netifaces.ifaddresses(i):
        interfaces.append(i)

for i in interfaces:
    if netifaces.ifaddresses(i)[netifaces.AF_INET][0]['addr'] == '127.0.0.1':
        loopback_intf = i
    if(netifaces.ifaddresses(i)[netifaces.AF_INET][0]['addr'] == local_ip ):
        local_intf =i
if loopback_intf ==None or local_intf == None:
    print("error in finding network interfaces")
    exit(1)
else:
    mac_addr = netifaces.ifaddresses(local_intf)[netifaces.AF_LINK][0]['addr']