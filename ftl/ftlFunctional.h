///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   ftlFunctional.h
/// @brief  Fishjam Template Library Functional Header File.
/// @author fujie
/// @version 0.6 
/// @date 03/30/2008
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_FUNCTIONAL_H
#define FTL_FUNCTIONAL_H
#pragma once

#include <atlstr.h>
#include <iosfwd>
#include <list>
#include <sstream>
#include "ftlTypes.h"
#include <atlstr.h>
//#include <WinSock2.h>
namespace FTL
{
    //找在指定字符串之间的子字符串, 比如 网络URL 中的 host 部分
    //  https://www.fishjam.com/s?ie=utf-8   => www.fishjam.com 
    FTLINLINE ATL::CAtlString SubStrBetween(const ATL::CAtlString& strFull, const ATL::CAtlString& strStart, const ATL::CAtlString& strEnd)
    {
        ATL::CAtlString strSub = strFull;

        int nStartPos = strFull.Find(strStart, 0);
        int nEndFindStart = 0;
        if (nStartPos > 0) //找到开头
        {
            nEndFindStart = nStartPos + strStart.GetLength();
        }

        int nSubStrLen = strFull.GetLength() - nEndFindStart;
        int nEndPos = strFull.Find(strEnd, nEndFindStart);
        if (nEndPos > 0)
        {
            nSubStrLen = nEndPos - nEndFindStart;
        }

        strSub = strFull.Mid(nEndFindStart, nSubStrLen);

        //FTLTRACEEX(FTL::tlTrace, TEXT("full:%s => sub:%s"), strFull, strSub);
        return strSub;
    }

    FTLINLINE size_t Split(const tstring& text, 
        const tstring& delimiter,
        bool bWithDelimeter,
        std::list<tstring>& tokens)
    {
        size_t len = text.length();
        size_t start = text.find_first_not_of(delimiter); //找到第一个不是分隔符的
        size_t stop = 0;
        while ( (start >=0) && (start < len))
        {
            stop = text.find_first_of(delimiter, start); //找到这之后的第一个分隔符
            if( (stop < 0) || stop > len)
            {
                stop = len;
            }
            if (bWithDelimeter && start > 0)
            {
                tokens.push_back(text.substr(start - 1, stop - start + 1));
            }
            else
            {
                tokens.push_back(text.substr(start, stop - start));
            }
            start = text.find_first_not_of(delimiter, stop + 1);
        }
        return tokens.size();
    }

    FTLINLINE size_t Split(const ATL::CAtlString& text, 
        const ATL::CAtlString& delimiter,
        bool bWithDelimeter,
        std::list<ATL::CAtlString>& tokens)
    {
        int len = text.GetLength();
        int start = 0;
        int stop = text.Find(delimiter, start);
        while ((stop = (text.Find(delimiter, start))) == start)
        {
            //ignore init repeat delimiter
            start = stop + 1;
        }

        while (-1 != stop)
        {
            if (bWithDelimeter && start > 0)
            {
                tokens.push_back(text.Mid(start - 1, stop - start + 1));
            }
            else
            {
                tokens.push_back(text.Mid(start, stop - start));
            }
            start = stop + 1;
            while ((stop = (text.Find(delimiter, start))) == start)
            {
                //ignore repeat delimiter
                start = stop + 1;
            }

        }
        if (start >= 0 && start < len)
        {
            tokens.push_back(text.Mid(start));
        }

        return tokens.size();
    }

}//namespace FTL

#endif //FTL_FUNCTIONAL_H