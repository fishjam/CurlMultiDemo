///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   ftlthread.h
/// @brief  Fishjam Template Library Thread Header File.
/// @author fujie
/// @version 0.6 
/// @date 03/30/2008
/// @defgroup ftlthread ftl thread function and class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_THREAD_H
#define FTL_THREAD_H
#pragma once

#ifndef FTL_BASE_H
#  error ftlthread.h requires ftlbase.h to be included first
#endif

#if defined(_MT)
#  include <process.h>    //for _beginthreadex
#endif

//#include <ftlFunctional.h>
//#include <ftlSharePtr.h>

#include <queue>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <memory>

namespace FTL
{
    //自动同步锁的基类
    class CFLockObject
    {
    public:
        virtual BOOL Lock(DWORD dwTimeout = INFINITE) = 0;
        virtual BOOL UnLock() = 0;
    };

    FTLEXPORT class CFNullLockObject : public CFLockObject
    {
    public:
        FTLINLINE virtual BOOL Lock(DWORD dwTimeout = INFINITE);
        FTLINLINE virtual BOOL UnLock();
    };

#ifdef FTL_DEBUG
#  define CRITICAL_SECTION_LOCK_TRACE(pCS, bTrace)  { pCS->SetTrace(bTrace);}
#else
#  define CRITICAL_SECTION_LOCK_TRACE(pCS, bTrace)
#endif

    //! class CFCriticalSection
    //! 用于线程同步的同步对象(临界区)
    FTLEXPORT class CFCriticalSection : public CFLockObject
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFCriticalSection);
    public:
        FTLINLINE CFCriticalSection();
        FTLINLINE ~CFCriticalSection();

        //一旦一个线程进入一个CS，它就能够一再地重复进入该CS,但需要保证 对应的UnLock
        FTLINLINE BOOL Lock(DWORD dwTimeout = INFINITE);
        FTLINLINE BOOL UnLock();
        FTLINLINE BOOL TryLock();
#ifdef FTL_DEBUG
        FTLINLINE BOOL IsLocked() const;
        FTLINLINE BOOL SetTrace(BOOL bTrace);
#endif
    private:
        CRITICAL_SECTION m_CritSec;
#ifdef FTL_DEBUG
        DWORD   m_currentOwner;     //指明是哪个线程锁定了该临界区
        BOOL    m_fTrace;           //指明是否要打印日志信息
        LONG    m_lockCount;        //用于跟踪线程进入关键代码段的次数,可以用于调试
#endif //FTL_DEBUG
    };

    //! 自动锁定同步对象, MFC 中 CSingleLock(&m_lockObj, TRUE) 表示构造函数中自动锁，析构自动解锁
    FTLEXPORT template<typename T = CFLockObject>
    class CFAutoLock
    {
    public:
        explicit FTLINLINE CFAutoLock<T>(T* pLockObj);
        FTLINLINE VOID Release();
        FTLINLINE ~CFAutoLock();
    private:
        T*   m_pLockObj;
    };

    //线程创建对象 -- TODO: MFC ?
    class CFCRTThreadTraits
    {
    public:
        static HANDLE CreateThread(LPSECURITY_ATTRIBUTES lpsa, DWORD dwStackSize, LPTHREAD_START_ROUTINE pfnThreadProc, void *pvParam, DWORD dwCreationFlags, DWORD *pdwThreadId) throw()
        {
            FTLASSERT(sizeof(DWORD) == sizeof(unsigned int)); // sanity check for pdwThreadId

            // _beginthreadex calls CreateThread which will set the last error value before it returns.
            return (HANDLE) _beginthreadex(lpsa, dwStackSize, (unsigned int (__stdcall *)(void *)) pfnThreadProc, pvParam, dwCreationFlags, (unsigned int *) pdwThreadId);
        }
    };

    class CFWin32ThreadTraits
    {
    public:
        static HANDLE CreateThread(LPSECURITY_ATTRIBUTES lpsa, DWORD dwStackSize, LPTHREAD_START_ROUTINE pfnThreadProc, void *pvParam, DWORD dwCreationFlags, DWORD *pdwThreadId) throw()
        {
            //CreateThread时C运行时库没有初始化
            return ::CreateThread(lpsa, dwStackSize, pfnThreadProc, pvParam, dwCreationFlags, pdwThreadId);
        }
    };
    //TODO:FTL中怎么判断默认创建者应该用谁？
#if defined(_MT)  //!defined(_ATL_MIN_CRT) && 
    typedef CFCRTThreadTraits DefaultThreadTraits;
#else
    typedef CFWin32ThreadTraits DefaultThreadTraits;
#endif

#ifdef _DEBUG
	#define FTL_MAX_THREAD_DEADLINE_CHECK   INFINITE
#else 
    #define FTL_MAX_THREAD_DEADLINE_CHECK   5000
