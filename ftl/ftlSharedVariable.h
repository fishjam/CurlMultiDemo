#ifndef FTL_SHARE_VARIABLE_H
#define FTL_SHARE_VARIABLE_H

#pragma once

#include "ftlDefine.h"

namespace FTL
{
    template<typename T>
    class CFSharedVariableT
    {
    public:
        typedef BOOL(CALLBACK* InitializeSharedVariableProc)(BOOL bFirstCreate, T& rValue);
        typedef BOOL(CALLBACK* FinalizeSharedVariableProc)(BOOL bFirstCreate, T& rValue);

        //pszShareName �����NULL�����Զ����� Exe������+PID ����������صĹ�����������ͬһ�����еĸ���ģ���ܹ��������
        FTLINLINE CFSharedVariableT(
            InitializeSharedVariableProc pInitializeProc,
            FinalizeSharedVariableProc pFinalizeProc,
            //ע��: ��Ϊ FTL �Ѿ�ʹ���� NULL �� ShareName ��ʽ�����������, ����Ҫ�����û��Լ��Ĺ������ʱ����ʹ�� NULL
            LPCTSTR pszShareName);
        FTLINLINE ~CFSharedVariableT();

        FTLINLINE T& GetShareValue();
    private:
        HANDLE		m_hMapping;
        BOOL		m_bFirstCreate;
		BOOL		m_isLocalValue;
        T*			m_pShareValue;
        FinalizeSharedVariableProc	m_pFinalizeProc;
    };
}

#endif //FTL_SHARE_VARIABLE_H

#ifndef USE_EXPORT
#  include "ftlSharedVariable.hpp"
#endif 