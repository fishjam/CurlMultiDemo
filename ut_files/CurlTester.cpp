#include "stdafx.h"
#include "CurlTester.h"

#if TEST_NET_CURL

#include <ftlCurl.h>
#include <ftlFile.h>

//TODO: https://github.com/microsoft/vcpkg/issues/2621
// #pragma comment(lib, "Ws2_32.Lib")
// #pragma comment(lib, "Crypt32.Lib")
// #pragma comment(lib, "Crypt32.Lib")
#ifdef _DEBUG
#  pragma comment(lib , "libcurl-d.lib")
#else 
#  pragma comment(lib , "libcurl.lib")
#endif 

FTL::FileFindResultHandle CCurlTester::OnFindFile(
    LPCTSTR pszFilePath, const WIN32_FIND_DATA& findData, LPVOID pParam)
{
    int maxCount = reinterpret_cast<int>(pParam);
    if (m_searchFiles.size() < maxCount)
    {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)  // file
        {
            m_searchFiles[findData.cFileName] = pszFilePath;
            m_dwTotalLongth += findData.nFileSizeLow;
        }
        return FTL::rhContinue;
    }
    
    FTLTRACE(TEXT(" file count %d, total size=%d"), m_searchFiles.size(), m_dwTotalLongth);
    return FTL::rhStop;
}

FTL::FileFindResultHandle CCurlTester::OnError(LPCTSTR pszFilePath, DWORD dwError, LPVOID pParam)
{
    FTL::CFAPIErrorInfo errInfo(dwError);
    FTLTRACEEX(FTL::tlError, TEXT("error %d(%s) for %s"), dwError, errInfo.GetConvertedInfo(), pszFilePath);
    return FTL::rhContinue;
}

void CCurlTester::OnTaskTimeOut(FTL::CFCurlTaskBase* pTask, CURLcode retCode, LPVOID pParam)
{

}

void CCurlTester::OnTaskComplete(FTL::CFCurlTaskBase* pTask, CURLcode retCode, long retResponseCode, LPVOID pParam)
{
    FTL::CFConversion convResult;
    FTL::CFCurlErrorInfo curlErrInfo(retCode);
    if (CURLE_OK != retCode)
    {
        FTLTRACEEX(FTL::tlError, TEXT("failed ret=%d(%s), responseCode=%d, result=%s"),
            retCode, curlErrInfo.GetConvertedInfo(), 
            retResponseCode, convResult.UTF8_TO_TCHAR(pTask->GetResult().c_str()));
    }
    else {
        FTLTRACE(TEXT("successful handle result: responseCode=%d, result=%s"),
            retResponseCode, convResult.UTF8_TO_TCHAR(pTask->GetResult().c_str()));
    }
}


size_t CCurlTester::read_from_file_handle(char *buffer, size_t size, size_t count, void *request)
{
    //注意: 针对 curl_off_t , 由于涉及到不同的环境(32/64等), 有预定义的格式化宏 CURL_FORMAT_CURL_OFF_T 
    BOOL bRet = FALSE;
    if (request != NULL)
    {
        HANDLE hReadFile = reinterpret_cast<HANDLE>(request);
        DWORD dwWantRead = (DWORD)(size * count);
        DWORD dwRealRead = 0;
        API_VERIFY(ReadFile(hReadFile, buffer, dwWantRead, &dwRealRead, NULL));
        FTLTRACEEX(FTL::tlDetail, TEXT("read ret=%d, want %d, real %d"),
            bRet, dwWantRead, dwRealRead);
        if (bRet)
        {
            return dwRealRead;
        }
    }
    return -1;
}

size_t CCurlTester::write_to_file_handle(char *buffer, size_t size, size_t count, void *request)
{
    BOOL bRet = FALSE;
    if (request != NULL)
    {
        HANDLE hWriteFile = reinterpret_cast<HANDLE>(request);
        DWORD dwWantWrite = (DWORD)(size * count);
        DWORD dwRealWrite = 0;

        API_VERIFY(WriteFile(hWriteFile, buffer, dwWantWrite, &dwRealWrite, NULL));
        FTLASSERT(dwWantWrite == dwRealWrite);

        FTLTRACEEX(FTL::tlDetail, TEXT("write ret=%d, want %d, real %d"),
            bRet, dwWantWrite, dwRealWrite);
        if (bRet)
        {
            return dwRealWrite;
        }
    }
    return 0;
}

size_t CCurlTester::write_to_string(char *buffer, size_t size, size_t count, void *request)
{
    if (request != NULL)
    {
        std::string* pWriteString = (reinterpret_cast<std::string*>(request));
#ifdef _DEBUG
        std::string strDebug;
        strDebug.append(buffer, size * count);
#endif 
        pWriteString->append(buffer, size * count);
        return size * count;
    }
    return -1;
}

size_t CCurlTester::read_header_callback(char *buffer, size_t size,
    size_t nitems, void *userdata)
{
    /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
    /* 'userdata' is set with CURLOPT_HEADERDATA */

    if (userdata != NULL) {
        CCurlTester* pThis = reinterpret_cast<CCurlTester*>(userdata);

    }
    std::string sbuffer(buffer, size * nitems);
    FTL::CFConversion conv;

    //FTLTRACE(TEXT("header = %s"), conv.UTF8_TO_TCHAR(sbuffer.c_str()));

    return nitems * size;
}


