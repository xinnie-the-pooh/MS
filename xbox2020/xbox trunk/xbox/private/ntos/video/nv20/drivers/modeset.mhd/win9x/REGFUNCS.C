 /*
    FILE:   regfuncs.c
    DATE:   4/8/99

    This file holds code to get certain things from the registry.
*/

#include "cmntypes.h"
#include "mywin.h"
#include "modeext.h"
#include "modeset.h"
#include "regfuncs.h"
#include "..\..\common\inc\nvreg.h"
#include "utils.h"
#include "debug.h"

extern  DEVICEANDTYPE   DeviceTypes[8];

/*
    GetLocalPath

    Purpose:    This routine extracts the local registry path
                from the lpRegData structure.

    Arguments:  lpRegData   LPREGDATA
                lpBuffer    Ptr to place to store registry path

    Returns:    The buffer is filled in with the
                registry path to ....\Display\000X

                Always returns TRUE.
*/
int CFUNC
GetLocalPath (LPREGDATA lpRegData, LPCHAR lpBuffer)
{
    lstrcpy (lpBuffer, lpRegData->szRegPath);
    return  (TRUE);
}


/*
    GetLocalNvidiaPath

    Purpose:    This routine extracts the local registry path
                from the RegData structure and tacks on the
                Nvidia substring.

    Arguments:  lpRegData   LPREGDATA
                lpBuffer    Ptr to place to store registry path

    Returns:    The buffer is filled in with the
                registry path to ....\Display\000X\Nvidia

                Always returns TRUE.
*/

int CFUNC
GetLocalNvidiaPath (LPREGDATA lpRegData, LPCHAR lpBuffer)
{
    // Get the local path
    GetLocalPath (lpRegData, lpBuffer);

    // Tack on Nvidia.
    lstrcat (lpBuffer, "\\NVidia");
    return  (TRUE);
}


/*
    GetLocalNvidiaDisplayPath

    Purpose:    This routine extracts the local registry path
                from the RegData structure and tacks on the
                Nvidia\Display string.

    Arguments:  lpRegData   LPREGDATA
                lpBuffer    Ptr to place to store registry path

    Returns:    The buffer is filled in with the
                registry path to ....\Display\000X\Nvidia\Display

                Always returns TRUE.
*/

int CFUNC
GetLocalNvidiaDisplayPath (LPREGDATA lpRegData, LPCHAR lpBuffer)
{
    // Get the local path
    GetLocalNvidiaPath (lpRegData, lpBuffer);

    // Tack on "Display"
    lstrcat (lpBuffer, "\\");
    lstrcat (lpBuffer, NV4_REG_DISPLAY_DRIVER_SUBKEY);
    return  (TRUE);
}


/*
    GetLocalNvidiaDisplayLogPath

    Purpose:    This routine extracts the local registry path
                from the RegData structure and tacks on the
                Nvidia\Display\LogDevice? string.

    Arguments:  lpRegData   LPREGDATA
                dwLogDevice 0 based logical device number
                lpBuffer    Ptr to place to store registry path

    Returns:    The buffer is filled in with the
                registry path to
                ....\Display\000X\Nvidia\Display\LogicalDevice?

                Always returns TRUE.
*/

int CFUNC
GetLocalNvidiaDisplayLogPath (LPREGDATA lpRegData, ULONG dwLogDevice,
                                LPCHAR lpBuffer)
{
    char    szTemp[32];

    // Get the local path
    GetLocalNvidiaDisplayPath (lpRegData, lpBuffer);

    // Tack on LogicalDevice? where ? is the logical device number
    wsprintf (szTemp, "\\LogicalDevice%ld", dwLogDevice);
    lstrcat (lpBuffer, szTemp);
    return  (TRUE);
}


