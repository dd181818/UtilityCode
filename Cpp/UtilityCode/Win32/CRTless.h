//
// Collection of (potentially unsafe) code that does not make
// use of the MSVC runtime, hence "CRTless".
//

#pragma warning(push)
#pragma warning(disable : 4577)
#include <cstdarg>
#include <type_traits>
#include <utility>
#include <Windows.h>
#pragma warning(pop)

#include "CRTless_runtime.h"

namespace CRTless {
///////////////////////////////////////////////////////////////////////////////
// Multithreaded named pipe server
class NamedPipeServerMT
{
public:
	const size_t kBufSize = 4096;

	struct Message
	{
		HANDLE hPipe;
		string request;
		string reply;
	};

	struct Listener
	{
		NamedPipeServerMT* server = nullptr;
		HANDLE hPipe = INVALID_HANDLE_VALUE;
		//HANDLE hThread = INVALID_HANDLE_VALUE;
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

			// Create a new listener instance & thread.
			auto listener = New<Listener>();
			listener->server = this;
			listener->hPipe = hPipe;
			CreateThread(nullptr, 0, StaticListenerThreadEntry, listener, 0, nullptr);
		}
	}

protected:
	virtual void HandleMessage(Message& msg)
	{
	}

private:
	static DWORD WINAPI StaticListenerThreadEntry(void* arg)
	{
		auto listener = reinterpret_cast<Listener*>(arg);
		listener->server->ListenerThreadEntry(*listener);
		return 0;
	}

	void ListenerThreadEntry(Listener& ls)
	{
		Message msg;
		while (1)
		{
			DWORD bytesRead;
			DWORD bytesWritten;

			msg.request.resize(kBufSize);
			BOOL fSuccess = ReadFile(
				ls.hPipe, 
				msg.request.data(),
				static_cast<DWORD>(msg.request.size()),
				&bytesRead,
				nullptr);

			if (!fSuccess || bytesRead == 0)
			{
				if (GetLastError() == ERROR_BROKEN_PIPE)
				{
				}
				else
				{
				}
				break;
			}

			msg.request.resize(static_cast<size_t>(bytesRead));
			msg.reply.clear();
			HandleMessage(msg);

			fSuccess = WriteFile(
				ls.hPipe,
				msg.reply.data(),
				static_cast<DWORD>(msg.reply.size()),
				&bytesWritten,
				nullptr);

			if (!fSuccess || msg.reply.size() != bytesWritten)
			{
				break;
			}
		}
		Printf("connection done\n");
		Delete<Listener>(&ls);
	}

private:
	string m_PipeName;
};
}
