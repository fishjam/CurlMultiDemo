#ifndef FTL_CURL_H
#define FTL_CURL_H
#pragma once

/*************************************************************************************************************
* https://blog.csdn.net/FPGATOM/article/details/104959606
* 
* TODO:
*   1. CURLSH *m_pCURLShare = curl_share_init();   //设置多个连接中共享的部分, 比如 DNS 解析
*      应该是按连接的 URL 设置共享?
*   2. multi 接口的用法历史(尚未确认?)
*      a. curl_multi_fdset + select 
*      b. curl_multi_wait 
*      c. curl_multi_poll
*   3. #if !CURL_AT_LEAST_VERSION(7, 80, 0)  <== 检测版本至少需要多少. 
* 
* libCurl(https://curl.se/libcurl/ , https://github.com/curl/curl) -- 基于C的多协议的URL传输库,
*   支持很多协议(DICT/FILE/FTP/HTTP/HTTPS/LDAP/POP3/RTSP/SCP/SMTP/TELNET/TFTP 等)
*   通常用于 HTTP/HTTPS + POST/GET/PUT 等, 支持代理,Cookies,用户密码加密认证等;
*   支持绝大多数平台, 如 Dos/Win/Mac/各种Linux+Unix 等
*   官方示例: https://curl.se/libcurl/c/allexamples.zip ,使用: gcc xxxx.c -lcurl , 编译成功的话,有 a.out 
*
************************************************************************************************************/

/*************************************************************************************************************
* 源码编译(https://curl.se/docs/install.html)
*   1.Win 上使用 VS2010 编译
*     cmake -DBUILD_CURL_EXE=OFF -DBUILD_SHARED_LIBS=OFF -DCURL_USE_OPENSSL=ON -DCMAKE_INSTALL_PREFIX=E:\Build\Curl 
*       -G "Visual Studio 10 2010 Win64" ..
*     cmake --build . --config Debug --target INSTALL
************************************************************************************************************/

