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
    //ע��: ��� curl_off_t , �����漰����ͬ�Ļ���(32/64��), ��Ԥ����ĸ�ʽ���� CURL_FORMAT_CURL_OFF_T 
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
    //���� fwrite ��ʵ�ַ�ʽ:
    //  size -- ÿ����Ԫ���ֽ���;
    //  nmemb -- Ҫд�ĵ�Ԫ����,

    CCurlTester* pThis = reinterpret_cast<CCurlTester*>(userp);
    
    if (pThis && buffer && size > 0)  //pThis && 
    {
        FTL::CFConversion conv;
        FTLTRACE(TEXT("writeData:%s"), conv.UTF8_TO_TCHAR((const char*)buffer));
    }

    //����ֵ��ʾ������ֽڸ���, ������� size*nmemb ��ʾ�д���
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

    //  ������� 1, ��ʾҪȡ��. ���� 0 ����
    return 0;
}

void CCurlTester::setUp()
{
    m_dwTotalLongth = 0;
    CURLcode retCode = CURL_LAST;
    long flags = CURL_GLOBAL_ALL;  //ȱʡ ��ʼ�����еĿ��ܵĵ���(SSL + Win32 �׽���),CURL_GLOBAL_DEFAULT

    //��ʼ�� libcurl ��
    CURL_VERIFY(curl_global_init(flags));

    //��ȡ easy interface �Ľӿ�ָ��, ���ƴ�һ�� FILE, һ�� CURL ��ζһ���Ự.
    m_pCurlEasy = curl_easy_init();
    CPPUNIT_ASSERT(NULL != m_pCurlEasy);

    m_pCurlMulti = curl_multi_init();
    CPPUNIT_ASSERT(NULL != m_pCurlMulti);

    m_pCURLShare = curl_share_init();
    CPPUNIT_ASSERT(NULL != m_pCURLShare);
}

void CCurlTester::tearDown()
{
    //�����Ự,�ͷ��ڴ�
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

    //�������� libcurl ��
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
    CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_FOLLOWLOCATION, 1L));  //ָ���Ƿ�ָ�������ض���

    //���ý��Ȼص�, �Ӷ����� cancel
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
        //��ָ���ַ��� POST ��url��(TODO: simplepost.c ��û������ CURLOPT_POST )
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_POST, 1L));
        //��Ҫ POST ������, �� json �ַ���.TODO: �Ƿ����ͨ���ص��ķ�ʽ���ṩ
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_POSTFIELDS, pCurlWorkParam->workParam.c_str()));
        //ָ�� POST �ĳ���,
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
        //�ϴ�ָ���ļ�
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

        //ָ�� PUT ����. TODO: ���ںܶ��ϴ�ʵ�ʶ��� POST(���һ�Ҫ������ meta ��Ϣ, ���ļ���, ��С��)
        CURL_VERIFY(curl_easy_setopt(pCurlEasy, CURLOPT_UPLOAD, 1L));  //TODO:ʾ��(fileupload.c) �� url �� file://xxx
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

    CURL_VERIFY(curl_easy_perform(pCurlEasy));  //ִ������
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
    //���ø������ӵĹ�����,���ÿ�����Ӷ�Ҫ����. 
    //�������Ժ�, ���õ� CURL ʵ����ʹ��. 
    //  curl_easy_setopt(m_pCurlEasy, CURLOPT_SHARE, m_ShareHandle);

    CURLSHcode shCode = CURLSHE_LAST;
    
    CURLSH_VERIFY(curl_share_setopt(m_pCURLShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT));

    CURLSH_VERIFY(curl_share_setopt(m_pCURLShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE));

    //DNS����,����ÿ�����Ӷ�Ҫ����һ��,����������̶߳�����,���ܻ���� CURLE_COULDNT_RESOLVE_HOST(6) ���� 
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
 *   ������ṹ,������������ Header/�Զ���resolveӳ��(CURLOPT_RESOLVE) �ȵ��б�,ʹ����Ϻ����ʹ�� curl_slist_free_all �ͷ�
 **********************************************************************************************************************/
