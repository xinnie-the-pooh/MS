 /***************************************************************************\
|*                                                                           *|
|*        Copyright (c) 1993-2000 NVIDIA, Corp.  All rights reserved.        *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.   NVIDIA, Corp. of Sunnyvale, California owns     *|
|*     the copyright  and as design patents  pending  on the design  and     *|
|*     interface  of the NV chips.   Users and possessors of this source     *|
|*     code are hereby granted  a nonexclusive,  royalty-free  copyright     *|
|*     and  design  patent license  to use this code  in individual  and     *|
|*     commercial software.                                                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*     Copyright (c) 1993-2000  NVIDIA, Corp.    NVIDIA  design  patents     *|
|*     pending in the U.S. and foreign countries.                            *|
|*                                                                           *|
|*     NVIDIA, CORP.  MAKES  NO REPRESENTATION ABOUT  THE SUITABILITY OF     *|
|*     THIS SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT     *|
|*     EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORP. DISCLAIMS     *|
|*     ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,  INCLUDING  ALL     *|
|*     IMPLIED   WARRANTIES  OF  MERCHANTABILITY  AND   FITNESS   FOR  A     *|
|*     PARTICULAR  PURPOSE.   IN NO EVENT SHALL NVIDIA, CORP.  BE LIABLE     *|
|*     FOR ANY SPECIAL, INDIRECT, INCIDENTAL,  OR CONSEQUENTIAL DAMAGES,     *|
|*     OR ANY DAMAGES  WHATSOEVER  RESULTING  FROM LOSS OF USE,  DATA OR     *|
|*     PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR OTHER     *|
|*     TORTIOUS ACTION, ARISING OUT  OF OR IN CONNECTION WITH THE USE OR     *|
|*     PERFORMANCE OF THIS SOURCE CODE.                                      *|
|*                                                                           *|
 \***************************************************************************/

/********************* NV4_INDEXED_IMAGE_FROM_CPU **************************\
*                                                                           *
* Module: CLASS060.C                                                        *
*   This module implements the NV4_INDEXED_IMAGE_FROM_CPU object class      *
*   and its corresponding methods.                                          *
*                                                                           *
*****************************************************************************
*                                                                           *
* History:                                                                  *
*                                                                           *
\***************************************************************************/
#include <nv_ref.h>
#include <nvrm.h>
#include <state.h>
#include <class.h>
#include <gr.h>   
#include <dma.h>   
#include <modular.h>
#include <fifo.h>
#include <os.h>
#include <nv32.h>
#include "smp.h"
#include "nvhw.h"

//
// Externs.
//


//---------------------------------------------------------------------------
//
//  Create/Destroy object routines.
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//
//  Miscellaneous class support routines.
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//
//  Class validation routines.
//
//---------------------------------------------------------------------------

