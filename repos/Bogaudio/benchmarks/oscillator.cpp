
#include <benchmark/benchmark.h>

#include "dsp/oscillator.hpp"

using namespace bogaudio::dsp;

static void BM_Phasor(benchmark::State& state) {
  Phasor p(44100.0, 440.0);
  for (auto _ : state) {
    p.next();
  }
}
BENCHMARK(BM_Phasor);

static void BM_SineOscillator(benchmark::State& state) {
  SineOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SineOscillator);

static void BM_SawOscillator(benchmark::State& state) {
  SawOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SawOscillator);

static void BM_SquareOscillator(benchmark::State& state) {
  SquareOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SquareOscillator);

static void BM_TriangleOscillator(benchmark::State& state) {
  TriangleOscillator o(44100.0, 440.0);
  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_TriangleOscillator);

static void BM_SineBankOscillator(benchmark::State& state) {
  SineBankOscillator o(44100.0, 440.0, 100);
  for (int i = 1, n = o.partialCount(); i <= n; ++i) {
    o.setPartial(i, i, 1.0 / (float)i);
  }

  for (auto _ : state) {
    o.next();
  }
}
BENCHMARK(BM_SineBankOscillator);