void CCurlTester::test_curl_slist()
{
    CURLcode retCode = CURL_LAST;

    curl_slist *pHeader = NULL;

    //TODO: ����ʹ�÷��صı���? �����µ� header ��Ϊ�����ͷ?
    pHeader = curl_slist_append(pHeader, "Accept: */*");
    curl_slist *pCheckHeader = pHeader;

    pHeader = curl_slist_append(pHeader, "Content-Type: application/json"); // application/x-www-form-urlencoded
    CPPUNIT_ASSERT(pHeader == pCheckHeader);   //���� ������NULL, ��˷��صĽ������ǰһ��.�µ�ֵ������ next �ڵ���

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

    //Ȼ��Ϳ��Խ������õ� easy curl ʵ��������(header)
    CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_HTTPHEADER, pHeader));
    //ִ��

    //�ͷ�
    curl_slist_free_all(pHeader);
    pHeader = NULL;
}

void CCurlTester::test_curlVersion()
{
    FTL::CFConversion convVer, convVerInfo, convHost, convSSLVersion;
    std::string strCurlVersion = curl_version();
    //CPPUNIT_ASSERT_EQUAL(std::string("libcurl/7.82.0-DEV Schannel zlib/1.2.11"), strCurlVersion);

    //��� curl �İ汾��Ϣ,�Լ�֧�ֵ�����,����:
    //  CURL_VERSION_HTTP2  <== �Ƿ�֧�� http2
    //  CURL_VERSION_HTTP3  <== �Ƿ�֧�� http3(QUIC + HTTP/3?)
    //  CURL_VERSION_ALTSVC <== �Ƿ�֧�� Alt-svc
    curl_version_info_data *pVerInfoData = curl_version_info(CURLVERSION_NOW);  //���ص��Ǿ�̬������ָ��, ��˲���Ҫ�ͷ�
    CPPUNIT_ASSERT(NULL != pVerInfoData);
    if (pVerInfoData)
    {
        FTL::CFStringFormater fmtFeatures;
        //20220405, ��װ��ʽ: vcpkg install curl[ssl]:x64-windows �Ľ��(ȱʡSchannel)
        //  age=9, version=7.82.0-DEV, host=Windows, sslVersion=Schannel,
        //  features = 0x110c2bdd(IPV6|SSL|LIBZ|NTLM|ASYNCHDNS|SPNEGO|LARGEFILE|SSPI|CURLDEBUG|KERBEROS5|UNIX_SOCKETS|ALTSVC|HSTS)

        //20220425 ��װ��ʽ: vcpkg install curl[openssl]:x64-windows �Ľ��(ʹ��OpenSSL)
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

            // CURLUE_NO_PORT (����Ҫ�ж˿�,������ȱʡ�� 80/443)
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

    //���ø������Բ���
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
//             //���ûص�����, ����������ʱ���е���,�Ӷ�ʵ���ض�����
// 
//             //�ص�����,��������ûص�����, Ĭ���� FILE* 
//             //CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_WRITEDATA, this));
//         }
//         //TODO: ���Ӧ������ݴ�ʱ��write_data�ᱻ��ε��ã���write_date�ķ���ֵ���� size * nmembʱ�� curl_easy_perform(curl);�ᱨ23����CURLE_WRITE_ERROR��
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
        //����TCP����Ϊ������
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
        * ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ�����
        * ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳���
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
        //��������?
        curl_easy_reset(m_pCurlEasy); //keep alive
    }
}