/*************************************************************************************************************
* 两种使用方式:
*   easy interface <== 采用阻塞的方式发送单条请求
*   multi interface <== 采用组合的方式可以一次性发送多条请求数据,多个请求异步进行
*     
*
* easy 接口(CURL *)使用流程:
*   1.curl_global_init <== 初始化 libcurl 库,似乎如果没有调用的话,会自动调用?
*   2.curl_easy_init <== 创建 CURL* 实例
*   3.curl_easy_setopt <== 设置各种属性和回调函数,通过 CURLoption 设置各种选项,类似 ioctl 函数
*     CURLOPT_URL 是必设选项,
*   4.curl_easy_perform <== 按照设置执行会话,此函数为阻塞调用, 在请求过程中(其他线程?或回调函数?)可以
*       通过 curl_easy_getinfo 获取该次请求的相关信息,如 响应码,下载的URL等.
*   5.curl_easy_cleanup <== 释放 CURL* 实例,在释放以前,可以多次设置新的 URL 并进行 perform, 从而重用连接.
*       TODO:在 multi 时实测发现似乎并不会释放,而是会重用.
*   6.curl_global_cleanup <== 析构 libcurl 库
*
* multi 接口(CURLM *)使用流程(将多个 easy handler 加入到一个stack中，同时发送请求, 异步非阻塞)
*   1.curl_global_init <== 初始化 libcurl 库
*   2.curl_multi_init <== 初始化一个multi handler对象, 
*     a.可以通过 curl_multi_setopt 设置属性(比如最大连接数 CURLMOPT_MAXCONNECTS)
*     b.可以通过 curl_share_init + curl_share_setopt 设置共享信息, 比如 CURL_LOCK_DATA_DNS 
*   3.curl_easy_init + curl_easy_setopt <== 初始化多个 easy handler 对象，并进行相关设置
*     a.建议通过 curl_easy_setopt 设置以下属性:
*       CURLOPT_SHARE = m_pCURLShare <== 设置各个连接需要共享的配置信息;
*       CURLOPT_NOSIGNAL = 1 <== 不使用信号唤醒,避免崩溃
*       CURLOPT_PRIVATE=xxx <== 设置各个 easy handler 的私有对象,类似上下文(Context),从而可以定制化. TODO: 怎么获取? get_opt?
*   4.curl_multi_add_handle <== 把 easy handler 加入 multi handler 对象中
*   5.进入 curl_multi_perform(异步执行请求,非阻塞调用,立刻返回,输出参数表示还在执行的任务数) 
*      + curl_multi_info_read(读取结果消息队列中的消息) 的循环, 进行并发访问(TODO: 是否可以动态添加?) <== 这种是 busyloop ?
*     TODO: 能在任何时候增加一个esay handler 给 multi 模式，即使 easy 已经在执行传输操作了
*     为了避免 busy loop, 需要阻塞等待有事件发生或超时才返回,从而读写数据;
*       a. curl_multi_poll <== 新版本才支持, 推荐方式, 等待时可被 curl_multi_wakeup 激活提前返回,如果没有socket,会等待超时
*       b. TODO: curl_multi_wait ? 实测不会阻塞,会直接返回,成为busyloop . 
*          TODO:文档说如果没有文件描述符可以等待(即所有的socket都执行完?),会立即返回(没有意义了?)
*       c. curl_multi_fdset + select 阻塞调用, curl_multi_timeout 可为 select 提供合适的超时值; 有 1024 连接的上限
*     
*   6.curl_multi_remove_handle <== 将结束的handle对象进行移除, 不再管理; 可以 curl_easy_cleanup 销毁,或重设 URL 等参数后重用.
*   7.curl_multi_cleanup <== 清除 multi handler 对象
* 
* 
* CURLoption <== 各种支持的选项,是 CURLOPT_Xxx 的各种枚举, 有 200+ 种
*   CURLOPT_ALTSVC <== 设置一个文件名(如 "altsvc.txt"), libcurl 会自动用于 altsvc 的 cache ? 有什么用处?
*   CURLOPT_ALTSVC_CTRL <== 设置哪一个版本的http支持. 如 (long)(CURLALTSVC_H1|CURLALTSVC_H2|CURLALTSVC_H3)
*   CURLOPT_POST/CURLOPT_PUT/CURLOPT_NOBODY(HEAD) <== 设置 http 的方式
*      NOBODY的话表示是 HEAD 请求,如只想查询文件的修改时间,大小等属性,而不想获取实际数据时
*   CURLOPT_ACCEPT_ENCODING <== 设置对特定压缩方式自动解码,支持的方式有 br,gzip,deflate. TODO: 如设置为空(""),表示都支持?
*   CURLOPT_BUFFERSIZE <== 指定接收缓冲区的首选大小(单位为字节),默认值为 CURL_MAX_WRITE_SIZE(16KB).
*   CURLOPT_CURLU(7.65+ 以上版本支持) <== 通过 CURLU 变量设置要访问的服务器信息,可通过 curl_url_set(curlu, CURLUPART_URL, ...) 的方式设置其中的各个部分
*   CURLOPT_CUSTOMREQUEST <== 自定义 Method? 比如设置成 "POST" 或 "NOTIFY"? 标准方法已有 CURLOPT_POST 等枚举
*   CURLOPT_DEFAULT_PROTOCOL <== 设置缺省协议? 示例:"https"
*   CURLOPT_DNS_CACHE_TIMEOUT <== DNS缓存超时时间,默认为 60 秒.如设置为 -1, 则永远不清缓存
*   CURLOPT_DNS_USE_GLOBAL_CACHE <== 使用系统DNS缓存,默认使用 libcurl 自身的DNS缓存
*   CURLOPT_ERRORBUFFER <== 设置接受错误消息的缓存, 必须在 curl_easy_perform 期间有效.
*   CURLOPT_FOLLOWLOCATION / CURLOPT_MAXREDIRS <== 设置是否自动处理 302 重定向以及最大重定向次数. 可通过 CURLINFO_REDIRECT_URL 获取地址
*   CURLOPT_FORBID_REUSE <== TODO: 是否使用长链接. 设置为1,在完成交互以后强迫断开连接，不重用
*   CURLOPT_FRESH_CONNECT <== TODO: 设置为1，强制获取一个新的连接，替代缓存中的连接
*   CURLOPT_HEADERFUNCTION 和 CURLOPT_HEADERDATA <== 设置接收到http头相应时的回调函数 和 回调参数
*   CURLOPT_HTTPHEADER <== 设置请求头(通过 curl_slist_append 进行自定义)
*   CURLOPT_HTTP_VERSION <== TODO: 可以指定http 的版本,比如 (long)CURL_HTTP_VERSION_3) , 
*     但如果(编译的curl/网站?)不支持的话,会返回 CURLE_UNSUPPORTED_PROTOCOL 错误
*   CURLOPT_IPRESOLVE <== 指定域名解析模式,支持 CURL_IPRESOLVE_WHATEVER(默认),支持 IPv4/v6
*   CURLOPT_MAX_RECV_SPEED_LARGE/CURLOPT_MAX_SEND_SPEED_LARGE <== 设置限速(下载/上传 过程中最大速度),单位为 Bytes/Sec
*   CURLOPT_MIMEPOST <== 可以设置 curl_mime 实例,表示 MIME POST 数据(比如文件上传等) 
*   CURLOPT_NOPROGRESS(false) + CURLOPT_PROGRESSFUNCTION + CURLOPT_PROGRESSDATA <== 设置是否调用数据传输进度的回调函数,
*     如果启用进度回调的话, 一秒调用一次.
*   CURLOPT_NOSIGNAL(0) <== TODO: 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。如果不设置这个选项,
*     libcurl将会发信号打断这个wait从而可能导致程序crash。 在多线程处理场景下使用超时选项时，会忽略signals对应的处理函数。
*   CURLOPT_POSTFIELDS/CURLOPT_POSTFIELDSIZE <== 传递一个作为 "POST" 操作的所有数据的字符串(const char*)和其字节大小(long)
*     TODO:可以不用指定 SIZE(会自动计算?)
*   CURLOPT_PRIVATE <== 设置私有数据(类似 Context),从而可以在 curl_multi_perform 执行后区分不同的 easy 句柄.
*   CURLOPT_RANGE/CURLOPT_RESUME_FROM(小于2G)/CURLOPT_RESUME_FROM_LARGE(大于2G) <== 断点续传相关设置
*   CURLOPT_READFUNCTION 和 CURLOPT_READDATA <== 设置上传数据时的回调函数 和 回调参数(如用于 POST 上传文件或内容等)
*   CURLOPT_REFERER <== 设置引用页(char *字符串), 可以通过 curl_easy_getinfo( CURLINFO_REFERER ) 获取
*   CURLOPT_RESOLVE <== 给指定的 主机名:端口号 提供自定义的IP地址,参数为 curl_list 列表( 类似在 hosts 中增加映射,一般用于调试?)
*     curl_slist* pHost = curl_slist_append(NULL, "www.fishjam.com:443:192.168.100.88"); 
*   CURLOPT_RTSP_STREAM_URI/CURLOPT_RTSP_REQUEST <== 设置 RTSP 协议配置
* ★CURLOPT_SHARE <== 设置在各个连接之间共享的内容(CURLSH* 实例,可通过 curl_share_setopt 对其设置如 CURL_LOCK_DATA_DNS 一类的共享, 
*     表示dns信息会锁定在共享部分,各部分重用,从而加快速度) 
*   CURLOPT_TCP_KEEPALIVE <== TOOD:如果设置为1,既将TCP连接为长连接
*   CURLOPT_TCP_KEEPIDLE <== TODO: 长连接的休眠时间,如 300L 表示 5 分钟
*   CURLOPT_TCP_KEEPINTVL <== TODO: 设置心跳发送时间，使得 socket 长时间保活，小于KEEPIDLE时间(如 20L 秒)
*   CURLOPT_TIMEOUT / CURLOPT_CONNECTIONTIMEOUT <== 设置 传输数据超时 / 连接超时, 如果为0(TODO: 默认?),表示永不超时
*   CURLOPT_UPLOAD(1) <== TODO: 设置成1表示上传到指定 URL(比如 file:///home/fishjam/upload 或 ftp:// ? )
*   CURLOPT_INFILESIZE_LARGE <== TODO: 设置 upload 时的文件大小?
*  ★CURLOPT_URL <== 必须,设置请求的URL,通常是包含 schema 的全路径
*   CURLOPT_USERAGENT <== 要求传递一个以 '\0' 结尾的字符串指针,表示 User-Agent 信息
*   CURLOPT_USERNAME/CURLOPT_PASSWORD  <== 设置用户名/密码的字符串,比如用于 imap 收发邮件时
*   CURLOPT_VERBOSE <== 如果将其设置为 1L, 会显示详细的操作信息,用于调试. TODO: 具体有哪些信息?
*   CURLOPT_WRITEFUNCTION 和 CURLOPT_WRITEDATA <== 设置接收数据的回调函数(参数同 fwrite) 和 回调参数
*     (不设置回调函数时默认要求 FILE* 指针),多用于数据保存(如下载文件,获取响应等),回调函数处理正确的话,
*     需要返回 size * nmemb(表示处理的字节数). 如果不设置这个参数,会将接收到的数据输出到 stdout ?
*   CURLOPT_SSL_VERIFYPEER / CURLOPT_SSL_VERIFYHOST <== 是否验证 SSL 
************************************************************************************************************/

