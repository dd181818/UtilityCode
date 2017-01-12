//
// Collection of code that does not make
// use of the MSVC runtime, hence "CRTless".
//

#include <cstdarg>
#include <type_traits>
#include <utility>
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

inline void* Malloc(size_t size)
{
	void* p = HeapAlloc(GetProcessHeap(), 0, size);
	CRTLESS_ASSERT(p);
	return p;
}

inline void Free(void* ptr)
{
	HeapFree(GetProcessHeap(), 0, ptr);
}

template <typename T, typename... Args>
inline T* New(Args&&... args)
{
	void* p = Malloc(sizeof(T));
	new (p) T(std::forward<Args>(args)...);
	return reinterpret_cast<T*>(p);
}

template <typename T>
inline T* NewArray(size_t numElements,
				   typename std::enable_if_t<std::is_trivially_constructible_v<T>>* = 0)
{
	void* p = Malloc(sizeof(T) * numElements);
	return reinterpret_cast<T*>(p);
}

template <typename T>
inline T* NewArray(size_t numElements,
				   typename std::enable_if_t<!std::is_trivially_constructible_v<T>, int> = 0)
{
	void* p = Malloc(sizeof(T) * numElements);
	T* elements = reinterpret_cast<T*>(p);
	for (size_t i = 0; i < numElements; i++)
		new (&elements[i]) T;
	return elements;
}

template <typename T>
inline void Delete(T* p,
				   typename std::enable_if_t<std::is_trivially_destructible_v<T>>* = 0)
{
	Free(p);
}

template <typename T>
inline void Delete(T* p,
				   typename std::enable_if_t<!std::is_trivially_destructible_v<T>, int> = 0)
{
	p->~T();
	Free(p);
}

template <typename T,
	typename = std::enable_if_t<std::is_trivially_destructible_v<T>>>
inline void DeleteArray(T* p)
{
	Free(p);
}

inline void* MemCpy(void* dest, const void* src, size_t num)
{
	auto pd = reinterpret_cast<char*>(dest);
	auto ps = reinterpret_cast<const char*>(src);
	while (num-- > 0)
	{
		*pd++ = *ps++;
	}
	return dest;
}

inline void* MemSet(void* dest, int value, size_t num)
{
	auto pd = reinterpret_cast<char*>(dest);
	const auto ch = static_cast<unsigned char>(value);
	while (num-- > 0)
	{
		*pd++ = ch;
	}
	return dest;
}

inline size_t Strlen(const char* str)
{
	return static_cast<size_t>(lstrlenA(str));
}
}

#pragma warning(push)
#pragma warning(disable : 4005)

#define MTL_namespace CRTless
#define MTL_memcpy(dest, src, num) MemCpy(dest, src, num)
#define MTL_memset(dest, fill, num) MemSet(dest, fill, num)
#define MTL_strlen(s) Strlen(s)
#define MTL_new(type, ...) New<type>(__VA_ARGS__)
#define MTL_new_array(type, n) NewArray<type>(n)
#define MTL_delete(p) Delete(p)
#define MTL_delete_array(p) DeleteArray(p)

#pragma warning(pop)

#include "../MTL/string.inl"