size_t CCurlTester::curlWriteDataFun(void *buffer, size_t size, size_t nmemb, void *userp)
{
    //参照 fwrite 的实现方式:
    //  size -- 每个单元的字节数;
    //  nmemb -- 要写的单元个数,

    CCurlTester* pThis = reinterpret_cast<CCurlTester*>(userp);
    
    if (pThis && buffer && size > 0)  //pThis && 
    {
        FTL::CFConversion conv;
        FTLTRACE(TEXT("writeData:%s"), conv.UTF8_TO_TCHAR((const char*)buffer));
    }

    //返回值表示处理的字节个数, 如果不是 size*nmemb 表示有错误
    return size * nmemb;
}

int CCurlTester::progress_callback(void *clientp,
    curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    //TODO: 
    //  dl <== download ?
    //  ul <== upload ?
    FTLTRACEEX(FTL::tlDetail, TEXT("curl progress, dlTotal=%lld, dlnow=%lld, ultotal=%lld, ulnow=%lld"),
        dltotal, dlnow, ultotal, ulnow);

    //  如果返回 1, 表示要取消. 返回 0 继续
    return 0;
}

void CCurlTester::setUp()
{
    m_dwTotalLongth = 0;
    CURLcode retCode = CURL_LAST;
    long flags = CURL_GLOBAL_ALL;  //缺省 初始化所有的可能的调用(SSL + Win32 套接字),CURL_GLOBAL_DEFAULT

    //初始化 libcurl 库
    CURL_VERIFY(curl_global_init(flags));

    //获取 easy interface 的接口指针, 类似打开一个 FILE, 一个 CURL 意味一个会话.
    m_pCurlEasy = curl_easy_init();
    CPPUNIT_ASSERT(NULL != m_pCurlEasy);

    m_pCurlMulti = curl_multi_init();
    CPPUNIT_ASSERT(NULL != m_pCurlMulti);

    m_pCURLShare = curl_share_init();
    CPPUNIT_ASSERT(NULL != m_pCURLShare);
}

void CCurlTester::tearDown()
{
    //结束会话,释放内存
    if (m_pCURLShare)
    {
        CURLSHcode shCode = CURLSHE_LAST;
        CURLSH_VERIFY(curl_share_cleanup(m_pCURLShare));
        m_pCURLShare = NULL;
    }

    if(m_pCurlMulti)
    {
        CURLMcode mRetCode = CURLM_LAST;
        CURLM_VERIFY(curl_multi_cleanup(m_pCurlMulti));
        m_pCurlMulti = NULL;
    }

    if(m_pCurlEasy)
    {
        curl_easy_cleanup(m_pCurlEasy);
        m_pCurlEasy = NULL;
    }

    //析构整个 libcurl 库
    curl_global_cleanup();
}

