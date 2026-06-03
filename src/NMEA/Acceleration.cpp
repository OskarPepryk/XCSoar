// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "NMEA/Acceleration.hpp"

void
AccelerationState::Complement(const AccelerationState &add) noexcept
{
  if (add.available && (!available || (add.real && !real))) {
    real = add.real;
    g_load = add.g_load;
    available = add.available;
  }

  if (!raw_available && add.raw_available)
    ProvideRawAcceleration(add.raw_timestamp,
                           add.accel_x, add.accel_y, add.accel_z);
}
