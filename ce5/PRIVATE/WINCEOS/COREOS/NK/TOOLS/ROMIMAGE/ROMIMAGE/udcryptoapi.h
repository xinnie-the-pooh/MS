//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// This source code is licensed under Microsoft Shared Source License
// Version 1.0 for Windows CE.
// For a copy of the license visit http://go.microsoft.com/fwlink/?LinkId=3223.
//
#pragma once

#ifndef __UDCRYPTOAPI_H_
#define __UDCRYPTOAPI_H_

//-----------------------------------------------------------------------------
//
// File: udcryptoapi.h
//
// Microsoft Confidential
//
//
// Description:
//     Unified DRM crypto library entry points.
//  
// History:
// -@- 10/25/00 (mikemarr)  - created
//
//-----------------------------------------------------------------------------

#include "udkeyblob.h"

//-----------------------------------------------------------------------------
// Function: UDRandomFill
//    Fill a buffer with cryptographically strong random bytes.  The fill
//  buffer is fed into the random number generator to further randomize its own
//  state, so it is good if pbFill already contains some random state.
//-----------------------------------------------------------------------------
extern HRESULT UDRandomFill(UINT cFill, BYTE *pbFill);

//-----------------------------------------------------------------------------
// Function: UDTrueRandomFill
//    Fill the buffer with truly random numbers.  This function is processor
//  dependent and may return E_FAIL.  It also takes about 10 milliseconds per 
//  byte, so should be used sparingly to compute seeds to faster RNG's.
//-----------------------------------------------------------------------------
extern HRESULT UDTrueRandomFill(UINT cFill, BYTE *pbFill);

//-----------------------------------------------------------------------------
// Memory Allocation Convention
//
// For functions of the form:
//      HRESULT Foo(..., UINT *pcOut, BYTE *pbOut);
//
//  - pbOut is allocated by the caller, and *pcOut is the size.
//  - If pbOut is NULL, *pcOut is written with the required size and S_OK is
//    returned.
//  - If *pcOut indicates a pbOut buffer that is too small, *pcOut is written 
//    with the required size and HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) 
//    is returned.
//  - On success, *pcOut is written with the number of bytes written to pbOut.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function: UDCreateDESKey
//    Create a random 56-bit DES symmetric key.
// Notes:
//  - pKey is allocated by the caller, and filled by this function.
//
//  Comment on uFlags usage:
//  ===========================
//  Use uUSE_EBOOK_DES flag when calling UDCreateDESKey() and UDInitKeyFromBinary() 
//  to get non-standard legacy eBook DES encryption/decryption
//  In all other cases, use default uUSE_STANDARD_DES flag
//  CALG_EBOOK_DES / CALG_DES is written into the KeyBlob header 
//  and referenced by UDEncrypt() and UDDecrypt() functions
//-----------------------------------------------------------------------------
extern HRESULT UDCreateDESKey(CKeyBlob *pKey, UINT uFlags=uUSE_STANDARD_DES);

//-----------------------------------------------------------------------------
// Function: UDCreateRSAKey
//    Create a random RSA key pair.  At least 512 and 1024 bit RSA keys are
//    supported.
// Notes:
//  - pKey is allocated by the caller, and filled by this function.
//  - requires advapi32.lib
//-----------------------------------------------------------------------------
extern HRESULT UDCreateRSAKey(UINT nBitLen, CKeyBlob *pKey);

//-----------------------------------------------------------------------------
// Function: UDInitKeyFromBinary
//    Initialize a key blob from a binary data like that generated by
//    UDWriteKeyToBinary or CPExportKey.  If the binary key data is not a
//    known format, NTE_BAD_KEY is returned.
// Notes:
//  - pKey is allocated by the caller, and filled by this function.
//
//  Comment on uFlags usage:
//  ===========================
//  Use uUSE_EBOOK_DES flag when calling UDCreateDESKey() and UDInitKeyFromBinary() 
//  to get non-standard legacy eBook DES encryption/decryption
//  In all other cases, use default uUSE_STANDARD_DES flag
//  CALG_EBOOK_DES / CALG_DES is written into the KeyBlob header 
//  and referenced by UDEncrypt() and UDDecrypt() functions
//-----------------------------------------------------------------------------
extern HRESULT UDInitKeyFromBinary( UINT cIn, BYTE *pbIn, CKeyBlob *pKey, 
                                        UINT uFlags=uUSE_STANDARD_DES);

