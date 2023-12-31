/*  
    Base definition of MIDI Transform Filter object 

    Copyright (c) Microsoft.  All rights reserved.
    



*/

#ifndef __MXF_H__
#define __MXF_H__

#include "private.h"

#define DMUS_KEF_EVENT_IN_USE       0x0004  //  This event is not presently in the allocator.
#define DMUS_KEF_EVENT_STATUS_STATE 0x0000  //  This event contains a running status byte only.
#define DMUS_KEF_EVENT_DATA1_STATE  0x4000  //  This event is a fragment that requires 1 more data byte.
#define DMUS_KEF_EVENT_DATA2_STATE  0x8000  //  This event is a fragment that requires 2 more data bytes.
#define DMUS_KEF_EVENT_SYSEX_STATE  0xC000  //  This event is a fragment that requires an EOX.

#define EVT_IN_USE(evt)         ((evt)->usFlags & DMUS_KEF_EVENT_IN_USE)
#define EVT_NOT_IN_USE(evt)     ((evt)->usFlags & DMUS_KEF_EVENT_IN_USE == 0)
#define EVT_PARSE_STATE(evt)   ((evt)->usFlags & DMUS_KEF_EVENT_SYSEX_STATE)
#define STATUS_STATE(evt)      (EVT_PARSE_STATE(evt) == DMUS_KEF_EVENT_STATUS_STATE)
#define DATA1_STATE(evt)       (EVT_PARSE_STATE(evt) == DMUS_KEF_EVENT_DATA1_STATE)
#define DATA2_STATE(evt)       (EVT_PARSE_STATE(evt) == DMUS_KEF_EVENT_DATA2_STATE)
#define SYSEX_STATE(evt)       (EVT_PARSE_STATE(evt) == DMUS_KEF_EVENT_SYSEX_STATE)
#define RUNNING_STATUS(evt)    (((evt)->cbEvent == 1) && (STATUS_STATE(evt)))

#define SET_EVT_IN_USE(evt)     ((evt)->usFlags |= DMUS_KEF_EVENT_IN_USE)
#define SET_EVT_NOT_IN_USE(evt) ((evt)->usFlags &= (~DMUS_KEF_EVENT_IN_USE))
#define SET_STATUS_STATE(evt) ((evt)->usFlags &= ~DMUS_KEF_EVENT_SYSEX_STATE)
#define SET_DATA1_STATE(evt)  ((evt)->usFlags = (((evt)->usFlags & ~DMUS_KEF_EVENT_SYSEX_STATE) | DMUS_KEF_EVENT_DATA1_STATE))
#define SET_DATA2_STATE(evt)  ((evt)->usFlags = (((evt)->usFlags & ~DMUS_KEF_EVENT_SYSEX_STATE) | DMUS_KEF_EVENT_DATA2_STATE))
#define SET_SYSEX_STATE(evt)  ((evt)->usFlags |= DMUS_KEF_EVENT_SYSEX_STATE)
#define CLEAR_STATE(evt)      (SET_STATUS_STATE(evt))


#if DBG
#define DumpDMKEvt(evt,lvl) \
    _DbgPrintF(lvl,("    PDMUS_KERNEL_EVENT:      0x%p",evt)); \
    if (evt) \
    { \
        if (evt->bReserved) \
        { \
            _DbgPrintF(lvl,("        bReserved:           0x%0.2X",evt->bReserved)); \
        } \
        else \
        { \
            _DbgPrintF(lvl,("        bReserved:           --")); \
        } \
        _DbgPrintF(lvl,("        cbStruct:            %d",evt->cbStruct)); \
        _DbgPrintF(lvl,("        cbEvent:             %d",evt->cbEvent)); \
        _DbgPrintF(lvl,("        usChannelGroup:      %d",evt->usChannelGroup)); \
        _DbgPrintF(lvl,("        usFlags:             0x%04X",evt->usFlags)); \
        _DbgPrintF(lvl,("        ullPresTime100ns:    0x%04X %04X  %04X %04X",WORD(evt->ullPresTime100ns >> 48),WORD(evt->ullPresTime100ns >> 32),WORD(evt->ullPresTime100ns >> 16),WORD(evt->ullPresTime100ns)));  \
        _DbgPrintF(lvl,("        ullBytePosition:     0x%04X %04X  %04X %04X",WORD(evt->ullBytePosition >> 48),WORD(evt->ullBytePosition >> 32),WORD(evt->ullBytePosition >> 16),WORD(evt->ullBytePosition)));  \
        if (evt->pNextEvt) \
        { \
            _DbgPrintF(lvl,("        pNextEvt:            0x%p",evt->pNextEvt)); \
        } \
        else \
        { \
            _DbgPrintF(lvl,("        pNextEvt:            --")); \
        } \
        if (PACKAGE_EVT(evt)) \
        { \
            _DbgPrintF(lvl,("        uData.pPackageEvt:   0x%p",evt->uData.pPackageEvt)); \
        } \
        else if (SHORT_EVT(evt)) \
        { \
            if (evt->cbEvent) \
            { \
                ULONGLONG data = 0; \
                for (int count = 0;count < evt->cbEvent;count++) \
                { \
                    data <<= 8; \
                    data += evt->uData.abData[count]; \
                } \
                _DbgPrintF(lvl,("        uData.abData:        0x%.*I64X",evt->cbEvent * 2,data)); \
            } \
            else \
            { \
                _DbgPrintF(lvl,("        uData.abData:        --")); \
            } \
        } \
        else \
        { \
            ULONGLONG   data; \
            ULONG       count; \
            PBYTE       dataPtr = evt->uData.pbData; \
            _DbgPrintF(lvl,("        uData.pbData:        0x%p",dataPtr)); \
            for (data = 0,count = 0;(count < sizeof(PBYTE)) && (count < evt->cbEvent);count++,dataPtr++) \
            { \
                data <<= 8; \
                data += *dataPtr; \
            } \
            _DbgPrintF(lvl,("            (uData.pbData):  0x%.*I64X",count * 2,data)); \
            while (count < evt->cbEvent) \
            { \
                int localCount; \
                for (localCount = 0, data = 0; \
                     (localCount < sizeof(PBYTE)) && (count < evt->cbEvent); \
                     localCount++,count++,dataPtr++) \
                { \
                    data <<= 8; \
                    data += *dataPtr; \
                } \
                _DbgPrintF(lvl,("                             0x%.*I64X",localCount * 2,data)); \
            } \
        } \
    }
#else
#define DumpDMKEvt(evt,level)
#endif

class CAllocatorMXF;

class CMXF
{
public:
    CMXF(CAllocatorMXF *AllocatorMXF) { m_AllocatorMXF = AllocatorMXF;};
    virtual ~CMXF(void) {};

    IMP_IMXF;

protected:
    CAllocatorMXF *m_AllocatorMXF;
};

#endif  //  __MXF_H__
