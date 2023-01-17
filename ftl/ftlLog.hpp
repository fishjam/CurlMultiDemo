#ifndef FTL_LOG_HPP
#define FTL_LOG_HPP
#pragma once

#ifdef USE_EXPORT
#  include "ftlLog.h"
#endif

#  include <io.h>
#  include <fcntl.h>
#  include <atlsecurity.h>

#include "ftlSharedVariable.h"
#include "ftlConversion.h"
//#include <atlcoll.h>
#include <map>

namespace FTL
{
    CFLogSink::CFLogSink(BOOL isDebug)
    {
        m_bDebug = isDebug;
    }
    CFLogSink::~CFLogSink()
    {

    }

    CFLogFileSink::CFLogFileSink(BOOL isDebug)
        :CFLogSink(isDebug)
    {

    }

    CFLogFileSinkLowIO::CFLogFileSinkLowIO(BOOL isDebug)
        :CFLogFileSink(isDebug)
    {
        m_nLogFile = -1;
    }
    CFLogFileSinkLowIO::~CFLogFileSinkLowIO()
    {
        Close();
    }

    BOOL CFLogFileSinkLowIO::IsValid()
    {
        return (m_nLogFile != -1);
    }

    BOOL CFLogFileSinkLowIO::Open(LPCTSTR pszFilePath)
    {
        //_wfopen
        //_tfsopen()
        BOOL bRet = FALSE;

        //默认是 O_TEXT ? Win 上会自动把 \r 改成 \r\n, 然后造成计算大小时出现偏差
        int oFlags = O_WRONLY | O_BINARY | O_CREAT | O_APPEND;

        int errNo = _tsopen_s(&m_nLogFile, pszFilePath, oFlags, _SH_DENYNO, _S_IREAD | _S_IWRITE);
        ATLASSERT(m_nLogFile > 0);
        if (m_nLogFile > 0)
        {
            bRet = TRUE;
        } else {
            //常见错误:
            //  2(ERROR_FILE_NOT_FOUND) <== 路径不存在,比如优盘已经断开
            //  13(ERROR_WRITE_PROTECT) <== 比如路径不可写,比如不可写的优盘
            SetLastError(errNo);
        }

        if (m_bDebug)
        {
            ATLTRACE(TEXT("ftl open %s result, m_nLogFile=%d, errNo=%d\n"), pszFilePath, m_nLogFile, errNo);
        }

        return bRet;
    }

    BOOL CFLogFileSinkLowIO::WriteLogBuffer(LPCVOID pBuffer, LONG nBufLen, LONG* pWritten)
    {
        BOOL bRet = FALSE;
        //在 NFS 等共享磁盘上 flock(fd, LOCK_EX|LOCK_NB) +  lseek(fd, 0, SEEK_END)  + flock(fd, LOCK_UN)
        int nTotalWrite = 0;
        int nWrite = 0;
        LPCVOID pWriteStart = pBuffer;
        LONG nRemainWant = nBufLen;
        do 
        {
            nWrite = _write(m_nLogFile, pWriteStart, nRemainWant);
            if (nWrite <= 0) //会等于0吗, 目前实测, 似乎只有 -1
            {
                DWORD dwLastErr = GetLastError();
                DWORD nErrno = errno;
                //常见错误:EINTR == errno
                //  1.errno
                //    22( ) <== TODO: 到底是表示 EINVAL(linux) 还是 ERROR_WRONG_DISK(win 的 GetLastError )?
                //  2.GetLastError()
                //    112(ERROR_DISK_FULL): 磁盘满
                //    995(ERROR_OPERATION_ABORTED): 写的时候网盘断开
                if (m_bDebug)
                {
                    TCHAR szMsgBuf[256] = { 0 };
                    StringCchPrintf(szMsgBuf, _countof(szMsgBuf), TEXT("Write log error, want %d, real %d, errno=%d, dwLastErr=%d\n"),
                        nBufLen, nWrite, errno, dwLastErr);
                    OutputDebugStringW(szMsgBuf);

                    ::MessageBox(::GetDesktopWindow(), szMsgBuf, TEXT("Wait Debug"), MB_OK);
                }
                ATLASSERT(FALSE && TEXT("_write failed"));
                break;
            }
            nRemainWant -= nWrite;
            nTotalWrite += nWrite;
            pWriteStart = (char*)pWriteStart + nWrite;
            if (nRemainWant != 0 && m_bDebug)
            {
                TCHAR szMsgBuf[256] = { 0 };
                StringCchPrintf(szMsgBuf, _countof(szMsgBuf), TEXT("Write log failed, want %d, real %d, lastErr=%d"),
                    nBufLen, nWrite, GetLastError());
                ::MessageBox(::GetDesktopWindow(), szMsgBuf, TEXT("Wait Debug"), MB_OK);
            }
        } while (nRemainWant > 0 && nWrite > 0);
        
        bRet = (nBufLen == nTotalWrite);
        if (pWritten)
        {
            *pWritten = nTotalWrite;
        }
        return bRet;
    }

	LONG CFLogFileSinkLowIO::GetLength()
	{
		return (LONG)_filelengthi64(m_nLogFile);
	}

    BOOL CFLogFileSinkLowIO::Flush()
    {
        BOOL bRet = FALSE;
        ATLASSERT(FALSE);  //how to ?
        if (m_nLogFile)
        {
            //fsync()
            //ioctl(fd, IOCTL_COMMAND, args); 
        }
        return bRet;
    }

