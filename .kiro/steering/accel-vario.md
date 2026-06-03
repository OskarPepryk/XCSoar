---
inclusion: fileMatch
fileMatchPattern: ['src/Computer/AccelVarioComputer.*', 'src/NMEA/Acceleration.*', 'android/src/NonGPSSensors.java']
---

# Accelerometer Vario Aid — Feature Notes

## Status
Infrastructure complete, FIR algorithm not yet implemented.
The `TODO` marker in `src/Computer/AccelVarioComputer.cpp::Update()` is the next entry point.

## Data Flow
```
Android hardware
  → NonGPSSensors.java (SENSOR_DELAY_GAME ≈50 Hz)
  → NativeSensorListener JNI
  → DeviceDescriptor::OnAccelerationSensor(float ddx, float ddy, float ddz)
  → NMEAInfo::acceleration.ProvideRawAcceleration(clock, x, y, z)
  → DeviceBlackboard::ScheduleMerge() → MergeThread wakes
  → BasicComputer::Compute() → AccelVarioComputer::Update()
  → ring buffer (64 samples, head/count)
```

## Timing Facts
- `BasicComputer::Compute()` is **event-driven**, not timer-driven. It fires once per incoming sensor event.
- `MergeThread` has `period_min=50ms` → hard ceiling of ~20 Hz regardless of sensor rate.
- Android `SENSOR_DELAY_GAME` nominal hint is 20ms (50 Hz), but actual rate is hardware-dependent and **irregular**. Do not assume fixed Δt.
- Each `AccelSample.t` carries the `basic.clock` timestamp at the moment the sample arrived — use `t` differences for Δt in the FIR, not a fixed constant.

## Key Structs
- `AccelerationState` (`src/NMEA/Acceleration.hpp`): holds both the legacy scalar `g_load` and the new raw 3-axis vector. Must remain trivially constructible (`NMEAInfo` is `static_assert`-ed).
- `AccelSample` (`src/Computer/AccelVarioComputer.hpp`): `{TimeStamp t; float x, y, z;}` — one entry in the ring buffer.
- `AccelVarioComputer::history[]`: 64-element ring, `head` points to next-write slot, `count` tracks how many valid entries exist.

## Setting
- Key: `AccelVarioEnabled` (profile) / `features.accel_vario_enabled` (runtime)
- Default: `false` (opt-in)
- UI: Settings → Glide Computer → expert rows → "Accel. vario aid" (Android only, `#ifdef ANDROID`)
- The accelerometer is **always subscribed** on Android regardless of this setting (unconditional in `Device/Factory.cpp`). The setting gates only data storage and processing.

## FIR Algorithm Entry Point
`AccelVarioComputer::Update()` in `src/Computer/AccelVarioComputer.cpp`:
- `history[]` contains up to 64 recent samples in chronological order: index `(head - count + HISTORY_SIZE) % HISTORY_SIZE` is the oldest, `(head - 1 + HISTORY_SIZE) % HISTORY_SIZE` is newest.
- Output should influence `data.brutto_vario` or a new dedicated field in `MoreData` (to be decided). Prefer a new field initially so the existing vario path is not disturbed.
- Coordinate frame: raw values are in **device frame** (no rotation applied). Gravity removal and frame transform are needed before using Z-axis as vertical acceleration.
- The g-load scalar in `AccelerationState::g_load` (populated by the separate `OnAccelerationSensor(double)` callback) is already normalised to g units — useful for a sanity check but not a substitute for the 3-axis data.

## Non-Android Platforms
`OnAccelerationSensor(float, float, float)` is declared in `SensorListener` only under `#ifdef ANDROID`. Other platforms never call it; `raw_available` stays `false`. The algorithm must handle `raw_available == false` gracefully (fall back to existing vario).
