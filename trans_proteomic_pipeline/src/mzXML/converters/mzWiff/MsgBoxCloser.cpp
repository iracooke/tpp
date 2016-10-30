// -*- mode: c++ -*-


/*
    File: AnalystImpl.cpp
    Description: shared implementation across Analyst & AnalystQS interfaces
    Date: July 31, 2007

    Copyright (C) 2007 Chee Hong WONG, Bioinformatics Institute


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include "stdafx.h"
#include "MsgBoxCloser.h"

using namespace std;

Semaphore::Semaphore()
{
	SECURITY_ATTRIBUTES securityAttributes;
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.lpSecurityDescriptor = NULL;
	securityAttributes.bInheritHandle = true;

	m_Semaphore = CreateSemaphore( &securityAttributes, 0, INT_MAX, NULL);
	// TODO: assert that m_Semaphore != NULL, GetLastError()
}

Semaphore::Semaphore(int nInitialCount,int nMaxCount)
{
	SECURITY_ATTRIBUTES securityAttributes;
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.lpSecurityDescriptor = NULL;
	securityAttributes.bInheritHandle = true;

	m_Semaphore = CreateSemaphore( &securityAttributes, nInitialCount, nMaxCount, NULL);
	// TODO: assert that m_Semaphore != NULL, GetLastError()
}

Semaphore::Semaphore(int nInitialCount)
{
	SECURITY_ATTRIBUTES securityAttributes;
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.lpSecurityDescriptor = NULL;
	securityAttributes.bInheritHandle = true;

	m_Semaphore = CreateSemaphore( &securityAttributes, nInitialCount, INT_MAX, NULL);
	// TODO: assert that m_Semaphore != NULL, GetLastError()
}

Semaphore::~Semaphore()
{
	CloseHandle(m_Semaphore);
	m_Semaphore = NULL;
}

void
Semaphore::Wait()
{
	DWORD dwResult = WaitForSingleObject(m_Semaphore, INFINITE);
	// TODO: assert that dwResult != WAIT_FAILED, GetLastError()
}

void
Semaphore::Wait(unsigned long timeoutmsec)
{
	DWORD dwResult = WaitForSingleObject(m_Semaphore, timeoutmsec);
	// TODO: assert that dwResult != WAIT_FAILED, GetLastError()
}

void
Semaphore::Signal()
{
	ReleaseSemaphore( m_Semaphore,1,NULL );
}

class MsgBoxCloserThreadInfo {
public:
	MsgBoxCloser   *m_pMsgBoxCloser;
	bool           m_fTerminate;  // Used to signal thread to terminate
	// Used by DismissExploreDataObjectsDialogProc
	unsigned long  m_nDialogCheckIntervalMSec;
	Semaphore      m_ThreadSemaphore;

public:
	MsgBoxCloserThreadInfo()
	{	MsgBoxCloserThreadInfo(NULL,15000);	}
	MsgBoxCloserThreadInfo(MsgBoxCloser	*pCloser, unsigned long	intervalMSec)
		:	m_pMsgBoxCloser(pCloser),
			m_fTerminate(false),
			m_nDialogCheckIntervalMSec(intervalMSec)
	{}
};

#include <process.h> // for _beginthreadex & _endthreadex

unsigned __stdcall DismissExploreDataObjectsDialogProc( void* dummy )
{
	MsgBoxCloserThreadInfo *pThreadInfo = (MsgBoxCloserThreadInfo *)dummy;
	MsgBoxCloser *pMsgBoxCloser = pThreadInfo->m_pMsgBoxCloser;

	const unsigned long nQuickIntervalMSec = 10000;  // check for 15 seconds max
	const unsigned long nQuickCheckIntervalMSec = 1000;
	unsigned long nAccummulatedMSec = 0;
	unsigned long nEffectiveDialogCheckIntervalMSec = nQuickCheckIntervalMSec;

	while (1)
	{
		try
		{
			pThreadInfo->m_ThreadSemaphore.Wait(nEffectiveDialogCheckIntervalMSec);

			// Terminate the thread upon request
			if (pThreadInfo->m_fTerminate)
				break;

			// Check ExploreDir dialog
			if (pMsgBoxCloser->dismissExploreDataObjectsDialog()) {
				nEffectiveDialogCheckIntervalMSec = pThreadInfo->m_nDialogCheckIntervalMSec;
			}

			if (nAccummulatedMSec>nQuickIntervalMSec) {
				nEffectiveDialogCheckIntervalMSec = pThreadInfo->m_nDialogCheckIntervalMSec;
			} else {
				nAccummulatedMSec += nEffectiveDialogCheckIntervalMSec;
			}
		}

		catch(...)
		{
			// we can't do anything, termination should be signalled
		}
	}
	delete pThreadInfo;
	_endthreadex( 0 );
	return 0;
}

MsgBoxCloser::MsgBoxCloser()
	:	m_pDialogThreadInfo(NULL),
		m_nDialogCheckIntervalSec(5),
		m_fStarted(false),
		m_hThread(NULL)
{
}

MsgBoxCloser::~MsgBoxCloser()
{
	stop();
}

int MsgBoxCloser::start()
{
	// if we have already started
	if (m_fStarted) return -1;

	// Set up parameters for starting MsgBox monitor thread
	m_pDialogThreadInfo = new MsgBoxCloserThreadInfo();
	m_pDialogThreadInfo->m_pMsgBoxCloser = this;
	m_pDialogThreadInfo->m_nDialogCheckIntervalMSec = m_nDialogCheckIntervalSec*1000;
	m_pDialogThreadInfo->m_fTerminate = false;

	// Spawn thread to periodically check ExploreDir dialog
	unsigned threadID;
	m_hThread = (HANDLE)_beginthreadex( NULL, 0, &DismissExploreDataObjectsDialogProc, (void *)m_pDialogThreadInfo, 0, &threadID );
	if (0==m_hThread)
	//if (1==_beginthread(DismissExploreDataObjectsDialogProc, 0, (void *)m_pDialogThreadInfo))
	{
		delete m_pDialogThreadInfo;
		m_pDialogThreadInfo = NULL;
		return errno;
	}
	
	m_fStarted = true;
	return 0;
}

void MsgBoxCloser::stop()
{
	if (NULL!=m_pDialogThreadInfo) {
		m_pDialogThreadInfo->m_fTerminate = true;
		m_pDialogThreadInfo->m_ThreadSemaphore.Signal();
		if (0 != m_hThread) {
			WaitForSingleObject( m_hThread, INFINITE );
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		m_pDialogThreadInfo = NULL;
	}
	m_fStarted = false;
}

BOOL CALLBACK enumDialogText(HWND hWnd, LPARAM lParam)
{
#define MAX_CLASS_NAME 1024
	TCHAR tchClassName[MAX_CLASS_NAME+1];
	if (GetClassName (hWnd, tchClassName, MAX_CLASS_NAME)>0) {
		if (_tcsicmp (_T("static"), tchClassName)==0) {
			int nLen = GetWindowTextLength (hWnd) + 1;
			TCHAR *ptchMsg = new TCHAR[nLen];
			if (GetWindowText(hWnd, ptchMsg, nLen)>0) {
				USES_CONVERSION;
				std::cerr << "WARNING: Dialog Message: " << T2A(ptchMsg) << std::endl;
			}
			delete [] ptchMsg;
		}
	}

	return TRUE;
}

bool MsgBoxCloser::dismissExploreDataObjectsDialog()
{
	/* code contributed by Dmitrii Tchekhovskoi */
	HWND hWnd = FindWindow( L"#32770", L"ExploreDataObjects" );
	if ( hWnd ) {
		std::cerr << std::endl << "WARNING: Found ABI ExploreDataObjects dialog, dismissing it to continue translation." << std::endl;
		EnumChildWindows(hWnd, enumDialogText, 0);

		/* the OK (default) button has Control ID=2 */
		SendMessage(hWnd, WM_COMMAND, 2, 0);

		return true;
	}

	return false;
}