    BOOL CFLogFileSinkLowIO::Close()
    {
        BOOL bRet = TRUE;
        if (m_nLogFile > 0)
        {
            _close(m_nLogFile);
            m_nLogFile = -1;
        }
        return bRet;
    }

    struct PROCESS_LOG_INFO
    {
        DWORD PID;
        HANDLE hMutexLock;
        HANDLE hLogFile;
    };

    FTLINLINE BOOL CALLBACK LogShareInitializeProc(BOOL bFirstCreate, FTL_LOG_SHARE_INFO& rValue)
    {

        BOOL bRet = FALSE;
        if (bFirstCreate)
        {
            ZeroMemory(&rValue, sizeof(FTL_LOG_SHARE_INFO));
            rValue.metaInfo.nFileIndex = 0;
            rValue.metaInfo.dwVersion = FTL_LOG_VERSION;
#ifdef FTL_DEBUG
            rValue.metaInfo.dwFlags = FTL_LOG_META_FLAG_DEBUG;
#else 
            rValue.metaInfo.dwFlags = 0;
#endif 

#if 1
            //NDrive 中不启用这些代码
            rValue.metaInfo.logThreshold = tlInfo;
#ifdef FTL_DEBUG
            rValue.metaInfo.logTarget = (FTL_LOG_TARGET_DEBUG_VIEW | FTL_LOG_TARGET_LOCAL_FILE);
#else		
            rValue.metaInfo.logTarget = (FTL_LOG_TARGET_LOCAL_FILE);
#endif 
            rValue.metaInfo.dateTimeFmt = ltfTimeLow;   //通常来说 Low 就足够了
            rValue.metaInfo.maxLogSize = 50 * 1024 * 1024;  //50M
            rValue.metaInfo.maxTotalSize = 200 * 1024 * 1024;	//200M

            TCHAR szLogPath[MAX_PATH] = { 0 }, szModuleName[MAX_PATH] = { 0 };
            ATLVERIFY(GetTempPath(_countof(szLogPath), szLogPath) > 0);

            GetModuleFileName(NULL, szModuleName, _countof(szModuleName));
            PathRemoveExtension(szModuleName);
            LPCTSTR pszModuleFileName = ::PathFindFileName(szModuleName);
            ATLASSERT(NULL != pszModuleFileName);
            StringCchPrintf(rValue.metaInfo.szBaseName, _countof(rValue.metaInfo.szBaseName), TEXT("%s_"), pszModuleFileName);

            StringCchPrintf(rValue.metaInfo.szLockMutextName, _countof(rValue.metaInfo.szLockMutextName), TEXT("%s"), pszModuleFileName);

            FTL::CFStringFormater formater;
            ATLVERIFY(SUCCEEDED(formater.AppendFormat(TEXT("FTL\\")))); //, pszModuleFileName

            ATLVERIFY(PathAppend(szLogPath, formater.GetString()));
            bRet = (CreateDirectory(szLogPath, NULL));
            ATLASSERT(bRet || ERROR_ALREADY_EXISTS == GetLastError());


            StringCchCopy(rValue.metaInfo.szLogPath, _countof(rValue.metaInfo.szLogPath), szLogPath);
            //TEXT("F:\\Fujie\\FJSDK_Export\\Windows\\FTL\\Debug\\"));
            //TEXT("G:\\Fujie\\FJSDK\\Windows\\FTL\\Debug\\"));

#endif 
        }

        ATLTRACE(TEXT("Leave LogShare Initialize Proc, msgIndex=%d, curLogSize=%d\n"),
            rValue.metaInfo.msgIndex, rValue.metaInfo.curLogSize);

        return bRet;
    }

    FTLINLINE BOOL CALLBACK LogShareFinalizeProc(BOOL bFirstCreate, FTL_LOG_SHARE_INFO& rValue)
    {
        BOOL bRet = FALSE;
        ATLTRACE(TEXT("Enter Log Share Finalize Proc, msgIndex=%d, curLogSize=%d\n"),
            rValue.metaInfo.msgIndex, rValue.metaInfo.curLogSize);

        return bRet;
    }

    CFLogger::CFLogger(FTL_LOG_SHARE_INFO& rLogShareInfo)
        :m_rLogShareInfo(rLogShareInfo)
    {
        _CheckAndUpdateByMetaFlags(m_rLogShareInfo.metaInfo.dwFlags);
        m_nCurInstanceIndex = InterlockedIncrement(&CFLogger::s_LoggerInstanceCount);
        ATL::CSecurityDesc sd;
        ATL::CDacl dacl;
        dacl.AddAllowedAce(ATL::Sids::World(), GENERIC_ALL, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE);
        sd.SetDacl(dacl);

        SECURITY_ATTRIBUTES sa = { 0 };
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = (LPVOID)sd.GetPSECURITY_DESCRIPTOR();
        sa.bInheritHandle = FALSE;

        m_pLockObj = new ATL::CMutex(&sa, FALSE, rLogShareInfo.metaInfo.szLockMutextName);

        if (m_isDebug)
        {
            ATLTRACE(TEXT("Enter CFLogger::CFLogger, index=%d, total=%d, this=0x%p\n"),
                m_nCurInstanceIndex, s_LoggerInstanceCount, this);
        }

        m_pLogWritter = NULL;
        m_nCurFileIndex = -1;
        m_dwCurLogDate = -1;
        ZeroMemory(m_szLogFilePath, sizeof(m_szLogFilePath));
    }