/*
    GetLocalNvidiaDisplayDevPath

    Purpose:    This routine extracts the local registry path
                from the RegData structure and tacks on the
                Nvidia\Display\%DEV#% string.

    Arguments:  lpRegData    LPREGDATA
                dwLogDevice  0 based logical device number
                lpDevData    LPDEVDATA
                lpBuffer     Ptr to place to store registry path

    Returns:    The buffer is filled in with the
                registry path to
                ..\Display\000X\Nvidia\Display\%DEV#%

                Always returns TRUE.
*/

int CFUNC
GetLocalNvidiaDisplayDevPath (LPREGDATA lpRegData, LPDEVDATA lpDevData,
                                    LPCHAR lpBuffer)
{
    char    szTemp[12];

    // Get the local path
    GetLocalNvidiaDisplayPath (lpRegData, lpBuffer);

    // Tack on PhysicalDevice? where ? is the physical device number
    wsprintf (szTemp, "\\%s%d", DeviceTypes[lpDevData->cType].lpszName,
                                lpDevData->cNumber);
    lstrcat (lpBuffer, szTemp);
    return  (TRUE);
}


/*
    GetLocalSubKey

    Purpose:    This routine gets the local subkey which is
                ....Display\000X

    Arguments:  lpRegData   LPREGDATA

    Returns:    The registry key to ....\Display\000X
                NULL if there was an error.
*/
ULONG CFUNC
GetLocalSubKey (LPREGDATA lpRegData)
{
    char    szBuffer[MAX_KEY_LEN];
    ULONG   hDisplay, lRet;

    // Get local path and concatenate NVidia to form the local base path
    GetLocalPath (lpRegData, szBuffer);

    // Open this key
    lRet = RegCreateKey (lpRegData->dwMainKey, szBuffer, &hDisplay);
    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (hDisplay);
}


/*
    GetLocalNvidiaSubKey

    Purpose:    This routine gets the local subkey which is
                ....Display\000X\Nvidia.

    Arguments:  lpRegData   LPREGDATA

    Returns:    The registry key to ....\Display\000X\Nvidia.
                NULL if there was an error.
*/
ULONG CFUNC
GetLocalNvidiaSubKey (LPREGDATA lpRegData)
{
    char    szBuffer[MAX_KEY_LEN];
    ULONG   hDisplay, lRet;

    // Get local path and concatenate NVidia to form the local base path
    GetLocalNvidiaPath (lpRegData, szBuffer);

    // Open this key
    lRet = RegCreateKey (lpRegData->dwMainKey, szBuffer, &hDisplay);
    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (hDisplay);
}


/*
    GetLocalNvidiaDisplaySubKey

    Purpose:    This routine gets the local subkey which is
                ....Display\000X\Nvidia\Display.

    Arguments:  lpRegData   LPREGDATA

    Returns:    The registry key to ....\Display\000X\Nvidia\Display
                NULL if there was an error.
*/
ULONG CFUNC
GetLocalNvidiaDisplaySubKey (LPREGDATA lpRegData)
{
    char    szBuffer[MAX_KEY_LEN];
    ULONG   hDisplay, lRet;

    // Get local path and concatenate NVidia/Display
    GetLocalNvidiaDisplayPath (lpRegData, szBuffer);

    // Open this key
    lRet = RegCreateKey (lpRegData->dwMainKey, szBuffer, &hDisplay);
    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (hDisplay);
}


/*
    GetLocalNvidiaDisplayLogSubKey

    Purpose:    This routine gets the local subkey which is
                ....Display\000X\Nvidia\Display\LogicalDevice?

    Arguments:  lpRegData   LPREGDATA
                dwLogDevice 0 based logical device number

    Returns:    The registry key to
                ..\Display\000X\Nvidia\Display\LogicalDevice?
                NULL if there was an error.
*/
ULONG CFUNC
GetLocalNvidiaDisplayLogSubKey (LPREGDATA lpRegData, ULONG dwLogDevice)
{
    char    szBuffer[MAX_KEY_LEN];
    ULONG   hDisplay, lRet;

    // Get local path and concatenate NVidia to form the local base path
    GetLocalNvidiaDisplayLogPath (lpRegData, dwLogDevice, szBuffer);

    // Open this key
    lRet = RegCreateKey (lpRegData->dwMainKey, szBuffer, &hDisplay);
    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (hDisplay);
}