RM_STATUS class060Validate
(
	PHWINFO             pDev,
    PRENDERCOMMONOBJECT pRenderObject,
    U032                andMask,
    U032                orMask
)
{
    RM_STATUS status = RM_OK;

    //
    // Validate the current configuration of this object.  Update the object
    // instance data structure accordingly.
    //
    if ((pRenderObject->RCPatchContext.Operation != NV_ROP_AND  ||  
          ((pRenderObject->RCPatchContext.Surface != NULL) && 
           (pRenderObject->RCPatchContext.Surface->Base.Class->Type != NV4_CONTEXT_SURFACE_SWIZZLED)))  &&
        (pRenderObject->RCPatchContext.Operation != NV_ROP_AND  ||  
                    pRenderObject->RCPatchContext.Pattern != NULL)  &&
	    (pRenderObject->RCPatchContext.Operation != NV_ROP_AND  ||  
                    pRenderObject->RCPatchContext.Rop != NULL)  &&
	    (pRenderObject->RCPatchContext.Operation != NV_BLEND_AND  ||  
                    pRenderObject->RCPatchContext.Beta1 != NULL)  &&
	    (pRenderObject->RCPatchContext.Operation != NV_SRCCOPY_PREMULT  ||  
                    pRenderObject->RCPatchContext.Beta4 != NULL)  &&
	    (pRenderObject->RCPatchContext.Operation != NV_BLEND_PREMULT  ||  
                    pRenderObject->RCPatchContext.Beta4 != NULL)  &&
	    (pRenderObject->RCPatchContext.Surface != NULL))
    {
        //
        // Patch status is valid
        //
        nvHalGrSetObjectContext(pDev,
                                ~DRF_DEF(_PGRAPH, _CTX_SWITCH1, _PATCH_STATUS, _VALID) & andMask,
                                DRF_DEF(_PGRAPH, _CTX_SWITCH1, _PATCH_STATUS, _VALID) | orMask,
                                pRenderObject->Common.Base.ChID,
                                pRenderObject->Common.Instance);
        pRenderObject->CValid = TRUE;
    } else {
        //
        // Patch status is invalid
        //
        nvHalGrSetObjectContext(pDev,
                                ~DRF_DEF(_PGRAPH, _CTX_SWITCH1, _PATCH_STATUS, _VALID) & andMask,
                                orMask,
                                pRenderObject->Common.Base.ChID,
                                pRenderObject->Common.Instance);
        pRenderObject->CValid = FALSE;
    }

    return (status);
}


//---------------------------------------------------------------------------
//
//  Class method routines.
//
//---------------------------------------------------------------------------

//
//  NOTE: Any method not listed here is most likely aliased to HWMethod()
//

RM_STATUS class060SetContextColorKey
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    POBJECT                 TempObject;
    PRENDERCOMMONOBJECT     pRenderObject;
    U032                    andMask, orMask;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetContextColorKey\r\n");

    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // We'll be updating this bit.
    //
    andMask = ~DRF_DEF(_PGRAPH, _CTX_SWITCH1, _CHROMA_KEY, _ENABLE);

    //
    // Verify the incoming data is valid for this method
    //
    if (Data == NV1_NULL_OBJECT) 
    {
        //
        // Set to the NULL object and disable the colorkey.
        //
        orMask = 0;
        pRenderObject->RCPatchContext.ColorKey = (PCOMMONOBJECT)NULL;
        
    }
    else if (fifoSearchObject(pDev, Data, pRenderObject->CBase.ChID, &TempObject) == RM_OK)
    {
        //
        // There is a corresponding object in this channel.  Validate.
        //
        switch (TempObject->Class->Type)
        {
            case NV4_CONTEXT_COLOR_KEY:
                //
                // This is a valid colorkey object
                //
                orMask = DRF_DEF(_PGRAPH, _CTX_SWITCH1, _CHROMA_KEY, _ENABLE);
                pRenderObject->RCPatchContext.ColorKey = (PCOMMONOBJECT)TempObject;
                break;
            case NV1_NULL:
            case NV1_NULL_OBJECT:
                //
                // A NULL class should disable
                //
                orMask = 0;
                pRenderObject->RCPatchContext.ColorKey = (PCOMMONOBJECT)NULL;
                break;
                
            default:
                //
                // Any other class is an error
                //
                DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
                return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT); 
                break;
        }
    }
    else
    {
        //
        // Object not found or data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }
    
    //
    // Update object validation
    //
    status = class060Validate(pDev, pRenderObject, andMask, orMask);
    
    return (status);
}