CURLcode CCurlTester::_setupEasyHandle(CURL* pCurlEasy, PCURL_WORK_PARAM pCurlWorkParam)
{
    BOOL bRet = FALSE;
    CURLcode retCode = CURL_LAST;
    
    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_URL, pCurlWorkParam->strFullUrl.c_str()));
    if (!pCurlWorkParam->strReffer.empty())
    {
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_REFERER, pCurlWorkParam->strReffer.c_str()));
    }

    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_VERBOSE, 1L));
    //CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_DEBUGFUNCTION, OnDebug));

    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_DNS_CACHE_TIMEOUT, 3));
    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_CONNECTTIMEOUT, 5));
    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_FOLLOWLOCATION, 1L));  //指定是否指定允许重定向

    //设置进度回调, 从而允许 cancel
    curl_easy_setopt(pCurlEasy, CURLOPT_NOPROGRESS, FALSE);
    curl_easy_setopt(pCurlEasy, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(pCurlEasy, CURLOPT_PROGRESSDATA, reinterpret_cast<void *>(this));

    //add header
    struct curl_slist *pHeader = nullptr;
    pHeader = curl_slist_append(pHeader, "Accept: */*");
    pHeader = curl_slist_append(pHeader, "Content-Type: application/json"); // application/x-www-form-urlencoded
    pHeader = curl_slist_append(pHeader, "Charsets: UTF-8");
    //pHeader = curl_slist_append(pHeader, "Expect:");
    pHeader = curl_slist_append(pHeader, "User-Agent: CurlTester");

    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_HTTPHEADER, pHeader));
    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_HEADERFUNCTION, CCurlTester::read_header_callback));
    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_HEADERDATA, this));

    if (!m_strCookie.empty()) {
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_COOKIE, m_strCookie.c_str()));
    }

    switch (pCurlWorkParam->workType)
    {
    case cwtGet:
    {
        curl_easy_setopt(pCurlEasy, CURLOPT_WRITEFUNCTION, CCurlTester::write_to_string);
        curl_easy_setopt(pCurlEasy, CURLOPT_WRITEDATA, &pCurlWorkParam->workParam);
        break;
    }
    case cwtPost:
    {
        //将指定字符串 POST 到url中(TODO: simplepost.c 中没有设置 CURLOPT_POST )
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_POST, 1L));
        //需要 POST 的内容, 如 json 字符串.TODO: 是否可以通过回调的方式来提供
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_POSTFIELDS, pCurlWorkParam->workParam.c_str()));
        //指定 POST 的长度,
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_POSTFIELDSIZE, pCurlWorkParam->workParam.length()));
        break;
    }
    case cwtDownload:
    {
        HANDLE hWriteFile = INVALID_HANDLE_VALUE;
        API_VERIFY(INVALID_HANDLE_VALUE != (hWriteFile = ::CreateFile(pCurlWorkParam->strFilePath.c_str(),
            GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)));
        if (!bRet)
        {
            FTLTRACEEX(FTL::tlError, TEXT("Create download file %s fail, error=%d"),
                pCurlWorkParam->strFilePath.c_str(), GetLastError());
            return CURLE_WRITE_ERROR;
        }
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_WRITEFUNCTION, CCurlTester::write_to_file_handle));
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_WRITEDATA, hWriteFile));

        break;
    }
    case cwtUpload:
    {
        //上传指定文件
        HANDLE hReadFile = INVALID_HANDLE_VALUE;
        API_VERIFY(INVALID_HANDLE_VALUE != (hReadFile = ::CreateFile(pCurlWorkParam->strFilePath.c_str(),
            GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)));
        if (!bRet)
        {
            FTLTRACEEX(FTL::tlError, TEXT("Open upload file %s fail, error=%d"),
                pCurlWorkParam->strFilePath.c_str(), GetLastError());
            return CURLE_READ_ERROR;
        }

        pCurlWorkParam->clearInfo = hReadFile;

        //指定 PUT 操作. TODO: 现在很多上传实际都是 POST(而且还要带各种 meta 信息, 如文件名, 大小等)
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_UPLOAD, 1L));  //TODO:示例(fileupload.c) 中 url 是 file://xxx
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_READFUNCTION, CCurlTester::read_from_file_handle));
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_READDATA, hReadFile));

        LARGE_INTEGER fileSize = { 0 };
        API_VERIFY(GetFileSizeEx(hReadFile, &fileSize));
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileSize.QuadPart));
        break;
    }
    default:
        FTLASSERT(FALSE && TEXT("missing workType handler"));
        break;
    }
    return retCode;
}

CURLcode CCurlTester::_performEasyHandle(CURL* pCurlEasy, BOOL bDumpInfo /* = TRUE */)
{
    CURLcode retCode = CURL_LAST;

    CURL_VERIFY(curl_easy_perform(pCurlEasy));  //执行任务
    CPPUNIT_ASSERT(CURLE_OK == retCode);
    if (bDumpInfo)
    {
        FTL::CFStringFormater formater;
        FTLTRACE(TEXT("curl info: %s"), FTL::CFCurlUtils::GetCurlInfo(formater, pCurlEasy));
    }

    return retCode;
}

CURLcode CCurlTester::_clearEasyHandleWithParam(CURL* pCurlEasy, PCURL_WORK_PARAM pCurlWorkParam)
{
    BOOL bRet = FALSE;
    CURLcode retCode = CURL_LAST;

    switch (pCurlWorkParam->workType)
    {
    case cwtGet:
        retCode = CURLE_OK;
        break;
    case cwtPost:
        break;
    case cwtDownload:
        break;
    case cwtUpload:
    {
        if (pCurlWorkParam->clearInfo)
        {
            HANDLE hFile = reinterpret_cast<HANDLE>(pCurlWorkParam->clearInfo);
            SAFE_CLOSE_HANDLE(hFile, INVALID_HANDLE_VALUE);
        }
    }
        break;
    default:
        FTLASSERT(FALSE && TEXT("missing workType handler"));
        break;

    }

    return retCode;
}


void CCurlTester::_prepareCurlShare()
{
    //设置各个连接的共享部分,免得每个连接都要处理. 
    //设置完以后, 设置到 CURL 实例里使用. 
    //  curl_easy_setopt(m_pCurlEasy, CURLOPT_SHARE, m_ShareHandle);

    CURLSHcode shCode = CURLSHE_LAST;
    
    CURLSH_VERIFY(curl_share_setopt(m_pCURLShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT));

    CURLSH_VERIFY(curl_share_setopt(m_pCURLShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE));

    //DNS共享,避免每个连接都要解析一遍,否则如果多线程多连接,可能会出现 CURLE_COULDNT_RESOLVE_HOST(6) 错误 
    CURLSH_VERIFY(curl_share_setopt(m_pCURLShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS));

    CURLSH_VERIFY(curl_share_setopt(m_pCURLShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_SHARE));
}

void CCurlTester::test_curl_mime()
{
    CURLcode retCode = CURL_LAST;

    // Multi POST ?
    curl_mime* pMime = curl_mime_init(m_pCurlEasy);
    curl_mimepart *pPart = curl_mime_addpart(pMime);

    CURL_VERIFY(curl_mime_name(pPart, "file"));
    CURL_VERIFY(curl_mime_filedata(pPart, "file_path"));
    CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_MIMEPOST, pMime));

    curl_mime_free(pMime);
    pMime = NULL;
}

