#ifndef __SKCONNCHECK_H
#define __SKCONNCHECK_H

/****************************************
*
*   skconncheck.h
*   License, copyright, author, etc.
*   Notes
*   
*****************************************/

#if defined(DEBUG_BUILD)
#define DEBUG_VERBOSE 1
#else
#define DEBUG_VERBOSE 0
#endif

#define SK_DEBUG_INFO(fmt, ...) \
            do { if (DEBUG_VERBOSE) fprintf(stderr, "INFO %s():%d: " fmt, \
                                    __func__, __LINE__, ##__VA_ARGS__); } while (0)

#define SK_DEBUG_ERROR(fmt, ...) \
            do { fprintf(stderr, "ERROR %s():%d: " fmt, \
                        __func__, __LINE__, ##__VA_ARGS__); } while (0)
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    /* TODO: Add more useful error codes */
    ERR_OK = 0,
    ERR_FAIL,
} SK_Error;

typedef enum
{
    /* Only supports GET requests */
    SK_REQ_TYPE_GET = 0,
    SK_REQ_TYPE_INVALID,
    SK_REQ_TYPE_MAX
} SK_REQ_TYPE;

typedef struct
{
    void* hPrivate; /*  Private pointer used by the underlying library to 
                        identify the session. */
    char *pcIPAddr;
    long lHttpResponseCode;
    double dNameLookupTime;
    double dConnectTime;
    double dStartTransferTime;
    double dTotalTime;    
} SKConnCheckSession;

/* Initialises the underlying Library */
SK_Error SKConnCheck_Init();

/* Initialises a session. Allocates and returns a session-data structure */
SK_Error SKConnCheck_SessionInit(SKConnCheckSession** ppSession);

/* Sets up some fields of the GET request including any additional HTTP headers
   passed in via plHeaders. plHeaders can be NULL */
SK_Error SKConnCheck_SessionPrepareRequest(SKConnCheckSession* pSession, int iRequestType, char** plHeaders);

/* Sends the request and blocks until a response is received */
SK_Error SKConnCheck_SessionSendRequest(SKConnCheckSession* pSession);

/* Sets the IP Address, response code and the timing info in the
   SKConnCheckSession structure. Must be called after
   SKConnCheck_SessionSendRequest() */
SK_Error SKConnCheck_SessionGetStats(SKConnCheckSession* pSession);

/* Closes the session and frees any allocations done in
   SKConnCheck_SessionInit() */
SK_Error SKConnCheck_SessionDeinit(SKConnCheckSession* pSession);

/* Deinitialises the underlying library */
SK_Error SKConnCheck_Deinit();

/* Appends a string to the Linked List passed in and returns the new head ptr.
   If the linked list head parameter is NULL, a new Linked list is created. */
void* SKConnCheck_LinkListAppendString(void* pLinkListHead, char * pString);

/* Frees a linked list created by SKConnCheck_LinkListAppendString() */
void SKConnCheck_LinkListFree(void* pLinkListHead);

#ifdef __cplusplus
}
#endif

#endif