#ifndef FTL_THREAD_HPP
#define FTL_THREAD_HPP
#pragma once

#ifdef USE_EXPORT
#  include "ftlthread.h"
#endif

namespace FTL
{
    BOOL CFNullLockObject::Lock(DWORD dwTimeout/* = INFINITE */)
    {
        UNREFERENCED_PARAMETER(dwTimeout);
        return TRUE;
    }

    BOOL CFNullLockObject::UnLock()
    {
        return TRUE;
    }

    CFCriticalSection::CFCriticalSection()
    {
#if (_WIN32_WINNT >= 0x0403)
        //使用 InitializeCriticalSectionAndSpinCount 可以提高性能
        ::InitializeCriticalSectionAndSpinCount(&m_CritSec,4000);
#else
        ::InitializeCriticalSection(&m_CritSec);
#endif

#ifdef FTL_DEBUG
        m_lockCount = 0;
        m_currentOwner = 0;
        m_fTrace = FALSE;
#endif
    }

    CFCriticalSection::~CFCriticalSection()
    {
#ifdef FTL_DEBUG
        FTLASSERT(m_lockCount == 0); // 析构前必须完全释放，否则可能造成死锁
#endif
        ::DeleteCriticalSection(&m_CritSec);
    }

    BOOL CFCriticalSection::Lock(DWORD dwTimeout/* = INFINITE*/)
    {
        UNREFERENCED_PARAMETER( dwTimeout );
        FTLASSERT(INFINITE == dwTimeout && ("CFCriticalSection NotSupport dwTimeOut"));
#ifdef FTL_DEBUG
        DWORD us = GetCurrentThreadId();
        DWORD currentOwner = m_currentOwner;
        if (currentOwner && (currentOwner != us)) // already owned, but not by us
        {
            if (m_fTrace) 
            {
                FTLTRACEEX(tlInfo,TEXT("Thread %d begin to wait for CriticalSection %x Owned by %d"),
                    us, &m_CritSec, currentOwner);
            }
        }
#endif
        ::EnterCriticalSection(&m_CritSec);

#ifdef FTL_DEBUG
        if (1 == InterlockedIncrement(&m_lockCount)) // we now own it for the first time.  Set owner information
        {
            m_currentOwner = us;
            if (m_fTrace) 
            {
                FTLTRACEEX(tlInfo,TEXT("Thread %d now owns CriticalSection %x"), m_currentOwner, &m_CritSec);
            }
        }
#endif
        return TRUE;
    }

    BOOL CFCriticalSection::UnLock()
    {
#ifdef FTL_DEBUG
        DWORD us = GetCurrentThreadId();
        FTLASSERT( us == m_currentOwner ); //just the owner can unlock it
        FTLASSERT( m_lockCount > 0 );
        if ( 0 == InterlockedDecrement(&m_lockCount)) 
        {
            // begin to unlock
            if (m_fTrace) 
            {
                FTLTRACEEX(tlInfo,TEXT("Thread %d releasing CriticalSection %x"), m_currentOwner, &m_CritSec);
            }
            m_currentOwner = 0;
        }
#endif
        ::LeaveCriticalSection(&m_CritSec);
        return TRUE;
    }

    BOOL CFCriticalSection::TryLock()
    {
        BOOL bRet = TryEnterCriticalSection(&m_CritSec);
        return bRet;
    }

#ifdef FTL_DEBUG
    BOOL CFCriticalSection::IsLocked() const
    {
        return (m_lockCount > 0);
    }
    BOOL CFCriticalSection::SetTrace(BOOL bTrace)
    {
        m_fTrace = bTrace;
        return TRUE;
    }
#endif

    template<typename T>
    CFAutoLock<T>::CFAutoLock(T* pLockObj)
    {
        FTLASSERT(pLockObj);
        m_pLockObj = pLockObj;
        m_pLockObj->Lock(INFINITE);
    }
    