RM_STATUS class060SetContextClip
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    POBJECT                 TempObject;
    PRENDERCOMMONOBJECT     pRenderObject;
    U032                    andMask, orMask;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetContextClip\r\n");

    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // We'll be updating this bit.
    //
    andMask = ~DRF_DEF(_PGRAPH, _CTX_SWITCH1, _USER_CLIP, _ENABLE);
    //
    // Verify the incoming data is valid for this method
    //
    if (Data == NV1_NULL_OBJECT) 
    {
        //
        // Set to the NULL object and disable the colorkey.
        //
        orMask = 0;
        pRenderObject->RCPatchContext.Clip = (PCOMMONOBJECT)NULL;
    }
    else if (fifoSearchObject(pDev, Data, pRenderObject->CBase.ChID, &TempObject) == RM_OK)
    {
        //
        // There is a corresponding object in this channel.  Validate.
        //
        switch (TempObject->Class->Type)
        {
            case NV1_IMAGE_BLACK_RECTANGLE:
                //
                // This is a valid colorkey object
                //
                orMask = DRF_DEF(_PGRAPH, _CTX_SWITCH1, _USER_CLIP, _ENABLE);
                pRenderObject->RCPatchContext.Clip = (PCOMMONOBJECT)TempObject;
                break;
            
            case NV1_NULL:
            case NV1_NULL_OBJECT:
                //
                // A NULL class should disable
                //
                orMask = 0;
                pRenderObject->RCPatchContext.Clip = (PCOMMONOBJECT)NULL;
                break;
                
            default:
                //
                // Any other class is an error
                //
                DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
                return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT); 
                break;
        }
    }
    else
    {
        //
        // Object not found or data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }
    
    //
    // Update object validation
    //
    status = class060Validate(pDev, pRenderObject, andMask, orMask);
    
    return (status);
}


RM_STATUS class060SetContextPattern
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    POBJECT                 TempObject;
    PRENDERCOMMONOBJECT     pRenderObject;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetContextPattern\r\n");

    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // Verify the incoming data is valid for this method
    //
    if (Data == NV1_NULL_OBJECT) 
    {
        //
        // Set to the NULL object
        //
        pRenderObject->RCPatchContext.Pattern = (PCOMMONOBJECT)NULL;
        
    }
    else if (fifoSearchObject(pDev, Data, pRenderObject->CBase.ChID, &TempObject) == RM_OK)
    {
        //
        // There is a corresponding object in this channel.  Validate.
        //
        switch (TempObject->Class->Type)
        {
            case NV4_CONTEXT_PATTERN:
                //
                // This is a valid object
                //            
                pRenderObject->RCPatchContext.Pattern = (PCOMMONOBJECT)TempObject;
                break;
            
            case NV1_NULL:
            case NV1_NULL_OBJECT:
                //
                // A NULL class should disable
                //
                pRenderObject->RCPatchContext.Pattern = (PCOMMONOBJECT)NULL;
                break;
                
            default:
                //
                // Any other class is an error
                //
                DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
                return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT); 
                break;
        }
    }
    else
    {
        //
        // Object not found or data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }
    
    //
    // Update object validation
    //
    status = class060Validate(pDev, pRenderObject, 0xffffffff, 0);
    
    return (status);
}


RM_STATUS class060SetContextRop
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    POBJECT                 TempObject;
    PRENDERCOMMONOBJECT     pRenderObject;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetContextRop\r\n");

    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // Verify the incoming data is valid for this method
    //
    if (Data == NV1_NULL_OBJECT) 
    {
        //
        // Set to the NULL object
        //
        pRenderObject->RCPatchContext.Rop = (PCOMMONOBJECT)NULL;
        
    }
    else if (fifoSearchObject(pDev, Data, pRenderObject->CBase.ChID, &TempObject) == RM_OK)
    {
        //
        // There is a corresponding object in this channel.  Validate.
        //
        switch (TempObject->Class->Type)
        {
            case NV3_CONTEXT_ROP:
                //
                // This is a valid object
                //            
                pRenderObject->RCPatchContext.Rop = (PCOMMONOBJECT)TempObject;
                break;
            
            case NV1_NULL:
            case NV1_NULL_OBJECT:
                //
                // A NULL class should disable
                //
                pRenderObject->RCPatchContext.Rop = (PCOMMONOBJECT)NULL;
                break;
                
            default:
                //
                // Any other class is an error
                //
                DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
                return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT); 
                break;
        }
    }
    else
    {
        //
        // Object not found or data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }
    
    //
    // Update object validation
    //
    status = class060Validate(pDev, pRenderObject, 0xffffffff, 0);
    
    return (status);
}


