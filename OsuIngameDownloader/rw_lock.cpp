#include "rw_lock.h"
#include "logger.h"
RWLockImpl::RWLockImpl() : m_readers(0), m_writersWaiting(0), m_writers(0)
{
	m_mutex = CreateMutex(NULL, FALSE, NULL);
	if (m_mutex == NULL)
		logger::WriteLog("[-] Lock: cannot create reader/writer lock");

	m_readEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (m_readEvent == NULL)
		logger::WriteLog("[-] Lock: cannot create reader/writer lock");

	m_writeEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (m_writeEvent == NULL)
		logger::WriteLog("[-] Lock: cannot create reader/writer lock");
}

RWLockImpl::~RWLockImpl()
{
	CloseHandle(m_mutex);
	CloseHandle(m_readEvent);
	CloseHandle(m_writeEvent);
}

inline void RWLockImpl::AddWriter()
{
	switch (WaitForSingleObject(m_mutex, INFINITE))
	{
	case WAIT_OBJECT_0:
		if (++m_writersWaiting == 1)
			ResetEvent(m_readEvent);
		ReleaseMutex(m_mutex);
		break;
	default:
		logger::WriteLog("[-] Lock: cannot lock reader/writer lock");
	}
}

inline void RWLockImpl::RemoveWriter()
{
	switch (WaitForSingleObject(m_mutex, INFINITE))
	{
	case WAIT_OBJECT_0:
		if (--m_writersWaiting == 0 && m_writers == 0)
			SetEvent(m_readEvent);
		ReleaseMutex(m_mutex);
		break;
	default:
		logger::WriteLog("[-] Lock: cannot lock reader/writer lock");
	}
}

void RWLockImpl::ReadLockImpl()
{
	HANDLE h[2];
	h[0] = m_mutex;
	h[1] = m_readEvent;
	switch (WaitForMultipleObjects(2, h, TRUE, INFINITE))
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		++m_readers;
		ResetEvent(m_writeEvent);
		ReleaseMutex(m_mutex);
		assert(m_writers == 0);
		break;
	default:
		logger::WriteLog("[-] Lock: cannot lock reader/writer lock");
	}
}

bool RWLockImpl::TryReadLockImpl()
{
	for (;;)
	{
		if (m_writers != 0 || m_writersWaiting != 0)
			return false;

		DWORD result = TryReadLockOnce();
		switch (result)
		{
		case WAIT_OBJECT_0:
		case WAIT_OBJECT_0 + 1:
			return true;
		case WAIT_TIMEOUT:
			continue;
		default:
			logger::WriteLog("[-] Lock: cannot lock reader/writer lock");
		}
	}
}

void RWLockImpl::WriteLockImpl()
{
	AddWriter();
	HANDLE h[2];
	h[0] = m_mutex;
	h[1] = m_writeEvent;
	switch (WaitForMultipleObjects(2, h, TRUE, INFINITE))
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		--m_writersWaiting;
		++m_readers;
		++m_writers;
		ResetEvent(m_readEvent);
		ResetEvent(m_writeEvent);
		ReleaseMutex(m_mutex);
		assert(m_writers == 1);
		break;
	default:
		RemoveWriter();
		logger::WriteLog("[-] Lock: cannot lock reader/writer lock");
	}
}

bool RWLockImpl::TryWriteLockImpl()
{
	AddWriter();
	HANDLE h[2];
	h[0] = m_mutex;
	h[1] = m_writeEvent;
	switch (WaitForMultipleObjects(2, h, TRUE, 1))
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		--m_writersWaiting;
		++m_readers;
		++m_writers;
		ResetEvent(m_readEvent);
		ResetEvent(m_writeEvent);
		ReleaseMutex(m_mutex);
		assert(m_writers == 1);
		return true;
	case WAIT_TIMEOUT:
		RemoveWriter();
	default:
		RemoveWriter();
		logger::WriteLog("[-] Lock: cannot lock reader/writer lock");
	}
	return false;
}

void RWLockImpl::UnlockImpl()
{
	switch (WaitForSingleObject(m_mutex, INFINITE))
	{
	case WAIT_OBJECT_0:
		m_writers = 0;
		if (m_writersWaiting == 0) SetEvent(m_readEvent);
		if (--m_readers == 0) SetEvent(m_writeEvent);
		ReleaseMutex(m_mutex);
		break;
	default:
		logger::WriteLog("[-] Lock: cannot unlock reader/writer lock");
	}
}

DWORD RWLockImpl::TryReadLockOnce()
{
	HANDLE h[2];
	h[0] = m_mutex;
	h[1] = m_readEvent;
	DWORD result = WaitForMultipleObjects(2, h, TRUE, 1);
	switch (result)
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		++m_readers;
		ResetEvent(m_writeEvent);
		ReleaseMutex(m_mutex);
		assert(m_writers == 0);
		return result;
	case WAIT_TIMEOUT:
	default:
		logger::WriteLog("[-] Lock: cannot lock reader/writer lock");
	}
	return result;
}