/************************************************************************************************************
* 其他辅助函数:
*   urlEncode: 
*     char *output = curl_easy_escape(curl, strRaw.c_str(), strRaw.size());
*     std::string strEncode(output);
*     curl_free(output);
*   urlDecode: 
*     char *output = curl_easy_unescape(curl, strEncode.c_str(), strEncode.size(), &outLength);
* 
************************************************************************************************************/

/************************************************************************************************************
* cURLpp(github.com/jpbarrette/curlpp) -- CURL 的 C++ 封装
************************************************************************************************************/

/************************************************************************************************************
* 高性能+高并发
*   方式: 长连接(复用连接,一个连接传递多次数据) + 异步非阻塞方式调用 + [TODO: 多线程]
************************************************************************************************************/

#ifndef FTL_BASE_H
#  error ftlCurl.h requires ftlbase.h to be included first
#endif

#include <curl/curl.h>
#include <stack>

namespace FTL
{

# define CURL_VERIFY(x)   \
        retCode = (x);\
        if(CURLE_OK != retCode)\
        {\
            REPORT_ERROR_INFO(FTL::CFCurlErrorInfo, retCode, x);\
        }

# define CURLM_VERIFY(x)   \
        mRetCode = (x);\
        if(CURLM_OK != mRetCode)\
        {\
            REPORT_ERROR_INFO(FTL::CFCurlMultiErrorInfo, mRetCode, x);\
        }

# define CURLSH_VERIFY(x)   \
        shCode = (x);\
        if(CURLSHE_OK != shCode)\
        {\
            REPORT_ERROR_INFO(FTL::CFCurlShErrorInfo, shCode, x);\
        }

# define CURLFORM_VERIFY(x)   \
        formCode = (x);\
        if(CURL_FORMADD_OK != formCode)\
        {\
            REPORT_ERROR_INFO(FTL::CFCurlFormErrorInfo, formCode, x);\
        }

# define CURLU_VERIFY(x) \
        cuCode = (x);\
        if(CURLUE_OK != cuCode)\
        {\
            REPORT_ERROR_INFO(FTL::CFCurlUrlErrorInfo, cuCode, x);\
        }

# define CURLU_VERIFY_EXCEPT1(x, e1) \
        cuCode = (x);\
        if(CURLUE_OK != cuCode && e1 != cuCode)\
        {\
            REPORT_ERROR_INFO(FTL::CFCurlUrlErrorInfo, cuCode, x);\
        }


