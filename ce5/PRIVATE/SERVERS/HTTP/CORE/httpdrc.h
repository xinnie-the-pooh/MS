//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// This source code is licensed under Microsoft Shared Source License
// Version 1.0 for Windows CE.
// For a copy of the license visit http://go.microsoft.com/fwlink/?LinkId=3223.
//

#define IDS_LINKTOPARENTDIR		5


#define RESBASE_body		    6

// The offsets need to match the enum RESPONSESTATUS.  These correspond to HTTP response codes.
#define IDS_MOVED                 (RESBASE_body + 1)
#define IDS_BADREQ                (RESBASE_body + 3)
#define IDS_UNAUTHORIZED          (RESBASE_body + 4)
#define IDS_FORBIDDEN             (RESBASE_body + 5)
#define IDS_NOTFOUND              (RESBASE_body + 6)
#define IDS_INTERNALERR           (RESBASE_body + 7)
#define IDS_NOTIMPLEM             (RESBASE_body + 8)
#define IDS_METHOD_NOTALLOWED     (RESBASE_body + 12)
#define IDS_CONFLICT              (RESBASE_body + 13)
#define IDS_LENGTHREQUIRED        (RESBASE_body + 14)
#define IDS_PRECONDITION_FAILED   (RESBASE_body + 15)
#define IDS_REQUESTTOOLARGE       (RESBASE_body + 16)
#define IDS_UNSUPPORTEDMEDIA      (RESBASE_body + 17)
#define IDS_LOCKED                (RESBASE_body + 18)
#define IDS_INSUFFICIENT_STORAGE  (RESBASE_body + 19)

// Misc Codes.
#define IDS_SERVER_BUSY            30


// Codes that are not part of HTTP, but are generated by the web server for log.
#define IDS_HTTPD_STARTUP             50
#define IDS_HTTPD_SHUTDOWN_START      51
#define IDS_HTTPD_EXCEPTION           52
#define IDS_HTTPD_FILT_EXCEPTION      53
#define IDS_HTTPD_EXT_EXCEPTION       54
#define IDS_HTTPD_NO_REGKEY           55
#define IDS_HTTPD_DISABLED            56
#define IDS_HTTPD_SHUTDOWN_COMPLETE   57
#define IDS_HTTPD_NO_VROOTS           58
#define IDS_HTTPD_BIND_ERROR          59
#define IDS_HTTPD_AUTH_INIT_ERROR     60
#define IDS_HTTPD_SSL_INIT_ERROR      61