RM_STATUS class060SetContextBeta1
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    POBJECT                 TempObject;
    PRENDERCOMMONOBJECT     pRenderObject;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetContextBeta1\r\n");

    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // Verify the incoming data is valid for this method
    //
    if (Data == NV1_NULL_OBJECT) 
    {
        //
        // Set to the NULL object
        //
        pRenderObject->RCPatchContext.Beta1 = (PCOMMONOBJECT)NULL;
        
    }
    else if (fifoSearchObject(pDev, Data, pRenderObject->CBase.ChID, &TempObject) == RM_OK)
    {
        //
        // There is a corresponding object in this channel.  Validate.
        //
        switch (TempObject->Class->Type)
        {
            case NV1_BETA_SOLID:
                //
                // This is a valid object
                //            
                pRenderObject->RCPatchContext.Beta1 = (PCOMMONOBJECT)TempObject;
                break;
            
            case NV1_NULL:
            case NV1_NULL_OBJECT:
                //
                // A NULL class should disable
                //
                pRenderObject->RCPatchContext.Beta1 = (PCOMMONOBJECT)NULL;
                break;
                
            default:
                //
                // Any other class is an error
                //
                DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
                return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT); 
                break;
        }
    }
    else
    {
        //
        // Object not found or data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }
    
    //
    // Update object validation
    //
    status = class060Validate(pDev, pRenderObject, 0xffffffff, 0);
    
    return (status);
}


RM_STATUS class060SetContextBeta4
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    POBJECT                 TempObject;
    PRENDERCOMMONOBJECT     pRenderObject;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetContextBeta4\r\n");

    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // Verify the incoming data is valid for this method
    //
    if (Data == NV1_NULL_OBJECT) 
    {
        //
        // Set to the NULL object
        //
        pRenderObject->RCPatchContext.Beta4 = (PCOMMONOBJECT)NULL;
        
    }
    else if (fifoSearchObject(pDev, Data, pRenderObject->CBase.ChID, &TempObject) == RM_OK)
    {
        //
        // There is a corresponding object in this channel.  Validate.
        //
        switch (TempObject->Class->Type)
        {
            case NV4_CONTEXT_BETA:
                //
                // This is a valid object
                //            
                pRenderObject->RCPatchContext.Beta4 = (PCOMMONOBJECT)TempObject;
                break;
            
            case NV1_NULL:
            case NV1_NULL_OBJECT:
                //
                // A NULL class should disable
                //
                pRenderObject->RCPatchContext.Beta4 = (PCOMMONOBJECT)NULL;
                break;
                
            default:
                //
                // Any other class is an error
                //
                DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
                return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT); 
                break;
        }
    }
    else
    {
        //
        // Object not found or data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }
    
    //
    // Update object validation
    //
    status = class060Validate(pDev, pRenderObject, 0xffffffff, 0);
    
    return (status);
}