/***********************************************************************************************************************
 * curl_slist:
 *   单链表结构,可以用于设置 Header/自定义resolve映射(CURLOPT_RESOLVE) 等的列表,使用完毕后记着使用 curl_slist_free_all 释放
 **********************************************************************************************************************/
void CCurlTester::test_curl_slist()
{
    CURLcode retCode = CURL_LAST;

    curl_slist *pHeader = NULL;

    //TODO: 必须使用返回的变量? 返回新的 header 作为链表的头?
    pHeader = curl_slist_append(pHeader, "Accept: */*");
    curl_slist *pCheckHeader = pHeader;

    pHeader = curl_slist_append(pHeader, "Content-Type: application/json"); // application/x-www-form-urlencoded
    CPPUNIT_ASSERT(pHeader == pCheckHeader);   //由于 链表不是NULL, 因此返回的结果和以前一样.新的值保存在 next 节点里

    pHeader = curl_slist_append(pHeader, "Charsets: UTF-8");
    //pHeader = curl_slist_append(pHeader, "Expect:");
    pHeader = curl_slist_append(pHeader, "User-Agent: CurlTester");

    int totalCount = 0;
    while (pCheckHeader != NULL)
    {
        totalCount++;
        pCheckHeader = pCheckHeader->next;
    }
    CPPUNIT_ASSERT_EQUAL(4, totalCount);

    //然后就可以将其设置到 easy curl 实例里面了(header)
    CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_HTTPHEADER, pHeader));
    //执行

    //释放
    curl_slist_free_all(pHeader);
    pHeader = NULL;
}

void CCurlTester::test_curlVersion()
{
    FTL::CFConversion convVer, convVerInfo, convHost, convSSLVersion;
    std::string strCurlVersion = curl_version();
    //CPPUNIT_ASSERT_EQUAL(std::string("libcurl/7.82.0-DEV Schannel zlib/1.2.11"), strCurlVersion);

    //检查 curl 的版本信息,以及支持的特性,比如:
    //  CURL_VERSION_HTTP2  <== 是否支持 http2
    //  CURL_VERSION_HTTP3  <== 是否支持 http3(QUIC + HTTP/3?)
    //  CURL_VERSION_ALTSVC <== 是否支持 Alt-svc
    curl_version_info_data *pVerInfoData = curl_version_info(CURLVERSION_NOW);  //返回的是静态变量的指针, 因此不需要释放
    CPPUNIT_ASSERT(NULL != pVerInfoData);
    if (pVerInfoData)
    {
        FTL::CFStringFormater fmtFeatures;
        //20220405, 安装方式: vcpkg install curl[ssl]:x64-windows 的结果(缺省Schannel)
        //  age=9, version=7.82.0-DEV, host=Windows, sslVersion=Schannel,
        //  features = 0x110c2bdd(IPV6|SSL|LIBZ|NTLM|ASYNCHDNS|SPNEGO|LARGEFILE|SSPI|CURLDEBUG|KERBEROS5|UNIX_SOCKETS|ALTSVC|HSTS)

        //20220425 安装方式: vcpkg install curl[openssl]:x64-windows 的结果(使用OpenSSL)
        //  libcurl/7.82.0-DEV OpenSSL/3.0.2 (Schannel) zlib/1.2.12, 
        //  age=9, verInfo=7.82.0-DEV, host=Windows, sslVersion=OpenSSL/3.0.2 (Schannel), 
        //  features = 0x116c2bdd(IPV6|SSL|LIBZ|NTLM|DEBUG|ASYNCHDNS|SPNEGO|LARGEFILE|SSPI|CURLDEBUG|KERBEROS5|UNIX_SOCKETS|HTTPS_PROXY|MULTI_SSL|ALTSVC|HSTS)
        FTLTRACE(TEXT("curlVersion=%s, age=%d, verInfo=%s, host=%s, sslVersion=%s, features = 0x%x(%s)"), 
            convVer.UTF8_TO_TCHAR(strCurlVersion.c_str()),
            pVerInfoData->age, 
            convVerInfo.UTF8_TO_TCHAR(pVerInfoData->version),
            convHost.UTF8_TO_TCHAR(pVerInfoData->host),
            convSSLVersion.UTF8_TO_TCHAR(pVerInfoData->ssl_version),
            pVerInfoData->features, FTL::CFCurlUtils::
        (fmtFeatures, pVerInfoData->features));
    }
}

void CCurlTester::test_curl_urlApi()
{
    CURLU * pUrl = curl_url(); /* get a handle to work with */
    CURLUcode cuCode = CURLUE_LAST;
    CPPUNIT_ASSERT(NULL != pUrl);
    if (pUrl)
    {
        char *pScheme = NULL;
        char *pHost = NULL;
        char *pPort = NULL;
        char *pPath = NULL;

        const char* fullUrls[] = {
            "http://www.fishjam.com/path/index.html",
            "../another/second.html",
            "http://127.0.0.1:443/search.api?user=fishjam",
        };

        for (int i = 0; i < _countof(fullUrls); i++)
        {
            CURLU_VERIFY(curl_url_set(pUrl, CURLUPART_URL, fullUrls[i], 0));

            CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_SCHEME, &pScheme, 0));
            CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_HOST, &pHost, 0));

            // CURLUE_NO_PORT (必须要有端口,哪怕是缺省的 80/443)
            CURLU_VERIFY_EXCEPT1(curl_url_get(pUrl, CURLUPART_PORT, &pPort, 0), CURLUE_NO_PORT);  

            CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_PATH, &pPath, 0));