    CFLogger::~CFLogger()
    {
        InterlockedDecrement(&CFLogger::s_LoggerInstanceCount);
        if (m_isDebug)
        {
            ATLTRACE(TEXT("Enter CFLogger::~CFLogger, index=%d, remain=%d, this=0x%p\n"),
                m_nCurInstanceIndex, s_LoggerInstanceCount, this);
        }

        Close(TRUE);

        if (m_pLogWritter)
        {
            //在 nDrive 项目里面, 由于存在全局变量, 在日志模块释放完以后,全局变量的析构还会再次调用日志模块,
            //因此此处不能 delete, 否则会造成空指针异常, 此处的问题就是可能会有内存泄漏( static 变量, 内存泄漏不高)
            m_pLogWritter->Close();

            delete m_pLogWritter;
            m_pLogWritter = NULL;
        }

        SAFE_DELETE(m_pLockObj);
    }

    BOOL CFLogger::GetTraceOptions(LoggerTraceOptions& traceOptions)
    {
        ATL::CMutexLock locker(*m_pLockObj);
        BOOL bFillOptions = FALSE;

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_FLAGS))
        {
            traceOptions.dwMetaFlags = m_rLogShareInfo.metaInfo.dwFlags;
            bFillOptions = TRUE;
        }

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_LOG_PATH))
        {
            traceOptions.pszLogPath = m_rLogShareInfo.metaInfo.szLogPath;
            bFillOptions = TRUE;
        }
        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_LOG_THRESHOLD))
        {
            traceOptions.logThreshold = m_rLogShareInfo.metaInfo.logThreshold;
            bFillOptions = TRUE;
        }
        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_DATETIME_FMT))
        {
            traceOptions.dateTimeFmt = m_rLogShareInfo.metaInfo.dateTimeFmt;
            bFillOptions = TRUE;
        }
        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_LOG_TARGET))
        {
            traceOptions.logTarget = m_rLogShareInfo.metaInfo.logTarget;
            bFillOptions = TRUE;
        }

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_MAX_LOG_FILE_SIZE))
        {
            traceOptions.maxLogFileSize = m_rLogShareInfo.metaInfo.maxLogSize;
            bFillOptions = TRUE;
        }

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_MAX_TOTAL_LOG_SIZE))
        {
            traceOptions.maxTotalLogSize = m_rLogShareInfo.metaInfo.maxTotalSize;
            bFillOptions = TRUE;
        }

        ATLASSERT(bFillOptions);  //please set the dwOptionsFlags before get options
        if (!bFillOptions)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        return bFillOptions;
    }

    VOID CFLogger::SetTraceOptions(const LoggerTraceOptions& traceOptions)
    {
        ATL::CMutexLock locker(*m_pLockObj);

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_FLAGS))
        {
            m_rLogShareInfo.metaInfo.dwFlags = traceOptions.dwMetaFlags;

            _CheckAndUpdateByMetaFlags(m_rLogShareInfo.metaInfo.dwFlags);
        }

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_LOG_PATH) && traceOptions.pszLogPath)
        {
            StringCchCopy(m_rLogShareInfo.metaInfo.szLogPath, _countof(m_rLogShareInfo.metaInfo.szLogPath), traceOptions.pszLogPath);
        }
        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_LOG_THRESHOLD))
        {
            m_rLogShareInfo.metaInfo.logThreshold = traceOptions.logThreshold;
        }
        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_DATETIME_FMT))
        {
            m_rLogShareInfo.metaInfo.dateTimeFmt = traceOptions.dateTimeFmt;
        }

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_LOG_TARGET))
        {
            m_rLogShareInfo.metaInfo.logTarget = traceOptions.logTarget;
        }

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_MAX_LOG_FILE_SIZE))
        {
            m_rLogShareInfo.metaInfo.maxLogSize = traceOptions.maxLogFileSize;
        }

        if ((traceOptions.dwOptionsFlags & LOGGER_TRACE_OPTION_MAX_TOTAL_LOG_SIZE))
        {
            m_rLogShareInfo.metaInfo.maxTotalSize = traceOptions.maxTotalLogSize;
        }
    }

    BOOL CFLogger::CheckLevel(TraceLevel level)
    {
        BOOL bRet = (level >= m_rLogShareInfo.metaInfo.logThreshold);
        return bRet;
    }

    LPCTSTR CFLogger::GetLevelName(TraceLevel level)
    {
        LPCTSTR pszLevelName = TEXT("U");  //Unknown
        switch (level)
        {
        case tlDetail:
            pszLevelName = TEXT("D");
            break;
        case tlInfo:
            pszLevelName = TEXT("I");
            break;
        case tlTrace:
            pszLevelName = TEXT("T");
            break;
        case tlWarn:
            pszLevelName = TEXT("W");
            break;
        case tlError:
            pszLevelName = TEXT("E");
            break;
        default:
            ATLASSERT(FALSE);
            break;
        }
        return pszLevelName;
    }

    ULONG CFLogger::GetNextTraceSequenceNumber()
    {
        ULONG nNextSeq = InterlockedIncrement(&m_rLogShareInfo.metaInfo.msgIndex);
        return nNextSeq;
    }

    SYSTEMTIME CFLogger::GetLocalTimeAsSystemTimeWithMicroSec(BOOL highPrecision, DWORD& dwOutMicroSec)
    {
		const int TIME_RESULT_TO_SECOND = (1000 * 1000 * 10);
        SYSTEMTIME  localSysTime = { 0 };
		
        if (highPrecision)
        {
            //TODO: 是否有更好的方法来获得微秒级的时间
            //  没有 GetLocalTimeAsFileTime() 一类的方法
            FILETIME    utcFileTime = { 0 }, localFileTime = { 0 };
            GetSystemTimeAsFileTime(&utcFileTime);  //可以得到 微秒级的时间

            ATLVERIFY(FileTimeToLocalFileTime(&utcFileTime, &localFileTime));

            //必须要转换成 LONGLONG 才能获取微秒数
            LARGE_INTEGER largeInt = { 0 };
            largeInt.HighPart = localFileTime.dwHighDateTime;
            largeInt.LowPart = localFileTime.dwLowDateTime;

            dwOutMicroSec = largeInt.QuadPart % TIME_RESULT_TO_SECOND / 10;		//微秒数
            largeInt.QuadPart = largeInt.QuadPart / TIME_RESULT_TO_SECOND * TIME_RESULT_TO_SECOND;  //取整,只保留到秒数

            ATLVERIFY(FileTimeToSystemTime(&localFileTime, &localSysTime));
        }
        else {
            GetLocalTime(&localSysTime);
            dwOutMicroSec = localSysTime.wMilliseconds * 1000;  //将其从毫秒转换为微妙
        }
        return localSysTime;
    }

    LPCWSTR CFLogger::GetCurrentTimeStrW(LPWSTR pszTimeBuf, INT nTimeBufCount)
    {
        HRESULT hr = E_FAIL;

        SYSTEMTIME curSysTime = { 0 };
        DWORD dwMicroSec = 0;

        const LogDateTimeFormat curDateTimeFormat = m_rLogShareInfo.metaInfo.dateTimeFmt;
        if (ltfTicket == curDateTimeFormat)
        {
            hr = StringCchPrintf(pszTimeBuf, nTimeBufCount, L"%d", GetTickCount());
        }
        //需要高精度的微秒级别时间戳
        if (ltfDateTimeHigh == curDateTimeFormat || ltfTimeHigh == curDateTimeFormat)
        {
            SYSTEMTIME curSysTime = GetLocalTimeAsSystemTimeWithMicroSec(TRUE, dwMicroSec);
            if (ltfDateTimeHigh == curDateTimeFormat)
            {
                hr = StringCchPrintfW(pszTimeBuf, nTimeBufCount, L"%04d-%02d-%02d %02d:%02d:%02d.%06d",
                    curSysTime.wYear, curSysTime.wMonth, curSysTime.wDay,
                    curSysTime.wHour, curSysTime.wMinute, curSysTime.wSecond, dwMicroSec);
            }
            else 
            {
                hr = StringCchPrintfW(pszTimeBuf, nTimeBufCount, L"%02d:%02d:%02d.%06d",
                    curSysTime.wHour, curSysTime.wMinute, curSysTime.wSecond, dwMicroSec);
            }
        }
        else if (ltfDateTimeLow == curDateTimeFormat || ltfTimeLow == curDateTimeFormat)
        {
            SYSTEMTIME curSysTime = GetLocalTimeAsSystemTimeWithMicroSec(FALSE, dwMicroSec);
            if (ltfDateTimeLow == curDateTimeFormat)
            {
                hr = StringCchPrintfW(pszTimeBuf, nTimeBufCount, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
                    curSysTime.wYear, curSysTime.wMonth, curSysTime.wDay,
                    curSysTime.wHour, curSysTime.wMinute, curSysTime.wSecond, curSysTime.wMilliseconds);
            }
            else
            {
                hr = StringCchPrintfW(pszTimeBuf, nTimeBufCount, L"%02d:%02d:%02d.%03d",
                    curSysTime.wHour, curSysTime.wMinute, curSysTime.wSecond, curSysTime.wMilliseconds);
            }
        }
        else {
            ATLASSERT(FALSE && TEXT("new date time format"));
        }

        ATLASSERT(SUCCEEDED(hr));
        return pszTimeBuf;
    }

    LPCSTR CFLogger::GetCurrentTimeStrA(LPSTR pszTimeBuf, INT nTimeBufCount)
    {
        WCHAR szTimeStr[32] = { 0 };
        GetCurrentTimeStrW(szTimeStr, _countof(szTimeStr));

        //不会出错
        ATLVERIFY(0 != WideCharToMultiByte(CP_UTF8, 0, szTimeStr, -1, pszTimeBuf, nTimeBufCount, NULL, NULL));

        return pszTimeBuf;
    }

    BOOL /*__cdecl*/ CFLogger::WriteLogInfoExW(TraceLevel level, const LPCWSTR lpszFormat, ...)
    {
        BOOL bRet = TRUE;
        ATL::CAtlStringW strFormater;
        
        va_list argsp;
        va_start(argsp, lpszFormat);
        strFormater.FormatV(lpszFormat, argsp);
        //strAtl.FormatMessageV(lpszFormat, argsp);
        va_end(argsp);

        if (FTL_LOG_TARGET_DEBUG_VIEW & m_rLogShareInfo.metaInfo.logTarget)
        {
            OutputDebugStringW(strFormater.GetString());
        }
        if (FTL_LOG_TARGET_LOCAL_FILE & m_rLogShareInfo.metaInfo.logTarget)
        {
            FTL::CFConversion conv;
            INT nLength = 0;
            LPCSTR pszUTF8 = conv.UTF16_TO_UTF8(strFormater.GetString(), &nLength);

            BOOL bFlush = (level >= tlError);
            bRet = (nLength == WriteLogBuffer(pszUTF8, nLength, bFlush));
        }
        return bRet;
    }

    BOOL /*__cdecl*/ CFLogger::WriteLogInfoExA(TraceLevel level, const LPCSTR lpszFormat, ...)
    {
        BOOL bRet = TRUE;
        ATL::CAtlStringA formater;

        va_list argsp;
        va_start(argsp, lpszFormat);
        formater.FormatV(lpszFormat, argsp);
        //strAtl.AppendFormatV(lpszFormat, argsp);
        va_end(argsp);

        if (FTL_LOG_TARGET_DEBUG_VIEW & m_rLogShareInfo.metaInfo.logTarget)
        {
            OutputDebugStringA(formater.GetString());
        }
        if (FTL_LOG_TARGET_LOCAL_FILE & m_rLogShareInfo.metaInfo.logTarget)
        {
            //BOOL bFlush = (level >= tlError);
            INT nLength = formater.GetLength();
            bRet = (nLength == WriteLogBuffer(formater.GetString(), nLength, FALSE));
        }
        return bRet;
    }


    VOID CFLogger::Close(BOOL bFlush)
    {
        BOOL bRet = FALSE;

        ATL::CMutexLock locker(*m_pLockObj);
        if (m_pLogWritter)
        {
            if (bFlush)
            {
                Flush();
            }
            m_pLogWritter->Close();
        }
        m_nCurFileIndex = -1;
        m_dwCurLogDate = -1;
    }

    BOOL CFLogger::_Rotate(BOOL bFlush, BOOL bIncFileIndex)
    {
        BOOL bRet = FALSE;
        if (m_isDebug)
        {
            ATLTRACE(TEXT("Enter _Rotate, bFlush=%d, bIncFileIndex=%d msgndex=%d, fileIndex=%d, curLogSize=%d\n"),
                bFlush, bIncFileIndex, m_rLogShareInfo.metaInfo.msgIndex,
                m_rLogShareInfo.metaInfo.nFileIndex, m_rLogShareInfo.metaInfo.curLogSize);
        }

        Close(bFlush);
        _CleanOldLogs();

        m_rLogShareInfo.metaInfo.curLogSize = 0;
        if (bIncFileIndex)
        {
            InterlockedIncrement(&m_rLogShareInfo.metaInfo.nFileIndex);
        }
        bRet = (_CreateLogFile());
        ATLASSERT(bRet);
        return bRet;
    }

    FTLINLINE DWORD CFLogger::_GetCurLogDateInfo(const SYSTEMTIME& st)
    {
        DWORD dwCheckLogDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay * 1; // +st.wHour * 1;
        //以下部分用于调试时加速日志的切换
        //DWORD dwCheckLogDate = st.wMonth * 1000000 + st.wDay * 10000 + st.wHour * 100 + st.wMinute * 1;
        return dwCheckLogDate;
    }

    FTLINLINE BOOL CFLogger::_CleanOldLogs()
    {
        HRESULT hr = E_FAIL;
        TCHAR szFindLogFiles[MAX_PATH] = { 0 };
        ULONG nTotalLogSize = 0;

        std::map<ATL::CAtlString, DWORD>  fileSizeMap;

        hr = StringCchPrintf(szFindLogFiles, _countof(szFindLogFiles), TEXT("%s%s") TEXT("*.log"),
            m_rLogShareInfo.metaInfo.szLogPath, m_rLogShareInfo.metaInfo.szBaseName);

        WIN32_FIND_DATA findData = { 0 };
        HANDLE hFindFile = FindFirstFile(szFindLogFiles, &findData);
        if (INVALID_HANDLE_VALUE != hFindFile)
        {
            do
            {
                ATLASSERT(0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)); // file
                if (0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    ATL::CAtlString fullPath;
                    fullPath.Format(TEXT("%s%s"), m_rLogShareInfo.metaInfo.szLogPath, findData.cFileName);
                    //fileSizeMap.SetAt(fullPath, )
                    fileSizeMap[fullPath] = findData.nFileSizeLow;  //small than 4G
                    nTotalLogSize += (ULONG)findData.nFileSizeLow;
                }
            } while (::FindNextFile(hFindFile, &findData));
            ATLVERIFY(FindClose(hFindFile));
            hFindFile = INVALID_HANDLE_VALUE;
        }

        while (nTotalLogSize > m_rLogShareInfo.metaInfo.maxTotalSize && !fileSizeMap.empty())
        {
            ATL::CAtlString strFirstFilePath = fileSizeMap.begin()->first;
            ULONG nFirstFileSize = fileSizeMap.begin()->second;

            if (m_isDebug)
            {
                ATLTRACE(TEXT("cur Total Log Size(%u) > allow maxTotalSize(%u), will remove %s(%d)\n"),
                    nTotalLogSize, m_rLogShareInfo.metaInfo.maxTotalSize, strFirstFilePath, nFirstFileSize);
            }

            ::DeleteFile(strFirstFilePath);	    //有可能会被占用,而造成删除失败, 如果失败则不要断言
            nTotalLogSize -= nFirstFileSize;  //不管是否成功都认为成功,否则删除失败时(如很久不写日志的进程还占用文件),会造成无限循环

            fileSizeMap.erase(fileSizeMap.begin());
        }

        return TRUE;
    }


    LONG CFLogger::WriteLogBuffer(LPCSTR pszUtf8, INT nLength, BOOL bFlush)
    {
        BOOL bRet = FALSE;
        //FTL::CFConversion conv;
        //LPCSTR pszUtf8 = conv.TCHAR_TO_UTF8(pszMessage, &nLength);

        {
            ATL::CMutexLock locker(*m_pLockObj);

            //放在这里,才能保证能获取到设置的 m_isDebug 值
            if (NULL == m_pLogWritter)
            {
                m_pLogWritter = new CFLogFileSinkLowIO(m_isDebug);
            }

            SYSTEMTIME st = { 0 };
            GetLocalTime(&st);

            //当前文件达到单个文件最大(先写, 再关,从而保证其他进程可以检测到这个文件不能再打开继续写)
            BOOL isLarger = (m_rLogShareInfo.metaInfo.maxLogSize > 0 && m_rLogShareInfo.metaInfo.curLogSize + nLength > m_rLogShareInfo.metaInfo.maxLogSize);

            //TODO: 当一个很久都没有写日志的进程再次尝试写时
            BOOL curIsLarger = (m_rLogShareInfo.metaInfo.maxLogSize > 0 && m_rLogShareInfo.metaInfo.curLogSize > m_rLogShareInfo.metaInfo.maxLogSize);

            ///按日期风格检查的话, 已经变化了,需要先关再写
            DWORD dwCheckLogDate = _GetCurLogDateInfo(st);
            BOOL isLogDataTimeChanged = (m_dwCurLogDate != -1 && m_dwCurLogDate != dwCheckLogDate);

            // 整体的文件索引已经变化, 当前正在写的文件索引不同,则需要更改(先关, 再写), 比如一个很久都没有写日志的进程再次尝试写时会出现这种情况
            BOOL hasChangedByOtherProcess = (m_nCurFileIndex != -1 && m_nCurFileIndex != m_rLogShareInfo.metaInfo.nFileIndex);

            BOOL bNeedWriteAfterRotate = !hasChangedByOtherProcess;

            //逻辑分类:
            //  1.正常写;
            //  2.写了以后关闭: 单个文件超过大小限制,写入以后, 其他进程再次打开对应文件, 才可以知道已经切换; 会造成超过阈值的 bug
            //  3.关闭以后再写: 当前文件已经足够大了 或 检测到其他进程已经切换了
            //do
            {
                //日期时间已经变化
                if (isLogDataTimeChanged)
                {
                    if (m_isDebug)
                    {
                        ATLTRACE(TEXT("dataTime changed: %d => %d, reset index from %d to 0\n"),
                            m_dwCurLogDate, dwCheckLogDate, m_rLogShareInfo.metaInfo.nFileIndex);
                    }
                    _Rotate(TRUE, FALSE);
                    m_rLogShareInfo.metaInfo.nFileIndex = 0;
                }
                if (hasChangedByOtherProcess)
                {
                    if (m_isDebug)
                    {
                        ATLTRACE(TEXT("has changed by other process:%d => %d\n"),
                            m_nCurFileIndex, m_rLogShareInfo.metaInfo.nFileIndex);
                    }
                    _Rotate(FALSE, FALSE);
                    //Close(FALSE);  //注意: 已经改变, 不能将缓冲 flush 到当前指向的文件中
                }
                if (curIsLarger) {
                    if (m_isDebug)
                    {
                        ATLTRACE(TEXT("cur file is larger, curLogSize=%d\n"), m_rLogShareInfo.metaInfo.curLogSize);
                    }
                    _Rotate(FALSE, FALSE);
                    //Close(FALSE);  //不能将缓冲 flush 到当前指向的文件中
                }
                //if (bNeedWriteAfterRotate)
                {
                    if (!m_pLogWritter->IsValid())
                    {
                        _CleanOldLogs();
                        bRet = _CreateLogFile();
                        ATLASSERT(bRet);
                    }

                    if (m_pLogWritter->IsValid())
                    {
                        LONG nWrittenLength = 0;
                        if (m_rLogShareInfo.metaInfo.nMsgLength > 0
                            && nLength + m_rLogShareInfo.metaInfo.nMsgLength >= LOG_MSG_BUFFER_SIZE)
                        {
                            //缓冲区满了, 输出
                            bRet = Flush();
                            ATLASSERT(bRet);
                        }

                        //如果缓冲区足够大,则拷贝到缓冲区, 否则直接输出
                        if (nLength < LOG_MSG_BUFFER_SIZE)
                        {
                            memcpy(m_rLogShareInfo.szMsgBuffer + m_rLogShareInfo.metaInfo.nMsgLength,
                                pszUtf8, nLength);
                            m_rLogShareInfo.metaInfo.nMsgLength += nLength;
                            nWrittenLength = nLength;
                            bRet = TRUE;
                        }
                        else {
                            bRet = m_pLogWritter->WriteLogBuffer(pszUtf8, nLength, &nWrittenLength);
                            ATLASSERT(bRet);
                            ATLASSERT(nLength == nWrittenLength);
                        }

                        if (bRet)
                        {
                            InterlockedExchangeAdd((LONG*)&m_rLogShareInfo.metaInfo.curLogSize, nWrittenLength);
                        }
                    }

                    if (isLarger)
                    {
                        if (m_isDebug)
                        {
                            ATLTRACE(TEXT("isLarger, index=%d/%d, curLogSize=%d, nLength=%d, m_rLogShareInfo.nMsgLength=%d\n"),
                                m_nCurFileIndex, m_rLogShareInfo.metaInfo.nFileIndex,
                                m_rLogShareInfo.metaInfo.curLogSize, nLength, m_rLogShareInfo.metaInfo.nMsgLength);
                        }
                        _Rotate(TRUE, TRUE);
                        //Close(TRUE);
                        //API_VERIFY(_CreateLogFile());
                    }
                }
                //} while (hasChangedByOtherProcess);
            }
            //fwrite(pszUtf8, 1, nLength, m_pLogFile);
        }

        DWORD dwCurTicket = GetTickCount();
        if (bFlush ||	//要求强制刷新(比如 tlError)
            (dwCurTicket < m_rLogShareInfo.metaInfo.lastFlushTime  //溢出
                || dwCurTicket >= m_rLogShareInfo.metaInfo.lastFlushTime + FTL_LOG_FLUSH_MAX_INTERVAL)  //超出阈值
            )
        {
            bRet = Flush();
            ATLASSERT(bRet);
        }

        return nLength;
    }

    BOOL CFLogger::Flush()
    {
        BOOL bRet = TRUE;
        ATL::CMutexLock locker(*m_pLockObj);
        ATLASSERT(m_pLogWritter);

        m_rLogShareInfo.metaInfo.lastFlushTime = GetTickCount();

        //ATLTRACE(TEXT("Enter Flush, nMsgLength=%d, isValid=%d\n"), m_rLogShareInfo.metaInfo.nMsgLength, m_pLogWritter->IsValid());
        if (m_rLogShareInfo.metaInfo.nMsgLength > 0 && m_pLogWritter->IsValid())
        {
            //FTLASSERT(FALSE); //如果禁用自定义缓冲区的话,就不会到这里, 因为 nMsgLength 始终是 0

            LONG nWriteBuffer = 0;
            bRet = m_pLogWritter->WriteLogBuffer(m_rLogShareInfo.szMsgBuffer,
                m_rLogShareInfo.metaInfo.nMsgLength, &nWriteBuffer);
            ATLASSERT(bRet);

            if (m_rLogShareInfo.metaInfo.nMsgLength != nWriteBuffer)
            {
                ATLTRACE(TEXT("!!! Flush want write %d, real write %d, err=%d\n"),
                    m_rLogShareInfo.metaInfo.nMsgLength, nWriteBuffer, GetLastError());
            }
            ATLASSERT(m_rLogShareInfo.metaInfo.nMsgLength == nWriteBuffer);

            //disk full?

            //TODO: 实际上只需要设置 nMsgLength 即可,但清零可以保证没有垃圾,方便查看问题
            ZeroMemory(m_rLogShareInfo.szMsgBuffer, LOG_MSG_BUFFER_SIZE);
            m_rLogShareInfo.metaInfo.nMsgLength = 0;
        }


        return bRet;
    }

    BOOL CFLogger::_CheckAndUpdateByMetaFlags(DWORD dwMetaFlags)
    {
        m_isDebug = (dwMetaFlags & FTL_LOG_META_FLAG_DEBUG);

        return TRUE;
    }

    BOOL CFLogger::_CreateLogFile()
    {
        BOOL bRet = FALSE;
        HRESULT hr = E_FAIL;
        TCHAR szCheckLogFilePath[MAX_PATH] = { 0 };

        SYSTEMTIME st = { 0 };
        GetLocalTime(&st);
        DWORD dwCheckLogDate = _GetCurLogDateInfo(st);

#define TRY_CREATE_LOG_COUNT  1
        BOOL bCreateNewLogFile = FALSE;
        LONG maxTryCount = 10;      //最多尝试创建文件多少次(注意:如果再次启动程序时, nFileIndex 会被重置,可能有问题, 尝试保留或计算 nFileIndex ?)
        ULONG nCurFileSize = 0;

        LONG nTryFileIndex = m_rLogShareInfo.metaInfo.nFileIndex;
        do
        {
            if (nTryFileIndex >= 1)
            {
                hr = StringCchPrintf(szCheckLogFilePath, _countof(szCheckLogFilePath), TEXT("%s%s") TEXT("%d_%04u.log"),
                    m_rLogShareInfo.metaInfo.szLogPath, m_rLogShareInfo.metaInfo.szBaseName,
                    dwCheckLogDate, nTryFileIndex);
            } else {
                hr = StringCchPrintf(szCheckLogFilePath, _countof(szCheckLogFilePath), TEXT("%s%s") TEXT("%d.log"),
                    m_rLogShareInfo.metaInfo.szLogPath, m_rLogShareInfo.metaInfo.szBaseName,
                    dwCheckLogDate);
            }

            ATLASSERT(SUCCEEDED(hr));
            {
                nCurFileSize = 0;
                HANDLE hCheckFile = ::CreateFile(szCheckLogFilePath,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE != hCheckFile)
                {
                    LARGE_INTEGER nFileSize = { 0 };
                    if (::GetFileSizeEx(hCheckFile, &nFileSize)) {
                        nCurFileSize = (ULONG)nFileSize.QuadPart;
                    }
                    CloseHandle(hCheckFile);
                    hCheckFile = NULL;
                }
                if (m_isDebug)
                {
                    //注意: 如果多个进程操作的是同一个文件, A 进程写了(设置了内存中的长度,但没有 Flush), B进程无法准确获取到 真实长度. nCurFileSize 可能会小于实际值
                    ATLTRACE(TEXT("check %s, maxTryCount=%d, nCurFileSize=%d, m_rLogShareInfo { curLogSize=%d, msgLength=%d maxSize=%d }\n"),
                        szCheckLogFilePath, maxTryCount, nCurFileSize, m_rLogShareInfo.metaInfo.curLogSize,
                        m_rLogShareInfo.metaInfo.nMsgLength, m_rLogShareInfo.metaInfo.maxLogSize);
                }

                if (m_rLogShareInfo.metaInfo.maxLogSize > 0 && nCurFileSize >= m_rLogShareInfo.metaInfo.maxLogSize)
                {
                    nTryFileIndex = InterlockedIncrement(&m_rLogShareInfo.metaInfo.nFileIndex);
                    continue;
                }

                bRet = m_pLogWritter->Open(szCheckLogFilePath);
                ATLASSERT(bRet);
                if (bRet)
                {
                    hr = StringCchCopy(m_szLogFilePath, _countof(m_szLogFilePath), szCheckLogFilePath);
                    ATLASSERT(SUCCEEDED(hr));

                    //FTLASSERT(m_rLogShareInfo.curLogSize == 0); // 如果 Close + _CreateLogFile 不是原子操作, 这个断言可能会不满足(其他进程写了)
                    // 出现这个断言, 可以考虑在 425 行的 API_VERIFY(_CreateLogFile());
                    if (0 == m_rLogShareInfo.metaInfo.curLogSize)
                    {
                        m_rLogShareInfo.metaInfo.curLogSize = nCurFileSize; //第一次启动
                    }

                    //m_nLocalCurLogSize = nCurFileSize;
                    //nTryFileIndex = m_rLogShareInfo.nFileIndex;
                    bCreateNewLogFile = TRUE;
                }
                else {
                    nTryFileIndex = InterlockedIncrement(&m_rLogShareInfo.metaInfo.nFileIndex);
                }
            }
        } while (!bCreateNewLogFile && --maxTryCount >= 0);

        if (m_pLogWritter->IsValid())
        {
			if (0 == m_pLogWritter->GetLength()) {
				//刚刚创建,则写入 UTF8 的 BOM 头

				UCHAR bufUTF8ROM[] = { 0xEF , 0xBB , 0xBF };
				LONG nLength = 0;
				bRet = m_pLogWritter->WriteLogBuffer(bufUTF8ROM, sizeof(bufUTF8ROM), &nLength);

				ATLASSERT(bRet);
				ATLASSERT(sizeof(bufUTF8ROM) == nLength);
				ATLASSERT(m_pLogWritter->GetLength() == sizeof(bufUTF8ROM));

				nCurFileSize += sizeof(bufUTF8ROM);
			}
            m_dwCurLogDate = dwCheckLogDate;
            if (m_isDebug)
            {
                ATLTRACE(TEXT("Create log file successful, index=%d, nCurFileSize=%d, m_dwCurLogDate=%d, %s\n"),
                    nTryFileIndex, nCurFileSize, m_dwCurLogDate, szCheckLogFilePath);
            }
            m_nCurFileIndex = nTryFileIndex;
        }
        else {
            ATLTRACE(TEXT("Create log file fail, err=%d, maxTryCount=%d, path=%s\n"), 
                GetLastError(), maxTryCount, szCheckLogFilePath);
        }

        return bRet;
    }

    __declspec(selectany) FTL::CFSharedVariableT<FTL_LOG_SHARE_INFO> g_GlobalLogShareInfo(
        LogShareInitializeProc, LogShareFinalizeProc, NULL);

    __declspec(selectany) ULONG FTL::CFLogger::s_LoggerInstanceCount = 0L;
    //__declspec(selectany) FTL_LOG_SHARE_INFO*  CFLogger::s_pFtlLogShareInfo = NULL;
    __declspec(selectany) CFLogger CFLogger::s_Logger(g_GlobalLogShareInfo.GetShareValue());


    CFLogger& CFLogger::GetInstance()
    {
        //static CFLogger loggerInstance(g_GlobalLogShareInfo.GetShareValue());
        //return loggerInstance;
        return s_Logger;
    }

    LPCWSTR CFLogger::GetBaseNameW(LPCWSTR pszFilePath)
    {
        LPCWSTR pszBaseName = pszFilePath;
        //return full path for debug, only file for release
#ifndef _DEBUG
        if (pszFilePath)
        {
            pszBaseName = PathFindFileNameW(pszFilePath);
            if (!pszBaseName)
            {
                pszBaseName = pszFilePath;
            }
        }
#endif 
        return pszBaseName;
    }

    LPCSTR CFLogger::GetBaseNameA(LPCSTR pszFilePath)
    {
        LPCSTR pszBaseName = pszFilePath;
        //return full path for debug, only file for release
#ifndef _DEBUG
        if (pszFilePath)
        {
            pszBaseName = PathFindFileNameA(pszFilePath);
            if (!pszBaseName)
            {
                pszBaseName = pszFilePath;
            }
        }
#endif 
        return pszBaseName;
    }

}
#endif //FTL_LOG_HPP