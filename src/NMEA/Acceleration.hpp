// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "time/Stamp.hpp"

/**
 * State of acceleration of aircraft
 */
struct AccelerationState
{
  /**
   * Is G-load information available?
   * @see Gload
   */
  bool available;

  /**
   * Is the G-load information coming from a connected device (true) or
   * was it calculated by XCSoar (false)
   */
  bool real;

  /**
   * G-Load information of external device (if available)
   * or estimated (assuming balanced turn)
   * @see AccelerationAvailable
   */
  double g_load;

  /**
   * Is the raw 3-axis accelerometer vector available?
   * Set by the internal smartphone sensor via ProvideRawAcceleration().
   */
  bool raw_available;

  /** Timestamp of the raw accelerometer sample (monotonic clock). */
  TimeStamp raw_timestamp;

  /** Raw accelerometer vector in device frame [m/s²]. */
  float accel_x, accel_y, accel_z;

  void Reset() noexcept {
    available = false;
    raw_available = false;
  }

  void ProvideGLoad(double _g_load, bool _real=true) noexcept {
    g_load = _g_load;
    real = _real;
    available = true;
  }

  void ProvideRawAcceleration(TimeStamp t,
                              float x, float y, float z) noexcept {
    raw_timestamp = t;
    accel_x = x;
    accel_y = y;
    accel_z = z;
    raw_available = true;
  }

  /**
   * Adds data from the specified object, unless already present in
   * this one.
   */
  void Complement(const AccelerationState &add) noexcept;
};