/*
    GetLocalNvidiaDisplayDevSubKey

    Purpose:    This routine gets the local subkey which is
                ..Display\000X\Nvidia\Display\LogicalDevice?\PhysicalDevice?

    Arguments:  lpRegData    LPREGDATA
                lpDevData    LPDEVDATA

    Returns:    The registry key to
                ..\Display\000X\Nvidia\Display\LogicalDevice?
                NULL if there was an error.
*/
ULONG CFUNC
GetLocalNvidiaDisplayDevSubKey (LPREGDATA lpRegData, LPDEVDATA lpDevData)
{
    char    szBuffer[MAX_KEY_LEN];
    ULONG   hDisplay, lRet;

    // Get local path and concatenate NVidia to form the local base path
    GetLocalNvidiaDisplayDevPath (lpRegData, lpDevData, szBuffer);

    // Open this key
    lRet = RegCreateKey (lpRegData->dwMainKey, szBuffer, &hDisplay);
    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (hDisplay);
}


/*
    GetRegValue

    Purpose:    This routine reads a decimal value from the key passed in

    Arguments:  hKey        key which holds value to read
                lpStr       string pointing to value name
                nDefault    value to return if lpStr didn't exist

    Returns:    The registry key and entry are read.
                nDefault is returned if there is an error or lpStr
                doesn't exist
*/
int CFUNC
GetRegValue (ULONG hKey, LPCHAR lpStr, int nDefault)
{
    ULONG   dwNum, dwSize;
    long    lRet;

    // Query the value from the lpStr entry
    dwNum = 0;
    dwSize = 4;
    lRet = RegQueryValueEx (hKey, lpStr, NULL, NULL, (LPCHAR) &dwNum, &dwSize);

    // CLose the key
    RegCloseKey (hKey);

    // If there was an error or no such key, then use the default value
    if  ((lRet != ERROR_SUCCESS) || (dwSize == 0))
        dwNum = (ULONG) nDefault;

    return  ((int) dwNum);
}


/*
    GetLocalRegValue

    Purpose:    This routine reads a decimal value from the local subkey
                which is ....Display\000X

    Arguments:  lpRegData   LPREGDATA
                lpStr       string pointing to value name
                nDefault    value to return if lpStr didn't exist

    Returns:    The value of the lpStr entry in the
                ...\Display\000X key.
                nDefault is returned if there is an error or lpStr
                doesn't exist
*/
int CFUNC
GetLocalRegValue (LPREGDATA lpRegData, LPCHAR lpStr, int nDefault)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalSubKey (lpRegData)))
        return  (nDefault);

    return  (GetRegValue (hDisplay, lpStr, nDefault));
}


/*
    GetLocalNvidiaRegValue

    Purpose:    This routine reads a decimal value from the local subkey
                which is ....Display\000X\Nvidia

    Arguments:  lpRegData   LPREGDATA
                lpStr       string pointing to value name
                nDefault    value to return if lpStr didn't exist

    Returns:    The value of the lpStr entry in the
                ...\Display\000X\Nvidia key.
                nDefault is returned if there is an error or lpStr
                doesn't exist
*/

int CFUNC
GetLocalNvidiaRegValue (LPREGDATA lpRegData, LPCHAR lpStr, int nDefault)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaSubKey (lpRegData)))
        return  (nDefault);

    return  (GetRegValue (hDisplay, lpStr, nDefault));
}


/*
    GetLocalNvidiaDisplayRegValue

    Purpose:    This routine reads a decimal value from the local subkey
                which is ....Display\000X\Nvidia\Display

    Arguments:  lpRegData   LPREGDATA
                lpStr       string pointing to value name
                nDefault    value to return if lpStr didn't exist

    Returns:    The value of the lpStr entry in the
                ...\Display\000X\Nvidia\Display key.
                nDefault is returned if there is an error or lpStr
                doesn't exist
*/