RM_STATUS class060SetContextSurface
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    POBJECT                 TempObject;
    PRENDERCOMMONOBJECT     pRenderObject;
    U032                    andMask, orMask;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetContextSurface\r\n");

    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // We'll be updating this bit.
    //
    andMask = ~DRF_DEF(_PGRAPH, _CTX_SWITCH1, _SWIZZLE, _ENABLE);

    //
    // Verify the incoming data is valid for this method
    //
    if (Data == NV1_NULL_OBJECT) 
    {
        //
        // Set to the NULL object
        //
        orMask = 0;
        pRenderObject->RCPatchContext.Surface = (PCOMMONOBJECT)NULL;
    }
    else if (fifoSearchObject(pDev, Data, pRenderObject->CBase.ChID, &TempObject) == RM_OK)
    {
        //
        // There is a corresponding object in this channel.  Validate.
        //
        switch (TempObject->Class->Type)
        {
            case NV4_CONTEXT_SURFACES_2D:
                //
                // This is a valid object
                //
                orMask = 0;
                pRenderObject->RCPatchContext.Surface = (PCOMMONOBJECT)TempObject;
                break;
            
	        case NV4_CONTEXT_SURFACE_SWIZZLED:
                //
                // This is a valid swizzled object
                //
                orMask = DRF_DEF(_PGRAPH, _CTX_SWITCH1, _SWIZZLE, _ENABLE);
                pRenderObject->RCPatchContext.Surface = (PCOMMONOBJECT)TempObject;
                break;
            
            case NV1_NULL:
            case NV1_NULL_OBJECT:
                //
                // A NULL class should disable
                //
                orMask = 0;
                pRenderObject->RCPatchContext.Surface = (PCOMMONOBJECT)NULL;
                break;
                
            default:
                //
                // Any other class is an error
                //
                DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
                return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT); 
                break;
        }
    }
    else
    {
        //
        // Object not found or data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid object: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }
    
    //
    // Update object validation
    //
    status = class060Validate(pDev, pRenderObject, andMask, orMask);
    
    return (status);
}


RM_STATUS class060SetOperation
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    PRENDERCOMMONOBJECT     pRenderObject;
    U032                    andMask, orMask;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetOperation\r\n");
    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // Verify the incoming data is valid for this method
    //
    if (Data < NV4_GR_OPERATIONS) 
    {
        //
        // Set the new value
        //
        andMask = ~(DRF_MASK(NV_PGRAPH_CTX_SWITCH1_PATCH_CONFIG) << DRF_SHIFT(NV_PGRAPH_CTX_SWITCH1_PATCH_CONFIG));
        orMask = DRF_NUM(_PGRAPH, _CTX_SWITCH1, _PATCH_CONFIG, Data);
        pRenderObject->RCPatchContext.Operation = Data;     // save the arch value for validation
    }
    else
    {
        //
        // Data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetContext with invalid data: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }
    
    //
    // Update object validation
    //
    status = class060Validate(pDev, pRenderObject, andMask, orMask);
    
    return (status);
}

RM_STATUS class060SetColorConversion
(
	PHWINFO pDev,
    POBJECT Object,
    PMETHOD Method,
    U032    Offset,
    U032    Data
)
{
    PRENDERCOMMONOBJECT     pRenderObject;
    RM_STATUS               status = RM_OK;

    DBG_PRINT_STRING(DEBUGLEVEL_TRACEINFO, "NVRM: class060SetColorConversion\r\n");

    //
    // If we're an NV10, kick it back as an error, since POSTDITHER_2D lives
    // in PGRAPH_DEBUG_2 there (and NV5_INDEXED_IMAGE should be used anyway).
    //
    if (IsNV10orBetter(pDev))
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);

    //
    // A nice cast to make the code more readable.
    //
    pRenderObject = (PRENDERCOMMONOBJECT)Object;

    //
    // Set the dither mode.
    //
    // Since this comes in as a synchronous graphics exception, we can
    // assume we own the channel and can change the dither mode.
    //
    // The HAL graphics exception handler will be sure that we modify the
    // hardware, so we just handle error checking here.
    // 
    if ((Data != NV064_SET_COLOR_CONVERSION_DITHER) &&
        (Data != NV064_SET_COLOR_CONVERSION_TRUNCATE))
    {
        //
        // Data invalid.
        //
        DBG_PRINT_STRING_VALUE(DEBUGLEVEL_ERRORS, "NVRM: SetColorConversion with invalid data: ", Data);
        return (NV060_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT);
    }

    return (status);
}