//             CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_USER, &pUser, 0));
//             CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_PASSWORD, &pPassword, 0));
//             CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_OPTIONS, &pOptions, 0));
//             CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_QUERY, &pQuery, 0));
//             CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_FRAGMENT, &pFragment, 0));
//             CURLU_VERIFY(curl_url_get(pUrl, CURLUPART_ZONEID, &pZoneId, 0));


            FTLTRACEA("fullUrls=%s, pScheme=%s, pHost=%s, pPort=%s, pPath=%s", fullUrls[i], pScheme, pHost, pPort, pPath);

            curl_free(pScheme);
            curl_free(pHost);
            curl_free(pPort);
            curl_free(pPath);
        }

        curl_url_cleanup(pUrl);
        pUrl = NULL;
    }
}

void CCurlTester::test_easy_functions()
{
    CURLcode retCode = CURL_LAST;
    CPPUNIT_ASSERT(NULL != m_pCurlEasy);

    OutputDebugString(TEXT("OutputDebugString String\n"));
    OutputDebugStringA("OutputDebugStringA String\n");

    CURL_WORK_PARAM workParam;
    workParam.workType = cwtGet;
    workParam.strFullUrl = "https://www.baidu.com";

    //设置各种属性参数
    CURL_VERIFY(_setupEasyHandle(m_pCurlEasy, &workParam));
    CURL_VERIFY(_performEasyHandle(m_pCurlEasy, TRUE));

    FTLTRACEA("Get Data from %s, result is %s", workParam.strFullUrl.c_str(), workParam.workParam.c_str());
    CURL_VERIFY(_clearEasyHandleWithParam(m_pCurlEasy, &workParam));

//     if (m_pCurlEasy)
//     {
// 
//         BOOL bWriteToFile = FALSE;
//         FILE* pLocalWrite = NULL;
//         if (bWriteToFile)
//         {
//             pLocalWrite = fopen("test_curl_download.html", "wb");
//             CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_WRITEDATA, pLocalWrite));
//         }
//         else {
//             //设置回调函数, 当满足条件时进行调用,从而实现特定功能
// 
//             //回调参数,如果不设置回调函数, 默认是 FILE* 
//             //CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_WRITEDATA, this));
//         }
//         //TODO: 如果应答的数据大时，write_data会被多次调用，若write_date的返回值不是 size * nmemb时， curl_easy_perform(curl);会报23错误（CURLE_WRITE_ERROR）
// 
//         if (bWriteToFile && pLocalWrite)
//         {
//             fclose(pLocalWrite);
//             pLocalWrite = NULL;
//         }
//     }
}

void CCurlTester::test_high_performance()
{
    FUNCTION_BLOCK_TRACE(1);
    //CURL_WORK_PARAM CurlWorkParam;
    //PCURL_WORK_PARAM pCurlWorkParam = &CurlWorkParam;
    CURLcode retCode = CURL_LAST;
    FTL::CFConversion conv;
    BOOL bRet = FALSE;
    CURL* pCurlEasy = curl_easy_init();
    CPPUNIT_ASSERT(NULL != pCurlEasy);
    if (pCurlEasy)
    {
        std::string strUrls[] = {
            "https://tenfei03.cfp.cn/creative/vcg/800/version23/VCG21154786fd8.jpg",
            "https://tenfei05.cfp.cn/creative/vcg/800/new/VCG21gic20007790-ATV.jpg",
            "https://alifei03.cfp.cn/creative/vcg/800/new/VCG21gic20003915.jpg"
        };
        std::string strResponse;
        //设置TCP连接为长连接
        curl_easy_setopt(pCurlEasy, CURLOPT_TCP_KEEPALIVE, 1L);

        //CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_URL, pCurlWorkParam->strFullUrl.c_str()));
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_VERBOSE, 1));
        //CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_DEBUGFUNCTION, OnDebug));

        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_READFUNCTION, NULL));

        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_WRITEFUNCTION, write_to_file_handle));  //OnWriteData

       /* enable TCP keep-alive for this transfer */

        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_TCP_KEEPALIVE, 1L));

        /* keep-alive idle time to 120 seconds */

        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_TCP_KEEPIDLE, 120L));

        /* interval time between keep-alive probes: 60 seconds */

        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_TCP_KEEPINTVL, 20L));

        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_TIMEOUT, 30));

        /**
        * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
        * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
        */
        //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_CONNECTTIMEOUT, 20));

        for (int i = 0; i < _countof(strUrls); i++)
        {
            CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_URL, strUrls[i].c_str()));
            CString strDownloadFile;
            strDownloadFile.Format(TEXT("curl_download_%d.jpg"), i);
            HANDLE hWriteFile = INVALID_HANDLE_VALUE;

            API_VERIFY(INVALID_HANDLE_VALUE != (hWriteFile = ::CreateFile(strDownloadFile,
                GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)));

            CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_WRITEDATA, hWriteFile));

            CURL_VERIFY(curl_easy_perform(pCurlEasy));
            //FTLTRACE(TEXT("response=%s"), conv.UTF8_TO_TCHAR(strResponse.c_str()));
            SAFE_CLOSE_HANDLE(hWriteFile, INVALID_HANDLE_VALUE);
            hWriteFile = INVALID_HANDLE_VALUE;
        }

        curl_easy_cleanup(pCurlEasy);
        pCurlEasy = NULL;
    }
}