int CFUNC
GetLocalNvidiaDisplayRegValue (LPREGDATA lpRegData, LPCHAR lpStr, int nDefault)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplaySubKey (lpRegData)))
        return  (nDefault);

    return  (GetRegValue (hDisplay, lpStr, nDefault));
}


/*
    GetLocalNvidiaDisplayLogRegValue

    Purpose:    This routine reads a decimal value from the local subkey
                which is
                ..Display\000X\Nvidia\Display\LogicalDevice?

    Arguments:  lpRegData   LPREGDATA
                dwLogDevice 0 based logical device number
                lpStr       string pointing to value name
                nDefault    value to return if lpStr didn't exist

    Returns:    The value of the lpStr entry in the
                ...\Display\000X\Nvidia\Display\LogicalDevice? key.
                nDefault is returned if there is an error or lpStr
                doesn't exist
*/
int CFUNC
GetLocalNvidiaDisplayLogRegValue (LPREGDATA lpRegData, ULONG dwLogDevice,
                                    LPCHAR lpStr, int nDefault)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplayLogSubKey (lpRegData, dwLogDevice)))
        return  (nDefault);

    return  (GetRegValue (hDisplay, lpStr, nDefault));
}


/*
    GetLocalNvidiaDisplayDevRegValue

    Purpose:    This routine reads a decimal value from the local subkey
                which is
                ..Display\000X\Nvidia\Display\LogicalDevice?\PhysDevice?

    Arguments:  lpRegData    LPREGDATA
                lpDevData    LPDEVDATA
                lpStr        string pointing to value name
                nDefault     value to return if lpStr didn't exist

    Returns:    The value of the lpStr entry in the
                ...\Display\000X\Nvidia\Display\%DEV#% key.
                nDefault is returned if there is an error or lpStr
                doesn't exist
*/
int CFUNC
GetLocalNvidiaDisplayDevRegValue (LPREGDATA lpRegData, LPDEVDATA lpDevData,
                                    LPCHAR lpStr, int nDefault)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplayDevSubKey (lpRegData, lpDevData)))
        return  (nDefault);

    return  (GetRegValue (hDisplay, lpStr, nDefault));
}


/*
    GetRegString

    This routine reads a string value from the ...\DISPLAY\000X key.
    If the value specified by the lpStr string does not exist or there
    is any error in reading the registry, then FALSE is returned. Otherwise,
    TRUE is returned and lpRet gets set to the string read from the registry.
*/
int CFUNC
GetRegString (ULONG hKey, LPCHAR lpStr, LPCHAR lpRet)
{
    ULONG   dwSize, lRet;

    // Query the value from the lpStr entry
    dwSize = MAX_KEY_LEN;
    lRet = RegQueryValueEx (hKey, lpStr, NULL, NULL, lpRet, &dwSize);

    // Close the key
    RegCloseKey (hKey);

    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (TRUE);
}


/*
    GetLocalRegString

    Purpose:    This routine reads a string value from the local subkey
                which is ....Display\000X

    Arguments:  lpRegData   LPREGDATA
                lpStr   string pointing to entry name
                lpRet   place to return string

    Returns:    FALSE is returned if the string doesn't exist
                If TRUE is returned then lpRet is filled in with
                the string value of the entry given by lpStr
*/

int CFUNC
GetLocalRegString (LPREGDATA lpRegData, LPCHAR lpStr, LPCHAR lpRet)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalSubKey (lpRegData)))
        return  (FALSE);

    return  (GetRegString (hDisplay, lpStr, lpRet));
}


