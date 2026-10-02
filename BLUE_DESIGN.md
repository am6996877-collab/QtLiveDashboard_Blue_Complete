# Blue instrument-cluster design

This complete project merges the blue dashboard into the supplied
QtLiveDashboard.zip. It is based on Session 11, not Session 14.

The original CANWorker, all three ECU folders, and run_demo.sh are preserved.
The UI is adapted to this worker's one-argument faultStatusUpdated(bool) signal.
There is no UDS tester, fault manager or detailed DTC service in the supplied
base project. The check-engine icon can display the existing 0x0C1 bit-0 flag;
the bundled Engine ECU does not generate that optional frame.

New appearance: charcoal background, blue-ring speed/RPM gauges, central gear,
coolant and battery readings, check-engine icon, lower RPM/throttle bars and
engine/ignition/door indicators. Actual CAN frames drive all telemetry.
Fuel is marked unavailable because it is not transmitted.

The original Session 11 coolant conversion remains byte 4 * 0.5 - 40.
Do not use Session 14's different coolant conversion with these ECUs.

## Checks performed

- Verified the CAN worker, ECU files and launcher are byte-for-byte unchanged
  from the supplied ZIP.
- Verified required CMake source files and local headers exist.
- Verified all CANWorker signals have UI connections and the fault connection
  matches this project's single-bool signature.
- Gauge mathematics was compiled and checked separately in C++17.

Full Qt5/Linux compilation, visual rendering and live CAN testing remain to be
done on Linux. They were not performed on this Windows host.

## Manual display check

With vcan0 up, run only ./build/dashboard, then send:

```bash
cansend vcan0 0C0#401F0000FA320100
cansend vcan0 0D0#0370170100000000
cansend vcan0 320#00018C0000000000
```

Expected: 2000 RPM, 60 km/h, gear D, 85 C, 20% throttle, running engine,
ignition ON, 14 V and all doors closed.

```bash
cansend vcan0 320#2F078C0000000000
cansend vcan0 0C1#01
```

Expected: all doors open, hazard taking priority over turn signals, and amber
check-engine icon. Send 0C1#00 to clear the icon. This is a display test only;
the project does not implement diagnostic fault clearing or a UDS service.