//-----------------------------------------------------------------------------
// Function: UDWriteKeyToBinary
//    Write a key blob out to binary.  Generally, this is a CSP compatible
//    format (i.e. BLOBHEADER key blob) like that written with CPExportKey.
//    uFlags =
//       uWRITE_KEY_PUBLIC_ONLY - only write the public portion of a public/
//                                private key pair
//       uWRITE_KEY_LEGACY_DES  - dump the DES key without the blob header
// Notes:
//  - Uses memory allocation convention described above
//-----------------------------------------------------------------------------
#define uWRITE_KEY_PUBLIC_ONLY  0x0001
#define uWRITE_KEY_LEGACY_DES   0x0002
extern HRESULT UDWriteKeyToBinary(CKeyBlob *pKey, UINT uFlags, 
                                  UINT *pcOut, BYTE *pbOut);

//-----------------------------------------------------------------------------
// Function: UDWriteKeyToXML
//    Write a key blob out in XrML compliant wide character XML.  For 
//    public/private key pairs, only the public portion is written.  If the key 
//    blob represents a symmetric key, NTE_BAD_KEY is returned.
// Notes:
//  - Uses memory allocation convention described above
//-----------------------------------------------------------------------------
extern HRESULT UDWriteKeyToXML(CKeyBlob *pKey, UINT uFlags, 
                               UINT *pcOut, WCHAR *pwOut);

//-----------------------------------------------------------------------------
// Function: UDEncrypt/UDDecrypt
//    Encode/Decode a block of content with a key.  CKeyBlob can be any key 
//    type, including but not limited to symmetric, public, and private keys.
//    If the keyblob represents a public/private key pair, the public key is 
//    applied to the content for encryption, and the private is applied for 
//    decryption.  If key is invalid or of an unknown type, NTE_BAD_KEY is
//    returned.  NTE_BAD_LEN is returned from Decrypt if the input buffer
//    does not have block aligned size.
//
// Notes:
//  - Uses memory allocation convention described above
//-----------------------------------------------------------------------------
// RSA:
//    Our implementation of RSA encodes blocks of content in the following 
//    format:
//       ---------------------------------------------------------------
//       | content = (cModLen - 2) bytes | block length = 2 bytes | ...
//       ---------------------------------------------------------------
//    The final block can be partial, in which case it is zero-filled.  Decrypt
//    will return CRYPT_E_BAD_ENCODE if the block length is found to be larger
//    than (cModLen - 2).  This usually happens because the encrypted content 
//    is being decoded with a private key that does not match the public key 
//    that encoded the content.
//-----------------------------------------------------------------------------
// DES:
//    DES performs encryption on 8-byte aligned blocks.  The last block can
//    be partial, in which case it is zero-filled.  The original size is
//    not recorded in any way, so Decrypt will require a buffer that is the
//    block-aligned size, with the final block zero-filled.
//-----------------------------------------------------------------------------
extern HRESULT UDEncrypt(CKeyBlob *pKey, UINT cIn, BYTE *pbIn, 
                         UINT *pcOut, BYTE *pbOut);

extern HRESULT UDDecrypt(CKeyBlob *pKey, UINT cIn, BYTE *pbIn, 
                         UINT *pcOut, BYTE *pbOut);

//-----------------------------------------------------------------------------
// Function: UDCreateDigest
//    Compute a cryptograpically strong hash of a block of content.  If a type 
//    is not supported, NTE_BAD_ALGID is returned.
// Notes:
//  - Uses memory allocation convention described above
//-----------------------------------------------------------------------------
#define uDIGEST_TYPE_SHA1 0x0000
extern HRESULT UDCreateDigest(UINT uType, UINT cContent, BYTE *pbContent, 
                              UINT *pcDigest, BYTE *pbDigest);

//-----------------------------------------------------------------------------
// Function: UDSign
//    Sign a block of data (typically a digest) using the private key of a 
//    public/private key pair.  NTE_BAD_KEY is returned for symmetric and
//    public keys.
// Notes:
//  - Uses memory allocation convention described above
//-----------------------------------------------------------------------------
extern HRESULT UDSign(CKeyBlob *pPrivKey, UINT cDigest, BYTE *pbDigest, 
                      UINT *pcSignature, BYTE *pbSignature);

//-----------------------------------------------------------------------------
// Function: UDVerifySignature
//    Check that the given signature is based on the given digest and the 
//    public key of a public/private key pair.  NTE_BAD_KEY is returned for 
//    symmetric and private keys.  S_OK is returned if the signatures match, 
//    NTE_BAD_SIGNATURE if they don't.
//-----------------------------------------------------------------------------
extern HRESULT UDVerifySignature(CKeyBlob *pPubKey,
                                 UINT cDigest, BYTE *pbDigest, 
                                 UINT cSignature, BYTE *pbSignature);

#endif // #ifndef __UDCRYPTOAPI_H_