void CCurlTester::test_multi_functions()
{
    //curl_multi_info_read <== ��ȡÿһ��ִ����ɵĲ�����Ϣ,����з���ֵ(�ṹ��ָ��),���� msg �ֶο����ж��Ƿ�Ϊ CURLMSG_DONE
    // curl_multi_socket_action �� curl_multi_perform
    CURLMcode mRetCode = CURLM_LAST;

    //TODO: ��ȡ���õĳ�ʱֵ?
    long timeout_ms = 1000;

    CURLM_VERIFY(curl_multi_add_handle(m_pCurlMulti, m_pCurlEasy));

    CURLM_VERIFY(curl_multi_timeout(m_pCurlMulti, &timeout_ms)); //get timeout ms instead
    CURLM_VERIFY(curl_multi_setopt(m_pCurlMulti, CURLMOPT_MAXCONNECTS, 15));  //TODO: ���������?

    CURLM_VERIFY(curl_multi_remove_handle(m_pCurlMulti, m_pCurlEasy));


    int still_running = 0;
    //������ִ������multi stack�е��շ��¼������ջ���ǿյ�ֱ�ӷ��ء���������running_handles�᷵�ص�ǰ��δ������easy handler������
    CURLM_VERIFY(curl_multi_perform(m_pCurlMulti, &still_running));
    int repeats = 0;
    do
    {
        int numfds = 0;
        CURLM_VERIFY(curl_multi_wait(m_pCurlMulti, NULL, 0, timeout_ms, &numfds));
        if(!numfds) { //0 ��ʾ���߳�ʱ,����û�еȴ��ļ�������,
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

    //�ȴ���ʱ

    BOOL isOldAPI = TRUE;

    //�ɵ�APIʹ�� curl_multi_fdset ���� select ���� poll ģ�ʹ���
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
        //�µ� API(�����Ƽ�),ʵ�ַ���,û�� select �� 1024 ����, ���ܸ�?
        //  TODO: ��ѯ multi �ϵ�����easy�����һֱ����ֱ��������һ�����������߳�ʱ
        CURLM_VERIFY(curl_multi_wait(m_pCurlMulti,
                              NULL, //��Ҫ������socket
                              0,
                              1000,
                              &numfds)); //���ر��������¼�����,��Ϊ0��ʾ��ʱ����û���¼��ȴ�
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

    //���� CURL ʵ�����Թ����������, ���� DNS ����, Connect ���ӵ�.
    CURL_VERIFY(curl_easy_setopt(m_pCurlEasy, CURLOPT_SHARE, m_pCURLShare));
}

void CCurlTester::test_ssl() 
{
    //ʹ�� SSL ʱ�Է�����֤�����֤��ʽ:
    //  1.����֤(�򵥵�����ȫ)
    curl_easy_setopt(m_pCurlEasy, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurlEasy, CURLOPT_SSL_VERIFYHOST, 0L);  // �Ƿ���֤������ƥ��(����ͨ��IP���ʷ�����ʱ)

    //  2.��֤֤��,���Ա���DNS�ٳ�,�м��˹�����. �����ܻ���Ϊ����������֤��(����֤�鵽��)��ʧ��?
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


    //���ö�ȡ���ݵĻص�
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

        //curl_easy_perform ʱ�����ļ�����
        CURLFORM_VERIFY(curl_formadd(&m_pFormPost, &pLastPost,
            CURLFORM_COPYNAME, "sendfile",
            CURLFORM_FILE, "postit2.c",
            CURLFORM_END));

        //�ļ���
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

//���Ը����쳣���
void CCurlTester::_curlMgr_error_case(FTL::CFCurlManager& rCurlManager)
{
    {
        //�ϴ�ʱָ��һ�������ڵ��ļ�(����ȴ��ϴ�ǰ,�ļ��ѱ�ɾ��)
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
    //TODO: ������  CFCurlTaskMimePost(CURLOPT_MIMEPOST) ��  CFCurlTaskHttpPost(CURLOPT_HTTPPOST) ����һ��,ֻ�ǵ��õķ�����ͬ?

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
    //�ȴ��������񶼽���
    while (curlManager.GetRemainTasks() > 0)
    {
        Sleep(1000);
    }

    curlManager.Stop();

    FTL::FormatMessageBox(NULL, TEXT("Elapse Time"), MB_OK, TEXT("elapse time: %d ms"),
        elapseCounter.GetElapseTime() / NANOSECOND_PER_MILLISECOND);
}

#endif //TEST_NET_CURLd