    template<typename T>
    VOID CFAutoLock<T>::Release() 
    {
        if (m_pLockObj)
        {
            m_pLockObj->UnLock();
            m_pLockObj = NULL;
        }
    }

    template<typename T>
    CFAutoLock<T>::~CFAutoLock()
    {
        Release();
    }


    CFEventChecker::CFEventChecker(HANDLE hEventStop,HANDLE hEventContinue)
    {
        m_hEventStop = hEventStop;
        m_hEventContinue = hEventContinue;
        FTLASSERT(m_hEventStop);
        FTLASSERT(m_hEventContinue);
    }

    CFEventChecker::~CFEventChecker()
    {
        m_hEventStop = NULL;
        m_hEventContinue = NULL;
    }

    FTLThreadWaitType CFEventChecker::GetWaitType(DWORD dwTimeOut /* = INFINITE */)
    {
        HANDLE waitEvent[] = 
        {
            {m_hEventStop},
            {m_hEventContinue}
        };

        FTLThreadWaitType waitType = ftwtError;
        DWORD dwResult = WaitForMultipleObjects(sizeof(waitEvent)/sizeof(waitEvent[0]),waitEvent,FALSE,dwTimeOut);
        switch(dwResult)
        {
        case WAIT_OBJECT_0: // stop
            waitType = ftwtStop;
            break;
        case WAIT_OBJECT_0 + 1: //continue
            waitType = ftwtContinue;
            break;
        case WAIT_TIMEOUT:
            waitType = ftwtTimeOut;
            break;
        default:
            FTLTRACEEX(tlError,_T("CFEventChecker::GetWaitType Error!!!"));
            waitType = ftwtError;
            FTLASSERT(FALSE);
            break;
        }
        return waitType;
    }

    FTLThreadWaitType CFEventChecker::GetWaitTypeEx(HANDLE* pUserHandles, 
        DWORD nUserHandlCount, DWORD* pResultHandleIndex,
        BOOL  bCheckContinue /*= FALSE */,
        DWORD dwTimeOut /* = INFINITE */)
    {
        FTLASSERT(nUserHandlCount + 2 < MAXIMUM_WAIT_OBJECTS);
        FTLASSERT(NULL != pResultHandleIndex);

        DWORD dwWaitCount = 1;
        HANDLE waitEvent[MAXIMUM_WAIT_OBJECTS] = 
        {
            {m_hEventStop},
            0
        };
        if (pUserHandles && nUserHandlCount > 0)
        {
            CopyMemory(&waitEvent[1], pUserHandles, sizeof(HANDLE) * nUserHandlCount);
            dwWaitCount += nUserHandlCount;
        }
        if (bCheckContinue)
        {
            waitEvent[dwWaitCount++] = m_hEventContinue;
        }

        FTLThreadWaitType waitType = ftwtError;
        DWORD dwResult = WaitForMultipleObjects(dwWaitCount, waitEvent, FALSE, dwTimeOut);
        if (WAIT_OBJECT_0 == dwResult)
        {
            //stop
            waitType = ftwtStop;
        }
        else if(WAIT_OBJECT_0 + 1 <= dwResult && dwResult <= WAIT_OBJECT_0 + nUserHandlCount)
        {
            //user handle
            waitType = ftwtUserHandle;
            if (pResultHandleIndex)
            {
                *pResultHandleIndex = (dwResult - WAIT_OBJECT_0 - 1);
            }
        }
        else if(bCheckContinue && WAIT_OBJECT_0 + nUserHandlCount + 1 == dwResult)
        {
            // continue
            waitType = ftwtContinue;
        }
        else if(WAIT_TIMEOUT == dwResult)
        {
            waitType = ftwtTimeOut;
        }
        else
        {
            FTLTRACEEX(tlError,_T("CFEventChecker::GetWaitTypeEx Error!!!"));
            waitType = ftwtError;
            FTLASSERT(FALSE);
        }
        return waitType;
    }

