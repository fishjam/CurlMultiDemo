// CurlMultiDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CurlMultiDemo.h"

FTL::FileFindResultHandle CCurlMultiDemo::OnFindFile(
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

FTL::FileFindResultHandle CCurlMultiDemo::OnError(LPCTSTR pszFilePath, DWORD dwError, LPVOID pParam)
{
    FTL::CFAPIErrorInfo errInfo(dwError);
    FTLTRACEEX(FTL::tlError, TEXT("error %d(%s) for %s"), dwError, errInfo.GetConvertedInfo(), pszFilePath);
    return FTL::rhContinue;
}

void CCurlMultiDemo::OnTaskTimeOut(FTL::CFCurlTaskBase* pTask, CURLcode retCode, LPVOID pParam)
{

}

void CCurlMultiDemo::OnTaskComplete(FTL::CFCurlTaskBase* pTask, CURLcode retCode, long retResponseCode, LPVOID pParam)
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

int CCurlMultiDemo::_curlMgr_multiPartPost(FTL::CFCurlManager& rCurlManager) 
{
    FTL::CFFileFinder fileFinder;
    fileFinder.SetCallback(this, LPVOID(1000));

    fileFinder.Find(TEXT("D:\\vcpkg\\installed\\"),
        TEXT("*.txt;*.md;*.h;"));
    FTLTRACE(TEXT("will upload %d files"), m_searchFiles.size());

    CString strUrl;
    int BATCH_SIZE = 1;

#if SUPPORT_MULTI_FILES
    BATCH_SIZE = 50;
#endif

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

            //支持 MultiPart 上传文件的服务器(最好支持单次传递上传多个文件),可以测试 batch 上传
            //示例:https://github.com/vert-x3/vertx-examples/blob/4.x/web-examples/src/main/java/io/vertx/example/web/upload/Server.java
            strUrl = TEXT("http://127.0.0.1:8080/form");
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

    return (int)m_searchFiles.size();
}

void CCurlMultiDemo::runDemo() 
{
    BOOL bRet = FALSE;
    FTL::CFCurlManager curlManager;
    curlManager.SetCallBack(this, this);
    curlManager.Start();

    FTL::CFElapseCounter elapseCounter;

    int uploadCount = _curlMgr_multiPartPost(curlManager);
    //_curlMgr_error_case(curlManager);

    //Sleep(60 * 1000);
    //等待所有任务都结束
    while (curlManager.GetRemainTasks() > 0)
    {
        Sleep(1000);
    }
    curlManager.Stop();


    FTL::FormatMessageBox(NULL, TEXT("elapse time"), MB_OK, 
        TEXT("upload %d files, elapse time: %d ms"), uploadCount,
        elapseCounter.GetElapseTime() / NANOSECOND_PER_MILLISECOND);
}


int main()
{
    CCurlMultiDemo demo;
    demo.runDemo();
    return 0;
}

