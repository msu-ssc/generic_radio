# Generic Radio - NOS3 Component

## IRIS transmit-power extension (simulator)

The IRIS simulator stores transmit power as an unsigned integer in milliwatts,
from 0 through 100 inclusive. It starts at 0 and resets to 0 on simulator
DISABLE. For now this is a stored setting only: it does not alter simulated
signal strength, forwarding, or link availability (including at 0 mW).

All device commands remain 9 bytes: `DE AD`, one command byte, four payload
bytes in big-endian order, then `BE EF`.

| Device command | ID | Payload | Response |
| --- | --- | --- | --- |
| Set transmit power | `02` | Power in mW | No immediate reply; read extended HK to confirm |
| Request extended housekeeping | `03` | Four zero bytes (ignored) | 20-byte reply below |

For example, set 50 mW with `DE AD 02 00 00 00 32 BE EF`, then request
extended HK with `DE AD 03 00 00 00 00 BE EF`. Values above 100 are rejected
with a simulator warning; power and the command counter are unchanged.
Accepted power commands increment the existing counter. Disabled simulators
ignore these commands, as they do the original commands.

Extended housekeeping uses this layout (offsets are zero-based):

| Bytes | Field |
| --- | --- |
| 0–1 | Header `DE AD` |
| 2–5 | Command counter, unsigned big-endian 32-bit |
| 6–9 | Existing configuration, unsigned big-endian 32-bit |
| 10–13 | Existing proximity signal, unsigned big-endian 32-bit |
| 14–17 | Transmit power in mW, unsigned big-endian 32-bit |
| 18–19 | Trailer `BE EF` |

Command `00` still returns the original 16-byte housekeeping packet, so the
current flight app continues to work. Flight-software and COSMOS support for
commands `02` and `03` will be added in the next development steps. These are
device-protocol IDs, not cFS ground-command function codes.

Run the standalone power protocol checks from this component's root:

```sh
g++ -std=c++11 -Wall -Wextra -Werror -Isim/inc sim/tests/iris_radio_power_test.cpp -o /tmp/iris_radio_power_test
/tmp/iris_radio_power_test
```

This repository contains the NOS3 Generic Radio Component.
This includes flight software (FSW), ground software (GSW), simulation, and support directories.

## Overview
This generic radio component is a socket based device that accepts multiple commands, including requests for telemetry and data.
The available FSW is for use in the core Flight System (cFS) while the GSW supports COSMOS.
A NOS3 simulation is available which includes a generic_radio data provider.


# Device Communications
The protocol, commands, and responses of the component are captured below.
This section is essentially the device Interface Control Definition (ICD).

## Protocol
The protocol in use is UDP sockets.
The radio is maintains various sockets for use:
* Communication Interfaces
  - FSW to Radio Primary
    * UDP 5010
  - Radio Primary to FSW
    * UDP 5011
  - FSW to Radio Proximity
    * UDP 7010
  - Radio Proximity to FSW
    * UDP 7011
  - GSW to Radio
    * UDP 6010
  - Radio to GSW
    * UDP 6011
* Radio Component Interface
  - FSW to Radio
    * UDP 5014
  - Radio to FSW
    * UDP 5015

The radio will immediately attempt to forward data send on the communication interface with no storage capability.

## Commands
Should commmands involve a reply, the device immediately sends the reply.
Device commands are all formatted in the same manner and are fixed in size:
* uint16, 0xDEAD
* uint8, command identifier
  - (0) Get Housekeeping
  - (1) Set Configuration
* uint32, command payload
  - Unused for all but set configuration command
* uint16, 0xBEEF

## Response
Response formats are as follows:
* Housekeeping
  - uint16, 0xDEAD
  - uint32, Command Counter
    * Increments for each command received
  - uint32, Configuration
    * Internal configuration settings used by the device
  - uint32, Proximity Signal Strength
    * Self reported status of the component where zero is no signal
  - uint16, 0xBEEF


# Radio Flight Software
The FSW enables an abstraction of the device communicatications captured above.
For the radio the FSW accepts the follopwing commands:
* GENERIC_RADIO_CMD_MID
  - (0) NOOP
  - (1) Reset Counters
  - (2) Set Configuration
  - (3) Proximity Forward
* GENERIC_RADIO_REQ_HK_MID
  - (0) Request Housekeeping


# Configuration
The various configuration parameters available for each portion of the component are captured below.

## FSW
Refer to the file [fsw/platform_inc/generic_radio_platform_cfg.h](fsw/platform_inc/generic_radio_platform_cfg.h) for the default
configuration settings, as well as a summary on overriding parameters in mission-specific repositories.

## Simulation
The default configuration returns data initialized by the values in the simulation configuration settings used in the NOS3 simulator configuration file.
The radio configuration options for this are captured in [./sim/cfg/nos3-radio-simulator.xml](./sim/cfg/nos3-radio-simulator.xml) for ease of use.

# Documentation
If this generic_radio application had an ICD and/or test procedure, they would be linked here.

## Releases
We use [SemVer](http://semver.org/) for versioning. For the versions available, see the tags on this repository.
* v1.0.0
  - Initial release
