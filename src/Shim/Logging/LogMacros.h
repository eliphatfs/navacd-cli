#pragma once

#include "CoreTypes.h"
#include <cstdio>
#include <cstdarg>

// UE logging replacement - simplified fprintf-based logging

// Log category stub
struct FLogCategoryBase
{
	const char* Name;
	constexpr FLogCategoryBase(const char* InName) : Name(InName) {}
};

// Declare a log category
#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity, CompileTimeVerbosity) \
	extern FLogCategoryBase CategoryName;

// DEFINE_LOG_CATEGORY: marked inline so that force-include'ing this via
// UECompat.h does not produce a "multiple definition" link error.
#define DEFINE_LOG_CATEGORY(CategoryName) \
	inline FLogCategoryBase CategoryName(#CategoryName);

// UE_LOGF replacement - prints to stderr
#define UE_LOGF(CategoryName, Verbosity, Format, ...) \
	std::fprintf(stderr, "[%s] " Format "\n", #CategoryName, ##__VA_ARGS__)

// UE_LOG replacement (more common form in UE)
#define UE_LOG(CategoryName, Verbosity, Format, ...) \
	std::fprintf(stderr, "[%s] " Format "\n", #CategoryName, ##__VA_ARGS__)

// Verbosity levels (just for documentation; we always print)
struct ELogVerbosity
{
	enum Type : uint8
	{
		NoLogging = 0,
		Fatal,
		Error,
		Warning,
		Display,
		Log,
		Verbose,
		VeryVerbose,
		All = VeryVerbose,
	};
};

// LogGeometryProcessing category
DECLARE_LOG_CATEGORY_EXTERN(LogGeometryProcessing, Display, Display);