void CCurlTester::test_keepalive()
{
    CURLcode retCode = CURL_LAST;
    for (int i = 0; i < 10; i++)
    {
        //重用连接?
        curl_easy_reset(m_pCurlEasy); //keep alive
    }
}

void CCurlTester::test_multi_functions()
{
    //curl_multi_info_read <== 获取每一个执行完成的操作信息,如果有返回值(结构体指针),则其 msg 字段可以判断是否为 CURLMSG_DONE
    // curl_multi_socket_action 或 curl_multi_perform
    CURLMcode mRetCode = CURLM_LAST;

    //TODO: 获取设置的超时值?
    long timeout_ms = 1000;

    CURLM_VERIFY(curl_multi_add_handle(m_pCurlMulti, m_pCurlEasy));

    CURLM_VERIFY(curl_multi_timeout(m_pCurlMulti, &timeout_ms)); //get timeout ms instead
    CURLM_VERIFY(curl_multi_setopt(m_pCurlMulti, CURLMOPT_MAXCONNECTS, 15));  //TODO: 最大连接数?

    CURLM_VERIFY(curl_multi_remove_handle(m_pCurlMulti, m_pCurlEasy));


    int still_running = 0;
    //启动后将执行所有multi stack中的收发事件。如果栈上是空的直接返回。函数参数running_handles会返回当前还未结束的easy handler个数。
    CURLM_VERIFY(curl_multi_perform(m_pCurlMulti, &still_running));
    int repeats = 0;
    do
    {
        int numfds = 0;
        CURLM_VERIFY(curl_multi_wait(m_pCurlMulti, NULL, 0, timeout_ms, &numfds));
        if(!numfds) { //0 表示或者超时,或者没有等待文件描述符,
            if(repeats++ > 60){
                //time out
                break;
            } else {
                Sleep(10);  // sleep 10 milliseconds
                continue;
            }
        }else{
            repeats = 0;
        }
        CURLM_VERIFY(curl_multi_perform(m_pCurlMulti, &still_running));

         int msgs_left = 0;
        CURLMsg *msg = NULL;
        while ((msg = curl_multi_info_read(m_pCurlMulti, &msgs_left)) != NULL){
            if (msg->msg == CURLMSG_DONE) {
                //msg->easy_handle
                //retCode = return_code = msg->data.result; CURLE_OK==retCode ?
                long http_response_code = -1;
                char* http_url = NULL;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_response_code);
                curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &http_url);
                //curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &xxxx);
                printf("[%s]fetch done, response[%d]\n",http_url,http_response_code );
            }
        }
    }while(still_running);

    //等待超时

    BOOL isOldAPI = TRUE;

    //旧的API使用 curl_multi_fdset 设置 select 或者 poll 模型触发
    if(isOldAPI) {
        fd_set fdread, fdwrite, fdexcep;
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);
        int maxfd = 0;
        timeval timeout = { 0 };
        //get file descriptors from the transfers
        CURLM_VERIFY(curl_multi_fdset(m_pCurlMulti, &fdread, &fdwrite, &fdexcep, &maxfd));
        int rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
        switch(rc){
            case 0:
            default:
                //timeout or readable/writable sockets
                curl_multi_perform(m_pCurlMulti, &still_running);
            break;
        }
    } else {
        int numfds = 0;
        if(mRetCode == CURLM_OK){
            /* wait for activity, timeout or "nothing" */
            CURLM_VERIFY(curl_multi_wait(m_pCurlMulti, NULL, 0, 1000, &numfds));
        }
        //新的 API(官网推荐),实现方便,没有 select 的 1024 限制, 性能高?
        //  TODO: 轮询 multi 上的所有easy句柄，一直阻塞直到至少有一个被触发或者超时
        CURLM_VERIFY(curl_multi_wait(m_pCurlMulti,
                              NULL, //需要监听的socket
                              0,
                              1000,
                              &numfds)); //返回被触发的事件数量,若为0表示超时或者没有事件等待
    }
}

void CCurlTester::test_progress()
{
    CURLcode retCode = CURL_LAST;

    CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_NOPROGRESS, FALSE));
    CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_XFERINFOFUNCTION, progress_callback));
    CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_PROGRESSDATA, reinterpret_cast<void *>(this)));
}

void CCurlTester::test_share_functions()
{
    CURLcode retCode = CURL_LAST;
    _prepareCurlShare();

    //各个 CURL 实例可以共享相关内容, 比如 DNS 解析, Connect 连接等.
    CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_SHARE, m_pCURLShare));
}

