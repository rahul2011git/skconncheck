#include <stdio.h>
#include <curl/curl.h>

#define CHECK_eError \
                if (eError != CURLE_OK) \
                {\
                    printf("%s(%d)ERROR: Error Code %d (%s)\n",\
                        __FUNCTION__, __LINE__, eError, curl_easy_strerror(eError));\
                }

int main (void)
{
    CURLcode eError = CURLE_OK;
    CURL *hCurl = NULL;
    
    printf("libcURL version: %s\n", curl_version());
    
    eError = curl_global_init(CURL_GLOBAL_ALL);
    
    CHECK_eError;
    
    if ((hCurl = curl_easy_init()) == NULL)
    {
        printf("hCurl is NULL");
    }
        
    
    eError = curl_easy_setopt(hCurl, CURLOPT_URL, "http://www.google.com/");
    CHECK_eError;

    struct curl_slist *pHeaderList = NULL;
    if ((pHeaderList = curl_slist_append(pHeaderList, "Content-Type: text/xml")) == NULL)
    {
        printf("hCurl is NULL");
    }

    eError = curl_easy_setopt(hCurl, CURLOPT_HTTPHEADER, pHeaderList);
    CHECK_eError;
    
    eError = curl_easy_perform(hCurl);
    CHECK_eError;
    
    char *pcIPAddr;
    long lHttpResponseCode;
    double dNameLookupTime;
    double dConnectTime;
    double dStartTransferTime;
    double dTotalTime;
    
    eError = curl_easy_getinfo(hCurl, CURLINFO_PRIMARY_IP, &pcIPAddr);
    eError = 1;
    CHECK_eError;
    eError = curl_easy_getinfo(hCurl, CURLINFO_RESPONSE_CODE, &lHttpResponseCode);
    CHECK_eError;
    eError = curl_easy_getinfo(hCurl, CURLINFO_NAMELOOKUP_TIME, &dNameLookupTime);
    CHECK_eError;
    eError = curl_easy_getinfo(hCurl, CURLINFO_CONNECT_TIME, &dConnectTime); 
    CHECK_eError;
    eError = curl_easy_getinfo(hCurl, CURLINFO_STARTTRANSFER_TIME, &dStartTransferTime);
    CHECK_eError;
    eError = curl_easy_getinfo(hCurl, CURLINFO_TOTAL_TIME, &dTotalTime); 
    CHECK_eError;

    printf("\
    CURLINFO_PRIMARY_IP = %s, CURLINFO_RESPONSE_CODE = %ld, CURLINFO_NAMELOOKUP_TIME = %f, \
    CURLINFO_CONNECT_TIME = %f , CURLINFO_STARTTRANSFER_TIME = %f, CURLINFO_TOTAL_TIME = %f\n",       
    pcIPAddr,lHttpResponseCode,dNameLookupTime,
    dConnectTime, dStartTransferTime, dTotalTime);
    
    curl_easy_cleanup(hCurl);
    
    return eError;
}