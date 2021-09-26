#ifndef TIMING_H
#define TIMING_H

#include <mutex>
#include <chrono>
#include <cstdint>
#include <stack>
#include <thread>

// TODO: move following to cpp file
using namespace std::chrono_literals;

class Timing
{
public:
	Timing() { start(); }

	void start()
	{
		while (!callstack.empty())
			callstack.pop();
		push();
	}

	void push()
	{
		std::lock_guard<std::mutex> l(m);
		auto tp = std::chrono::high_resolution_clock::now();
		callstack.push(tp);
	}

	int64_t pop()
	{
		std::lock_guard<std::mutex> l(m);
		if (!callstack.size())
			return 0;
		auto start = callstack.top();
		callstack.pop();
		auto end = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::microseconds>(end -
																	 start)
			.count();
	}

	int64_t elapsed()
	{
		std::lock_guard<std::mutex> l(m);
		if (!callstack.size())
			return 0;
		auto start = callstack.top();
		auto end = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::microseconds>(end -
																	 start)
			.count();
	}

	int64_t restart()
	{
		int64_t elapsed = pop();
		push();
		return elapsed;
	}

	int elapsedMili() { return elapsed() / 1000; }

	static void sleepMsec(int val) { std::this_thread::sleep_for(1ms * val); }

protected:
	std::mutex m;
	std::stack<std::chrono::high_resolution_clock::time_point> callstack;
};

#endif // TIMING_H
