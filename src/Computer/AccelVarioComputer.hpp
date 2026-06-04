// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "time/Stamp.hpp"

#include <array>
#include <cstddef>

struct MoreData;
struct FeaturesSettings;

/** A single timestamped raw accelerometer sample. */
struct AccelSample {
  TimeStamp t;
  float x, y, z;
};

/**
 * Collects raw accelerometer samples from the smartphone internal sensor
 * for variometer aiding.  Maintains a ring buffer of recent samples ready
 * for a future FIR filter implementation.
 */
class AccelVarioComputer {
  static constexpr std::size_t HISTORY_SIZE = 64;

  std::array<AccelSample, HISTORY_SIZE> history;
  std::size_t head = 0;
  std::size_t count = 0;
  float smoothed_g_load = 1.0f;

public:
  void Reset() noexcept {
    head = 0;
    count = 0;
    smoothed_g_load = 1.0f;
  }

  /**
   * Appends a new sample to the ring buffer, computes a moving-average
   * acceleration integral, and blends the result into data.brutto_vario.
   * Call once per BasicComputer::Compute() invocation.
   */
  void Update(MoreData &data,
              const FeaturesSettings &features) noexcept;

  [[gnu::pure]]
  std::size_t Count() const noexcept { return count; }
};
