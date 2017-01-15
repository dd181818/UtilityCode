//
// NamedPipeMessageServerMT
//

#pragma once

#include <cassert>
#include <string>
#include <thread>
#include <Windows.h>

namespace UtilityCode {
///////////////////////////////////////////////////////////////////////////////
// Multithreaded named pipe server
class NamedPipeMessageServerMT
{
public:
	static const size_t kBufSize = 16 * 1024 * 1024;
	static_assert(sizeof(size_t) == 8, "no 32bit support");

	struct Message
	{
		HANDLE hPipe;
		std::string request;
		std::string reply;
	};

	struct Listener
	{
		NamedPipeMessageServerMT* server = nullptr;
		HANDLE hPipe = INVALID_HANDLE_VALUE;
	};

public:
	NamedPipeMessageServerMT(const std::string& pipeName)
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
				PIPE_TYPE_BYTE |
				PIPE_READMODE_BYTE |
				PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES,
				static_cast<DWORD>(kBufSize),
				static_cast<DWORD>(kBufSize),
				0,
				nullptr);

			assert(hPipe != INVALID_HANDLE_VALUE);

			bool connected = ConnectNamedPipe(hPipe, nullptr) ?
				true : (GetLastError() == ERROR_PIPE_CONNECTED);

			if (!connected)
			{
				CloseHandle(hPipe);
				continue;
			}

			// Create a new listener instance & thread.
			auto listener = new Listener;
			listener->server = this;
			listener->hPipe = hPipe;
			std::thread([this, listener]() {
				ListenerThreadEntry(*listener);
			}).detach();
		}
	}

protected:
	virtual void HandleMessage(Message& msg)
	{
	}

private:
	bool Read(HANDLE hPipe, void* dest, size_t numBytes)
	{
		DWORD bytesRead;

		BOOL fSuccess = ReadFile(
			hPipe,
			dest,
			static_cast<DWORD>(numBytes),
			&bytesRead,
			nullptr);

		if (!fSuccess || bytesRead == 0)
		{
			return false;
		}

		assert(bytesRead == numBytes);
		return true;
	}

	bool Write(HANDLE hPipe, const void* src, size_t numBytes)
	{
		DWORD bytesWritten;

		BOOL fSuccess = WriteFile(
			hPipe,
			src,
			static_cast<DWORD>(numBytes),
			&bytesWritten,
			nullptr);

		if (!fSuccess)
		{
			return false;
		}

		assert(bytesWritten == numBytes);
		return true;
	}

	void ListenerThreadEntry(Listener& ls)
	{
		Message msg;
		while (1)
		{
			size_t msgLen;
			if (!Read(ls.hPipe, &msgLen, sizeof(msgLen)))
			{
				break;
			}

			msg.request.resize(msgLen);

			if (!Read(ls.hPipe, const_cast<char*>(msg.request.data()), msgLen))
			{
				break;
			}

			msg.reply.clear();
			HandleMessage(msg);

			msgLen = msg.reply.size();
			if (!Write(ls.hPipe, &msgLen, sizeof(msgLen)))
			{
				break;
			}

			if (msgLen > 0)
			{
				if (!Write(ls.hPipe, msg.reply.data(), msgLen))
					break;
			}
		}

		delete &ls;
	}

private:
	std::string m_PipeName;
};
}
