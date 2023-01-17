///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   ftlbase.hpp
/// @brief  Fishjam Template Library Base Implemention File.
/// @author fujie
/// @version 0.6 
/// @date 03/30/2008
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_BASE_HPP
#define FTL_BASE_HPP
#pragma once

#ifdef USE_EXPORT
#  include "ftlbase.h"
#endif

#include <stdint.h>

namespace FTL
{

    LPTSTR FtlPathFindFileName(LPCTSTR pPath)
    {
        LPCTSTR pszFileName = pPath;
        if (pszFileName)
        {
            LPCTSTR pszLastSlash = pszFileName;
            while (NULL != *pszLastSlash )
            {
                pszLastSlash++;
                if (_T('\\') == *pszLastSlash || _T('/') == *pszLastSlash)
                {
                    pszFileName = (pszLastSlash+1);
                }
            }
        }
        return (LPTSTR)pszFileName;
    }

    void FtlPathRemoveExtension(LPTSTR pszPath)
    {
        LPTSTR pszTmpPos = FtlPathFindFileName(pszPath);
        if (pszTmpPos)
        {
            LPTSTR pszLastDot = NULL;
            while (NULL != *pszTmpPos)
            {
                if (_T('.') == *pszTmpPos)
                {
                    pszLastDot = pszTmpPos;
                }
                pszTmpPos++;
            }
            if (pszLastDot != NULL)
            {
                *pszLastDot = NULL;
            }
        }
    }

    template<typename T>
    BOOL IsSameNumber(const T& expected, const T& actual, const T& delta)
    {
        if (FTL_ABS(expected - actual) <= delta)
        {
            return TRUE;
        }
        return FALSE;
    }

    template<typename T>
    void SwapValue(T& value1, T& value2)
    {
        T temp =  value1;
        value1 = value2;
        value2 = temp;
    }

    CFLastErrorRecovery::CFLastErrorRecovery()
    {
        m_dwLastError = GetLastError();
    }
    CFLastErrorRecovery::~CFLastErrorRecovery()
    {
        SetLastError(m_dwLastError);
    }


//     BOOL CALLBACK _FtlGlobalShareInfoInitialize(BOOL bFirstCreate, FTLGlobalShareInfo& rShareInfo)
//     {
//         BOOL bRet = TRUE;
// 		if (bFirstCreate)
// 		{
// 			FTLASSERT(rShareInfo.dwTraceTlsIndex == 0);
// 			FTLASSERT(rShareInfo.dwBlockElapseTlsIndex == 0);
// 
// 			rShareInfo.dwTraceTlsIndex = TlsAlloc();
// 			FTLASSERT(TLS_OUT_OF_INDEXES != rShareInfo.dwTraceTlsIndex);
// 			if (TLS_OUT_OF_INDEXES != rShareInfo.dwTraceTlsIndex)
// 			{
// 				TlsSetValue(rShareInfo.dwTraceTlsIndex, NULL);
// 			}
// 			rShareInfo.dwBlockElapseTlsIndex = TlsAlloc();
// 			FTLASSERT(TLS_OUT_OF_INDEXES != rShareInfo.dwBlockElapseTlsIndex);
// 			if (TLS_OUT_OF_INDEXES != rShareInfo.dwBlockElapseTlsIndex)
// 			{
// 				TlsSetValue(rShareInfo.dwBlockElapseTlsIndex, NULL);
// 			}
// 
// 			rShareInfo.nTraceSequenceNumber = 0;
// 			rShareInfo.nBlockElapseId = 0;
// 		}
//         return bRet;
//     }
// 
//     BOOL CALLBACK _FtlGlobalShareInfoFinalize(BOOL bFirstCreate, FTLGlobalShareInfo& rShareInfo)
//     {
// 		//if (bFirstCreate) -- TODO: 需要引用计数?
// 		{
// 			if (TLS_OUT_OF_INDEXES != rShareInfo.dwTraceTlsIndex)
// 			{
// 				//void* pFileWriter = TlsGetValue(rShareInfo.dwTraceTlsIndex);
// 				//CFFastTrace::CFTFileWriter*
// #if 0
// 				FTLASSERT(NULL == TlsGetValue(rShareInfo.dwTraceTlsIndex));			//相关的资源必须已经释放
// #endif
// 				TlsFree(rShareInfo.dwTraceTlsIndex);
// 				rShareInfo.dwTraceTlsIndex = TLS_OUT_OF_INDEXES;
// 			}
// 			if (TLS_OUT_OF_INDEXES != rShareInfo.dwBlockElapseTlsIndex)
// 			{
// 				//BlockElapseInfo*
// 				FTLASSERT(NULL == TlsGetValue(rShareInfo.dwBlockElapseTlsIndex));	//相关的资源必须已经释放
// 				TlsFree(rShareInfo.dwBlockElapseTlsIndex);
// 				rShareInfo.dwBlockElapseTlsIndex = TLS_OUT_OF_INDEXES;
// 			}
// 		}
//         return TRUE;
//     }

