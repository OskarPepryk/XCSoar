// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "AccelVarioComputer.hpp"
#include "Settings.hpp"
#include "NMEA/MoreData.hpp"

#include <cmath>

static constexpr float EARTH_G = 9.80665f;
static constexpr std::size_t FILTER_SAMPLES = 30;

void
AccelVarioComputer::Update(MoreData &data,
                           const FeaturesSettings &features) noexcept
{
  if (!features.accel_vario_enabled || !data.acceleration.raw_available)
    return;

  AccelSample &s = history[head];
  s.t = data.acceleration.raw_timestamp;
  s.x = data.acceleration.accel_x;
  s.y = data.acceleration.accel_y;
  s.z = data.acceleration.accel_z;

  head = (head + 1) % HISTORY_SIZE;
  if (count < HISTORY_SIZE)
    ++count;

  if (count < 2)
    return;

  // Moving average over the last FILTER_SAMPLES (or all available) samples.
  const std::size_t n = std::min(count, FILTER_SAMPLES);

  float sum = 0;
  for (std::size_t i = 0; i < n; ++i) {
    const std::size_t idx = (head + HISTORY_SIZE - 1 - i) % HISTORY_SIZE;
    const AccelSample &a = history[idx];
    sum += std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
  }
  const float avg_accel = sum / static_cast<float>(n);

  // Net vertical acceleration (positive = upward), ignoring g-loading in turns.
  const float net_accel = avg_accel - EARTH_G;

  // Time span covered by the window (oldest to newest sample).
  const std::size_t oldest_idx = (head + HISTORY_SIZE - n) % HISTORY_SIZE;
  const std::size_t newest_idx = (head + HISTORY_SIZE - 1) % HISTORY_SIZE;
  const float dt = static_cast<float>(
      (history[newest_idx].t - history[oldest_idx].t).count());

  if (dt <= 0)
    return;

  // Rough integral: avg net acceleration * time window ≈ Δv (m/s).
  const double accel_vario = static_cast<double>(net_accel * dt);

  // Blend into brutto_vario if a barometric source is present.
  if (data.brutto_vario_available)
    data.brutto_vario += accel_vario;
}
