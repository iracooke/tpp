// -*- mode: c++ -*-


/*
    File: AnalystImpl.h
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


#pragma once

#include <windows.h>

class Semaphore
{
public:
	Semaphore();
	Semaphore(int nInitialCount);
	Semaphore(int nInitialCount,int nMaxCount);
	~Semaphore();

	void Wait();
	void Wait(unsigned long timeoutmsec);
	void Signal();

private:
	HANDLE m_Semaphore;
};

class MsgBoxCloserThreadInfo;
class MsgBoxCloser {
private:
	MsgBoxCloserThreadInfo	*m_pDialogThreadInfo;
	unsigned long m_nDialogCheckIntervalSec;
	bool m_fStarted;
	HANDLE m_hThread;
public:
	MsgBoxCloser();
	~MsgBoxCloser();
	int start();
	void stop();
	bool dismissExploreDataObjectsDialog();
};