    FTLThreadWaitType CFEventChecker::SleepAndCheckStop(DWORD dwTimeOut)
    {
        FTLThreadWaitType waitType = ftwtError;
        DWORD dwResult = WaitForSingleObject(m_hEventStop,dwTimeOut);
        switch (dwResult)
        {
        case WAIT_OBJECT_0: //Stop
            waitType = ftwtStop;
            break;
        case WAIT_TIMEOUT:
            waitType = ftwtTimeOut;
            break;
        default:  //how can come here ?
            waitType = ftwtError;
            break;
        }
        return waitType;
    }

    template <typename T>
    CFSyncEventUtility<T>::CFSyncEventUtility()
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
    }

    template <typename T>
    CFSyncEventUtility<T>::~CFSyncEventUtility()
    {
        CFAutoLock<CFLockObject> locker(&m_LockObject);
        ClearAllEvent();
    }

    template <typename T>
    size_t CFSyncEventUtility<T>::GetSyncEventCount()
    {
        size_t count = 0;
        {
            CFAutoLock<CFLockObject> locker(&m_LockObject);
            count = m_AllSyncEventMap.size();
        }
        return count;
    }

    template <typename T>
    void CFSyncEventUtility<T>::ClearAllEvent()
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        CFAutoLock<CFLockObject> locker(&m_LockObject);
        for (SYNC_EVENT_MAP::iterator iter = m_AllSyncEventMap.begin(); 
            iter != m_AllSyncEventMap.end();
            ++iter)
        {
            if ((*iter).second.bCreateEvent)
            {
                HANDLE hEvent = (*iter).second.hEvent;
                FTLASSERT(hEvent);
                CloseHandle(hEvent);
            }
        }
        m_AllSyncEventMap.clear();
    }

    template <typename T>
    void CFSyncEventUtility<T>::AddEvent(T t, HANDLE hEvent /* = NULL */)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        SyncEventInfo eventInfo = {0};
        if (NULL == hEvent)
        {
            eventInfo.hEvent = ::CreateEvent(NULL,TRUE, FALSE, NULL);
            eventInfo.bCreateEvent = true;
        }
        else
        {
            eventInfo.hEvent = hEvent;
            eventInfo.bCreateEvent = false;
        }

        {
            CFAutoLock<CFLockObject> locker(&m_LockObject);
            FTLASSERT(m_AllSyncEventMap.find(t) == m_AllSyncEventMap.end());
            m_AllSyncEventMap[t] = eventInfo;
        }
    }

    template <typename T>
    void CFSyncEventUtility<T>::SetEvent(T t)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        CFAutoLock<CFLockObject> locker(&m_LockObject);

        SYNC_EVENT_MAP::iterator iter = m_AllSyncEventMap.find(t);
        FTLASSERT(iter != m_AllSyncEventMap.end());
        if (iter != m_AllSyncEventMap.end())
        {
            ::SetEvent(iter->second.hEvent);
        }
    }

    template <typename T>
    void CFSyncEventUtility<T>::ResetEvent(T t)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        CFAutoLock<CFLockObject> locker(&m_LockObject);

        SYNC_EVENT_MAP::iterator iter = m_AllSyncEventMap.find(t);
        FTLASSERT(iter != m_AllSyncEventMap.end());
        if (iter != m_AllSyncEventMap.end())
        {
            ::ResetEvent(iter->second.hEvent);
        }
    }

    template <typename T>
    void CFSyncEventUtility<T>::SetAllEvent()
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        CFAutoLock<CFLockObject> locker(&m_LockObject);
        SYNC_EVENT_MAP::iterator iter = m_AllSyncEventMap.begin();
        for (; iter != m_AllSyncEventMap.end(); ++iter)
        {
            ::SetEvent(iter->second.hEvent);
        }
    }

    template <typename T>
    void CFSyncEventUtility<T>::ResetAllEvent()
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        CFAutoLock<CFLockObject> locker(&m_LockObject);
        SYNC_EVENT_MAP::iterator iter = m_AllSyncEventMap.begin();
        for (; iter != m_AllSyncEventMap.end(); ++iter)
        {
            ::ResetEvent(iter->second.hEvent);
        }
    }

    template <typename T>
    BOOL CFSyncEventUtility<T>::WaitAllEvent(DWORD dwMilliseconds /* = INFINITE */)
    {
        FUNCTION_BLOCK_TRACE(0);
        BOOL bRet = FALSE;

        std::vector<HANDLE> hAllEvents;
        SYNC_EVENT_MAP::iterator iter = m_AllSyncEventMap.begin();
        for (; iter != m_AllSyncEventMap.end(); ++iter)
        {
            hAllEvents.push_back(iter->second.hEvent);
        }

        DWORD dwResult = ::WaitForMultipleObjects((DWORD)hAllEvents.size(), &hAllEvents[0], TRUE, dwMilliseconds);
        if (WAIT_OBJECT_0 == dwResult)
        {
            bRet = TRUE;
        }
        return bRet;
    }

    template <typename T>
    T CFSyncEventUtility<T>::WaitOneEvent(DWORD dwMilliseconds /* = INFINITE */)
    {
        std::vector<HANDLE> hAllEvents;
        SYNC_EVENT_MAP::iterator iter = m_AllSyncEventMap.begin();
        for (; iter != m_AllSyncEventMap.end(); ++iter)
        {
            hAllEvents.push_back(iter->second.hEvent);
        }
        DWORD dwResult = ::WaitForMultipleObjects(hAllEvents.size(), &hAllEvents[0], FALSE, dwMilliseconds);

        T tRet;
        int index = 0;
        for (SYNC_EVENT_MAP::iterator iter = m_AllSyncEventMap.begin(); iter != m_AllSyncEventMap.end(); ++iter)
        {
            if (index == waitIndex)
            {
                tRet = iter->first;
                break;
            }
            index++
        }
        return tRet; 
    }

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // must be 0x1000
        LPSTR pszName; // pointer to name (in user addr space)
        DWORD dwThreadID; // thread ID (-1=caller thread)
        DWORD dwFlags; // reserved for future use, must be zero
    } THREADNAME_INFO;

    struct StartThreadProxyStruct
    {
        LPTHREAD_START_ROUTINE	pfn;
        LPVOID			        param;
    };
    static DWORD WINAPI StartThreadProxy(void* param)
    {
        //	_control87(_EM_DENORMAL | _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT,  _MCW_EM);
        //	_control87(_PC_53, _MCW_PC);
#if 0
        //从XpCommonLib中学来，作用是什么？
        _control87(_RC_CHOP, _MCW_RC);
        setlocale(LC_ALL, "");
#endif 
        StartThreadProxyStruct* pProxyStruct = (StartThreadProxyStruct*)param;
        DWORD retval = (*pProxyStruct->pfn)(pProxyStruct->param);
        delete pProxyStruct;
        return retval;
    }


    template <typename ThreadTraits>
    CFThread<ThreadTraits>::CFThread(HANDLE hEventStop/* = NULL*/,HANDLE hEventContinue/* = NULL*/)
    {
        m_hThread	= NULL;
        if (NULL == hEventStop)
        {
            m_hEventStop	= ::CreateEvent(NULL,TRUE,FALSE,NULL);
            m_bCreateEventStop = TRUE;
        }
        else
        {
            m_hEventStop = hEventStop;
            m_bCreateEventStop = FALSE;
        }

        if (NULL == hEventContinue)
        {
            m_hEventContinue = ::CreateEvent(NULL,TRUE,TRUE,NULL);
            m_bCreateEventContinue = TRUE;
        }
        else
        {
            m_hEventContinue = hEventContinue;
            m_bCreateEventContinue = FALSE;
        }
        m_pEventChecker = new CFEventChecker(m_hEventStop, m_hEventContinue);
        //m_ThreadState = ftsStopped;
    }
    template <typename ThreadTraits>
    CFThread<ThreadTraits>::~CFThread()
    {
        BOOL bRet = FALSE;
        if( IsThreadRunning())
        {
            FTLASSERT(!_T("Please Stop Thread Before App Exit"));
            API_VERIFY(StopAndWait(FTL_MAX_THREAD_DEADLINE_CHECK,TRUE));
        }
        SAFE_DELETE(m_pEventChecker);

        if (m_bCreateEventContinue)
        {
            SAFE_CLOSE_HANDLE(m_hEventContinue,NULL);
        }
        else
        {
            m_hEventContinue = NULL;
        }
        if (m_bCreateEventStop)
        {
            SAFE_CLOSE_HANDLE(m_hEventStop,NULL);
        }
        else
        {
            m_hEventStop = NULL;
        }
    }
    template <typename ThreadTraits>
    FTLThreadWaitType CFThread<ThreadTraits>::GetThreadWaitType(DWORD dwTimeOut /* = INFINITE */)
    {
        FTLThreadWaitType waitType = ftwtError;
        waitType = m_pEventChecker->GetWaitType(dwTimeOut);
        return waitType;
    }

    template <typename ThreadTraits>
    FTLThreadWaitType CFThread<ThreadTraits>::GetThreadWaitTypeEx(HANDLE* pUserHandles, DWORD nUserHandlCount, 
        DWORD* pResultHandleIndex, 
        BOOL  bCheckContinue /* = FALSE */,
        DWORD dwTimeOut /* = INFINITE */)
    {
        FTLThreadWaitType waitType = ftwtError;
        waitType = m_pEventChecker->GetWaitTypeEx(pUserHandles, nUserHandlCount, 
            pResultHandleIndex, bCheckContinue, dwTimeOut);
        return waitType;
    }

    template <typename ThreadTraits>
    FTLThreadWaitType CFThread<ThreadTraits>::SleepAndCheckStop(DWORD dwTimeOut)
    {
        FTLThreadWaitType waitType = ftwtError;
        waitType = m_pEventChecker->SleepAndCheckStop(dwTimeOut);
        return waitType;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::Start(LPTHREAD_START_ROUTINE pfnThreadProc,void *pvParam ,BOOL resetEvent/* = TRUE*/)
    {
        BOOL bRet = FALSE;
        FTLASSERT(pfnThreadProc);

        if (NULL == pfnThreadProc)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if( IsThreadRunning())
        {
            FTLASSERT(!TEXT("must stop thread before restart"));
            //SetLastError(ERROR_ALREADY_INITIALIZED);
            return TRUE;
        }

        if( m_hThread )
        {
            FTLASSERT(FALSE && TEXT("stop thread before call start again"));
            ::CloseHandle( m_hThread );
            m_hThread = NULL;
        }

        if (resetEvent)
        {
            ResetEvent(m_hEventStop);
            SetEvent(m_hEventContinue);
        }

        DWORD threadid = 0;
        StartThreadProxyStruct* pProxyStruct = new StartThreadProxyStruct;
        pProxyStruct->param = pvParam;
        pProxyStruct->pfn = pfnThreadProc;

        API_VERIFY(NULL != (m_hThread = ThreadTraits::CreateThread(NULL,0,
            StartThreadProxy,pProxyStruct,0,&threadid)));
        if( !m_hThread )
        {
            delete pProxyStruct;
            return FALSE;
        }
        //m_ThreadState = ftsRunning;
        return bRet;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::Stop()
    {
        BOOL bRet = TRUE;
		if (IsThreadRunning())
		{
			API_VERIFY(::SetEvent( m_hEventStop));
			// allow thread to run at higher priority during kill process
			::SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
		}
        return (bRet);
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::StopAndWait(DWORD dwTimeOut /* = FTL_MAX_THREAD_DEADLINE_CHECK */, 
        BOOL bCloseHandle /* = TRUE */, BOOL bTerminateIfTimeOut /* = TRUE */)
    {
        BOOL bRet = TRUE;
		API_VERIFY(Stop());
		API_VERIFY_EXCEPT1(Wait(dwTimeOut,bCloseHandle, bTerminateIfTimeOut), ERROR_TIMEOUT);
        return bRet;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::Pause()
    {
        BOOL bRet = FALSE;
        API_VERIFY(::ResetEvent(m_hEventContinue));
        return bRet;
    }
    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::Resume()
    {
        BOOL bRet = FALSE;
        API_VERIFY(::SetEvent(m_hEventContinue));
        return bRet;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::LowerPriority()
    {
        BOOL bRet = FALSE;
        int priority = ::GetThreadPriority(m_hThread); 
        API_VERIFY(::SetThreadPriority(m_hThread, priority - 1));
        return bRet;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::RaisePriority()
    {
        BOOL bRet = FALSE;
        int priority = ::GetThreadPriority(m_hThread); 
        API_VERIFY(::SetThreadPriority(m_hThread, priority + 1));
        return bRet;
    }

    template <typename ThreadTraits>
    int CFThread<ThreadTraits>::GetPriority() const
    {
        int priority = ::GetThreadPriority(m_hThread); 
        return priority;
    }


    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::SetPriority(int priority)
    {
        BOOL bRet = FALSE;
        API_VERIFY(::SetThreadPriority(m_hThread, priority));
        return bRet;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::IsThreadRunning() const
    {
        if( !m_hThread )
            return FALSE;

        DWORD	dwExitCode = 0;
        BOOL bRet = FALSE;
        //判断线程是否结束
        API_VERIFY(::GetExitCodeThread( m_hThread, &dwExitCode ));
        if (FALSE == bRet)
        {
            return FALSE;
        }
        bRet = (dwExitCode == STILL_ACTIVE ? TRUE : FALSE);
        return bRet;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::HadRequestStop() const
    {
        DWORD dwResult = WaitForSingleObject(m_hEventStop,0);
        BOOL bRet = (dwResult != WAIT_TIMEOUT);
        return bRet;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::HadRequestPause() const
    {
        DWORD dwResult = WaitForSingleObject(m_hEventContinue,0);
        BOOL bRet = (dwResult == WAIT_TIMEOUT);
        return bRet;
    }

    template <typename ThreadTraits>
    BOOL CFThread<ThreadTraits>::Wait(DWORD dwTimeOut/* = INFINITE*/,BOOL bCloseHandle/* = TRUE*/, 
		BOOL bTerminateIfTimeOut /* = TRUE */)
    {
        BOOL bRet = TRUE;
        if (NULL != m_hThread)
        {
            DWORD dwRet = WaitForSingleObject(m_hThread,dwTimeOut);
            switch(dwRet)
            {
            case WAIT_OBJECT_0:  //Stopped
                bRet = TRUE;
                break;
            case WAIT_TIMEOUT:
				FTLTRACEEX(tlWarn, TEXT("WARNING!!!: Wait For Thread %d(ms) TimeOut, Handle=0x%x"), 
					dwTimeOut, m_hThread);
				if (bTerminateIfTimeOut)
				{
					FTLASSERT(FALSE && TEXT("WaitFor Time Out"));
					TerminateThread(m_hThread, DWORD(-1));
				}
                bRet = FALSE;
				SetLastError(ERROR_TIMEOUT);
                break;
            default:
                FTLASSERT(FALSE);
                bRet = FALSE;
                break;
            }
            if (bCloseHandle)
            {
                SAFE_CLOSE_HANDLE(m_hThread,NULL);
            }
        }
        return bRet;
    }

}//namespace FTL

#endif //FTL_THREAD_HPP

