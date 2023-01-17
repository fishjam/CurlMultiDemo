#pragma once

#include "TestConfig.h"
#include <cppunit/extensions/HelperMacros.h>
//#include <ftlFunctional.h>

/***********************************************************************************************************************
* TODO:
* 
* ����׼��:
*   1. ͨ�� vcpkg ��װ curl (ע��:���ú� VS �� ����)
*      Win:vcpkg install curl[openssl]:x64-windows  <== ѡ�� OpenSSL ��ΪSSLʵ��, �ᰲװ���µ� openssl 3.x �汾
*         vcpkg install curl[ssl]:x86-windows curl[ssl]:x64-windows   <== ���� OK <== ����ƽ̨�Զ�ѡ��ȱʡSSL����װ
*        Win: vcpkg install curl[ssl]:x86-windows-static curl[ssl]:x64-windows-static  <== �����ܰ�װ��,���͵�ǰ�Ĺ��������ƺ���ͬ
*      Linux: vcpkg install curl[openssl]  , ����ʾ��: g++ simple.c `pkg-config --libs --cflags libcurl` -o test
*        
*      Mac: TODO: 
*   2. #include <curl/curl.h>
*   3. ���� Debug �� Release, ���� libcurl-d.lib �� libcurl.lib
*   4. ����ʵ��,ͬʱ֧�� (x86|x64) x (Windows|...) x (Debug|Release) 
*   
* 
* CMakeLists.txt:
*   FIND_PACKAGE(CURL CONFIG REQUIRED)
*   cmake_print_variables(CURL_FOUND CURL_INCLUDE_DIRS CURL_LIBS CURL_LIBRARIES)
*   target_include_directories(test PRIVATE ${CURL_INCLUDE_DIRS})
*   target_link_libraries(main PRIVATE ${CURL_LIBRARIES})
*
* xmake.lua:
*   add_requires("libcurl")
*   add_packages("libcurl")
**********************************************************************************************/

/**********************************************************************************************

**********************************************************************************************/

#if TEST_NET_CURL

#include <curl/curl.h>
#include <ftlFile.h>
#include <ftlCurl.h>

enum CURL_WORK_TYPE
{
    cwtGet,
    cwtPost,
    cwtDownload,
    cwtUpload
};

typedef struct CURL_WORK_PARAM {
    CURL_WORK_TYPE workType;
    //std::string  strHost;
    std::string  strFullUrl;    // �� host:port/url ������ URL
    std::string  strReffer;     // CURLOPT_REFERER
    std::wstring strFilePath;  // upload or download file path
    std::string  workParam;

    void* clearInfo;            //������Ҫ�� _clearEasyHandleWithParam ���ͷŵ���Ϣ

    CURL_WORK_PARAM()
    {
        clearInfo = NULL;
    }
}*PCURL_WORK_PARAM;

class CCurlTester : 
    public CPPUNIT_NS::TestFixture,
    public FTL::IFileFindCallback,
    public FTL::ICurlMgrCallBack
{
public:
    //IFileFindCallback
    DWORD    m_dwTotalLongth;
    virtual FTL::FileFindResultHandle OnFindFile(LPCTSTR pszFilePath, const WIN32_FIND_DATA& findData, LPVOID pParam);
    virtual FTL::FileFindResultHandle OnError(LPCTSTR pszFilePath, DWORD dwError, LPVOID pParam);

    //ICurlMgrCallBack
    virtual void OnTaskTimeOut(FTL::CFCurlTaskBase* pTask, CURLcode retCode, LPVOID pParam);
    virtual void OnTaskComplete(FTL::CFCurlTaskBase* pTask, CURLcode retCode, long retResponseCode, LPVOID pParam);
    
public:
    CPPUNIT_TEST_SUITE(CCurlTester);
    CPPUNIT_TEST(test_curl_mime);
    CPPUNIT_TEST(test_curl_slist);
    CPPUNIT_TEST(test_curlVersion);
    CPPUNIT_TEST(test_curl_urlApi);
    CPPUNIT_TEST(test_easy_functions);
    CPPUNIT_TEST(test_high_performance);
    CPPUNIT_TEST(test_keepalive);
    CPPUNIT_TEST(test_multi_functions);
    CPPUNIT_TEST(test_progress);
    CPPUNIT_TEST(test_prov2_fs_upload);
    CPPUNIT_TEST(test_share_functions);
    CPPUNIT_TEST(test_ssl);
    CPPUNIT_TEST(test_upload);
    CPPUNIT_TEST(test_curlMgr);
    CPPUNIT_TEST(sample_test);
    CPPUNIT_TEST_SUITE_END();

    DECLARE_DEFAULT_TEST_CLASS_WITHOUT_SETUP_TEARDOWN(CCurlTester);
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_curl_mime();
    void test_curl_slist();
    void test_curlVersion();
    void test_curl_urlApi();
    void test_easy_functions();
    void test_high_performance();
    void test_keepalive();
    void test_multi_functions();
    void test_progress();
    void test_prov2_fs_upload();
    void test_share_functions();
    void test_ssl();
    void test_upload();
    void test_curlMgr();
    void sample_test();
private:
    typedef std::map<CString, CString> SEARCH_FILES_MAP;   //filename => fullPath
    SEARCH_FILES_MAP  m_searchFiles;

    CURL            *m_pCurlEasy;
    CURLM           *m_pCurlMulti;
    CURLSH          *m_pCURLShare;

    //CURLcode _open(const char* pHost);

    CURLcode _setupEasyHandle(CURL* pCurlEasy, PCURL_WORK_PARAM pCurlWorkParam);
    CURLcode _performEasyHandle(CURL* pCurlEasy, BOOL bDumpInfo = TRUE);
    CURLcode _clearEasyHandleWithParam(CURL* pCurlEasy, PCURL_WORK_PARAM pCurlWorkParam);
    void _prepareCurlShare();

    std::string     m_strDataBuffer;
    std::string     m_strCookie;
    //�ɹ���?
    //FTL::binarystream   m_streamData;

    static size_t read_from_file_handle(char *buffer, size_t size, size_t count, void *request);
    static size_t write_to_file_handle(char *buffer, size_t size, size_t count, void *request);
    static size_t write_to_string(char *buffer, size_t size, size_t count, void *request);
    static size_t read_header_callback(char *buffer, size_t size, size_t count, void *userdata);

    static size_t curlWriteDataFun(void *buffer, size_t size, size_t nmemb, void *userp);

    //���Ȼص�, ����0��ʾ����, ���� 1 ��ʾȡ��
    static int progress_callback(void *clientp, 
        curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    //�ϴ�����ʱ,��ȡ���ݵĻص�
    static size_t read_data_callback(void *ptr, size_t size, size_t nmemb, void *request);

private:
    void _curlMgr_get(FTL::CFCurlManager& rCurlManager);
    void _curlMgr_download(FTL::CFCurlManager& rCurlManager);
    void _curlMgr_multiPartPost(FTL::CFCurlManager& rCurlManager);
    void _curlMgr_error_case(FTL::CFCurlManager& rCurlManager);
};

#endif //TEST_NET_CURL
