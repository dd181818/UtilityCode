//
// Collection of (potentially unsafe) code that does not make
// use of the MSVC runtime, hence "CRTless".
//

#include <cstdarg>
#include <Windows.h>

#define CRTLESS_PRINTF_BUFFER_SIZE 512
#define CRTLESS_PRINTF_TO_CONSOLE 1
#define CRTLESS_PRINTF_TO_DEBUG 0
#define CRTLESS_ASSERT(x) { if(!(x)) CRTless::_Assert(#x, __FILE__, __LINE__); }

namespace CRTless {
inline void _Assert(const char* exprString, const char* file, int line)
{
	__debugbreak();
}

inline int Printf(const char* fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);

	char str[CRTLESS_PRINTF_BUFFER_SIZE];
	int outLen = wvsprintfA(str, fmt, argList);

#if CRTLESS_PRINTF_TO_CONSOLE
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CRTLESS_ASSERT(hStdOut != INVALID_HANDLE_VALUE);
	DWORD bytesWritten;
	if (!WriteConsoleA(hStdOut, str, static_cast<DWORD>(outLen),
					   &bytesWritten, nullptr))
	{
		CRTLESS_ASSERT(0);
	}
#endif

#if CRTLESS_PRINTF_TO_DEBUG
	OutputDebugStringA(str);
#endif

	va_end(argList);
	return outLen;
}
}
