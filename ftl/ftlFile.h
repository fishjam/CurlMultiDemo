#ifndef FTL_FILE_H
#define FTL_FILE_H
#pragma once

#ifndef FTL_BASE_H
#  error ftlfile.h requires ftlbase.h to be included first
#endif
#include <atlstr.h>

namespace FTL
{
//Class
    class CFStructuredStorageFile;
//Function

}//namespace FTL

#include <WinIoctl.h>

namespace FTL
{
    enum FileFindResultHandle
    {
        rhContinue,
        rhSkipDir,  //just use for skip special dir
        rhStop,
    };

    class IFileFindCallback
    {
    public:
        virtual FileFindResultHandle OnFindFile(LPCTSTR pszFilePath, const WIN32_FIND_DATA& findData, LPVOID pParam) = 0;
        virtual FileFindResultHandle OnError(LPCTSTR pszFilePath, DWORD dwError, LPVOID pParam) = 0;
    };

    //同步查找指定目录 -- 采用了递推方式来避免递归方式的问题
    class CFFileFinder
    {
    public:
        FTLINLINE CFFileFinder();
        FTLINLINE VOID SetCallback(IFileFindCallback* pCallBack, LPVOID pParam);
        FTLINLINE BOOL Find(LPCTSTR pszDirPath, LPCTSTR pszFilter = _T("*.*"), BOOL bRecursive = TRUE);
    protected:
        IFileFindCallback*  m_pCallback;
        LPVOID              m_pParam;
        //调试代码, 检测当前打开的目录个数(即 FindFirstFile 后尚未 FindClose 的个数)
        LONG                m_nOpenFolderCount;
        ATL::CAtlString          m_strDirPath;
        ATL::CAtlString          m_strFilter;
        typedef std::deque<ATL::CAtlString>  FindDirsContainer;      //递归方式时保存需要查找的子目录
        FindDirsContainer  m_FindDirs;
        typedef std::list<ATL::CAtlString>   FindFiltersContainer;   //保存通过分号区分开的多种扩展名
        FindFiltersContainer    m_FindFilters;
        FTLINLINE   BOOL _isMatchFilterFile(LPCTSTR pszFileName);
    };
}//namespace FTL

#endif //FTL_FILE_H

#ifndef USE_EXPORT
#  include "ftlFile.hpp"
#endif