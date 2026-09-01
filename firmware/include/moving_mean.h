#pragma once

#include <stddef.h>

// Fixed-size moving mean backed by a ring buffer. It does not allocate memory
// dynamically, which keeps its memory use predictable on the ESP32.
template <typename T, size_t WindowSize>
class MovingMean {
 public:
  static_assert(WindowSize > 0, "MovingMean window size must be greater than zero");

  MovingMean() : sum_(0), nextIndex_(0), sampleCount_(0) {}

  T add(T sample) {
    if (sampleCount_ < WindowSize) {
      samples_[nextIndex_] = sample;
      sum_ += sample;
      ++sampleCount_;
    } else {
      sum_ -= samples_[nextIndex_];
      samples_[nextIndex_] = sample;
      sum_ += sample;
    }

    nextIndex_ = (nextIndex_ + 1) % WindowSize;
    return value();
  }

  T value() const {
    if (sampleCount_ == 0) return T(0);
    return sum_ / static_cast<T>(sampleCount_);
  }

  size_t sampleCount() const {
    return sampleCount_;
  }

  void reset() {
    sum_ = T(0);
    nextIndex_ = 0;
    sampleCount_ = 0;
  }

 private:
  T samples_[WindowSize];
  T sum_;
  size_t nextIndex_;
  size_t sampleCount_;
};
