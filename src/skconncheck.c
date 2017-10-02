#include "skconncheck.h"
#include <stdlib.h>
#include <curl/curl.h>

#if !defined(DEBUG_BUILD)
static FILE* gpfOutputFile = NULL;
#endif

SK_Error SKConnCheck_Init()
{
    SK_DEBUG_INFO("Using libCurl version %s\n", curl_version());

    CURLcode eError = curl_global_init(CURL_GLOBAL_ALL);
    
    if (CURLE_OK != eError)
    {
        SK_DEBUG_ERROR("Libarary initialisation failed. Error Code %d (%s)\n",
            eError, curl_easy_strerror(eError));

        return ERR_FAIL;
    }
    else
    {
        return ERR_OK;
    }
}

SK_Error SKConnCheck_SessionInit(SKConnCheckSession** ppSession)
{
    SKConnCheckSession* pSession = (SKConnCheckSession*)
                                    malloc(sizeof(SKConnCheckSession));
    
    if (NULL == pSession)
    {
        SK_DEBUG_ERROR("Failed to allocate memory for pSession\n");
        return ERR_FAIL;
    }
    
    CURL* hCurl = curl_easy_init();
    
    if (NULL == hCurl)
    {
        SK_DEBUG_ERROR("Failed to initialise session.\n");
                
        return ERR_FAIL;
    }

/* Redirect the recieved data to /dev/null if it's not a DEBUG build */
#if !defined(DEBUG_BUILD)
    gpfOutputFile = fopen("/dev/null","w");
    curl_easy_setopt(hCurl, CURLOPT_WRITEDATA, gpfOutputFile);
#endif
        
    pSession->hPrivate = (void*)hCurl;
    *ppSession = pSession;
    
    return ERR_OK;
}


SK_Error SKConnCheck_SessionPrepareRequest(SKConnCheckSession* pSession,
            int iRequestType, char** plHeaders)
{
    if (NULL == pSession)
    {
        SK_DEBUG_ERROR("pSession is NULL\n");
        
        return ERR_FAIL;
    }
    
    if (SK_REQ_TYPE_GET != iRequestType)
    {
        SK_DEBUG_ERROR("Invalid Request Type. Request type %d not supported\n",
                        iRequestType);
        
        return ERR_FAIL;
    }

    CURL* hCurl = (CURL*) pSession->hPrivate;
    CURLcode eError = CURLE_OK;

    if (CURLE_OK != (eError = curl_easy_setopt(hCurl, CURLOPT_URL, "http://www.google.com/")))
    {
        SK_DEBUG_ERROR("Failed to set URL. "\
                        "Error Code %d (%s)\n",
                        eError, curl_easy_strerror(eError));

        return ERR_FAIL;
    }
    
    if (NULL != plHeaders)
    {
        if (CURLE_OK != (eError = curl_easy_setopt(hCurl, CURLOPT_HTTPHEADER, plHeaders)))
        {
            SK_DEBUG_ERROR("Failed to set HTTP Headers. "\
                            "Error Code %d (%s)\n",
                            eError, curl_easy_strerror(eError));
        }
    }
    else
    {
        SK_DEBUG_INFO("No additional HTTP headers specified. plheaders is NULL\n");
    }

#if defined(DEBUG_BUILD)
    if (CURLE_OK != (eError = curl_easy_setopt(hCurl, CURLOPT_VERBOSE, 1)))
    {
        SK_DEBUG_ERROR("setopt failed."\
                        "Error Code %d (%s)\n",
                        eError, curl_easy_strerror(eError));
    }
                        
    if (CURLE_OK != (eError = curl_easy_setopt(hCurl, CURLOPT_HEADER, 1)))
    {
        SK_DEBUG_ERROR("setopt failed."\
                        "Error Code %d (%s)\n",
                        eError, curl_easy_strerror(eError));
    }
#endif
    
/*
TODO: Useful options to handle malicious servers 
CURLOPT_TIMEOUT
CURLOPT_LOW_SPEED_LIMIT
CURLOPT_MAXREDIRS
*/

    return ERR_OK;
}

SK_Error SKConnCheck_SessionSendRequest(SKConnCheckSession* pSession)
{
    if (NULL == pSession)
    {
        SK_DEBUG_ERROR("pSession is NULL\n");
        
        return ERR_FAIL;
    }
    
    CURLcode eError = CURLE_OK;
    
    if (CURLE_OK != (eError = curl_easy_perform((CURL*)pSession->hPrivate)))
    {
        SK_DEBUG_ERROR("Failed to send request. "\
                        "Error Code %d (%s)\n",
                        eError, curl_easy_strerror(eError));

        return ERR_FAIL;
    }

    return ERR_OK;
}

SK_Error SKConnCheck_SessionGetStats(SKConnCheckSession* pSession)
{
    CURL* hCurl = (CURL*) pSession->hPrivate;
    
    if (CURLE_OK != curl_easy_getinfo(hCurl, CURLINFO_PRIMARY_IP, &pSession->pcIPAddr))
    {
        return ERR_FAIL;
    }
    
    if (CURLE_OK != curl_easy_getinfo(hCurl, CURLINFO_RESPONSE_CODE, &pSession->lHttpResponseCode))
    {
        return ERR_FAIL;
    }
    
    if (CURLE_OK != curl_easy_getinfo(hCurl, CURLINFO_NAMELOOKUP_TIME, &pSession->dNameLookupTime))
    {
        return ERR_FAIL;
    }
    
    if (CURLE_OK != curl_easy_getinfo(hCurl, CURLINFO_CONNECT_TIME, &pSession->dConnectTime))
    {
        return ERR_FAIL;
    }
    
    if (CURLE_OK != curl_easy_getinfo(hCurl, CURLINFO_STARTTRANSFER_TIME, &pSession->dStartTransferTime))
    {
        return ERR_FAIL;
    }
    
    if (CURLE_OK != curl_easy_getinfo(hCurl, CURLINFO_TOTAL_TIME, &pSession->dTotalTime))
    {
        return ERR_FAIL;
    }

    return ERR_OK;
}

SK_Error SKConnCheck_SessionDeinit(SKConnCheckSession* pSession)
{
    if (NULL == pSession)
    {
        SK_DEBUG_ERROR("pSession is NULL\n");
        
        return ERR_FAIL;
    }

#if !defined(DEBUG_BUILD)
    if (NULL != gpfOutputFile)
    {
        fclose(gpfOutputFile);
    }
#endif
    
    curl_easy_cleanup((CURL*) pSession->hPrivate);

    free(pSession);
    return ERR_OK;
}

SK_Error SKConnCheck_Deinit()
{
    curl_global_cleanup();
    
    return ERR_OK;
}

void* SKConnCheck_LinkListAppendString(void* pLinkListHead, char * pString)
{
    return (void*)curl_slist_append((struct curl_slist*)pLinkListHead, pString);
}

void SKConnCheck_LinkListFree(void* pLinkListHead)
{
    curl_slist_free_all((struct curl_slist*)pLinkListHead);
}