    template<typename TBase, typename INFO_TYPE, LONG bufLen>
    CFConvertInfoT<TBase,INFO_TYPE,bufLen>::CFConvertInfoT(INFO_TYPE info)
        :m_Info(info)
    {
        ZeroMemory(m_bufInfo,sizeof(m_bufInfo));
    }
    template<typename TBase, typename INFO_TYPE, LONG bufLen>
    CFConvertInfoT<TBase,INFO_TYPE,bufLen>::~CFConvertInfoT()
    {

    }
    template<typename TBase, typename INFO_TYPE, LONG bufLen>
    LPCTSTR CFConvertInfoT<TBase,INFO_TYPE,bufLen>::GetConvertedInfo()
    {
        TBase* pThis = static_cast<TBase*>(this);
        return pThis->ConvertInfo();
    }

    template<typename TBase, typename INFO_TYPE, LONG bufLen>
    INFO_TYPE CFConvertInfoT<TBase,INFO_TYPE,bufLen>::GetInfo() const
    {
        return m_Info;
    }

    template<typename TBase, typename INFO_TYPE, LONG bufLen>
    void CFConvertInfoT<TBase,INFO_TYPE,bufLen>::SetInfo(INFO_TYPE info)
    {
        m_Info = info;
        m_bufInfo[0] = NULL;
    }

    CFAPIErrorInfo::CFAPIErrorInfo(DWORD dwError) : CFConvertInfoT<CFAPIErrorInfo,DWORD>(dwError)
    {
        m_LanguageID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
        //m_LanguageID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);//本地语言
        m_isValidErrCode = FALSE;  //尚未检测
    }

    DWORD CFAPIErrorInfo::SetLanguageID(DWORD dwLanguageID)
    {
        DWORD dwOldLanguageID = m_LanguageID;
        m_LanguageID = dwLanguageID;
        return dwOldLanguageID;
    }

    BOOL CFAPIErrorInfo::isValidErrCode()
    {
        return m_isValidErrCode;
    }
    
