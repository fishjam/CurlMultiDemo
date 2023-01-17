#ifndef FTL_FILE_HPP
#define FTL_FILE_HPP
#pragma once

#ifdef USE_EXPORT
#  include "ftlfile.h"
#endif

#include <atlpath.h>
#include <shlobj.h>

namespace FTL
{
    CFFileFinder::CFFileFinder()
    {
        m_pCallback = NULL;
        m_pParam = NULL;
        m_nOpenFolderCount = 0;
    }

    VOID CFFileFinder::SetCallback(IFileFindCallback* pCallBack, LPVOID pParam)
    {
        m_pCallback = pCallBack;
        m_pParam = pParam;
    }

    BOOL CFFileFinder::Find(LPCTSTR pszDirPath, 
        LPCTSTR pszFilter /* =_T("*.*") */, 
        BOOL bRecursive /* = TRUE */)
    {
        BOOL bRet = FALSE;
        FTLASSERT(m_pCallback);
        if (!m_pCallback)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        
        //TCHAR szFullPath[65535] = { 0 };   //MAX_PATH 太小,目前 LONGPATH 已经到了 xxx ?

        m_strDirPath = pszDirPath;
        m_strDirPath.Replace(TEXT('/'), TEXT('\\'));  //TODO: 把所有路径全部替换?

        m_strFilter = pszFilter;
		FTL::Split(m_strFilter, _T(";"), false, m_FindFilters);
        m_FindDirs.push_back(m_strDirPath);
        FileFindResultHandle resultHandler = rhContinue;

        //注意: 虽然 CPath 能比较好的处理路径问题, 但不支持 LongPath,最大 MAX_PATH

        while (!m_FindDirs.empty() && rhStop != resultHandler)
        {
            ATL::CAtlString& strFindPath = m_FindDirs.front();
            FTLASSERT(!strFindPath.IsEmpty());

            TCHAR szLastChar = strFindPath.GetAt(strFindPath.GetLength() - 1);
            if (szLastChar != TEXT('\\') && szLastChar != TEXT('/') )
            {
                strFindPath.Append(TEXT("\\"));
            }

            ATL::CAtlString strRealFindPath(strFindPath);  //增加了 扩展名和 LongPath 处理,已经不是原始的路径

            if (1 == m_FindFilters.size() && !bRecursive)
            {
                //非递归模式搜索单个扩展名的文件,才指定搜索, 否则都按 *.* 进行搜索
                strRealFindPath.Append(m_strFilter);
            }
            else
            {
                strRealFindPath.Append(TEXT("*.*"));
            }

            if (strRealFindPath.GetLength() >= MAX_PATH)
            {
                strRealFindPath = TEXT("\\\\?\\") + strRealFindPath;
            }

            WIN32_FIND_DATA findData = { 0 };

            HANDLE hFind = NULL;
            //TODO: FindFirstFileEx（path, FindExInfoStandard, &findData, FindExSearchNameMatch, NULL, 0)
            API_VERIFY_EXCEPT2(((hFind = FindFirstFile(strRealFindPath, &findData)) != INVALID_HANDLE_VALUE),
                ERROR_FILE_NOT_FOUND, ERROR_ACCESS_DENIED);
            if (bRet)
            {
                m_nOpenFolderCount++;
                //FTLTRACE(TEXT("m_nOpenFolderCount=%d"), m_nOpenFolderCount);
                do 
                {
                    ATL::CAtlString pathFullFindResult(strFindPath);
                    pathFullFindResult += findData.cFileName;
                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if ((lstrcmpi(findData.cFileName, _T(".")) != 0) 
                            && (lstrcmpi(findData.cFileName, _T("..")) != 0))
                        {
                            resultHandler = m_pCallback->OnFindFile(pathFullFindResult, findData, m_pParam);
                            //normal dir 
                            if (bRecursive && (rhContinue == resultHandler))
                            {
                                m_FindDirs.push_back(pathFullFindResult);
                            }
                            //else(rhSkipDir), just skip it
                        }
                    }
                    else
                    {
                        //如果是压缩文件，可以获取压缩文件的信息
                        //if (data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) { GetCompressedFileSize() }
                        if (_isMatchFilterFile(findData.cFileName))
                        {
                            resultHandler = m_pCallback->OnFindFile(pathFullFindResult, findData, m_pParam);
                        }
                    }

                    API_VERIFY_EXCEPT1(FindNextFile(hFind, &findData), ERROR_NO_MORE_FILES);
                } while (bRet && (rhStop != resultHandler));
                API_VERIFY(FindClose(hFind));
                m_nOpenFolderCount--;
            }
            else {
                DWORD dwLastError = GetLastError();
                m_pCallback->OnError(strFindPath, dwLastError, m_pParam);
            }
            m_FindDirs.pop_front();
        }

        FTLASSERT(0 == m_nOpenFolderCount);
        return bRet;
    }

	//TODO: 系统提供了 PathMatchSpec(支持通配符 * 和 ?, 也支持分号分开的多个,如 "*.txt;*.tmp;*.log" )
	BOOL CFFileFinder::_isMatchFilterFile(LPCTSTR pszFileName)
    {
        BOOL bMatch = FALSE;
        for (FindFiltersContainer::iterator iter = m_FindFilters.begin();
            iter != m_FindFilters.end(); 
            ++iter)
        {
            const ATL::CAtlString& strFilter = *iter;
            bMatch = PathMatchSpec(pszFileName, strFilter); // CFStringUtil::IsMatchMask(pszFileName, strFilter, FALSE);
            if (bMatch)
            {
                break;
            }
        }
        return bMatch;
    }

}

#endif //FTL_FILE_HPP