#endif

    //! 模版线程类
    typedef enum tagFTLThreadWaitType
    {
        ftwtStop, 
        ftwtContinue,
        ftwtTimeOut,
        ftwtError,

        ftwtUserHandle,
    }FTLThreadWaitType;
    

    FTLEXPORT class CFEventChecker
    {
		DISABLE_COPY_AND_ASSIGNMENT(CFEventChecker);
    public:
        FTLINLINE CFEventChecker(HANDLE hEventStop,HANDLE hEventContinue);
        FTLINLINE ~CFEventChecker();
        FTLINLINE FTLThreadWaitType GetWaitType(DWORD dwTimeOut = INFINITE);
        FTLINLINE FTLThreadWaitType GetWaitTypeEx(HANDLE* pUserHandles, DWORD nUserHandlCount, 
            DWORD* pResultHandleIndex, BOOL  bCheckContinue = FALSE, DWORD dwTimeOut = INFINITE);
        FTLINLINE FTLThreadWaitType SleepAndCheckStop(DWORD dwTimeOut);
    private:
        HANDLE		    m_hEventStop;
        HANDLE          m_hEventContinue;
    };

    FTLEXPORT template <typename T>
    class CFSyncEventUtility
    {
    public:
        FTLINLINE CFSyncEventUtility(void);
        FTLINLINE ~CFSyncEventUtility(void);

        FTLINLINE void ClearAllEvent();
        FTLINLINE void AddEvent(T t, HANDLE hEvent = NULL);
        FTLINLINE void SetEvent(T t);
        FTLINLINE void ResetEvent(T t);
        FTLINLINE void SetAllEvent();
        FTLINLINE void ResetAllEvent();
        FTLINLINE BOOL WaitAllEvent(DWORD dwMilliseconds = INFINITE);
        FTLINLINE size_t GetSyncEventCount();
        T WaitOneEvent(DWORD dwMilliseconds /* = INFINITE */);
    private:
        mutable CFCriticalSection   m_LockObject;
        struct SyncEventInfo
        {
            BOOL            bCreateEvent;
            HANDLE          hEvent;
        };
        typedef std::map<T, SyncEventInfo> SYNC_EVENT_MAP;
        SYNC_EVENT_MAP  m_AllSyncEventMap;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 一个支持安全的 暂停、继续、停止 功能的线程框架
    FTLEXPORT template <typename ThreadTraits = DefaultThreadTraits>
    class CFThread
    {
		DISABLE_COPY_AND_ASSIGNMENT(CFThread);
    public:
        /// 构造函数，创建线程类实例
        /// @param [in] hEventStop 外部传入的在线程停止时激发的事件，如果为NULL，线程内部会自己创建一个。
        /// @param [in] hEventContinue 外部传入的保证线程能力运行的事件(如果该事件未被激发，则线程会等待--暂停)，
        ///                            如果为NULL，线程内部会自己创建一个
        /// @note  hEventStop 和 hEventContinue 都必须是手动重置事件
        FTLINLINE CFThread(HANDLE hEventStop = NULL,HANDLE hEventContinue = NULL);

        FTLINLINE virtual ~CFThread();
    public:
        FTLINLINE FTLThreadWaitType GetThreadWaitType(DWORD dwTimeOut = INFINITE);

        //可以等待用户提供的事件(必须是类似Stop一类的结束事件 -- 如等待进程结束等)
        FTLINLINE FTLThreadWaitType GetThreadWaitTypeEx(HANDLE* pUserHandles, 
            DWORD nUserHandlCount, 
            DWORD* pResultHandleIndex, 
            BOOL  bCheckContinue = FALSE,
            DWORD dwTimeOut  = INFINITE);
        FTLINLINE virtual FTLThreadWaitType SleepAndCheckStop(DWORD dwTimeOut);
        FTLINLINE BOOL Start(LPTHREAD_START_ROUTINE pfnThreadProc, void *pvParam, BOOL resetEvent = TRUE);
        FTLINLINE BOOL Wait(DWORD dwTimeOut = INFINITE, BOOL bCloseHandle = TRUE, BOOL bTerminateIfTimeOut = TRUE);
        FTLINLINE BOOL Stop();
        FTLINLINE BOOL StopAndWait(DWORD dwTimeOut = FTL_MAX_THREAD_DEADLINE_CHECK, BOOL bCloseHandle = TRUE,
			BOOL bTerminateIfTimeOut = TRUE );
        FTLINLINE BOOL Pause();
        FTLINLINE BOOL Resume();
        FTLINLINE BOOL LowerPriority();
        FTLINLINE BOOL RaisePriority();
        FTLINLINE int  GetPriority() const;
        FTLINLINE BOOL SetPriority(int priority);
        FTLINLINE BOOL IsThreadRunning() const;
        FTLINLINE BOOL HadRequestStop() const;
        FTLINLINE BOOL HadRequestPause() const;
    protected:
        //unsigned int	m_Id;			//!< Thread ID
        HANDLE		    m_hEventStop;
        HANDLE          m_hEventContinue;
        HANDLE		    m_hThread;		//!< Thread Handle
        CFEventChecker* m_pEventChecker;
    private:
        BOOL            m_bCreateEventStop;
        BOOL            m_bCreateEventContinue;
    };

}//namespace FTL

#endif //FTL_THREAD_H

#ifndef USE_EXPORT
#  include "ftlthread.hpp"
#endif