    LPCTSTR CFAPIErrorInfo::ConvertInfo()
    {
        HRESULT hr = E_FAIL;
        if (NULL == m_bufInfo[0])
        {
            switch (m_Info)
            {
#ifndef STATUS_DLL_NOT_FOUND
#  define STATUS_DLL_NOT_FOUND      ((DWORD )0xC0000135L)
#endif 
                //Side By Side Error
                HANDLE_CASE_TO_STRING(m_bufInfo,_countof(m_bufInfo),ERROR_SXS_CANT_GEN_ACTCTX);
                HANDLE_CASE_TO_STRING(m_bufInfo,_countof(m_bufInfo),STATUS_DLL_NOT_FOUND);
                HANDLE_CASE_TO_STRING(m_bufInfo,_countof(m_bufInfo),EXCEPTION_ACCESS_VIOLATION);
            default:
                break;
            }
            if (NULL == m_bufInfo[0])
            {
                LPTSTR pszMsgBuf = NULL;
                DWORD dwCount = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER 
                    | FORMAT_MESSAGE_FROM_SYSTEM 
                    | FORMAT_MESSAGE_IGNORE_INSERTS,    //消息定义中的插入序列(类似 %s 一类)会被一直忽略和跳过直到输出缓冲区不改变
                    NULL,
                    m_Info,
                    m_LanguageID,
                    (LPTSTR)&pszMsgBuf,
                    0,
                    NULL);

                if (!pszMsgBuf)
                {
                    static LPCTSTR pszErrorModule[] = {
                        TEXT("wininet.dll"),
                        TEXT("netmsg.dll"),
                        //TEXT("p2p.dll"),  // HRESULT_FACILITY(hrError) == FACILITY_P2P
                    };

                    for (int i = 0; i < _countof(pszErrorModule); i++)
                    {
                        //Just get loaded module in the calling process 
                        HMODULE hModule = GetModuleHandle(pszErrorModule[i]);//
                        //LoadLibraryEx(pszErrorModule[i], NULL, DONT_RESOLVE_DLL_REFERENCES);
                        if (hModule != NULL) 
                        {
                            dwCount = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                //| FORMAT_MESSAGE_FROM_SYSTEM,
                                hModule, m_Info, m_LanguageID,
                                (LPTSTR)&pszMsgBuf, 0, NULL);
                            //FreeLibrary(hModule);

                            if (pszMsgBuf)
                            {
                                //find message
                                break;
                            }
                        }
                    }
                }

                if(0 != dwCount && NULL != pszMsgBuf )
                {
                    m_isValidErrCode = TRUE;
                    size_t length = _tcsclen(pszMsgBuf);
                    while (length > 0 
                        && (pszMsgBuf[length - 1] == _T('\r') || pszMsgBuf[length - 1] == _T('\n')))
                    {
                        pszMsgBuf[length - 1]=_T('\0');
                        length--;
                    }

                    //lstrcpyn(m_bufInfo,pszMsgBuf,_countof(m_bufInfo) - 1);
                    hr = StringCchPrintf(m_bufInfo, _countof(m_bufInfo) - 1, TEXT("%s"), pszMsgBuf);
                    //ATLASSERT(SUCCEEDED(hr));  //对于有些错误值,英文时默认的长度不足,这里只能获取部分错误描述
                    LocalFree(pszMsgBuf);
                }
                else //无法获取错误值
                {
                    m_isValidErrCode = FALSE;
                    hr = StringCchPrintf(m_bufInfo,_countof(m_bufInfo),TEXT("Unknown Error:0x%08x"),m_Info);
                    ATLASSERT(SUCCEEDED(hr));
                }
            }
            else {
                //直接返回错误码的情况
                m_isValidErrCode = TRUE;
            }
        }
        return m_bufInfo;
    }

