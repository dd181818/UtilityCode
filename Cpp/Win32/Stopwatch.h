//
// Stopwatch
//

#pragma once

#include <functional>
#include <stdint.h>
#include <string>

#include <Windows.h>

namespace Utility {

#if 0
// Use like this:
void StopwatchTest()
{
	Utility::Stopwatch sw;

	sw.ResetAndStart("Test1");
	Sleep(1000);
	sw.StopAndPrint();

	sw.ResetAndStart("Test2");
	Sleep(1000);
	sw.StopAndPrint();

	sw.Reset("Average time");
	for (int i = 0; i < 10; i++)
	{
		sw.Start();
		Sleep(10);
		sw.Stop();
		Sleep(50);
	}
	sw.PrintAverage();

	return 0;
}
#endif

class Stopwatch
{
public:
	static inline uint64_t ReadPerformanceCounter()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return li.QuadPart;
	}

	static inline uint64_t ReadPerformanceFrequency()
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		return li.QuadPart;
	}

	static double TicksToSeconds(uint64_t ticks)
	{
		uint64_t freq = ReadPerformanceFrequency();
		return static_cast<double>(ticks) / freq;
	}

public:
	void Start()
	{
		m_AvgCounter++;
		m_Start = ReadPerformanceCounter();
	}

	void Start(const std::string& context)
	{
		m_AvgCounter++;
		m_Context = context;
		m_Start = ReadPerformanceCounter();
	}

	void Stop()
	{
		m_End = ReadPerformanceCounter();
		m_Delta += m_End - m_Start;
	}

	void StopAndPrint()
	{
		Stop();
		Print();
	}

	void Reset(const std::string& context = "")
	{
		m_Delta = 0;
		m_Start = 0;
		m_End = 0;
		m_Context = context;
		m_AvgCounter = 0;
	}

	void ResetAndStart()
	{
		Reset();
		Start();
	}

	void ResetAndStart(const std::string& context)
	{
		Reset();
		Start(context);
	}

	uint64_t GetElapsedTicks() const
	{
		return m_Delta;
	}

	double GetElapsedSeconds() const
	{
		return TicksToSeconds(m_Delta);
	}

	std::string ToString() const
	{
		return ToString(m_Delta);
	}

	void Print()
	{
		Print(m_Delta);
	}

	void PrintAverage()
	{
		uint64_t avgTicks = 0;
		if (m_AvgCounter)
		{
			avgTicks = static_cast<uint64_t>(std::ceil(
				static_cast<double>(m_Delta) / static_cast<double>(m_AvgCounter)));
		}
		Print(avgTicks);
	}

private:
	std::string ToString(uint64_t delta) const
	{
		return "Ticks: " + std::to_string(delta)
			+ " (" + std::to_string(TicksToSeconds(delta)) + " seconds)";
	}

	void Print(uint64_t ticks) const
	{
		printf("[Stopwatch]          %-30s: ", m_Context.c_str());
		printf("%s\n", ToString(ticks).c_str());
	}

private:
	uint64_t m_Start = 0;
	uint64_t m_End = 0;
	uint64_t m_Delta = 0;
	uint64_t m_AvgCounter = 0;
	std::string m_Context;
};

}
