//
// Collection of (potentially unsafe) code that does not make
// use of the MSVC runtime, hence "CRTless".
//

#include <cstdarg>
#include <type_traits>
#include <utility>
#include <Windows.h>

#include "CRTless_runtime.h"

namespace CRTless {
///////////////////////////////////////////////////////////////////////////////
// Multithreaded named pipe server
class NamedPipeServerMT
{
public:
	const size_t kBufSize = 4096;

	struct Listener
	{
		NamedPipeServerMT* server = nullptr;
		Listener* next = nullptr;
		HANDLE hPipe = INVALID_HANDLE_VALUE;
		HANDLE hThread = INVALID_HANDLE_VALUE;
	};

public:
	NamedPipeServerMT(const string_view& pipeName)
		: m_PipeName(pipeName)
	{
	}

	void Run()
	{
		while (true)
		{
			HANDLE hPipe = CreateNamedPipeA(
				m_PipeName.c_str(),
				PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_MESSAGE |
				PIPE_READMODE_MESSAGE |
				PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES,
				static_cast<DWORD>(kBufSize),
				static_cast<DWORD>(kBufSize),
				0,
				nullptr);

			CRTLESS_ASSERT(hPipe != INVALID_HANDLE_VALUE);

			bool connected = ConnectNamedPipe(hPipe, nullptr) ?
				true : (GetLastError() == ERROR_PIPE_CONNECTED);

			if (!connected)
			{
				CloseHandle(hPipe);
				continue;
			}

			// Create and link a new listener
			auto listener = New<Listener>();
			listener->server = this;
			listener->next = m_ListenersHead;
			listener->hPipe = hPipe;
			listener->hThread = CreateThread(
				nullptr,
				0,
				StaticListenerThreadEntry,
				listener,
				0,
				nullptr);

			m_ListenersHead = listener;
		}
	}

private:
	static DWORD WINAPI StaticListenerThreadEntry(void* arg)
	{
		auto listener = reinterpret_cast<Listener*>(arg);
		listener->server->ListenerThreadEntry(listener);
		return 0;
	}

	void ListenerThreadEntry(Listener* listener)
	{
		Printf("Listening...\n");
	}

private:
	string m_PipeName;
	Listener* m_ListenersHead = nullptr;
};
}
