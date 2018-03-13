#if!defined CONVERSIONS_HPP
#define CONVERSIONS_HPP
#include <chrono>

inline std::chrono::duration<float> nanosecondsToSeconds(std::chrono::nanoseconds const time)
{
	return std::chrono::duration_cast<std::chrono::duration<float>>(time);
}

inline std::chrono::nanoseconds secondsToNanoseconds(std::chrono::duration<float> const time)
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(time);
}

inline float nanosecondsToFrequency(std::chrono::nanoseconds const time)
{
	auto const timeInSeconds = nanosecondsToSeconds(time);

	return 1.f / timeInSeconds.count();
}

inline unsigned int nanosecondToBpm(std::chrono::nanoseconds const time)
{
	auto const sixty = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds{60});

	return (sixty / time);
}

inline unsigned int frequencyToBpm(float const frequency)
{
	auto const cycleDurationInSeconds = 1.f / frequency;

	return static_cast<unsigned int>(60.f / cycleDurationInSeconds);
}

inline float bpmToFrequency(unsigned int bpm)
{
	return 60.f / static_cast<float>(bpm);
}

inline std::chrono::nanoseconds bpmToNanoseconds(unsigned int bpm)
{
	auto const sixty = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds{60});

	return sixty / bpm;
}

#endif
