# Session 11 Starter — Build Your Own Live Dashboard

This is guideline-based, not fill-in-the-blank: nothing here tells you the
exact line of code to write. It tells you what your finished dashboard must
be able to do, and leaves *how it looks* up to you.

**Everyone in the class is building the same functionality on the same
widget set — but every dashboard should look different.** Don't copy a
classmate's colors/layout/fonts. Pick your own theme (dark, light,
racing-style, minimalist, whatever) and your own arrangement of the
widgets. Two submissions with identical styling will be flagged.

---

## What you're building

A Qt5 window that reads live CAN traffic from `vcan0` (produced by the
Engine ECU, Transmission ECU, and BCM from Sessions 07–08 — this lab bundles
their solution source under `ecus/` so you don't need anything from those
sessions' own folders) on a background thread, and displays every value those
ECUs actually transmit. No hardcoded values anywhere in the finished version
— everything on screen should move because a CAN frame arrived.

---

## The widget contract

`src/MainWindow.h` already declares every widget you need, with the exact
member name and type you must use — **do not rename, remove, or add to
this list**. This is what makes your `CANWorker` wiring gradable even
though your styling isn't the same as anyone else's.

| Member                                                                      | Type                   | Fed by           | Signal                                                      |
| --------------------------------------------------------------------------- | ---------------------- | ---------------- | ----------------------------------------------------------- |
| `speedometer_`                                                            | `SpeedometerWidget*` | Transmission ECU | `speedUpdated(float)`                                     |
| `rpm_bar_`                                                                | `QProgressBar*`      | Engine ECU       | `rpmUpdated(int)`                                         |
| `gear_label_`                                                             | `QLabel*`            | Transmission ECU | `gearUpdated(int)`                                        |
| `temp_label_`                                                             | `QLabel*`            | Engine ECU       | `tempUpdated(float)`                                      |
| `door_fl_label_`/`door_fr_label_`/`door_rl_label_`/`door_rr_label_` | `QLabel*`            | BCM              | `doorStatusUpdated(bool,bool,bool,bool)`                  |
| `throttle_bar_`                                                           | `QProgressBar*`      | Engine ECU       | `throttleUpdated(float)`                                  |
| `engine_status_label_`                                                    | `QLabel*`            | Engine ECU       | `engineRunningChanged(bool)`                              |
| `ignition_label_`                                                         | `QLabel*`            | BCM              | `ignitionChanged(bool)`                                   |
| `turn_signal_label_`                                                      | `QLabel*`            | BCM              | `turnSignalsChanged(bool,bool)` + `hazardChanged(bool)` |
| `battery_label_`                                                          | `QLabel*`            | BCM              | `batteryVoltageUpdated(float)`                            |

The exact CAN frame layout (which byte, what scale factor) for each of
these is in this session's `content.md`, Section 4.1 — that table is your
decoding reference for `CANWorker::decodeAndEmit()`.

What's genuinely open:

- **Layout**: `QVBoxLayout`/`QHBoxLayout`/`QGridLayout`/nested layouts,
  group widgets under `QGroupBox`es, arrange in columns — your call.
- **Styling**: a `setStyleSheet()` theme, per-widget fonts/colors,
  `QProgressBar` chunk colors, window size — your call.
- **Initial/placeholder text**: whatever you want each label to show
  before the first CAN frame arrives (it'll be overwritten within a
  second of `./dashboard` starting).
- **Extra decoration**: icons, a window background image, a custom-painted
  widget beyond `SpeedometerWidget` — all fine, as long as the required
  contract widgets are still present and correctly wired.

What's not open: the member names/types above, and which CAN signal drives
which member.

---

## Two things to implement

### 1. `CANWorker` — decode the CAN frames

`src/CANWorker.cpp` has `TODO`s in `openSocket()` and `decodeAndEmit()`.
This is identical in shape to every ECU you've written since Session 07:
`socket()` → `ioctl(SIOCGIFINDEX)` → `bind()` for the socket, and
scale/offset/bitfield decoding for the frame fields — see `content.md`
Section 4.1 for the full byte layout of `0x0C0`/`0x0D0`/`0x320`.

### 2. `MainWindow` — build the UI and wire it to live data

`src/MainWindow.cpp` constructs the widgets from the contract above with
neutral placeholders so the project compiles out of the box. Replace the
construction/layout/styling with your own design, then:

1. Add a `QThread*` and a `CANWorker*` member to `MainWindow.h`.
2. In the constructor, construct `can_worker_` with **no parent**, then
   `can_worker_->moveToThread(can_thread_)` — the worker must have no
   parent from another thread at the moment it's moved.
3. `connect(can_thread_, &QThread::started, can_worker_, &CANWorker::run);`
4. `connect()` every `CANWorker` signal to its widget. Signals with no
   extra logic (`rpmUpdated` → `rpm_bar_->setValue`,
   `speedUpdated` → `speedometer_->setSpeed`) can connect directly.
   Signals that need formatting or combining (`tempUpdated`, `gearUpdated`,
   `doorStatusUpdated`, and the two that share `turn_signal_label_`) need a
   lambda in between.
5. `can_thread_->start();`
6. Implement `~MainWindow()` to shut the thread down: call
   `can_worker_->stop()` directly (not through `connect()`), then
   `can_thread_->quit()` and `can_thread_->wait()`. `content.md` Section 5
   explains why this exact order matters and why `running_` in
   `CANWorker` must be `std::atomic<bool>`, not plain `bool`.

Because `can_worker_` runs on a different thread than `MainWindow`, every
one of these `connect()` calls is automatically delivered as a
`Qt::QueuedConnection` — you never write the connection type explicitly.

---

## Before you run it

Start the ECU network in separate terminals — nothing will move on the
dashboard without live CAN traffic:

```bash
./engine_ecu --demo    # Terminal 1 (Session 07)
./transmission_ecu     # Terminal 2 (Session 08)
./bcm                   # Terminal 3 (Session 08)
candump vcan0             # Terminal 4 — sanity check that frames are flowing
```

**Use `--demo` on the Engine ECU here.** Without it, `rpm_` ramps once
toward a fixed target and then holds — normal and correct for Session 07's
own lab exercise, but it means that if you start `engine_ecu` before the
dashboard (as above), RPM and vehicle speed (which Transmission ECU derives
directly from Engine ECU's RPM) may have already gone flat by the time your
`CANWorker` connects, and you'd wrongly suspect a wiring bug. `--demo` makes
the Engine ECU sweep RPM through a repeating idle↔redline cycle forever, so
your speedometer needle and RPM bar keep moving no matter when you start
the dashboard relative to the ECUs. See the Bonus section at the bottom of
this file for a way to automate this whole startup sequence instead of
opening four terminals by hand.

```bash
mkdir build && cd build
cmake ..
make                    # builds engine_ecu, transmission_ecu, bcm, and dashboard together
./dashboard
```

---

## Self-check before you submit

- [ ] Every widget in the contract table is visible on screen and updates
  live — no widget still shows a hardcoded/static value once the ECUs
  are running.
- [ ] Speedometer needle moves with vehicle speed, RPM bar with engine RPM.
- [ ] Door labels flip to "OPEN" when the BCM's scripted door-open window
  runs, and back to "closed" afterward.
- [ ] Turn/hazard label correctly prioritizes hazard over a single turn
  signal (both signals feed one label).
- [ ] The window stays responsive the whole time — no freezing.
- [ ] Closing the window exits cleanly, no hang, no crash.
- [ ] Your styling is visibly your own — not the same color scheme/layout
  as a classmate's dashboard.

---

## Bonus: Automating the Demo Startup

Manually opening four terminals every time you want to see the dashboard
run gets old fast, and it has a real pitfall: the Engine ECU's `rpm_` ramps
toward a fixed `target_rpm_` and then holds (Session 07's intended default
behavior). Start it well before the dashboard — which "Before you run it"
above invites you to do — and RPM/Transmission-derived speed may already be
static by the time `CANWorker` connects. That looks exactly like a wiring
bug even though nothing is wrong.

This starter includes two things that fix both problems at once, entirely
optional to use:

1. **`engine_ecu --demo`** (`ecus/engine_ecu/`) — an opt-in flag, off by
   default. With it, instead of settling on a fixed `target_rpm_`, the
   Engine ECU continuously sweeps its target through a repeating
   idle↔redline cycle (`DEMO_IDLE_RPM`/`DEMO_PEAK_RPM`/`DEMO_CYCLE_MS` in
   `engine_ecu.h`), so `rpm_` — and Transmission ECU's `vehicle_speed_`,
   which is derived directly from the RPM frames it receives — never goes
   static, no matter when the dashboard connects. Without `--demo`,
   behavior is byte-for-byte identical to Session 07's own lab exercise.
2. **`cmake --build . --target run_demo`** (from your `build/` directory,
   after the normal `cmake ..`/`make` once) — builds all four binaries if
   needed, starts Engine ECU (`--demo`), Transmission ECU, and BCM in the
   background, then launches `./dashboard` in the foreground. Closing the
   dashboard (or Ctrl+C) automatically stops the three background ECUs.
   Under the hood this just runs `run_demo.sh` with the four built binary
   paths — see that file, and the `add_custom_target(run_demo ...)` block
   at the bottom of `CMakeLists.txt`, if you're curious how it's wired.

Neither of these is required for the exercise or graded on — they exist
purely so you don't have to juggle four terminals every time you want to
see your dashboard live.