/*
    GetLocalNvidiaRegString

    Purpose:    This routine reads a string value from the local subkey
                which is ....Display\000X\Nvidia

    Arguments:  lpRegData   LPREGDATA
                lpStr   string pointing to entry name
                lpRet   place to return string

    Returns:    FALSE is returned if the string doesn't exist
                If TRUE is returned then lpRet is filled in with
                the string value of the entry given by lpStr
*/
int CFUNC
GetLocalNvidiaRegString (LPREGDATA lpRegData, LPCHAR lpStr, LPCHAR lpRet)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaSubKey (lpRegData)))
        return  (FALSE);

    return  (GetRegString (hDisplay, lpStr, lpRet));
}


/*
    GetLocalNvidiaDisplayRegString

    Purpose:    This routine reads a string value from the local subkey
                which is ....Display\000X\Nvidia\Display

    Arguments:  lpRegData   LPREGDATA
                lpStr   string pointing to entry name
                lpRet   place to return string

    Returns:    FALSE is returned if the string doesn't exist
                If TRUE is returned then lpRet is filled in with
                the string value of the entry given by lpStr
*/
int CFUNC
GetLocalNvidiaDisplayRegString (LPREGDATA lpRegData, LPCHAR lpStr, LPCHAR lpRet)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplaySubKey (lpRegData)))
        return  (FALSE);

    return  (GetRegString (hDisplay, lpStr, lpRet));
}


/*
    GetLocalNvidiaDisplayLogRegString

    Purpose:    This routine reads a string value from the local subkey
                which is
                ..Display\000X\Nvidia\Display\LogicalDevice?

    Arguments:  lpRegData   LPREGDATA
                dwLogDevice 0 based logical device number
                lpStr       string pointing to entry name
                lpRet       place to return string

    Returns:    FALSE is returned if the string doesn't exist
                If TRUE is returned then lpRet is filled in with
                the string value of the entry given by lpStr
*/
int CFUNC
GetLocalNvidiaDisplayLogRegString (LPREGDATA lpRegData, ULONG dwLogDevice,
                                    LPCHAR lpStr, LPCHAR lpRet)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplayLogSubKey (lpRegData, dwLogDevice)))
        return  (FALSE);

    return  (GetRegString (hDisplay, lpStr, lpRet));
}


/*
    GetLocalNvidiaDisplayDevRegString

    Purpose:    This routine reads a string value from the local subkey
                which is
                ..Display\000X\Nvidia\Display\%DEV#%

    Arguments:  lpRegData   LPREGDATA
                lpDevData   LPDEVDATA
                lpStr       string pointing to entry name
                lpRet       place to return string

    Returns:    FALSE is returned if the string doesn't exist
                If TRUE is returned then lpRet is filled in with
                the string value of the entry given by lpStr
*/
int CFUNC
GetLocalNvidiaDisplayDevRegString (LPREGDATA lpRegData, LPDEVDATA lpDevData,
                                    LPCHAR lpStr, LPCHAR lpRet)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplayDevSubKey (lpRegData, lpDevData)))
        return  (FALSE);

    return  (GetRegString (hDisplay, lpStr, lpRet));
}


/*
    DeleteRegEntry

    Purpose:    This routine deletes an entry from a registry key.

    Arguments:  hKey    Key containgin element to delete
                lpStr   string pointing to entry name to delete

    Returns:    Always returns TRUE
*/
int WINAPI
DeleteRegEntry (ULONG hKey, LPCHAR lpStr)
{
    // Delete the value
    RegDeleteValue (hKey, lpStr);

    // CLose the key
    RegCloseKey (hKey);

    return  (TRUE);
}


/*
    DeleteLocalRegEntry

    Purpose:    This routine deletes an entry from the registry
                path ....\Display\000X

    Arguments:  lpRegData   LPREGDATA
                lpStr   string pointing to entry name to delete

    Returns:    Always returns TRUE
*/
int WINAPI
DeleteLocalRegEntry (LPREGDATA lpRegData, LPCHAR lpStr)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalSubKey (lpRegData)))
        return  (FALSE);

    return  (DeleteRegEntry (hDisplay, lpStr));
}