void CCurlTester::test_ssl() 
{
    //使用 SSL 时对服务器证书的验证方式:
    //  1.不验证(简单但不安全)
    curl_easy_setopt(m_pCurlEasy, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurlEasy, CURLOPT_SSL_VERIFYHOST, 0L);  // 是否验证主机名匹配(比如通过IP访问服务器时)

    //  2.验证证书,可以避免DNS劫持,中间人攻击等. 但可能会因为服务器更换证书(比如证书到期)而失败?
    curl_easy_setopt(m_pCurlEasy, CURLOPT_CAINFO, "<your certificate path>");
}

void CCurlTester::test_upload() 
{
    BOOL bRet = FALSE;
    CURLcode retCode = CURL_LAST;
    TCHAR szCurModulePath[MAX_PATH] = { 0 };
    API_VERIFY(0 != GetModuleFileName(NULL, szCurModulePath, _countof(szCurModulePath)));

    CURL_WORK_PARAM curlParam;
    curlParam.workType = cwtUpload;
    curlParam.strFullUrl = "http://localhost:10080/file/upload";
    curlParam.strFilePath = szCurModulePath;
    CURL_VERIFY(_setupEasyHandle(m_pCurlEasy, &curlParam));
    CURL_VERIFY(curl_easy_perform(m_pCurlEasy));

#if 0

    std::string strContentLength = "Content-Length: ";
    char cSize[255] = { 0 };
    strContentLength.append(_itoa(dwSize, cSize, 10));
    pHeader = curl_slist_append(pHeader, strContentLength.c_str());
    pHeader = curl_slist_append(pHeader, ToAnsiStr(m_strContentRange.c_str()).c_str());


    //设置读取数据的回调
    curl_easy_setopt(m_pCurlEasy, CURLOPT_READFUNCTION, read_data_callback);
    curl_easy_setopt(m_pCurlEasy, CURLOPT_READDATA, reinterpret_cast<void *>(this));
    curl_easy_setopt(m_pCurlEasy, CURLOPT_INFILESIZE, m_dwUploadTotolSize);
#endif 
}


#if 0
class CFCurlTaskMimePost : public CFCurlTaskBase
{
public:
    curl_mime*          m_pMime;
public:
    CFCurlTaskMimePost(CString strUrl)
        : CFCurlTaskBase(strUrl) 
    {
        m_pMime = NULL;
    }
    ~CFCurlTaskMimePost()
    {
        if (m_pMime)
        {
            curl_mime_free(m_pMime);
            m_pMime = NULL;
        }
    }
    virtual void _OnSetupEasyHandler(CURL* pEasyHandler)
    {
        __super::_OnSetupEasyHandler(pEasyHandler);

        m_pMime = curl_mime_init(pEasyHandler);
        /* Fill in the file upload field */
        curl_mimepart* pSendFile = curl_mime_addpart(m_pMime);

        curl_mime_name(pSendFile, "sendfile");
        curl_mime_filedata(pSendFile, "multi-post.c");

        /* Fill in the filename field */
        curl_mimepart* pFileName = curl_mime_addpart(m_pMime);
        curl_mime_name(pFileName, "filename");
        curl_mime_data(pFileName, "multi-post.c", CURL_ZERO_TERMINATED);


        /* Fill in the submit field too, even if this is rarely needed */
        curl_mimepart* pSubmit = curl_mime_addpart(m_pMime);
        curl_mime_name(pSubmit, "submit");
        curl_mime_data(pSubmit, "send", CURL_ZERO_TERMINATED);

        /* initialize custom header list (stating that Expect: 100-continue is not wanted */
        static const char buf[] = "Expect:";
        m_pHeaderList = curl_slist_append(m_pHeaderList, buf);

        curl_easy_setopt(pEasyHandler, CURLOPT_HTTPHEADER, m_pHeaderList);
        curl_easy_setopt(pEasyHandler, CURLOPT_MIMEPOST, m_pMime);
    }
};

//multi-part form post
class CFCurlTaskHttpPost : public CFCurlTaskBase 
{
public:
    struct curl_httppost *m_pFormPost;
    //struct curl_httppost *m_pLastptr;
public:
    CFCurlTaskHttpPost(CString strUrl)
        : CFCurlTaskBase(strUrl)
    {
        m_pFormPost = NULL;
    }
    ~CFCurlTaskHttpPost()
    {
        if (m_pFormPost)
        {
            curl_formfree(m_pFormPost);
            m_pFormPost = NULL;
        }
    }
    virtual void _OnSetupEasyHandler(CURL* pEasyHandler)
    {
        CURLcode retCode = CURL_LAST;
        CURLFORMcode formCode = CURL_FORMADD_LAST;
        curl_httppost* pLastPost = NULL;

        //curl_easy_perform 时加载文件内容
        CURLFORM_VERIFY(curl_formadd(&m_pFormPost, &pLastPost,
            CURLFORM_COPYNAME, "sendfile",
            CURLFORM_FILE, "postit2.c",
            CURLFORM_END));

        //文件名
        CURLFORM_VERIFY(curl_formadd(&m_pFormPost,
            &pLastPost,
            CURLFORM_COPYNAME, "filename",
            CURLFORM_COPYCONTENTS, "postit2.c",
            CURLFORM_END));

        //Fill in the submit field too, even if this is rarely needed
        CURLFORM_VERIFY(curl_formadd(&m_pFormPost,
            &pLastPost,
            CURLFORM_COPYNAME, "submit",
            CURLFORM_COPYCONTENTS, "send",
            CURLFORM_END));

        CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_HTTPPOST, m_pFormPost));

    }
};
#endif 


