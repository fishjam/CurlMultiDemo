#ifndef FTL_CURL_HPP
#define FTL_CURL_HPP
#pragma once

#ifdef USE_EXPORT
#  include "ftlCurl.h"
#endif

namespace FTL
{
	CFCurlErrorInfo::CFCurlErrorInfo(CURLcode code)
		: CFConvertInfoT<CFCurlErrorInfo, CURLcode>(code)
	{
	}

	LPCTSTR CFCurlErrorInfo::ConvertInfo()
	{
		BOOL bRet = TRUE;
		if (NULL == m_bufInfo[0])
		{
			const char* pCurlError = curl_easy_strerror(m_Info);
			//FTL::CFConversion conv;
			//FTLTRACE(TEXT("curlError=%s"), conv.UTF8_TO_TCHAR(pCurlError));
			switch (m_Info)
			{
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OK);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_UNSUPPORTED_PROTOCOL);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FAILED_INIT);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_URL_MALFORMAT);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_NOT_BUILT_IN);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_COULDNT_RESOLVE_PROXY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_COULDNT_RESOLVE_HOST);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_COULDNT_CONNECT);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_WEIRD_SERVER_REPLY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_REMOTE_ACCESS_DENIED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_ACCEPT_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_WEIRD_PASS_REPLY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_ACCEPT_TIMEOUT);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_WEIRD_PASV_REPLY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_WEIRD_227_FORMAT);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_CANT_GET_HOST);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_HTTP2);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_COULDNT_SET_TYPE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_PARTIAL_FILE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_COULDNT_RETR_FILE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE20);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_QUOTE_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_HTTP_RETURNED_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_WRITE_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE24);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_UPLOAD_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_READ_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OUT_OF_MEMORY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OPERATION_TIMEDOUT);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE29);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_PORT_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_COULDNT_USE_REST);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE32);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_RANGE_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_HTTP_POST_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_CONNECT_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_BAD_DOWNLOAD_RESUME);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FILE_COULDNT_READ_FILE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_LDAP_CANNOT_BIND);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_LDAP_SEARCH_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE40);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FUNCTION_NOT_FOUND);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_ABORTED_BY_CALLBACK);  //在 progress 回调中返回 1 取消.如上传,下载时的取消
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_BAD_FUNCTION_ARGUMENT);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE44);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_INTERFACE_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE46);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_TOO_MANY_REDIRECTS);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_UNKNOWN_OPTION);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_TELNET_OPTION_SYNTAX);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE50);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE51);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_GOT_NOTHING);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_ENGINE_NOTFOUND);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_ENGINE_SETFAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SEND_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_RECV_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_OBSOLETE57);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_CERTPROBLEM);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_CIPHER);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_PEER_FAILED_VERIFICATION);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_BAD_CONTENT_ENCODING);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_LDAP_INVALID_URL);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FILESIZE_EXCEEDED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_USE_SSL_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SEND_FAIL_REWIND);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_ENGINE_INITFAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_LOGIN_DENIED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_TFTP_NOTFOUND);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_TFTP_PERM);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_REMOTE_DISK_FULL);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_TFTP_ILLEGAL);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_TFTP_UNKNOWNID);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_REMOTE_FILE_EXISTS);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_TFTP_NOSUCHUSER);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_CONV_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_CONV_REQD);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_CACERT_BADFILE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_REMOTE_FILE_NOT_FOUND);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSH);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_SHUTDOWN_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_AGAIN);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_CRL_BADFILE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_ISSUER_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_PRET_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_RTSP_CSEQ_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_RTSP_SESSION_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_FTP_BAD_FILE_LIST);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_CHUNK_FAILED);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_NO_CONNECTION_AVAILABLE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_PINNEDPUBKEYNOTMATCH);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_SSL_INVALIDCERTSTATUS);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_HTTP2_STREAM);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLE_RECURSIVE_API_CALL);

				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_LAST);
			default:
				FTLTRACE(TEXT("Unknown curl code %d"), m_Info);
				StringCchPrintf(m_bufInfo, _countof(m_bufInfo), TEXT("Unknown:%d"), m_Info);
				FTLASSERT(FALSE);
				break;
			}
		}
		return m_bufInfo;
	}

	CFCurlMultiErrorInfo::CFCurlMultiErrorInfo(CURLMcode code)
		: CFConvertInfoT<CFCurlMultiErrorInfo, CURLMcode>(code)
	{
	}

	LPCTSTR CFCurlMultiErrorInfo::ConvertInfo()
	{
		BOOL bRet = TRUE;
		if (NULL == m_bufInfo[0])
		{
			const char* pCurlError = curl_multi_strerror(m_Info);
			//FTL::CFConversion conv;
			//FTLTRACE(TEXT("curlError=%s"), conv.UTF8_TO_TCHAR(pCurlError));
			switch (m_Info)
			{
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_CALL_MULTI_PERFORM);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_OK);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_BAD_HANDLE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_BAD_EASY_HANDLE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_OUT_OF_MEMORY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_INTERNAL_ERROR);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_BAD_SOCKET);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_UNKNOWN_OPTION);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_ADDED_ALREADY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_RECURSIVE_API_CALL);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_WAKEUP_FAILURE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_BAD_FUNCTION_ARGUMENT);


				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLM_LAST);
			default:
				FTLTRACEEX(FTL::tlError, TEXT("Unknown curl multi code %d"), m_Info);
				StringCchPrintf(m_bufInfo, _countof(m_bufInfo), TEXT("Unknown:%d"), m_Info);
				FTLASSERT(FALSE);
				break;
			}
		}
		return m_bufInfo;
	}


	CFCurlShErrorInfo::CFCurlShErrorInfo(CURLSHcode code)
		: CFConvertInfoT<CFCurlShErrorInfo, CURLSHcode>(code)
	{
	}

	LPCTSTR CFCurlShErrorInfo::ConvertInfo()
	{
		BOOL bRet = TRUE;
		if (NULL == m_bufInfo[0])
		{
			switch (m_Info)
			{
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLSHE_OK);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLSHE_BAD_OPTION);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLSHE_IN_USE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLSHE_INVALID);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLSHE_NOMEM);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLSHE_NOT_BUILT_IN);

				//HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLSHE_LAST);
			default:
				FTLTRACE(TEXT("Unknown curlsh code %d"), m_Info);
				StringCchPrintf(m_bufInfo, _countof(m_bufInfo), TEXT("Unknown:%d"), m_Info);
				FTLASSERT(FALSE);
				break;
			}
		}
		return m_bufInfo;
	}

	CFCurlFormErrorInfo::CFCurlFormErrorInfo(CURLFORMcode code)
		: CFConvertInfoT<CFCurlFormErrorInfo, CURLFORMcode>(code)
	{
	}

	LPCTSTR CFCurlFormErrorInfo::ConvertInfo()
	{
		BOOL bRet = TRUE;
		if (NULL == m_bufInfo[0])
		{
			switch (m_Info)
			{
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_OK);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_MEMORY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_OPTION_TWICE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_NULL);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_UNKNOWN_OPTION);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_INCOMPLETE);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_ILLEGAL_ARRAY);
				HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_DISABLED);

				//HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_LAST);
			default:
				FTLTRACE(TEXT("Unknown form code code %d"), m_Info);
				StringCchPrintf(m_bufInfo, _countof(m_bufInfo), TEXT("Unknown:%d"), m_Info);
				FTLASSERT(FALSE);
				break;
			}
		}
		return m_bufInfo;
	}


    CFCurlUrlErrorInfo::CFCurlUrlErrorInfo(CURLUcode code)
        : CFConvertInfoT<CFCurlUrlErrorInfo, CURLUcode>(code)
    {
    }

    LPCTSTR CFCurlUrlErrorInfo::ConvertInfo()
    {
        BOOL bRet = TRUE;
        if (NULL == m_bufInfo[0])
        {
            switch (m_Info)
            {
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_HANDLE);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_PARTPOINTER);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_MALFORMED_INPUT);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_PORT_NUMBER);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_UNSUPPORTED_SCHEME);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_URLDECODE);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_OUT_OF_MEMORY);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_USER_NOT_ALLOWED);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_UNKNOWN_PART);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_SCHEME);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_USER);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_PASSWORD);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_OPTIONS);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_HOST);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_PORT);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_QUERY);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_FRAGMENT);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_NO_ZONEID);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_FILE_URL);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_FRAGMENT);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_HOSTNAME);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_IPV6);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_LOGIN);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_PASSWORD);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_PATH);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_QUERY);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_SCHEME);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_SLASHES);
                HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURLUE_BAD_USER);
                //HANDLE_CASE_TO_STRING(m_bufInfo, _countof(m_bufInfo), CURL_FORMADD_LAST);
            default:
                FTLTRACE(TEXT("Unknown Url api code %d"), m_Info);
                StringCchPrintf(m_bufInfo, _countof(m_bufInfo), TEXT("Unknown:%d"), m_Info);
                FTLASSERT(FALSE);
                break;
            }
        }
        return m_bufInfo;
    }

	LPCTSTR CFCurlUtils::GetFeaturesInfo(FTL::CFStringFormater& formater, int curlFeatures, LPCTSTR pszDivide)
	{
		int oldCurlFeatures = curlFeatures;

		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_IPV6, TEXT("IPV6"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_KERBEROS4, TEXT("KERBEROS4"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_SSL, TEXT("SSL"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_LIBZ, TEXT("LIBZ"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_NTLM, TEXT("NTLM"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_GSSNEGOTIATE, TEXT("GSSNEGOTIATE"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_DEBUG, TEXT("DEBUG"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_ASYNCHDNS, TEXT("ASYNCHDNS"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_SPNEGO, TEXT("SPNEGO"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_LARGEFILE, TEXT("LARGEFILE"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_IDN, TEXT("IDN"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_SSPI, TEXT("SSPI"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_CONV, TEXT("CONV"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_CURLDEBUG, TEXT("CURLDEBUG"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_TLSAUTH_SRP, TEXT("TLSAUTH_SRP"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_NTLM_WB, TEXT("NTLM_WB"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_HTTP2, TEXT("HTTP2"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_GSSAPI, TEXT("GSSAPI"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_KERBEROS5, TEXT("KERBEROS5"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_UNIX_SOCKETS, TEXT("UNIX_SOCKETS"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_PSL, TEXT("PSL"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_HTTPS_PROXY, TEXT("HTTPS_PROXY"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_MULTI_SSL, TEXT("MULTI_SSL"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_BROTLI, TEXT("BROTLI"), pszDivide);

		//Alternative Service (替代服务?)
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_ALTSVC, TEXT("ALTSVC"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_HTTP3, TEXT("HTTP3"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_ZSTD, TEXT("ZSTD"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_UNICODE, TEXT("UNICODE"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_HSTS, TEXT("HSTS"), pszDivide);
		HANDLE_COMBINATION_VALUE_TO_STRING_EX(formater, curlFeatures, CURL_VERSION_GSASL, TEXT("GSASL"), pszDivide);

		FTLASSERT(0 == curlFeatures);
		if (0 != curlFeatures)
		{
			FTLTRACEEX(FTL::tlWarn, TEXT("Get Curl Version Features Not Complete, total=0x%x, remain=0x%x"),
				curlFeatures, curlFeatures);
		}

		return formater.GetString();

	}

	LPCTSTR CFCurlUtils::GetCurlInfo(FTL::CFStringFormater& formater, CURL* pEasyCurl)
	{
		HRESULT hr = E_FAIL;
		CURLcode retCode = CURL_LAST;
		FTLASSERT(NULL != pEasyCurl);

        typedef std::tuple<CURLINFO, LPCSTR, int, LPCSTR> CURL_INFOS;
		ATL::CAtlStringA strFormaterA;
		if (pEasyCurl)
		{
			FTL::CFConversion convInfoValue;
            CURL_INFOS curlInfos[] = {

                CURL_INFOS(CURLINFO_PRIMARY_IP, ("primaryIp"), CURLINFO_STRING, ("%s")), //
                CURL_INFOS(CURLINFO_PRIMARY_PORT, ("primaryPort"), CURLINFO_LONG, ("%ld")), //

                CURL_INFOS(CURLINFO_LOCAL_IP, ("localIp"), CURLINFO_STRING, ("%s")), //
                CURL_INFOS(CURLINFO_LOCAL_PORT, ("localPort"), CURLINFO_LONG, ("%ld")), //


                CURL_INFOS(CURLINFO_CONTENT_TYPE, ("contentType"), CURLINFO_STRING, ("%s")), //比如 text/html; charset=UTF-8

                //响应码 (CURLINFO_HTTP_CODE)
                CURL_INFOS(CURLINFO_RESPONSE_CODE, ("responseCode"), CURLINFO_LONG, ("%ld")),

                //下载完成后的速度(总大小/总时间?)，单位是:字节/秒
                CURL_INFOS(CURLINFO_SPEED_DOWNLOAD_T, ("speedDownload"), CURLINFO_OFF_T, ("%" CURL_FORMAT_CURL_OFF_T)),

				//上传时的速度,如果不是上传,则值是0
                CURL_INFOS(CURLINFO_SPEED_UPLOAD_T, ("speedUpload"), CURLINFO_OFF_T, ("%" CURL_FORMAT_CURL_OFF_T)),

				//执行的总时间,单位为 微秒
                CURL_INFOS(CURLINFO_TOTAL_TIME_T, ("totalTime"), CURLINFO_OFF_T, ("%" CURL_FORMAT_CURL_OFF_T)),

                CURL_INFOS(CURLINFO_REFERER, ("referer"), CURLINFO_STRING, ("%s")),
			};

			for (int i = 0; i < _countof(curlInfos); i++)
			{
				char* pCharValue = NULL;  //char 类型 info
                long nLongValue = 0;
				curl_off_t offsetValue = 0;

				CURLINFO checkCurlInfo = std::get<0>(curlInfos[i]);
				LPCSTR  pszInfoName = std::get<1>(curlInfos[i]);
				int infoType = std::get<2>(curlInfos[i]);
				LPCSTR pszValueFormat = std::get<3>(curlInfos[i]);

				void* pValue = NULL;
				switch (infoType)
				{
				case CURLINFO_STRING: pValue = &pCharValue; break;
                case CURLINFO_LONG: pValue = &nLongValue; break;
                case CURLINFO_OFF_T: pValue = &offsetValue; break;
				default: FTLASSERT(FALSE); break;
				}

				CURL_VERIFY(curl_easy_getinfo(pEasyCurl, checkCurlInfo, pValue));
				if (pValue)
				{
					//TODO: 是否需要释放内存?
					strFormaterA.AppendFormat("%s=", pszInfoName);

					switch (infoType)
					{
					case CURLINFO_STRING:
						strFormaterA.AppendFormat(pszValueFormat, pCharValue);
						//free(pValue); 
						break;
					case CURLINFO_OFF_T:
						strFormaterA.AppendFormat(pszValueFormat, offsetValue);
						break;
                    case CURLINFO_LONG:
                        strFormaterA.AppendFormat(pszValueFormat, nLongValue);
                        break;
					default: FTLASSERT(FALSE); 
                        break;
					}
					strFormaterA.Append(",");

				}
			}
		}

		FTL::CFConversion conv;
		formater.Format(TEXT("%s"), conv.UTF8_TO_TCHAR(strFormaterA));

        return formater.GetString();
	}

    LPCTSTR CFCurlUtils::GetConnectInfo(FTL::CFStringFormater& formater, CURL* pEasyCurl) 
    {
        CURLcode retCode = CURL_LAST;
        char *pLocalIp = NULL;
        char *pRemoteIp = NULL;
        long  nLocalPort = 0, nRemotePort = 0;

        CURL_VERIFY(curl_easy_getinfo(pEasyCurl, CURLINFO_PRIMARY_IP, &pRemoteIp));
        CURL_VERIFY(curl_easy_getinfo(pEasyCurl, CURLINFO_PRIMARY_PORT, &nRemotePort));
        CURL_VERIFY(curl_easy_getinfo(pEasyCurl, CURLINFO_LOCAL_IP, &pLocalIp));
        CURL_VERIFY(curl_easy_getinfo(pEasyCurl, CURLINFO_LOCAL_PORT, &nLocalPort));
        
        CAtlStringA strFormatA;
        strFormatA.Format("%s:%d=>%s:%d", pLocalIp, nLocalPort, pRemoteIp, nRemotePort);
        
        FTL::CFConversion conv;
        formater.Format(TEXT("%s"), conv.UTF8_TO_TCHAR(strFormatA));
        return formater.GetString();
    }

    LPCTSTR CFCurlUtils::GetCurlInfoTypeStr(curl_infotype type) 
    {
        switch (type) 
        {
            //curl 的调试信息,比如 (STATE 转换,处理流程等),可以将data作为日志进行打印
            HANDLE_CASE_RETURN_STRING_EX(CURLINFO_TEXT, TEXT("Text"));  

            //以下部分可以抓包获取,一般不打印data(可能是乱码)
            HANDLE_CASE_RETURN_STRING_EX(CURLINFO_HEADER_IN, TEXT("HeaderIn"));     //接收到的数据和Header等
            HANDLE_CASE_RETURN_STRING_EX(CURLINFO_HEADER_OUT, TEXT("HeaderOut"));   //发出的请求和Header等
            HANDLE_CASE_RETURN_STRING_EX(CURLINFO_DATA_IN, TEXT("DataIn"));
            HANDLE_CASE_RETURN_STRING_EX(CURLINFO_DATA_OUT, TEXT("DataOut"));
            HANDLE_CASE_RETURN_STRING_EX(CURLINFO_SSL_DATA_IN, TEXT("SslDataIn"));
            HANDLE_CASE_RETURN_STRING_EX(CURLINFO_SSL_DATA_OUT, TEXT("SslDataOut"));
        default:
            FTLTRACEEX(FTL::tlWarn, TEXT("Unknown info type %d"), type);
            FTLASSERT(FALSE);
            return TEXT("Unknown");
        }
    }

	CFCurlTaskBase::CFCurlTaskBase() {
		m_pHeaderList = NULL;
		m_method = cmGet;
        m_isDebug = FALSE;
        m_usingCurlIndex = -1;
	}

	CFCurlTaskBase::~CFCurlTaskBase()
	{
        _InnerReset();  //调用的是虚函数, 子类需要在其析构中自行调用其 _InnerReset
	}

    void CFCurlTaskBase::Reset() 
    {
        _InnerReset();
    }

	void CFCurlTaskBase::SetUrl(const CString& strUrl)
	{
		m_strUrl = strUrl;
	}

    CString CFCurlTaskBase::GetUrl() const
    {
        return m_strUrl;
    }

	void CFCurlTaskBase::SetMethod(CurlMethod method)
	{
		m_method = method;
	}

    void CFCurlTaskBase::AddHeaderW(const CStringW& strName, const CStringW& strValue)
    {
        FTL::CFConversion convName, convValue;
        curl_slist* pHeaderList = NULL;
        CAtlStringA strNameA = convName.UTF16_TO_UTF8(strName);
        CAtlStringA strValueA = convValue.UTF16_TO_UTF8(strValue);
        AddHeaderA(strNameA, strValueA);
    }
	void CFCurlTaskBase::AddHeaderA(const CStringA& strName, const CStringA& strValue)
	{
		m_headers[strName] = strValue;
	}

    CURLcode CFCurlTaskBase::_OnSetupEasyHandler(CURL* pEasyHandler) {
        CURLcode retCode = CURL_LAST;

        if (pEasyHandler)
        {
            FTL::CFConversion conv;

            if (m_isDebug)
            {
                //TODO: 设置这个有什么用?
                CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_VERBOSE, 1L));

                CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_DEBUGFUNCTION, OnDebugFunction));
                CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_DEBUGDATA, this));
            }

            CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_NOPROGRESS, 0L));
            CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_PROGRESSFUNCTION, OnProgressFunction));
            CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_PROGRESSDATA, this));

			CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_URL, conv.TCHAR_TO_UTF8(m_strUrl)));
			CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_PRIVATE, this));

			CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_WRITEFUNCTION, OnWriteFunction));
			CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_WRITEDATA, this));

			CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_READFUNCTION, OnReadFunction));
			CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_READDATA, this));


#if 1
			//CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_TCP_KEEPALIVE, 1L));
			//CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_TCP_KEEPIDLE, 300L));
			//CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_TCP_KEEPINTVL, 200L));
#endif
			if (!m_headers.empty())
			{
				for (HeadersContainer::iterator iter = m_headers.begin();
					iter != m_headers.end(); ++iter) {
					CStringA strHeader;
					strHeader.Format("%s: %s", iter->first, iter->second);

                    m_pHeaderList = curl_slist_append(m_pHeaderList, strHeader);
				}
				CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_HTTPHEADER, m_pHeaderList));
			}
		}
        return retCode;
	}

	size_t CFCurlTaskBase::OnWriteFunction(char *data, size_t n, size_t l, void *userp)
	{
		CFCurlTaskBase* pThis = reinterpret_cast<CFCurlTaskBase*>(userp);

		return pThis->_InnerWriteFunction(data, n, l);
	}

	size_t CFCurlTaskBase::OnReadFunction(char *data, size_t n, size_t l, void *userp)
	{
		CFCurlTaskBase* pThis = reinterpret_cast<CFCurlTaskBase*>(userp);

		return pThis->_InnerReadFunction(data, n, l);
	}

    size_t CFCurlTaskBase::OnDebugFunction(CURL *pHandle, curl_infotype type, char *data, size_t size, void *userp)
    {
        CFCurlTaskBase* pThis = reinterpret_cast<CFCurlTaskBase*>(userp);
        return pThis->_InnerDebugFunction(pHandle, type, data, size);
    }

    int CFCurlTaskBase::OnProgressFunction(void *userp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
        CFCurlTaskBase* pThis = reinterpret_cast<CFCurlTaskBase*>(userp);
        return pThis->_InnerProgressFunction(dltotal, dlnow, ultotal, ulnow);
    }

	size_t CFCurlTaskBase::_InnerWriteFunction(char *data, size_t n, size_t l)
	{
		//默认将服务器返回的结果写到成员变量 m_strResult (比如 json ?),如果是下载的话,可以重载成写文件?
		//  TODO: 需要设置最大能接收的长度
		size_t writeCount = n * l;
		m_strResult.append(data, writeCount);

        FTLTRACE(TEXT("write %d char to result, result size=%d"), writeCount, m_strResult.size());
		return writeCount;
	}

	size_t CFCurlTaskBase::_InnerReadFunction(char *data, size_t n, size_t l)
	{
		FTLTRACEEX(FTL::tlError, TEXT("!!!want read %d x %d from local"), n, l);
		size_t readCount = -1;

		return readCount;
	}

    size_t CFCurlTaskBase::_InnerDebugFunction(CURL *pHandle, curl_infotype type, char *data, size_t size)
    {
        FTL::CFConversion conv;
        FTLTRACEA("type:curl handler:0x%x, %d(%s), size=%d, data=%s",
            pHandle, type, conv.TCHAR_TO_UTF8(CFCurlUtils::GetCurlInfoTypeStr(type)), size, 
            CURLINFO_TEXT == type ? data : "<not_print>");
        return size;
    }

    int CFCurlTaskBase::_InnerProgressFunction(double dltotal, double dlnow, double ultotal, double ulnow)
    {
        FTLTRACEEX(FTL::tlDetail, TEXT("dl: %g/%g , ul:%g/%g"), dlnow, dltotal, ulnow, ultotal);
        return 0;
    }

    void CFCurlTaskBase::_InnerReset()
    {
        if (m_pHeaderList)
        {
            curl_slist_free_all(m_pHeaderList);
            m_pHeaderList = NULL;
        }
        m_usingCurlIndex = -1;
    }

	CFCurlTaskDownload::CFCurlTaskDownload()
	{
		m_hLocalFile = INVALID_HANDLE_VALUE;
	}

	void CFCurlTaskDownload::SetLocalFile(const CString& strLocalFile)
	{
		m_strLocalFile = strLocalFile;
	}

    size_t CFCurlTaskDownload::_InnerWriteFunction(char *data, size_t n, size_t l) {
        BOOL bRet = FALSE;
        DWORD dwWritten = -1;
        if (INVALID_HANDLE_VALUE == m_hLocalFile)
        {
          API_VERIFY(INVALID_HANDLE_VALUE != (m_hLocalFile = 
              ::CreateFile(m_strLocalFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)));
        }
        if (INVALID_HANDLE_VALUE != m_hLocalFile)
        {
            DWORD writeCount = (DWORD)(n * l);
            API_VERIFY(::WriteFile(m_hLocalFile, data, writeCount, &dwWritten, NULL));
        }
        return dwWritten;
    }

    void CFCurlTaskDownload::HandleResult(CURLcode retCode)
    {
        BOOL bRet = FALSE;
        SAFE_CLOSE_HANDLE(m_hLocalFile, INVALID_HANDLE_VALUE);
        FTLTRACE(TEXT("download file %s, result=%d(%s)"), 
            m_strLocalFile, retCode, FTL::CFCurlErrorInfo(retCode).GetConvertedInfo());
    }

	CFCurlTaskMultiPartPost::CFCurlTaskMultiPartPost()
	{
		m_pMultiPart = NULL;
		m_pLastMultiPart = NULL;
	}
	CFCurlTaskMultiPartPost::~CFCurlTaskMultiPartPost()
	{
        _InnerReset();
	}

    void CFCurlTaskMultiPartPost::_InnerReset()
    {
        if (NULL != m_pMultiPart)
        {
            curl_formfree(m_pMultiPart);
            m_pMultiPart = NULL;
            m_pLastMultiPart = NULL;
        }
    }

	void CFCurlTaskMultiPartPost::AddMultiPartField(const CString& strKey, const CString& strValue)
	{
        CURLFORMcode formCode = CURL_FORMADD_LAST;
        FTL::CFConversion convKey, convValue;
        CURLFORM_VERIFY(curl_formadd(&m_pMultiPart,
            &m_pLastMultiPart,
            CURLFORM_COPYNAME, convKey.TCHAR_TO_UTF8(strKey),
            CURLFORM_COPYCONTENTS, convValue.TCHAR_TO_UTF8(strValue),
            CURLFORM_END));
	}

	void CFCurlTaskMultiPartPost::AddMultiPartFile(const CString& strKey, const CString& strFilePath)
	{
        CURLFORMcode formCode = CURL_FORMADD_LAST;
		FTL::CFConversion convKey, convFilePath;
        
        CURLFORM_VERIFY(curl_formadd(&m_pMultiPart, &m_pLastMultiPart,
			CURLFORM_COPYNAME, convKey.TCHAR_TO_UTF8(strKey),
			CURLFORM_FILE, convFilePath.TCHAR_TO_MBCS(strFilePath),
			CURLFORM_END));
	}

    CURLcode CFCurlTaskMultiPartPost::_OnSetupEasyHandler(CURL* pEasyHandler)
    {
        CURLcode retCode = CURL_LAST;
        CURL_VERIFY(__super::_OnSetupEasyHandler(pEasyHandler));

        FTLASSERT(m_pMultiPart); //通过 AddMultiPartXxx 会生成这个.
        if (m_pMultiPart)
        {
            CURL_VERIFY(curl_easy_setopt(pEasyHandler, CURLOPT_HTTPPOST, m_pMultiPart));
        }
        return retCode;
    }

    CFCurlManager::CFCurlManager()
    {
        m_pCurlMulti = NULL;
        m_pCallback = NULL;
        m_pCallbackParam = NULL;
        m_dwCallbackFilter = 0;
        m_nMaxRunningCount = 5;
        m_dwCurlTimeout = 60 * 1000; //TODO: 这个时间似乎无效, curl 内部自己在管理连接是否断开, 通过什么值?
        m_nCurlIdGenerator = 0;
        m_nRemainTasks = 0;
    }

    CFCurlManager::~CFCurlManager() {
        FTLASSERT(NULL == m_pCurlMulti);
    }

    VOID CFCurlManager::SetCallBack(ICurlMgrCallBack* pCallBack, LPVOID pParam, DWORD dwCallbackFilter /* = CURL_CB_FILTER_COMPLETE */)
    {
        m_pCallback = pCallBack;
        m_pCallbackParam = pParam;
        m_dwCallbackFilter = dwCallbackFilter;
    }

    BOOL CFCurlManager::Start()
    {
        BOOL bRet = FALSE;
        FTLASSERT(NULL == m_pCurlMulti);

        m_pCurlMulti = curl_multi_init();
        if (NULL == m_pCurlMulti)
        {
            FTLTRACE(TEXT("curl_multi_init failed"));
            return CURLE_FAILED_INIT;
        }

        /* Limit the amount of simultaneous connections curl should allow: */
        curl_multi_setopt(m_pCurlMulti, CURLMOPT_MAXCONNECTS, (long)10);

        API_VERIFY(m_threadCurlMultiLoop.Start(_CurlMultiLoopProc, this));
        return bRet;
    }
    BOOL CFCurlManager::Stop() {
        FTLTRACE(TEXT("befor stop"));
        CURLMcode mRetCode = CURLM_LAST;
        BOOL bRet = TRUE;

        if (m_pCurlMulti)
        {
            API_VERIFY(m_threadCurlMultiLoop.Stop());
            CURLM_VERIFY(curl_multi_wakeup(m_pCurlMulti));

            API_VERIFY(m_threadCurlMultiLoop.Wait());
            CURLM_VERIFY(curl_multi_cleanup(m_pCurlMulti));
            m_pCurlMulti = NULL;
        }

        _FreeAllCurlInfo();

        return bRet;
    }

    BOOL CFCurlManager::AddTask(const CFCurlTaskPtr& spCurlTask) {
        CURLMcode mRetCode = CURLM_LAST;

        FTLTRACE(TEXT("add curl task %s"), spCurlTask->GetUrl());
        FTL::CFAutoLock<FTL::CFLockObject> locker(&m_lockObject);
        m_waitingTasks.push(spCurlTask);
        CURLM_VERIFY(curl_multi_wakeup(m_pCurlMulti));
        InterlockedIncrement(&m_nRemainTasks);

        return (CURLM_OK == mRetCode);
    }

    DWORD WINAPI CFCurlManager::_CurlMultiLoopProc(LPVOID pParam)
    {
        CFCurlManager* pThis = static_cast<CFCurlManager*>(pParam);
        DWORD dwResult = pThis->_InnerCurlMultiLoopProc();
        return dwResult;
    }

    DWORD CFCurlManager::_InnerCurlMultiLoopProc()
    {
        DWORD dwResult = 0;
        CURLcode retCode = CURL_LAST;
        CURLMcode mRetCode = CURLM_LAST;
        CURLMsg* pMsg = NULL;

        int taskCount = 0;// (int)m_allTasks.size();
        int running_handles = 0;
        int msg_in_queues = 0;

        //程序没有退出,就一直循环.
        FTL::FTLThreadWaitType waitType = FTL::ftwtContinue;
        do
        {
            CURLM_VERIFY(curl_multi_perform(m_pCurlMulti, &running_handles));
            //FTLTRACEEX(FTL::tlTrace, TEXT("after curl_multi_perform, mRetCode=%d, taskCount=%d, still_alive=%d"),
            //   mRetCode, taskCount, running_handles);

            if (CURLM_OK == mRetCode)
            {
                while (NULL != (pMsg = curl_multi_info_read(m_pCurlMulti, &msg_in_queues))) {
                    //有了 成功/失败 等需要进行处理的情况,才会进到这里面来, 可能同时有多个
                    FTLTRACE(TEXT("after info read, msg=%d, msg_in_queues=%d"), pMsg->msg, msg_in_queues);

                    if (pMsg->msg == CURLMSG_DONE) {
                        CFCurlTaskBase* pCurlTask = NULL;
                        CURL_VERIFY(curl_easy_getinfo(pMsg->easy_handle, CURLINFO_PRIVATE, &pCurlTask));
                        if (pCurlTask)
                        {
                            CFreeCurlInfoPtr spFreeCurlInfo( new CFreeCurlInfo(pCurlTask->m_usingCurlIndex));
                            spFreeCurlInfo->pEasyCurlHandle = pMsg->easy_handle;
                            spFreeCurlInfo->dwLastActive = GetTickCount();
                            spFreeCurlInfo->strRouter = _GetUrlRouter(pCurlTask->GetUrl());
                            
                            long retResponseCode = 0;
                            CURL_VERIFY(curl_easy_getinfo(pMsg->easy_handle, CURLINFO_RESPONSE_CODE, &retResponseCode));

                            CURLcode retCode = pMsg->data.result;

                            //CurlTaskContainers::iterator iterRunningTask = m_runningTasks.find(spFreeCurlInfo->pEasyCurlHandle);
                            //FTLASSERT(iterRunningTask != m_runningTasks.end());
                            //CFCurlTaskPtr spCurTask = iterRunningTask->second;
                            //FTLASSERT(spCurTask.get() == pCurlTask);

                            if (m_pCallback
                                && (m_dwCallbackFilter & CURL_CB_FILTER_COMPLETE))
                            {
                                m_pCallback->OnTaskComplete(pCurlTask, retCode, retResponseCode, m_pCallback);
                            }
                            //pCurlTask->_OnHandleResult(retCode, retResponseCode);

                            //处理完毕以后再放回去
                            m_runningTasks.erase(spFreeCurlInfo->pEasyCurlHandle);
                            _ReturnFreeCurlHandle(spFreeCurlInfo);
                        }
                        CURLM_VERIFY(curl_multi_remove_handle(m_pCurlMulti, pMsg->easy_handle));
                        InterlockedDecrement(&m_nRemainTasks);
                    }
                    else {
                        FTLTRACEEX(FTL::tlError, TEXT("!!! curl_multi_info_read not Done, result=%d"), pMsg->msg);
                        FTLASSERT(FALSE); //什么情况会到这里?
                    }
                }
            }

            if (running_handles < m_nMaxRunningCount)
            {
                FTL::CFAutoLock<FTL::CFLockObject> locker(&m_lockObject);
                if(!m_waitingTasks.empty())
                {
                    CFCurlTaskPtr spTask = m_waitingTasks.front();
                    FTLTRACE(TEXT("running = %d, will start new task, url=%s"),
                        running_handles, spTask->GetUrl());

                    CFreeCurlInfoPtr spFreeCurlInfo = _GetFreeCurlHandle(spTask);
                    if (spFreeCurlInfo->pEasyCurlHandle)
                    {
                        CURL_VERIFY(spTask->_OnSetupEasyHandler(spFreeCurlInfo->pEasyCurlHandle));
                        if (CURLM_OK == retCode)
                        {
                            spTask->m_usingCurlIndex = spFreeCurlInfo->curlInstanceIndex;
                            m_runningTasks[spFreeCurlInfo->pEasyCurlHandle] = spTask;
                            CURLM_VERIFY(curl_multi_add_handle(m_pCurlMulti, spFreeCurlInfo->pEasyCurlHandle));
                        }
                        else {
                            //TODO: 如果失败的话, 是否应该放入失败队列? 但是, Setup 时会失败吗?
                            FTLTRACEEX(FTL::tlError, TEXT("setup easy handler error=%d, url=%s"), retCode, spTask->GetUrl());
                            _ReturnFreeCurlHandle(spFreeCurlInfo);
                        }
                    }
                    m_waitingTasks.pop();
                }
            }

            //阻塞等待,直到 有数据, 超时,出错或被 curl_multi_wakeup 唤醒
            //FTLTRACE(TEXT("before curl_multi poll/wait"));
#if 1       //如果没有数据时会阻塞
            CURLM_VERIFY(curl_multi_poll(m_pCurlMulti, NULL, 0, 5 * 1000, NULL));
#else       //TODO: 似乎不会阻塞?
            CURLM_VERIFY(curl_multi_wait(m_pCurlMulti, NULL, 0, 30 * 1000, NULL));
#endif 
            FTLTRACEEX(FTL::tlDetail, TEXT("after curl_multi poll/wait ,mRet=%d"), mRetCode);

            waitType = m_threadCurlMultiLoop.GetThreadWaitType(INFINITE);
        } while (FTL::ftwtContinue == waitType);
        return dwResult;
    }

    FTLINLINE CString CFCurlManager::_GetUrlRouter(const CString& strUrl) 
    {
        CString strRouter = FTL::SubStrBetween(strUrl, TEXT("://"), TEXT("/"));
        return strRouter;
    }

    FTLINLINE CFreeCurlInfoPtr CFCurlManager::_GetFreeCurlHandle(CFCurlTaskPtr spTask)
    {
        CFreeCurlInfoPtr spFreeCurlInfo;

        CString strUrl = spTask->GetUrl();
        CAtlString strRouter = _GetUrlRouter(spTask->GetUrl());

        //1.按 router 查找
        RouterEasyCurlContainer::iterator iterRouter = m_freeEasyCurlHandles.find(strRouter);
        if (iterRouter != m_freeEasyCurlHandles.end())
        {
            EasyCurlContainer& routerEasyCurls = iterRouter->second;
            while (!routerEasyCurls.empty())
            {
                spFreeCurlInfo = routerEasyCurls.front();
                routerEasyCurls.pop();

                //检查放入时间, 如果太长的话, 连接已经断开, 则不能重用, 直接释放
                DWORD dwElapse = GetTickCount() - spFreeCurlInfo->dwLastActive;
                if (dwElapse >= m_dwCurlTimeout)
                {
                    FTL::CFStringFormater formater;
                    FTL::CFCurlUtils::GetConnectInfo(formater, spFreeCurlInfo->pEasyCurlHandle);
                    FTLTRACEEX(FTL::tlWarn, TEXT("free timeout for curl %d(%p), elapse=%d, %s"),
                        spFreeCurlInfo->curlInstanceIndex, spFreeCurlInfo->pEasyCurlHandle, dwElapse, formater.GetString());

                    curl_easy_cleanup(spFreeCurlInfo->pEasyCurlHandle);
                    spFreeCurlInfo->pEasyCurlHandle = NULL;
                }
                else {
                    //尝试重用
                    FTLTRACE(TEXT("will reuse curl %p for url %s"), spFreeCurlInfo->pEasyCurlHandle, spTask->GetUrl());
                    break;
                }
            }
        }

        if (!spFreeCurlInfo)
        {
            spFreeCurlInfo = CFreeCurlInfoPtr(new CFreeCurlInfo(-1));
        }

        if (NULL == spFreeCurlInfo->pEasyCurlHandle)
        {
            //如果没有找到可重用的, 则创建一个新的并返回
            spFreeCurlInfo->curlInstanceIndex = InterlockedIncrement(&m_nCurlIdGenerator);
            spFreeCurlInfo->pEasyCurlHandle = curl_easy_init();

            FTLTRACE(TEXT("can not find free curl for %s, new index=%d(%p)"), 
                spTask->GetUrl(), spFreeCurlInfo->curlInstanceIndex, spFreeCurlInfo->pEasyCurlHandle);
        }

        return spFreeCurlInfo;
    }

    FTLINLINE CURLcode CFCurlManager::_ReturnFreeCurlHandle(CFreeCurlInfoPtr spFreeCurlInfo)
    {
        CURLcode retCode = CURL_LAST;

        FTLTRACE(TEXT("return free curl index=%d(%p) for router=%s"), 
            spFreeCurlInfo->curlInstanceIndex, spFreeCurlInfo->pEasyCurlHandle, spFreeCurlInfo->strRouter);
        m_freeEasyCurlHandles[spFreeCurlInfo->strRouter].push(spFreeCurlInfo);
        return retCode;
    }

    void CFCurlManager::_FreeAllCurlInfo()
    {
        for (RouterEasyCurlContainer::iterator iterRouter = m_freeEasyCurlHandles.begin();
            iterRouter != m_freeEasyCurlHandles.end();
            iterRouter ++) 
        {
            EasyCurlContainer& rEasyCurls = iterRouter->second;
            while (!rEasyCurls.empty())
            {
                CFreeCurlInfoPtr spFreeCurlInfo = rEasyCurls.front();
                FTLTRACE(TEXT("free curl %d(%p) for router=%s"),
                    spFreeCurlInfo->curlInstanceIndex, spFreeCurlInfo->pEasyCurlHandle, spFreeCurlInfo->strRouter);
                curl_easy_cleanup(spFreeCurlInfo->pEasyCurlHandle);
                rEasyCurls.pop();
            }
        }
        m_freeEasyCurlHandles.clear();
    }

} //namespace FTL

#endif //FTL_CURL_HPP