/*
    DeleteLocalNvidiaRegEntry

    Purpose:    This routine deletes an entry from the registry
                path ....\Display\000X\Nvidia

    Arguments:  lpRegData   LPREGDATA
                lpStr   string pointing to entry name to delete

    Returns:    Always returns TRUE
*/
int WINAPI
DeleteLocalNvidiaRegEntry (LPREGDATA lpRegData, LPCHAR lpStr)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaSubKey (lpRegData)))
        return  (FALSE);

    return  (DeleteRegEntry (hDisplay, lpStr));
}



/*
    DeleteLocalNvidiaDisplayRegEntry

    Purpose:    This routine deletes an entry from the registry
                path ....\Display\000X\Nvidia\Display

    Arguments:  lpRegData   LPREGDATA
                lpStr   string pointing to entry name to delete

    Returns:    Always returns TRUE
*/
int WINAPI
DeleteLocalNvidiaDisplayRegEntry (LPREGDATA lpRegData, LPCHAR lpStr)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplaySubKey (lpRegData)))
        return  (FALSE);

    return  (DeleteRegEntry (hDisplay, lpStr));
}


/*
    DeleteLocalNvidiaDisplayLogRegEntry

    Purpose:    This routine deletes an entry from the registry
                path ....\Display\000X\Nvidia\Display\LogicalDevice?

    Arguments:  lpRegData   LPREGDATA
                dwLogDevice 0 based Logical device number
                lpStr       string pointing to entry name to delete

    Returns:    Always returns TRUE
*/
int WINAPI
DeleteLocalNvidiaDisplayLogRegEntry (LPREGDATA lpRegData, ULONG dwLogDevice,
                                        LPCHAR lpStr)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplayLogSubKey (lpRegData, dwLogDevice)))
        return  (FALSE);

    return  (DeleteRegEntry (hDisplay, lpStr));
}


/*
    DeleteLocalNvidiaDisplayDevRegEntry

    Purpose:    This routine deletes an entry from the registry
                path
                ..Display\000X\Nvidia\Display\LogicalDevice?\PhysicalDevice?

    Arguments:  lpRegData   LPREGDATA
                lpDevData   LPDEVDATA
                lpStr       string pointing to entry name to delete

    Returns:    Always returns TRUE
*/
int WINAPI
DeleteLocalNvidiaDisplayDevRegEntry (LPREGDATA lpRegData, LPDEVDATA lpDevData,
                                        LPCHAR lpStr)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplayDevSubKey (lpRegData, lpDevData)))
        return  (FALSE);

    return  (DeleteRegEntry (hDisplay, lpStr));
}


/*
    SetRegString

    Purpose:    This routine writes an entry into the registry
                path passed in.

    Arguments:  hKey    key to write to
                lpStr   string pointing to entry name to set
                lpData  "string value" of entry
                dwDataLen  length of lpData in bytes to write to registry

    Returns:    TRUE    value was written
                FALSE   there was an error
*/
int CFUNC
SetRegString (ULONG hKey, LPCHAR lpStr, LPCHAR lpData, ULONG dwDataLen)
{
    long    lRet;

    // Write the value from the lpStr entry
    lRet = RegSetValueEx (hKey, lpStr, NULL, REG_SZ, lpData, dwDataLen);

    // Close the key
    RegCloseKey (hKey);

    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (TRUE);
}


/*
    SetRegValue

    Purpose:    This routine writes an entry into the registry
                path passed in.

    Arguments:  hKey    key to write to
                lpStr   string pointing to entry name to set
                lpData  "string value" of entry
                dwDataLen  length of lpData in bytes to write to registry

    Returns:    TRUE    value was written
                FALSE   there was an error
*/
int CFUNC
SetRegValue (ULONG hKey, LPCHAR lpStr, int nVal)
{
    long    lRet;
    ULONG   ulVal;

    ulVal = (ULONG) nVal;
    // Write the value from the lpStr entry
    lRet = RegSetValueEx (hKey, lpStr, NULL, REG_DWORD, (LPCHAR) &ulVal, sizeof (ulVal));

    // Close the key
    RegCloseKey (hKey);

    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (TRUE);
}


