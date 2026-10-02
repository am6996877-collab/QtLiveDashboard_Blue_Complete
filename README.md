# Blue Horizon - Complete Qt5 CAN Dashboard

This is the complete project, already merged with the blue-on-dark design.
Includes the dashboard, Engine ECU, Transmission ECU, BCM, and demo launcher.
No manual source-file merging is needed. Extract the ZIP into a new folder.

## Run on Ubuntu/Linux

Open a terminal in the extracted QtLiveDashboard_Blue folder:

```bash
sudo apt update
sudo apt install build-essential cmake qtbase5-dev libqt5svg5-dev can-utils
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
cmake -S . -B build
cmake --build build -j2
cmake --build build --target run_demo
```

If vcan0 already exists, skip the add command. Run from a Linux desktop session.
Closing the dashboard stops the ECU processes started by the launcher.
In another terminal, `candump vcan0` displays the underlying CAN traffic.

## What you will see

- Blue speedometer on the left and RPM gauge on the right.
- Central gear, coolant, battery and check-engine information.
- Bottom RPM/throttle bars and engine, ignition and door status.
- RPM, speed and throttle move with the Engine ECU's repeating demo cycle.
- Front-left door opens around seconds 5-7 of the BCM's 12-second cycle.
- Right turn activates around seconds 9-12.
- Gear D, ignition ON and battery 14 V stay constant in the bundled scenario.
- Fuel displays unavailable because the supplied ECU network has no fuel signal.
- Fault status remains unreceived unless a 0x0C1 frame is sent.

## Important project version

The uploaded base ZIP is Session 11. Its CAN worker and three ECUs are preserved,
including the transmission's filter and queue-draining fix. This is not a
Session 14 fault/UDS project: those additional backend sources were not in the
ZIP. The design is adapted to the base project's actual signal definitions.

See BLUE_DESIGN.md for changed UI behavior, manual test frames and verification
limits. The original course brief remains in STARTER_README.md.

## Testing status

Gauge math was tested in C++17. File-integrity, source/header completeness and
signal-connection checks passed. Full Qt compilation, GUI inspection and live
SocketCAN tests still require Linux with Qt5; they were not run here.
