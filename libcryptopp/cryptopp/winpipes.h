#ifndef CRYPTOPP_WINPIPES_H
#define CRYPTOPP_WINPIPES_H
// *****************************************************************************
// winpipes.h                                                      Tao3D project
// *****************************************************************************
//
// File description:
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "config.h"

#ifdef WINDOWS_PIPES_AVAILABLE

#include "network.h"
#include "queue.h"
#include <winsock2.h>

NAMESPACE_BEGIN(CryptoPP)

//! Windows Handle
class WindowsHandle
{
public:
	WindowsHandle(HANDLE h = INVALID_HANDLE_VALUE, bool own=false);
	WindowsHandle(const WindowsHandle &h) : m_h(h.m_h), m_own(false) {}
	virtual ~WindowsHandle();

	bool GetOwnership() const {return m_own;}
	void SetOwnership(bool own) {m_own = own;}

	operator HANDLE() {return m_h;}
	HANDLE GetHandle() const {return m_h;}
	bool HandleValid() const;
	void AttachHandle(HANDLE h, bool own=false);
	HANDLE DetachHandle();
	void CloseHandle();

protected:
	virtual void HandleChanged() {}

	HANDLE m_h;
	bool m_own;
};

//! Windows Pipe
class WindowsPipe
{
public:
	class Err : public OS_Error
	{
	public:
		Err(HANDLE h, const std::string& operation, int error);
		HANDLE GetHandle() const {return m_h;}

	private:
		HANDLE m_h;
	};

protected:
	virtual HANDLE GetHandle() const =0;
	virtual void HandleError(const char *operation) const;
	void CheckAndHandleError(const char *operation, BOOL result) const
		{assert(result==TRUE || result==FALSE); if (!result) HandleError(operation);}
};

//! pipe-based implementation of NetworkReceiver
class WindowsPipeReceiver : public WindowsPipe, public NetworkReceiver
{
public:
	WindowsPipeReceiver();

	bool MustWaitForResult() {return true;}
	bool Receive(byte* buf, size_t bufLen);
	unsigned int GetReceiveResult();
	bool EofReceived() const {return m_eofReceived;}

	unsigned int GetMaxWaitObjectCount() const {return 1;}
	void GetWaitObjects(WaitObjectContainer &container, CallStack const& callStack);

private:
	WindowsHandle m_event;
	OVERLAPPED m_overlapped;
	bool m_resultPending;
	DWORD m_lastResult;
	bool m_eofReceived;
};

//! pipe-based implementation of NetworkSender
class WindowsPipeSender : public WindowsPipe, public NetworkSender
{
public:
	WindowsPipeSender();

	bool MustWaitForResult() {return true;}
	void Send(const byte* buf, size_t bufLen);
	unsigned int GetSendResult();
	bool MustWaitForEof() { return false; }
	void SendEof() {}

	unsigned int GetMaxWaitObjectCount() const {return 1;}
	void GetWaitObjects(WaitObjectContainer &container, CallStack const& callStack);

private:
	WindowsHandle m_event;
	OVERLAPPED m_overlapped;
	bool m_resultPending;
	DWORD m_lastResult;
};

//! Windows Pipe Source
class WindowsPipeSource : public WindowsHandle, public NetworkSource, public WindowsPipeReceiver
{
public:
	WindowsPipeSource(HANDLE h=INVALID_HANDLE_VALUE, bool pumpAll=false, BufferedTransformation *attachment=NULL)
		: WindowsHandle(h), NetworkSource(attachment)
	{
		if (pumpAll)
			PumpAll();
	}

	NetworkSource::GetMaxWaitObjectCount;
	NetworkSource::GetWaitObjects;

private:
	HANDLE GetHandle() const {return WindowsHandle::GetHandle();}
	NetworkReceiver & AccessReceiver() {return *this;}
};

//! Windows Pipe Sink
class WindowsPipeSink : public WindowsHandle, public NetworkSink, public WindowsPipeSender
{
public:
	WindowsPipeSink(HANDLE h=INVALID_HANDLE_VALUE, unsigned int maxBufferSize=0, unsigned int autoFlushBound=16*1024)
		: WindowsHandle(h), NetworkSink(maxBufferSize, autoFlushBound) {}

	NetworkSink::GetMaxWaitObjectCount;
	NetworkSink::GetWaitObjects;

private:
	HANDLE GetHandle() const {return WindowsHandle::GetHandle();}
	NetworkSender & AccessSender() {return *this;}
};

NAMESPACE_END

#endif

#endif
