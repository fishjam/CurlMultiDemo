///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   ftlbase.h
/// @brief  Fishjam Template Library Base Header File.
/// @author fujie
/// @version 0.6 
/// @date 03/30/2008
/// @defgroup ftlbase ftl base function and class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_BASE_H
#define FTL_BASE_H
#pragma once

#ifndef __cplusplus
#  error FTL requires C++ compilation (use a .cpp suffix)
#endif

#include "ftlDefine.h"
#include "ftlTypes.h"
#include "ftlSharedVariable.h"
#include "ftlLog.h"

//在某些情况下(比如 IE插件), XXX_VERIFY 检测到错误时, _CrtDbgReport 无法弹出 assert 对话框
#ifndef USE_MESSAGEBOX_ERROR
#  define USE_MESSAGEBOX_ERROR 0
#endif 

//调用方可以重新指定日志输出和断言的机制，如果没有指定，使用默认(日后扩展为FastTrace？)
#if (!defined FTLASSERT)
#  if defined ATLTRACE
#    define FTLASSERT          ATLASSERT
#  elif defined TRACE
#    define FTLASSERT          ASSERT
#else
#   include <Windows.h>
#   include <tchar.h>
#   include <atlconv.h>
#   include <cassert>
#   define FTLASSERT          assert
#  endif
#endif 

#define STRSAFE_NO_DEPRECATE
#  include <strsafe.h>

#include <set>
#include <vector>
#include <map>
#include <atlstr.h>

namespace FTL
{
    //在每个类中使用该宏，可以使得测试类定义为产品类的友元类 -- 支持访问私有成员变量
    #define ENABLE_UNIT_TEST(cls) friend class cls##Tester; 

    #ifdef _DEBUG
    #  if USE_MESSAGEBOX_ERROR
    #    define DBG_REPORT(_err, e)    FormatMessageBox(::GetActiveWindow(), TEXT("Error Prompt"), MB_OKCANCEL|MB_ICONERROR|MB_DEFBUTTON1, TEXT("%s(0x%x, %d)\n in %s(%d), Debug Now?\n"), _err.GetConvertedInfo(), e, e, TEXT(__FUNCTION__), __LINE__)
    #  else
    #    define DBG_REPORT(_err, e)    _CrtDbgReportW(_CRT_ASSERT, TEXT(__FILE__), __LINE__, NULL, TEXT("%s(0x%x, %d)"), _err.GetConvertedInfo(), e, e)
    #  endif 
    #  define DBG_BREAK     _CrtDbgBreak
    #else
    #  define DBG_REPORT    __noop
    #  define DBG_BREAK     __noop  
    #endif

