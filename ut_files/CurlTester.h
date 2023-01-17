#pragma once

#include "TestConfig.h"
#include <cppunit/extensions/HelperMacros.h>
//#include <ftlFunctional.h>

/***********************************************************************************************************************
* TODO:
* 
* 环境准备:
*   1. 通过 vcpkg 安装 curl (注意:配置好 VS 的 集成)
*      Win:vcpkg install curl[openssl]:x64-windows  <== 选择 OpenSSL 作为SSL实现, 会安装最新的 openssl 3.x 版本
*         vcpkg install curl[ssl]:x86-windows curl[ssl]:x64-windows   <== 测试 OK <== 根据平台自动选择缺省SSL并安装
*        Win: vcpkg install curl[ssl]:x86-windows-static curl[ssl]:x64-windows-static  <== 测试能安装上,但和当前的工程配置似乎不同
*      Linux: vcpkg install curl[openssl]  , 编译示例: g++ simple.c `pkg-config --libs --cflags libcurl` -o test
*        
*      Mac: TODO: 
*   2. #include <curl/curl.h>
*   3. 根据 Debug 或 Release, 链接 libcurl-d.lib 或 libcurl.lib
*   4. 经过实测,同时支持 (x86|x64) x (Windows|...) x (Debug|Release) 
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
    std::string  strFullUrl;    // 带 host:port/url 的完整 URL
    std::string  strReffer;     // CURLOPT_REFERER
    std::wstring strFilePath;  // upload or download file path
    std::string  workParam;

    void* clearInfo;            //保存需要在 _clearEasyHandleWithParam 中释放的信息

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
    //可共享?
    //FTL::binarystream   m_streamData;

    static size_t read_from_file_handle(char *buffer, size_t size, size_t count, void *request);
    static size_t write_to_file_handle(char *buffer, size_t size, size_t count, void *request);
    static size_t write_to_string(char *buffer, size_t size, size_t count, void *request);
    static size_t read_header_callback(char *buffer, size_t size, size_t count, void *userdata);

    static size_t curlWriteDataFun(void *buffer, size_t size, size_t nmemb, void *userp);

    //进度回调, 返回0表示继续, 返回 1 表示取消
    static int progress_callback(void *clientp, 
        curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    //上传数据时,获取数据的回调
    static size_t read_data_callback(void *ptr, size_t size, size_t nmemb, void *request);

private:
    void _curlMgr_get(FTL::CFCurlManager& rCurlManager);
    void _curlMgr_download(FTL::CFCurlManager& rCurlManager);
    void _curlMgr_multiPartPost(FTL::CFCurlManager& rCurlManager);
    void _curlMgr_error_case(FTL::CFCurlManager& rCurlManager);
};

#endif //TEST_NET_CURL