/*
    SetLocalRegString

    Purpose:    This routine writes an entry into the registry
                path ....\Display\000X

    Arguments:  lpRegData   LPREGDATA
                lpStr   string pointing to entry name to set
                lpData  "string value" of entry
                dwDataLen  length of lpData in bytes to write to registry

    Returns:    TRUE    value was written
                FALSE   there was an error
*/
int CFUNC
SetLocalRegString (LPREGDATA lpRegData, LPCHAR lpStr, LPCHAR lpData, ULONG dwDataLen)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalSubKey (lpRegData)))
        return  (FALSE);

    return  (SetRegString (hDisplay, lpStr, lpData, dwDataLen));
}


/*
    SetLocalNvidiaRegString

    Purpose:    This routine writes an entry into the registry
                path ....\Display\000X\Nvidia

    Arguments:  lpRegData   LPREGDATA
                lpStr       string pointing to entry name to set
                lpData      "string value" of entry
                dwDataLen   length of lpData in bytes to write to registry

    Returns:    TRUE    value was written
                FALSE   there was an error
*/
int CFUNC
SetLocalNvidiaRegString (LPREGDATA lpRegData, LPCHAR lpStr, LPCHAR lpData, ULONG dwDataLen)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaSubKey (lpRegData)))
        return  (FALSE);

    return  (SetRegString (hDisplay, lpStr, lpData, dwDataLen));
}


/*
    SetLocalNvidiaRegValue

    Purpose:    This routine writes an entry into the registry
                path ....\Display\000X\Nvidia

    Arguments:  lpRegData   LPREGDATA
                lpStr       string pointing to entry name to set
                lpData      "string value" of entry
                dwDataLen   length of lpData in bytes to write to registry

    Returns:    TRUE    value was written
                FALSE   there was an error
*/
int CFUNC
SetLocalNvidiaRegValue (LPREGDATA lpRegData, LPCHAR lpStr, int nVal)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaSubKey (lpRegData)))
        return  (FALSE);

    return  (SetRegValue (hDisplay, lpStr, nVal));
}


/*
    SetLocalNvidiaDisplayRegString

    Purpose:    This routine writes an entry into the registry
                path ....\Display\000X\Nvidia\Display

    Arguments:  lpRegData   LPREGDATA
                lpStr       string pointing to entry name to set
                lpData      "string value" of entry
                dwDataLen   length of lpData in bytes to write to registry

    Returns:    TRUE    value was written
                FALSE   there was an error
*/
int CFUNC
SetLocalNvidiaDisplayRegString (LPREGDATA lpRegData, LPCHAR lpStr, LPCHAR lpData, ULONG dwDataLen)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplaySubKey (lpRegData)))
        return  (FALSE);

    return  (SetRegString (hDisplay, lpStr, lpData, dwDataLen));
}


/*
    SetLocalNvidiaDisplayLogRegString

    Purpose:    This routine writes an entry into the registry
                path
                ..\Display\000X\Nvidia\Display\LogicalDevice?

    Arguments:  lpRegData    LPREGDATA
                dwLogDevice  0 based logical device number
                lpStr        string pointing to entry name to set
                lpData       "string value" of entry
                dwDataLen    length of lpData in bytes to write to registry

    Returns:    TRUE    value was written
                FALSE   there was an error
*/
int CFUNC
SetLocalNvidiaDisplayLogRegString (LPREGDATA lpRegData, ULONG dwLogDevice,
                            LPCHAR lpStr, LPCHAR lpData, ULONG dwDataLen)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplayLogSubKey (lpRegData, dwLogDevice)))
        return  (FALSE);

    return  (SetRegString (hDisplay, lpStr, lpData, dwDataLen));
}