    FTLEXPORT class CFCurlErrorInfo : public CFConvertInfoT<CFCurlErrorInfo, CURLcode>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFCurlErrorInfo);
    public:
        FTLINLINE explicit CFCurlErrorInfo(CURLcode code);
        // curl_easy_strerror(retCode) 可以获取到错误描述
        FTLINLINE virtual LPCTSTR ConvertInfo();
    };

    FTLEXPORT class CFCurlMultiErrorInfo : public CFConvertInfoT<CFCurlMultiErrorInfo, CURLMcode>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFCurlMultiErrorInfo);
    public:
        FTLINLINE explicit CFCurlMultiErrorInfo(CURLMcode code);
        FTLINLINE virtual LPCTSTR ConvertInfo();
    };

    FTLEXPORT class CFCurlShErrorInfo : public CFConvertInfoT<CFCurlShErrorInfo, CURLSHcode>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFCurlShErrorInfo);
    public:
        FTLINLINE explicit CFCurlShErrorInfo(CURLSHcode code);
        FTLINLINE virtual LPCTSTR ConvertInfo();
    };

    FTLEXPORT class CFCurlFormErrorInfo : public CFConvertInfoT<CFCurlFormErrorInfo, CURLFORMcode>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFCurlFormErrorInfo);
    public:
        FTLINLINE explicit CFCurlFormErrorInfo(CURLFORMcode code);
        FTLINLINE virtual LPCTSTR ConvertInfo();
    };

    FTLEXPORT class CFCurlUrlErrorInfo : public CFConvertInfoT<CFCurlUrlErrorInfo, CURLUcode>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFCurlUrlErrorInfo);
    public:
        FTLINLINE explicit CFCurlUrlErrorInfo(CURLUcode code);
        FTLINLINE virtual LPCTSTR ConvertInfo();
    };

    

    class CFCurlUtils {
    public:
        //获取 curl_version_info_data::features 字段表示的特性支持情况 
        FTLINLINE static LPCTSTR GetFeaturesInfo(FTL::CFStringFormater& formater, int curlFeatures, LPCTSTR pszDivide = TEXT("|"));

        //通过 curl_easy_getinfo 函数获取其信息, 一般可用于调试, 
        //  TODO: 其内存是否需要释放? 比如 CURLINFO_CONTENT_TYPE 获取的 char* 字符串?
        FTLINLINE static LPCTSTR GetCurlInfo(FTL::CFStringFormater& formater, CURL* pEasyCurl);

        FTLINLINE static LPCTSTR GetConnectInfo(FTL::CFStringFormater& formater, CURL* pEasyCurl);

        FTLINLINE static LPCTSTR GetCurlInfoTypeStr(curl_infotype type);
    };


    class CFCurlTaskBase;
    class CFCurlManager;

	enum CurlMethod{
		cmGet,
		cmPost,
	};

    FTLEXPORT class CFCurlTaskBase {
    private:
        // 通过 CFCurlManager 管理的变量
        friend class CFCurlManager;
        LONG  m_usingCurlIndex;
	public:
		typedef std::map<CStringA, CStringA> HeadersContainer;
    public:
        FTLINLINE CFCurlTaskBase();
        FTLINLINE ~CFCurlTaskBase();

        FTLINLINE void Reset();  //释放
        FTLINLINE void SetDebugMode(BOOL isDebug) { m_isDebug = isDebug; }
		FTLINLINE void SetUrl(const CString& strUrl);
        FTLINLINE CString GetUrl() const;
		FTLINLINE void SetMethod(CurlMethod method);
        FTLINLINE void AddHeaderW(const CStringW& strName, const CStringW& strValue);
        FTLINLINE void AddHeaderA(const CStringA& strName, const CStringA& strValue);

        FTLINLINE const std::string& GetResult() const {
            return m_strResult;
        }
        
        FTLINLINE static size_t OnWriteFunction(char *data, size_t n, size_t l, void *userp);
        FTLINLINE static size_t OnReadFunction(char *data, size_t n, size_t l, void *userp);
        FTLINLINE static size_t OnDebugFunction(CURL *pHandle, curl_infotype type, char *data, size_t size, void *userp);
        FTLINLINE static int    OnProgressFunction(void *userp, double dltotal, double dlnow, double ultotal, double ulnow);

        FTLINLINE virtual size_t _InnerWriteFunction(char *data, size_t n, size_t l);
        FTLINLINE virtual size_t _InnerReadFunction(char *data, size_t n, size_t l);
        FTLINLINE virtual size_t _InnerDebugFunction(CURL *pHandle, curl_infotype type, char *data, size_t size);
        FTLINLINE virtual int    _InnerProgressFunction(double dltotal, double dlnow, double ultotal, double ulnow);
    protected:
        FTLINLINE virtual void _InnerReset();
        FTLINLINE virtual CURLcode _OnSetupEasyHandler(CURL* pEasyHandler);
    protected:
        CAtlString  m_strUrl;
        CurlMethod	m_method;
        std::string m_strResult;
        BOOL        m_isDebug;
        curl_slist* m_pHeaderList;
        HeadersContainer	m_headers;
    };
    typedef std::shared_ptr<CFCurlTaskBase> CFCurlTaskPtr;

    FTLEXPORT class CFCurlTaskGet : public CFCurlTaskBase
    {
        //TODO:curl_easy_setopt(m_httpCurl, CURLOPT_HTTPGET, 1L);
    };

    FTLEXPORT class CFCurlTaskPost : public CFCurlTaskBase
    {
    public:
        //TODO:curl_easy_setopt(m_httpCurl, CURLOPT_POST, 1L);
        //其他:
        //  Patch: curl_easy_setopt(m_httpCurl, CURLOPT_CUSTOMREQUEST, "PATCH");
        //  DELETE: curl_easy_setopt(m_httpCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
    };

    FTLEXPORT class CFCurlTaskDownload : public CFCurlTaskBase
    {
    public:
        HANDLE m_hLocalFile;
        CString m_strLocalFile;
        FTLINLINE CFCurlTaskDownload();
		FTLINLINE void SetLocalFile(const CString& strLocalFile);
        FTLINLINE virtual size_t _InnerWriteFunction(char *data, size_t n, size_t l);
        FTLINLINE virtual void HandleResult(CURLcode retCode);
    };

	//POST enctype="multipart/form-data"
	FTLEXPORT class CFCurlTaskMultiPartPost : public CFCurlTaskBase
	{
	private:
		curl_httppost*	m_pMultiPart;
		curl_httppost*	m_pLastMultiPart;
	public:
		FTLINLINE CFCurlTaskMultiPartPost();
		FTLINLINE ~CFCurlTaskMultiPartPost();
		FTLINLINE void AddMultiPartField(const CString& strKey, const CString& strValue);
		FTLINLINE void AddMultiPartFile(const CString& strKey, const CString& strFilePath);

        FTLINLINE virtual CURLcode _OnSetupEasyHandler(CURL* pEasyHandler);
    protected:
        FTLINLINE virtual void _InnerReset();
	};

    class CFreeCurlInfo {
    public:
        LONG  curlInstanceIndex;
        CURL* pEasyCurlHandle;
        DWORD dwLastActive;
        CString strRouter;

        CFreeCurlInfo(LONG curlIndex) {
            curlInstanceIndex = curlIndex;
            pEasyCurlHandle = NULL;
            dwLastActive = 0;
        }
    };
    typedef std::shared_ptr<CFreeCurlInfo> CFreeCurlInfoPtr;


    //用于 SetCallBack 时的 dwFilter
    #define CURL_CB_FILTER_COMPLETE     0x0001      //successful, fail, timeout, etc
    //#define CURL_CB_FILTER_PROGRESS     0x0002      

    class ICurlMgrCallBack {
    public:
        //virtual void OnTaskProgress(const FTL::CFCurlTaskPtr& spTask, )
        virtual void OnTaskTimeOut(FTL::CFCurlTaskBase* pTask, CURLcode retCode, LPVOID pParam) = 0;
        virtual void OnTaskComplete(FTL::CFCurlTaskBase* pTask, CURLcode retCode, long retResponseCode, LPVOID pParam) = 0;
    };

    //线程数: 1(m_threadCurlMultiLoop) + 4(CPU个数为8), reactor 模式?
    FTLEXPORT class CFCurlManager {
    public:
        FTLINLINE CFCurlManager();
        FTLINLINE ~CFCurlManager();
        FTLINLINE VOID SetCallBack(ICurlMgrCallBack* pCallBack, LPVOID pParam, DWORD dwCallbackFilter = CURL_CB_FILTER_COMPLETE);
        FTLINLINE BOOL SetTimeout(LONG nConnectTimeout, LONG nReadDataTimeout);
        FTLINLINE BOOL Start();
        FTLINLINE BOOL AddTask(const CFCurlTaskPtr& spCurlTask);
        FTLINLINE BOOL Stop();
        FTLINLINE LONG GetRemainTasks() const {
            return m_nRemainTasks;
        }
    private:
        FTLINLINE static DWORD WINAPI _CurlMultiLoopProc(LPVOID pParam);
        FTLINLINE DWORD _InnerCurlMultiLoopProc();

        FTLINLINE CString _GetUrlRouter(const CString& strUrl);
        FTLINLINE CFreeCurlInfoPtr _GetFreeCurlHandle(CFCurlTaskPtr spTask);
        FTLINLINE CURLcode _ReturnFreeCurlHandle(CFreeCurlInfoPtr spFreeCurlInfo);

        FTLINLINE void _FreeAllCurlInfo();

    private:
        typedef std::queue<CFreeCurlInfoPtr>                    EasyCurlContainer;
        typedef std::map<CString, EasyCurlContainer>            RouterEasyCurlContainer;
        typedef std::queue<CFCurlTaskPtr>       WaitingTaskContainer;
        typedef std::map<CURL*, CFCurlTaskPtr>  CurlTaskContainers;

        CURLM*  m_pCurlMulti;
        ICurlMgrCallBack*               m_pCallback;
        LPVOID                          m_pCallbackParam;
        DWORD                           m_dwCallbackFilter;

        WaitingTaskContainer            m_waitingTasks;         //外部线程加入,内部线程处理(需要加锁)
        CurlTaskContainers              m_runningTasks;         //都在 _InnerCurlMultiLoopProc 线程函数中处理,不需要加锁

        //volatile LONG                   m_nCurRunningCount;
        LONG                            m_nMaxRunningCount;
        LONG                            m_nCurlIdGenerator;     //用于计算 curl 的 index
        volatile LONG                   m_nRemainTasks;         //waiting + running 

        DWORD                           m_dwCurlTimeout;

        FTL::CFCriticalSection  m_lockObject;
        FTL::CFThread<FTL::DefaultThreadTraits> m_threadCurlMultiLoop;
        RouterEasyCurlContainer                 m_freeEasyCurlHandles; //针对每个 router(URL) 可以重用的 easy handler
    };
}//namespace FTL

#endif //FTL_CLR_H

#ifndef USE_EXPORT
#  include "ftlCurl.hpp"
#endif