FTL::CFConversion g_conv;

void CCurlTester::sample_test()
{
}

void CCurlTester::_curlMgr_get(FTL::CFCurlManager& rCurlManager)
{
    int LOOP_COUNT = 10;
    for (int i = 0; i < LOOP_COUNT; i++)
    {
        FTL::CFCurlTaskGet* pGet = new FTL::CFCurlTaskGet();
        pGet->SetUrl(TEXT("http://127.0.0.1:8080/check/version"));
        rCurlManager.AddTask(FTL::CFCurlTaskPtr(pGet));
    }
}

void CCurlTester::_curlMgr_download(FTL::CFCurlManager& rCurlManager)
{
    FTL::CFCurlTaskDownload* pDownload = new FTL::CFCurlTaskDownload();
    pDownload->SetUrl(TEXT("http://127.0.0.1/nginx/download/test_git-bash.exe"));
    pDownload->SetLocalFile(TEXT("test_git-bash.exe"));

    rCurlManager.AddTask(FTL::CFCurlTaskPtr(pDownload));
}

void CCurlTester::_curlMgr_multiPartPost(FTL::CFCurlManager& rCurlManager)
{
    FTL::CFFileFinder fileFinder;
    fileFinder.SetCallback(this, LPVOID(1000));

    fileFinder.Find(TEXT("E:\\boost_1000_files"), 
        TEXT("*.txt;*.md;*.hpp;*.h;*.ipp"));


    CString strUrl;

    int BATCH_SIZE = 10;

    int batchIndex = 0;
    FTL::CFCurlTaskMultiPartPost* pPostTask = NULL;
    for (SEARCH_FILES_MAP::iterator iter = m_searchFiles.begin();
        iter != m_searchFiles.end();
        ++iter)
    {
        FTLTRACE(TEXT("Will add upload file %s"), iter->second);
        if (0 == batchIndex)
        {
            pPostTask = new FTL::CFCurlTaskMultiPartPost();
            strUrl = TEXT("http://127.0.0.1:8080/file/upload");
        }
        batchIndex++;

        pPostTask->AddMultiPartFile(TEXT("file"), iter->second);

        if (BATCH_SIZE == batchIndex)
        {
            pPostTask->SetUrl(strUrl);
            rCurlManager.AddTask(FTL::CFCurlTaskPtr(pPostTask));
            pPostTask = NULL;

            batchIndex = 0;
        }
    }

    if (pPostTask)
    {
        pPostTask->SetUrl(strUrl);
        rCurlManager.AddTask(FTL::CFCurlTaskPtr(pPostTask));
    }
}

//测试各种异常情况
void CCurlTester::_curlMgr_error_case(FTL::CFCurlManager& rCurlManager)
{
    {
        //上传时指定一个不存在的文件(比如等待上传前,文件已被删除)
        //CCurlTester::OnTaskComplete|failed ret=26(CURLE_READ_ERROR), responseCode=0, result=

        FTL::CFCurlTaskMultiPartPost* pPostTask = new FTL::CFCurlTaskMultiPartPost();
        pPostTask->AddMultiPartFile(TEXT("file"), TEXT("D:\\NotExistFile.txt"));

        if (pPostTask)
        {
            CString strUrl = TEXT("http://127.0.0.1:8080/file/upload");

            pPostTask->SetUrl(strUrl);
            rCurlManager.AddTask(FTL::CFCurlTaskPtr(pPostTask));
        }
    }
}

void CCurlTester::test_curlMgr()
{
    FUNCTION_BLOCK_TRACE(1);
    //TODO: 看起来  CFCurlTaskMimePost(CURLOPT_MIMEPOST) 和  CFCurlTaskHttpPost(CURLOPT_HTTPPOST) 功能一样,只是调用的方法不同?

    BOOL bRet = FALSE;
    FTL::CFCurlManager curlManager;
    curlManager.SetCallBack(this, this);
    curlManager.Start();
    
    FTL::CFElapseCounter elapseCounter;

    //_curlMgr_get(curlManager);
    //_curlMgr_download(curlManager);
    _curlMgr_multiPartPost(curlManager);
    //_curlMgr_error_case(curlManager);

    //Sleep(60 * 1000);
    //等待所有任务都结束
    while (curlManager.GetRemainTasks() > 0)
    {
        Sleep(1000);
    }

    curlManager.Stop();

    FTL::FormatMessageBox(NULL, TEXT("Elapse Time"), MB_OK, TEXT("elapse time: %d ms"),
        elapseCounter.GetElapseTime() / NANOSECOND_PER_MILLISECOND);
}

#endif //TEST_NET_CURLd