/*
    SetLocalNvidiaDisplayDevRegString

    Purpose:    This routine writes an entry into the registry
                path
                ..\Display\000X\Nvidia\Display\LogicalDevice?\PhyscialDevice?

    Arguments:  lpRegData   LPREGDATA
                lpDevData   LPDEVDATA
                lpStr       string pointing to entry name to set
                lpData      "string value" of entry
                dwDataLen   length of lpData in bytes to write to registry

    Returns:    TRUE    value was written
                FALSE   there was an error
*/
int CFUNC
SetLocalNvidiaDisplayDevRegString (LPREGDATA lpRegData, LPDEVDATA lpDevData,
                            LPCHAR lpStr, LPCHAR lpData, ULONG dwDataLen)
{
    ULONG   hDisplay;

    if  (!(hDisplay = GetLocalNvidiaDisplayDevSubKey (lpRegData, lpDevData)))
        return  (FALSE);

    return  (SetRegString (hDisplay, lpStr, lpData, dwDataLen));
}


/*
    GetMonitorLocalSubKey

    Purpose:    This routine gets the key for the local monitor
                path which looks like ....\Monitor\000X

    Arguments:  lpRegData   LPREGDATA

    Returns:    The key to the ....\Monitor\000X registry path
                0 is returned if there is an error
*/
ULONG CFUNC
GetMonitorLocalSubKey (LPREGDATA lpRegData)
{
    ULONG   hMonitor, lRet;

    // Open this key
    lRet = RegCreateKey (lpRegData->dwMainKey, lpRegData->szRegPath, &hMonitor);
    if  (lRet != ERROR_SUCCESS)
        return  (FALSE);

    return  (hMonitor);
}


/*
    GetMonitorLocalRegString

    Purpose:    This routine reads a string value from the local subkey
                which is ....Monitor\000X

    Arguments:  lpRegData   LPREGDATA
                lpStr   string pointing to entry name
                lpRet   place to return string

    Returns:    FALSE is returned if the string doesn't exist
                If TRUE is returned then lpRet is filled in with
                the string value of the entry given by lpStr
*/
ULONG CFUNC
GetMonitorLocalRegString (LPREGDATA lpRegData, LPCHAR lpStr, LPCHAR lpData)
{
    ULONG   hMonitor;

    if  (!(hMonitor = GetMonitorLocalSubKey (lpRegData)))
        return  (FALSE);

    return  (GetRegString (hMonitor, lpStr, lpData));
}


/*
    PrintString0

    This routine prints a debug string.
*/
int CFUNC
PrintString0 (LPCHAR lpStr)
{
    OutputDebugString (lpStr);
    return  (TRUE);
}

/*
    PrintString1

    This routine prints a debug string with 1 argument.
*/
int CFUNC
PrintString1 (LPCHAR lpStr, ULONG dwValue)
{
    wsprintf (szDebug, lpStr, dwValue);
    OutputDebugString (szDebug);
    return  (TRUE);
}


/*
    MemoryAlloc

    This routine allocates the amount of memory requested and
    returns a ptr to the memory block.
*/
LPCHAR CFUNC
MemoryAlloc (ULONG dwSize)
{
    int     hMem;
    LPCHAR  lpMem;

    hMem = (int) GlobalAlloc (GMEM_SHARED, dwSize);

    if  (hMem)
    {
        lpMem = GlobalLock (hMem);

        if  (lpMem)
            return  (lpMem);
    }

    return  (FALSE);
}


/*
    MemoryFree

    This routine frees the memory block.
*/
int CFUNC
MemoryFree (LPULONG lpMem)
{
    int hMem;

    if  (lpMem)
    {
        hMem = (int) GlobalHandle ((USHORT) (((ULONG) lpMem) >> 16));

        if  (hMem)
        {
            GlobalUnlock (hMem);
            GlobalFree (hMem);
        }
    }

    return  (TRUE);
}

