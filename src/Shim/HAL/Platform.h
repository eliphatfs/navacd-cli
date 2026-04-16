#pragma once

#include "CoreTypes.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>

// ========================================================================
// FPlatformTime - high-resolution timing
// ========================================================================
struct FPlatformTime
{
	static double Seconds()
	{
		auto Now = std::chrono::high_resolution_clock::now();
		auto Duration = Now.time_since_epoch();
		return std::chrono::duration<double>(Duration).count();
	}

	static double Init() { return Seconds(); }
};

// ========================================================================
// FPlatformMisc - platform utilities
// ========================================================================
struct FPlatformMisc
{
	static void RequestExit(bool Force)
	{
		if (Force) std::_Exit(1);
		else std::exit(0);
	}

	static int32 NumberOfCores() { return 1; }
	static int32 NumberOfCoresIncludingHyperthreads() { return 1; }
};

// ========================================================================
// FPlatformAtomics - atomic operations (minimal stub)
// ========================================================================
struct FPlatformAtomics
{
	static int32 InterlockedIncrement(volatile int32* Value) { return ++(*Value); }
	static int32 InterlockedDecrement(volatile int32* Value) { return --(*Value); }
	static int32 InterlockedAdd(volatile int32* Value, int32 Amount) { return (*Value += Amount); }
	static int32 InterlockedCompareExchange(volatile int32* Dest, int32 Exchange, int32 Comparand)
	{
		if (*Dest == Comparand) { *Dest = Exchange; return Comparand; }
		return *Dest;
	}
};
