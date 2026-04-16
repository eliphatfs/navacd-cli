#pragma once

#include <mutex>

// UE's transactionally-safe mutex - map to std::mutex with UE-style Lock/Unlock
class FTransactionallySafeMutex : public std::mutex
{
public:
	using std::mutex::mutex;
	void Lock() { lock(); }
	void Unlock() { unlock(); }
};

class FTransactionallySafeRecursionMutex : public std::recursive_mutex
{
public:
	using std::recursive_mutex::recursive_mutex;
	void Lock() { lock(); }
	void Unlock() { unlock(); }
};

// Lock types
using FTransactionallySafeMutexScope = std::lock_guard<FTransactionallySafeMutex>;
