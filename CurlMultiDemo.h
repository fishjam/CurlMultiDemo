#pragma once

#include "ftlBase.h"
#include "ftlCurl.h"

class CCurlMultiDemo : 
    public FTL::IFileFindCallback,
    public FTL::ICurlMgrCallBack 
{
private:
    //IFileFindCallback
    DWORD    m_dwTotalLongth;
    virtual FTL::FileFindResultHandle OnFindFile(LPCTSTR pszFilePath, const WIN32_FIND_DATA& findData, LPVOID pParam);
    virtual FTL::FileFindResultHandle OnError(LPCTSTR pszFilePath, DWORD dwError, LPVOID pParam);

    //ICurlMgrCallBack
    virtual void OnTaskTimeOut(FTL::CFCurlTaskBase* pTask, CURLcode retCode, LPVOID pParam);
    virtual void OnTaskComplete(FTL::CFCurlTaskBase* pTask, CURLcode retCode, long retResponseCode, LPVOID pParam);

private:
    typedef std::map<CString, CString> SEARCH_FILES_MAP;   //filename => fullPath
    SEARCH_FILES_MAP  m_searchFiles;

    int _curlMgr_multiPartPost(FTL::CFCurlManager& rCurlManager);
public:
    void runDemo();
};