    //c -- error convert class, such as FTL::CFAPIErrorInfo
    //e -- error information, such as value return by GetLastError
    //x -- prompt information, such as call code
//    #ifdef FTL_DEBUG
    #  define REPORT_ERROR_INFO_EX(c, e, ev, x) \
         do{ \
             c _err(e);\
             USES_CONVERSION;\
             FTLTRACEEX(FTL::tlError, TEXT("Error!!! Reason = 0x%08x(%d,%s),Code:\"%s\" "),\
               (uint32_t)ev, (uint32_t)ev, _err.GetConvertedInfo(),TEXT(#x));\
             (1 != DBG_REPORT(_err, ev)) || \
               (DBG_BREAK(), 0);\
         }while(0)

    #  define REPORT_ERROR_INFO(c, e, x) REPORT_ERROR_INFO_EX(c, e, e, x)

    # define ERROR_RETURN_VERIFY(x)\
        FTLTRACEEX(FTL::tlDetail, TEXT("ERROR_RETURN_VERIFY call \"%s\""),TEXT(#x));\
        dwRet = (x);\
        if(ERROR_SUCCESS != dwRet)\
        {\
            REPORT_ERROR_INFO(FTL::CFAPIErrorInfo, dwRet, x);\
        }

    # define API_ASSERT(x)\
        FTLTRACEEX(FTL::tlDetail, TEXT("API_ASSERT call \"%s\""), TEXT(#x));\
        if(FALSE == (x))\
        {\
            FTL::CFLastErrorRecovery lastErrorRecory;\
            REPORT_ERROR_INFO(FTL::CFAPIErrorInfo, lastErrorRecory.GetError(), x);\
        }

    # define API_VERIFY(x)   \
        SetLastError(0); \
        FTLTRACEEX(FTL::tlDetail, TEXT("API_VERIFY call \"%s\""), TEXT(#x));\
        bRet = (x);\
        if(FALSE == bRet)\
        {\
            FTL::CFLastErrorRecovery lastErrorRecory;\
            REPORT_ERROR_INFO(FTL::CFAPIErrorInfo, lastErrorRecory.GetError(), x);\
        }

    # define API_VERIFY_EXCEPT1(x,e1)\
        SetLastError(0); \
        FTLTRACEEX(FTL::tlDetail, TEXT("API_VERIFY_EXCEPT1 call \"%s\""), TEXT(#x));\
        bRet = (x);\
        if(FALSE == bRet)\
        {\
            FTL::CFLastErrorRecovery lastErrorRecory;\
            if(lastErrorRecory.GetError() != e1)\
            {\
                REPORT_ERROR_INFO(FTL::CFAPIErrorInfo, lastErrorRecory.GetError(), x);\
            }\
        }
	# define API_VERIFY_EXCEPT2(x,e1,e2)\
        SetLastError(0); \
        FTLTRACEEX(FTL::tlDetail, TEXT("API_VERIFY_EXCEPT2 call \"%s\""), TEXT(#x));\
		bRet = (x);\
		if(FALSE == bRet)\
		{\
            FTL::CFLastErrorRecovery lastErrorRecory;\
			if(lastErrorRecory.GetError() != e1 && lastErrorRecory.GetError() != e2)\
			{\
				REPORT_ERROR_INFO(FTL::CFAPIErrorInfo, lastErrorRecory.GetError(),x);\
			}\
		}

    //如果返回 E_FAIL，并且支持 ISupportErrorInfo 的话，需要取得Rich Error 错误信息
    # define COM_VERIFY(x)   \
        hr = (x);\
        if(S_OK != hr)\
        {\
            REPORT_ERROR_INFO(FTL::CFComErrorInfo, hr,x);\
        }

	# define COM_VERIFY_RETURN_IF_FAILED(x)   \
        hr = (x);\
        if(S_OK != hr)\
        {\
            REPORT_ERROR_INFO(FTL::CFComErrorInfo, hr,x);\
			return; \
        }

	# define COM_VERIFY_RETURN_VALUE_IF_FAILED(x)   \
        hr = (x);\
        if(S_OK != hr)\
        {\
            REPORT_ERROR_INFO(FTL::CFComErrorInfo, hr,x);\
            return hr; \
        }

    # define COM_VERIFY_EXCEPT1(x,h1) \
        hr = (x);\
        if(S_OK != hr && (h1)!= hr)\
        {\
            REPORT_ERROR_INFO(FTL::CFComErrorInfo, hr,x);\
        }

    # define COM_VERIFY_EXCEPT2(x,h1,h2) \
        hr = (x);\
        if(S_OK != hr && (h1)!= hr && (h2) != hr)\
        {\
            REPORT_ERROR_INFO(FTL::CFComErrorInfo, hr,x);\
        }

    # define REG_VERIFY(x)   \
        lRet = (x);\
        if(ERROR_SUCCESS != lRet)\
        {\
            REPORT_ERROR_INFO(FTL::CFAPIErrorInfo, lRet,x);\
        }

    # define REG_VERIFY_EXCEPT1(x,ret1)   \
        lRet = (x);\
        if(ERROR_SUCCESS != lRet && (ret1) != lRet )\
        {\
            REPORT_ERROR_INFO(FTL::CFAPIErrorInfo, lRet,x);\
        }

    ////////////////////////////////////////////////////////////////////////////////////////

    #ifndef _countof
    # define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
    #endif

    #ifndef tstring
    #  if defined UNICODE 
	#     define tstring std::wstring
    #  else
	#     define tstring std::string
    #  endif 
    #endif


    //! @code SAFE_CLOSE_HANDLE(m_hFile,INVALID_HANDLE_VALUE); 注意 NULL 和 INVALID_HANDLE_VALUE
    #ifndef SAFE_CLOSE_HANDLE
    #  ifdef FTL_DEBUG
    #    define SAFE_CLOSE_HANDLE(h,v) if((v) != (h)) { BOOL oldbRet = bRet; API_VERIFY(::CloseHandle((h))); (h) = (v); bRet = oldbRet; }
    #  else
    #    define SAFE_CLOSE_HANDLE(h,v) if((v) != (h)) { ::CloseHandle((h)); (h) = (v); bRet = bRet; }
    #  endif
    #endif

	#ifndef SAFE_CLOSE_INTERNET_HANDLE
	#  ifdef FTL_DEBUG
	#    define SAFE_CLOSE_INTERNET_HANDLE(h) if(NULL != (h)) { BOOL oldbRet = bRet; API_VERIFY(::InternetCloseHandle((h))); (h) = NULL; bRet = oldbRet; }
	#  else
	#    define SAFE_CLOSE_INTERNET_HANDLE(h) if((NULL) != (h)) { ::InternetCloseHandle((h)); (h) = NULL; bRet = bRet; }
	#  endif
	#endif

    #ifndef SAFE_CLOSE_REG
    #  ifdef FTL_DEBUG
    #    define SAFE_CLOSE_REG(h) if(NULL != (h)) { BOOL oldbRet = bRet; API_VERIFY(ERROR_SUCCESS == ::RegCloseKey((h))); (h) = NULL; bRet = oldbRet; }
    #  else
    #    define SAFE_CLOSE_REG(h) if(NULL != (h)) { ::RegCloseKey((h)); (h) = NULL; }
    #  endif
    #endif


    #ifndef SAFE_RELEASE
    #  define SAFE_RELEASE(p)  if( NULL != ((p)) ){ (p)->Release(); (p) = NULL; }
    #endif 

	//CoTaskMemFree == CoGetMalloc (MEMCTX_TASK) + IMalloc::Free
	// http://blogs.msdn.com/b/oldnewthing/archive/2004/07/05/173226.aspx
    #ifndef SAFE_COTASKMEMFREE
    #  define SAFE_COTASKMEMFREE(p) if(NULL != (p)){ ::CoTaskMemFree((p)); (p) = NULL; }
    #endif

    #ifndef SAFE_FREE_LIBRARY
    #  define SAFE_FREE_LIBRARY(p) if(NULL != (p)){ ::FreeLibrary(static_cast<HMODULE>(p)); (p) = NULL; }
    #endif


    ////////////////////////////////////////////////////////////////////////////////////////
    //f is CFStringFormater
    //v is combine value(such as GetStyle return value), 
    //c is check type, such as WS_VISIBLE
	//s is append string, such as Visible
    //d is append string, such as "," or "|"
	#ifndef HANDLE_COMBINATION_VALUE_TO_STRING_EX
	#  define HANDLE_COMBINATION_VALUE_TO_STRING_EX(f, v, c, s, d) \
		if(((v) & (c)) == (c))\
		{\
			if(f.GetStringLength() != 0){ \
				f.AppendFormat(TEXT("%s%s"), d, s);\
			}else{\
				f.AppendFormat(TEXT("%s"), s);\
			}\
			(v) &= ~(c);\
		}
	#  define HANDLE_COMBINATION_VALUE_TO_STRING(f, v, c, d)	HANDLE_COMBINATION_VALUE_TO_STRING_EX(f, v, c, TEXT(#c), d) 
	#endif 

	#ifndef HANDLE_CASE_TO_STRING_EX
	# define HANDLE_CASE_TO_STRING_EX(buf,len,c, v)\
			case (c):\
				StringCchCopy(buf,len,v); \
			break;
	#endif
	# define HANDLE_CASE_TO_STRING(buf,len,c) HANDLE_CASE_TO_STRING_EX(buf, len, c, TEXT(#c))

	#ifndef HANDLE_CASE_TO_STRING_FORMATER
	# define HANDLE_CASE_TO_STRING_FORMATER(f, c)\
		case (c):\
		f.Format(TEXT("%s"), TEXT(#c));\
		break;
	#endif 

	#ifndef HANDLE_CASE_RETURN_STRING_EX
	# define HANDLE_CASE_RETURN_STRING_EX(c, v) \
		case (c):\
		return v;
	#endif 

    #ifndef HANDLE_CASE_RETURN_STRING
    # define HANDLE_CASE_RETURN_STRING(c) HANDLE_CASE_RETURN_STRING_EX(c, TEXT(#c))
    #endif 

}

namespace FTL
{
    template<typename T>
    BOOL IsSameNumber(const T& expected, const T& actual, const T& delta);

	template<typename T>
	void SwapValue(T& value1, T& value2);

    //自定义这几个函数，避免在 FTL 中引入 shlwapi.dll 中的函数(对方可能没有这些依赖)
    FTLINLINE LPTSTR FtlPathFindFileName(LPCTSTR pPath);
    FTLINLINE void   FtlPathRemoveExtension(LPTSTR pszPath);

    FTLEXPORT class CFLastErrorRecovery
    {
    public:
        FTLINLINE CFLastErrorRecovery();
        FTLINLINE ~CFLastErrorRecovery();
        DWORD GetError() const { return m_dwLastError; }
    private:
        DWORD   m_dwLastError;
    };

    FTLEXPORT template<typename TBase, typename INFO_TYPE, LONG bufLen = DEFAULT_BUFFER_LENGTH>
    class CFConvertInfoT
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFConvertInfoT);
    public:
        FTLINLINE explicit CFConvertInfoT(INFO_TYPE info);
        FTLINLINE virtual ~CFConvertInfoT();
        FTLINLINE LPCTSTR GetConvertedInfo();
        FTLINLINE INFO_TYPE GetInfo() const;
        FTLINLINE void SetInfo(INFO_TYPE info);
    public:
        FTLINLINE virtual LPCTSTR ConvertInfo() = 0;
    protected:
        INFO_TYPE   m_Info;
        TCHAR       m_bufInfo[bufLen];
    };


    FTLEXPORT class CFAPIErrorInfo : public CFConvertInfoT<CFAPIErrorInfo,DWORD>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFAPIErrorInfo);
    public:
        FTLINLINE explicit CFAPIErrorInfo(DWORD dwError);
        FTLINLINE DWORD SetLanguageID(DWORD dwLanguageID);
        
        //表示是否能查出来错误信息，有些错误码目前没有定义实际的意义(比如 35,40~49 等),
        //为了性能,这个信息只有在调用了 ConvertInfo 以后才能正确设置
        FTLINLINE BOOL isValidErrCode(); 
        FTLINLINE virtual LPCTSTR ConvertInfo();
	protected:
		DWORD	m_LanguageID;
        BOOL    m_isValidErrCode;
    };

    FTLEXPORT class CFComErrorInfo : public CFConvertInfoT<CFComErrorInfo,HRESULT>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFComErrorInfo);
    public:
        FTLINLINE explicit CFComErrorInfo(HRESULT hr);
        FTLINLINE virtual LPCTSTR ConvertInfo();
    protected:
        FTLINLINE LPCTSTR GetErrorFacility(HRESULT hr, LPTSTR pszFacility,DWORD dwLength);
    };

    // The typedef for the debugging output function.  Note that this matches OutputDebugString.
    typedef VOID (WINAPI *PFNDEBUGOUT)(LPCTSTR lpOutputString) ;

    //高性能的计时器，能用于计算花费的时间(如计算拷贝时的速度估计) -- 支持暂停/继续
    #define NANOSECOND_PER_MILLISECOND  (1000 * 1000)
    #define MILLISECOND_PER_SECOND      (1000)
    #define NANOSECOND_PER_SECOND       (NANOSECOND_PER_MILLISECOND * MILLISECOND_PER_SECOND)

    enum RunningStatus
    {
        rsStopped,
        rsRunning,
        rsPaused
    };

    class CFElapseCounter
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFElapseCounter);
    public:
        FTLINLINE CFElapseCounter();
        FTLINLINE ~CFElapseCounter();
		FTLINLINE BOOL Reset();
        FTLINLINE BOOL Start();
        FTLINLINE BOOL Pause();
        FTLINLINE BOOL Resume();
        FTLINLINE BOOL Stop();
        FTLINLINE RunningStatus GetStatus() const;

        //! 运行状态，获取当前时间到开始的时间
        //! 暂停状态，获取暂停结束时间到开始的时间
        //! 停止状态，获取结束到开始的时间
        FTLINLINE LONGLONG GetElapseTime(); //返回单位是纳秒(NS) 10^-9
    private:
        LARGE_INTEGER   m_Frequency;
        LARGE_INTEGER   m_StartTime;
        LARGE_INTEGER   m_PauseTime;
        LARGE_INTEGER   m_StopTime;
        RunningStatus   m_Status;
    };

// #ifdef __cplusplus
//     extern "C"
// #endif
//    void * _ReturnAddress(void);
#   pragma intrinsic(_ReturnAddress)

#ifndef DEFAULT_BLOCK_TRACE_THRESHOLD    //默认的追踪阈值为100毫秒
#  define DEFAULT_BLOCK_TRACE_THRESHOLD  (100)
#endif 

#ifndef MAX_TRACE_INDICATE_LEVEL        //默认最多追踪50层
#  define MAX_TRACE_INDICATE_LEVEL    (50)
#endif 

#pragma message( "  MAX_TRACE_INDICATE_LEVEL = " QQUOTE(MAX_TRACE_INDICATE_LEVEL) )

    enum TraceDetailType{
        TraceDetailNone = 0,
        TraceDetailExeName,
        TraceDetailModuleName,
    };

#  define FUNCTION_BLOCK_TRACE(elapse) \
    FTL::CFBlockElapse JOIN_TWO(minElapse, __LINE__) (TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), FTL::TraceDetailNone, _ReturnAddress(), NULL, (elapse))
#  define FUNCTION_BLOCK_NAME_TRACE(blockName,elapse) \
    FTL::CFBlockElapse JOIN_TWO(minElapse, __LINE__) (TEXT(__FILE__), __LINE__, blockName, FTL::TraceDetailNone, _ReturnAddress(), NULL, (elapse))

#  define FUNCTION_BLOCK_TRACE_WITH_THIS(elapse) \
   FTL::CFBlockElapse JOIN_TWO(minElapse, __LINE__) (TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), FTL::TraceDetailNone, NULL, this, (elapse))

#  define FUNCTION_BLOCK_NAME_TRACE_EX(blockName, detailType, elapse) \
    FTL::CFBlockElapse JOIN_TWO(minElapse, __LINE__) (TEXT(__FILE__), __LINE__, blockName, detailType, _ReturnAddress(), NULL, (elapse))

#  define FUNCTION_BLOCK_MODULE_NAME_TRACE(blockName, elapse) \
    TCHAR szModuleName[MAX_PATH] = {0};\
    GetModuleFileName(NULL, szModuleName, _countof(szModuleName));\
    TCHAR szTrace[MAX_PATH] = {0};\
    StringCchPrintf(szTrace, _countof(szTrace), TEXT("%s in %s(PID=%d)"),  blockName, FtlPathFindFileName(szModuleName), GetCurrentProcessId());\
    FTL::CFBlockElapse JOIN_TWO(minElapse, __LINE__) (TEXT(__FILE__),__LINE__, szTrace, FTL::TraceDetailNone, _ReturnAddress(), NULL, (elapse));


//#ifdef FTL_DEBUG        //由于BlockElapse会消耗大量的时间，因此，只在Debug状态下启用
    //! 跟踪程序的运行逻辑和效率
    //! @code FUNCTION_BLOCK_TRACE(1000) -- 指定的代码块需要在1000毫秒中执行完，否则打印日志
    class CFBlockElapse
    {
    public:
        //使用毫秒作为判断是否超时
        FTLINLINE CFBlockElapse(LPCTSTR pszFileName,DWORD line, 
            LPCTSTR pBlockName, TraceDetailType detailType, LPVOID pReturnAddr, 
            LPVOID pObjectAddr = NULL, DWORD MinElapse = 0);
        FTLINLINE ~CFBlockElapse(void);
    private:
        struct BlockElapseInfo
        {
            LONG   indent;
            TCHAR  bufIndicate[MAX_TRACE_INDICATE_LEVEL + 1];//增加最后的NULL所占的空间
            TCHAR  szDetailName[MAX_PATH];
        };
        const TCHAR* m_pszFileName;
        const TCHAR* m_pszBlkName;
        const TraceDetailType m_traceDetailType;
        const LPVOID m_pReturnAdr;
        const LPVOID m_pObjectAddr;
        const DWORD  m_Line;
        const DWORD m_MinElapse;
		LONG  m_nElapseId;
        DWORD m_StartTime;
    };

    //Function
    //显示Message前获取当前程序的活动窗体
    //HWND hWnd = (HWND)(AfxGetApp()->m_pMainWnd) ;
    //if ( FALSE == ::IsWindowVisible ( hWnd ) )
    //{
    //    hWnd = ::GetActiveWindow ( ) ;
    //}
    FTLINLINE int __cdecl FormatMessageBox(HWND hWnd, LPCTSTR lpCaption, UINT uType, LPCTSTR lpszFormat, ...);

} //namespace FTL

#endif// FTL_BASE_H

#ifndef USE_EXPORT
#  include "ftlbase.hpp"
#endif