#ifndef FACILITY_D3DX
#  define FACILITY_D3D                     0x876
#  define FACILITY_D3DX                    0x877
#  define FACILITY_DSound                  0x878
#endif 

    CFComErrorInfo::CFComErrorInfo(HRESULT hr) : CFConvertInfoT<CFComErrorInfo,HRESULT>(hr)
    {
    }

    LPCTSTR CFComErrorInfo::GetErrorFacility(HRESULT hr, LPTSTR pszFacility,DWORD dwLength)
    {
        int nFacility = HRESULT_FACILITY(hr);
        switch (nFacility)
        {
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_NULL);         //0
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_RPC);          //1
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_DISPATCH);     //2
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_STORAGE);      //3
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_ITF);          //4
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_WIN32);        //7
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_WINDOWS);      //8
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_SECURITY);     //9
            //HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_SSPI);       //9
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_CONTROL);      //10
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_CERT);         //11
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_INTERNET);     //12
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_MEDIASERVER);  //13
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_MSMQ);         //14
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_SETUPAPI);     //15
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_SCARD);        //16
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_COMPLUS);      //17
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_AAF);          //18
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_URT);          //19
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_ACS);          //20
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_DPLAY);        //21
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_UMI);          //22
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_SXS);          //23
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_WINDOWS_CE);   //24
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_HTTP);         //25
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_BACKGROUNDCOPY);//32
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_CONFIGURATION);//33
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_STATE_MANAGEMENT);//34
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_METADIRECTORY);//35

            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_D3D);           //0x876
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_D3DX);          //0x877
            HANDLE_CASE_TO_STRING(pszFacility,dwLength,FACILITY_DSound);        //0x878
        default:
            COM_VERIFY(StringCchPrintf(pszFacility,dwLength,TEXT("Unknown[%d]"),nFacility));
            break;
        }
        return pszFacility;
    }

    LPCTSTR CFComErrorInfo::ConvertInfo()
    {
        if (NULL == m_bufInfo[0])
        {
            HRESULT hr = E_FAIL;
            switch(m_Info)
            {
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), S_FALSE);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), E_NOTIMPL);                   //0x80000001L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), E_INVALIDARG);                //0x80000003L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), E_NOINTERFACE);               //0x80000004L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), E_FAIL);                      //0x80000008L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), E_UNEXPECTED);                //0x8000FFFFL
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), RPC_E_CHANGED_MODE);          //0x80010106L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), RPC_E_TOO_LATE);              //0x80010119L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), DISP_E_MEMBERNOTFOUND);       //0x80020003L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CO_E_NOTINITIALIZED);	        //0x800401F0L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CLASS_E_CLASSNOTAVAILABLE);   //0x80040111L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), REGDB_E_CLASSNOTREG);         //0x80040154L
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), E_ACCESSDENIED);              //0x80070005L

                //HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CONNECT_E_NOCONNECTION);
            default:
                TCHAR strFacility[20] = {0};
                GetErrorFacility(m_Info,strFacility,_countof(strFacility));

                CFAPIErrorInfo apiErrorInfo(m_Info);
                COM_VERIFY(StringCchPrintf(m_bufInfo,_countof(m_bufInfo),
                    TEXT("%s, Facility:%s"),apiErrorInfo.GetConvertedInfo(),strFacility));
                break;
            }
        }
        return m_bufInfo;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CFElapseCounter::CFElapseCounter()
    {
        BOOL bRet = FALSE;
        m_StopTime.QuadPart = 0;
        API_VERIFY(QueryPerformanceFrequency(&m_Frequency));
        API_VERIFY(QueryPerformanceCounter(&m_StartTime));
        if (0 == m_Frequency.QuadPart)
        {
            m_Frequency.QuadPart = 1; //防止被0除，什么样的系统上会出错？486或Win3.1？
        }
        m_PauseTime.QuadPart = m_StartTime.QuadPart;

        m_Status = rsRunning;
    }
    CFElapseCounter::~CFElapseCounter()
    {

    }
    RunningStatus CFElapseCounter::GetStatus() const
    {
        return m_Status;
    }

    BOOL CFElapseCounter::Reset()
    {        
        m_Status = rsStopped;
        m_StartTime.QuadPart = 0;
        m_PauseTime.QuadPart = 0;
        m_StopTime.QuadPart = 0;
        return TRUE;
    }

    BOOL CFElapseCounter::Start()
    {
        BOOL bRet = FALSE;
        API_VERIFY(QueryPerformanceCounter(&m_StartTime));
        m_PauseTime.QuadPart = m_StartTime.QuadPart;
        m_StopTime.QuadPart = 0;
        m_Status = rsRunning;
        return bRet;
    }
    BOOL CFElapseCounter::Pause()
    {
        FTLASSERT(rsRunning == m_Status||!TEXT("Must Start After Stop"));
        if (rsRunning != m_Status)
        {
            SetLastError(ERROR_BAD_COMMAND);
            return FALSE;
        }
        BOOL bRet = FALSE;
        API_VERIFY(QueryPerformanceCounter(&m_PauseTime));
        m_Status = rsPaused;
        return bRet;
    }
    BOOL CFElapseCounter::Resume()
    {
        FTLASSERT(rsPaused == m_Status);
        if (rsPaused != m_Status)
        {
            SetLastError(ERROR_BAD_COMMAND);
            return FALSE;
        }
        BOOL bRet = FALSE;
        LARGE_INTEGER   resumeTime;
        API_VERIFY(QueryPerformanceCounter(&resumeTime));
        m_StartTime.QuadPart += (resumeTime.QuadPart - m_PauseTime.QuadPart);
        m_PauseTime.QuadPart = m_StartTime.QuadPart;
        m_Status = rsRunning;
        return bRet;
    }
    BOOL CFElapseCounter::Stop()
    {
        BOOL bRet = TRUE;
        switch (m_Status)
        {
        case rsRunning://运行状态下，获取新的结束时间
            API_VERIFY(QueryPerformanceCounter(&m_StopTime));
            break;
        case rsPaused://暂停状态下，获取暂停的时间
            m_StopTime.QuadPart = m_PauseTime.QuadPart;
            break;
        case rsStopped://停止状态下，什么都不做
        default:
            break;
        }
        m_Status = rsStopped;
        return bRet;
    }
    LONGLONG CFElapseCounter::GetElapseTime() //返回单位是纳秒(NS)
    {
        //in some machine,will overflow,so change to double count, and convert back

        BOOL bRet = FALSE;
        LONGLONG elapseNs;
        switch (m_Status)
        {
        case rsRunning: //(m_PauseTime.QuadPart == m_StartTime.QuadPart) //运行状态，获取当前时间到开始的时间
            {
                LARGE_INTEGER   nowTime;
                API_VERIFY(QueryPerformanceCounter(&nowTime));
                elapseNs = (LONGLONG)((double)(nowTime.QuadPart - m_StartTime.QuadPart) * ( NANOSECOND_PER_SECOND ) /m_Frequency.QuadPart); 
            }
            break;
        case rsPaused: ////暂停状态，获取暂停结束时间到开始的时间
            elapseNs = (LONGLONG)((double)(m_PauseTime.QuadPart - m_StartTime.QuadPart) * ( NANOSECOND_PER_SECOND ) /m_Frequency.QuadPart);
            break;
        case rsStopped: //m_StopTime.QuadPart != 0  //已经停止，获取结束到开始的时间
            elapseNs = (LONGLONG)((double)(m_StopTime.QuadPart - m_StartTime.QuadPart) * ( NANOSECOND_PER_SECOND ) /m_Frequency.QuadPart); 
            break;
        default:
            FTLASSERT(FALSE);
            elapseNs = -1;
            break;
        }

        FTLASSERT(elapseNs >= 0);
        return elapseNs;
    }

    CFBlockElapse::CFBlockElapse(LPCTSTR pszFileName,DWORD line, 
        LPCTSTR pBlockName, TraceDetailType detailType, LPVOID pReturnAddr, LPVOID pObjectAddr, DWORD MinElapse/* = 0*/)
        :m_pszFileName(pszFileName)
        ,m_Line(line)
        ,m_pszBlkName(pBlockName)
        ,m_traceDetailType(detailType)
        ,m_pReturnAdr(pReturnAddr)
        ,m_pObjectAddr(pObjectAddr)
        ,m_MinElapse(MinElapse)

    {
		FTLTRACEEX(tlDetail, TEXT("%s(%d):\t TID(%d), obj=0x%p Enter %s"),
			m_pszFileName, m_Line, GetCurrentThreadId(), m_pObjectAddr, m_pszBlkName);

        m_StartTime = GetTickCount();
    }

    CFBlockElapse::~CFBlockElapse()
    {
        //Save old last error to avoid effect it
        CFLastErrorRecovery lastErrorRecory;
        HRESULT hr = E_FAIL;
        DWORD dwElapseTime = GetTickCount() - m_StartTime;
		if (dwElapseTime >= m_MinElapse) //m_MinElapse != 0 && 
		{
            FTLTRACEEX_FULL(tlWarn, m_pszFileName, m_Line, m_pszBlkName, TEXT("TID(%d), obj=0x%p, Elapse Too Long (Want-%dms:Real-%dms)"),
				GetCurrentThreadId(), m_pObjectAddr, m_MinElapse, dwElapseTime);
		}
    }

    int FormatMessageBox(HWND hWnd, LPCTSTR lpCaption, UINT uType, LPCTSTR lpszFormat, ...)
    {
        int nRet = 0; //Zero indicates that there is not enough memory to create the message box.
        HRESULT hr = E_FAIL;
        CFStringFormater formater;

        va_list pArgs;
        va_start(pArgs, lpszFormat);

        COM_VERIFY(formater.FormatV(lpszFormat,pArgs));
        va_end(pArgs);

        if (SUCCEEDED(hr) && NULL != formater.GetString())
        {
            nRet = ::MessageBox(hWnd,formater,lpCaption,uType);
        }
        return nRet;
    }
}

#endif //FTL_BASE_HPP