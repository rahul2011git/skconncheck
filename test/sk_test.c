#include <stdio.h>
#include <stdlib.h>
#include "skconncheck.h"
#include <signal.h>
#include <unistd.h>

#define CHECK_EXIT_IF_ERROR(ignore) \
                if (eError != ERR_OK) \
                {\
                    SK_DEBUG_ERROR("ERROR code %d\n", eError);\
                    return ERR_FAIL;\
                }

#define CHECK_EXIT_IF_NULL(x) \
                if (NULL == #x) \
                {\
                    SK_DEBUG_ERROR("%s is NULL\n", #x);\
                    return ERR_FAIL;\
                }

#define MAX_HTTP_REQUESTS 100
#define MAX_CMD_LINE_PARAMS (MAX_HTTP_REQUESTS + 2) /* MAX_HTTP_REQUESTS +
                                                        executable name +
                                                        -n parameter */

static double get_median_double(double* pdArray, int iSize);

void intHandler(int var)
{
    (void)var;
    SK_DEBUG_ERROR("You pressed CTRL-C\n");
}

int main(int argc, char** argv)
{
    if(argc > MAX_CMD_LINE_PARAMS)
    {
        SK_DEBUG_ERROR("Too many parameters\n");
        
        return ERR_FAIL;
    }
    
    SK_Error eError = ERR_OK;
    
    signal(SIGINT, intHandler);
    
    eError = SKConnCheck_Init();
    CHECK_EXIT_IF_ERROR(eError);

    int iOpt;
    int iNumRequests = 1;
    void* pHeadersList = NULL;

#if !defined(DEBUG_BUILD)
    opterr = 0; /* Disable getopt error messages */
#endif

    while((iOpt = getopt(argc, argv, "H:n:")) != -1)
    {
        switch(iOpt)
        {
            case 'H':
            SK_DEBUG_INFO("-H %s\n", optarg);
            
            pHeadersList = SKConnCheck_LinkListAppendString(pHeadersList, optarg);
            
            if(NULL == pHeadersList)
            {
                SK_DEBUG_ERROR("Failed to create list of headers\n");
                return ERR_FAIL;
            }
            break;
            
            case 'n':
            SK_DEBUG_INFO("-n %s\n", optarg);
            
            iNumRequests = atoi(optarg);
            
            if ((iNumRequests > MAX_HTTP_REQUESTS) || (iNumRequests < 0))
            {
                SK_DEBUG_ERROR("Invalid number of requests."
                    " Max requests supported: 100\n");
                return ERR_FAIL;
            }
            break;
            
            default:
                SK_DEBUG_ERROR("Invalid Arg -%c\n", optopt);
                fprintf(stderr, "Usage:\n%s [-H <HTTP Header>]... "
                    "[-n <Number of HTTP requests to make>]\n"
                    "Max HTTP Headers supported: 100. "
                    "Max number of requests supported: 100.\n", argv[0]);
                
                return ERR_FAIL;
            break;
            
        }
    }
    
    SKConnCheckSession *pSession = NULL;
    
    eError = SKConnCheck_SessionInit(&pSession);
    CHECK_EXIT_IF_ERROR(eError);
    CHECK_EXIT_IF_NULL(pSession);

    eError = SKConnCheck_SessionPrepareRequest(pSession, SK_REQ_TYPE_GET, pHeadersList);
    CHECK_EXIT_IF_ERROR(eError);

    double* pdNameLookupTime = malloc(sizeof(double)*iNumRequests);
    double* pdConnectTime = malloc(sizeof(double)*iNumRequests);
    double* pdStartTransferTime = malloc(sizeof(double)*iNumRequests);
    double* pdTotalTime = malloc(sizeof(double)*iNumRequests);

    CHECK_EXIT_IF_NULL(pdNameLookupTime);
    CHECK_EXIT_IF_NULL(pdConnectTime);
    CHECK_EXIT_IF_NULL(pdStartTransferTime);
    CHECK_EXIT_IF_NULL(pdTotalTime);
    
    for (int i = 0; i < iNumRequests; i++)
    {
        eError = SKConnCheck_SessionSendRequest(pSession);
        CHECK_EXIT_IF_ERROR(eError);

        eError = SKConnCheck_SessionGetStats(pSession);
        CHECK_EXIT_IF_ERROR(eError);
        
        pdNameLookupTime[i] = pSession->dNameLookupTime;
        pdConnectTime[i] = pSession->dConnectTime;
        pdStartTransferTime[i] = pSession->dStartTransferTime;
        pdTotalTime[i] = pSession->dTotalTime;
                
        SK_DEBUG_INFO("\
        CURLINFO_PRIMARY_IP = %s, CURLINFO_RESPONSE_CODE = %ld, \
        CURLINFO_NAMELOOKUP_TIME = %f, CURLINFO_CONNECT_TIME = %f, \
        CURLINFO_STARTTRANSFER_TIME = %f, CURLINFO_TOTAL_TIME = %f\n",       
        pSession->pcIPAddr, pSession->lHttpResponseCode,
        pSession->dNameLookupTime, pSession->dConnectTime,
        pSession->dStartTransferTime, pSession->dTotalTime);
    }
    
    /* print the information in the required format */
    fprintf(stderr, "SKTEST;%s;%ld;"
            "%f;%f;"
            "%f;%f\n",
            pSession->pcIPAddr, pSession->lHttpResponseCode,
            get_median_double(pdNameLookupTime, iNumRequests), get_median_double(pdConnectTime, iNumRequests),
            get_median_double(pdStartTransferTime, iNumRequests), get_median_double(pdTotalTime, iNumRequests));

    /* Cleanup */
    free(pdNameLookupTime);
    free(pdConnectTime);
    free(pdStartTransferTime);
    free(pdTotalTime);
    pdNameLookupTime = NULL;
    pdConnectTime = NULL;
    pdStartTransferTime = NULL;
    pdTotalTime = NULL;
    
    if (NULL != pHeadersList)
    {
        SKConnCheck_LinkListFree(pHeadersList);
        pHeadersList = NULL;
    }
    
    eError = SKConnCheck_SessionDeinit(pSession);
    CHECK_EXIT_IF_ERROR(eError);
    pSession = NULL;

    SKConnCheck_Deinit();
    
    return ERR_OK;
}

static int compare_double (const void *pVal1, const void *pVal2)
{
    CHECK_EXIT_IF_NULL(pVal1);
    CHECK_EXIT_IF_NULL(pVal2);
    
    const double *pdVal1 = (const double *) pVal1;
    const double *pdVal2 = (const double *) pVal2;

    return (*pdVal1 > *pdVal2) - (*pdVal1 < *pdVal2);
}

static double get_median_double(double* pdArray, int iSize)
{
    CHECK_EXIT_IF_NULL(pdArray);
    CHECK_EXIT_IF_NULL(iSize); /* Check if size is zero */
    
    qsort(pdArray, iSize, sizeof (double), compare_double);
    
    if(iSize % 2)
    {
        return pdArray[iSize/2];
    }
    else
    {
        return (pdArray[(iSize/2)] + pdArray[(iSize/2) - 1])/2;
    }
}
