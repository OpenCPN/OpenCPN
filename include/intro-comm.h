/**
\page  comm Communications Overview

                  ┌─────────────────────────────────────────────────────────┐
    Plugins       │      Plugins using decoded and raw data messaging       │
                  └─────────────────────────────────────────────────────────┘           
                  
                  ┌─────────────────────────────────────────────────────────┐
                  │                    Plugin message API                   │
                  └───────────────────────────────────────────────┐         │
                                                                  │         │
                  ┌───────────────────────────────────────────┐   │         │
    Application   │     Application messages (decoded data)   │   │         │
                  ├╶╶╶╶╶╶-╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶┤   │         │
                  │      Message source priorities            │   │         │
                  │           Protocol handlers               │   │         │
                  │                                           │   │         │
                  │  nmea0183    nmea2000    signalK   ...    │   │         │
                  └───────────────────────────────────────────┘   └─────────┘
    
                  ┌─────────────────────────────────────────────────────────┐
    Transport     │             Navigation messages (raw data)              │
                  └─────────────────────────────────────────────────────────┘
    
                  ┌─────────────────────────────────────────────────────────┐
                  │                   Driver registry                       │
    Drivers       ├╶╶╶╶╶╶╶-┬╶╶╶╶╶╶╶╶╶╶-┬╶╶╶╶╶╶╶┬╶╶╶╶╶╶╶╶╶╶┬╶╶╶╶╶╶╶╶╶┬╶╶╶╶╶╶╶┤
                  │ 0183   │ Actisense │ Linux │ iKonvert │ SignalK │       │
                  │        │ NGT-1     │ CAN   │          │ +ipv4   │ ...   │
                  │        │           │       │          │ TCP/IP  │       │
                  └────────┴───────────┴───────┴──────────┴─────────┴───────┘
    

All internal communications described here is based on the \ref observable
library which is available in *libs*. This library implements a basic
observable pattern allowing parties to listen a. k. a. subscribe to
incoming messages.

All new-style drivers are named like *comm-drv-\**.
These handles the low-level communication details.
Incoming messages from the outside are made available on the navigation
messages bus NavMsgBus.
Both core parts and plugins listens to this bus, acting on messages when
they arrive.

These messages are of type NavMsg defined in *comm_navmsg.h*.
This is a unified, common type for all sorts of messages including NMEA2000,
NMEA0183 and SignalK.

In the core, *CommDecoder* acts as a dispatcher which listens to the 
message bus and invokes various parts when messages arrives. 
*CommDecoder* also handles priorities when receiving the same data from 
multiple sources, selecting the data source to use.

For plugins, a simplified API is available, see \ref plugincomms.

When sending messages, upper layers needs to know which driver to use.
To that end, drivers registers themselves in the CommDriverRegistry which
maintains an actual list of available drivers.
**/
