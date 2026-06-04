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

  // Moving average magnitude over the last FILTER_SAMPLES samples.
  const std::size_t n = std::min(count, FILTER_SAMPLES);
  float sum = 0;
  for (std::size_t i = 0; i < n; ++i) {
    const std::size_t idx = (head + HISTORY_SIZE - 1 - i) % HISTORY_SIZE;
    const AccelSample &a = history[idx];
    sum += std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
  }
  const float avg_accel = sum / static_cast<float>(n);

  // Exponential LPF on g_load (α=0.05 ≈ 1s time constant at 20 Hz).
  const float raw_g = data.acceleration.available
    ? static_cast<float>(data.acceleration.g_load)
    : 1.0f;
  smoothed_g_load += 0.05f * (raw_g - smoothed_g_load);
  const float net_accel = avg_accel - smoothed_g_load * EARTH_G;

  // Instantaneous dt between the two most recent samples.
  const std::size_t prev_idx = (head + HISTORY_SIZE - 2) % HISTORY_SIZE;
  const std::size_t last_idx = (head + HISTORY_SIZE - 1) % HISTORY_SIZE;
  const float dt = static_cast<float>(
      (history[last_idx].t - history[prev_idx].t).count());

  if (dt <= 0 || dt > 0.5f)  // sanity: ignore stale/missing samples
    return;

  // Δv = net_accel * dt — instantaneous vertical velocity contribution.
  const double accel_vario = static_cast<double>(net_accel * dt);

  if (data.brutto_vario_available)
    data.brutto_vario += accel_vario;
}
