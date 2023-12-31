//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft shared
// source or premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license agreement,
// you are not authorized to use this source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the SOURCE.RTF on your install media or the root of your tools installation.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
#include <kernel.h>

void KernelFindMemory (void);
void Reschedule (void);
void SafeIdentifyCpu (void);
void KernelInit (void);

//
// x86 specfic NK Globals
//
ULONGLONG   *g_pGDT;
PADDRMAP    g_pOEMAddressTable;
DWORD       g_dwRAMOfstVA2PA = 0x80000000;



#pragma pack(push, 1)           // We want this structure packed exactly as declared!

typedef struct {
    BYTE        PushEax;
    BYTE        Pushad;
    BYTE        MovEsi;
    PFNVOID     pfnHandler;
    BYTE        JmpNear;
    DWORD       JmpOffset;
    DWORD       Padding;
} INTHOOKSTUB, * PINTHOOKSTUB;

INTHOOKSTUB g_aIntHookStubs[32] = {0};

KIDTENTRY g_aIntDescTable[MAXIMUM_IDTVECTOR+1];

const FWORDPtr g_IDTBase = {sizeof(g_aIntDescTable), &g_aIntDescTable };

#pragma pack(pop)

KTSS MainTSS;
KTSS DoubleTSS;
KTSS NMITSS;

BYTE TASKStack[0x800];      // task stack. proabaly don't need to be this big, but allocate 2k to be safe

char SyscallStack[128];     // temporary stack for syscall trap

void Int1Fault(void);
void Int2Fault(void);
void Int3Fault(void);
void GeneralFault(void);
void PageFault(void);
void InvalidOpcode(void);
void ZeroDivide(void);
void CommonIntDispatch(void);
void Int22KCallHandler(void);
PFNVOID Int20SyscallHandler(void);



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
DumpTSS(
    KTSS *ptss
    ) 
{
    NKDbgPrintfW(L"TSS=%8.8lx EIP=%8.8lx Flags=%8.8lx\r\n",
            ptss, ptss->Eip, ptss->Eflags);
    NKDbgPrintfW(L"Eax=%8.8lx Ebx=%8.8lx Ecx=%8.8lx Edx=%8.8lx\r\n",
            ptss->Eax, ptss->Ebx, ptss->Ecx, ptss->Edx);
    NKDbgPrintfW(L"Esi=%8.8lx Edi=%8.8lx Ebp=%8.8lx Esp=%8.8lx\r\n",
            ptss->Esi, ptss->Edi, ptss->Ebp, ptss->Esp);
    NKDbgPrintfW(L"CS=%4.4lx DS=%4.4lx ES=%4.4lx SS=%4.4lx FS=%4.4lx GS=%4.4lx\r\n",
            ptss->Cs, ptss->Ds, ptss->Es, ptss->Ss, ptss->Fs, ptss->Gs);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
NonMaskableInterrupt(void) 
{
    g_pKData->cNest = -1;
    NKDbgPrintfW(L"\r\nNMI -- backlink=%4.4x\r\n", NMITSS.Backlink);
    DumpTSS(&MainTSS);
    OEMNMIHandler();
    __asm {
        cli
        hlt
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
DoubleFault(void) 
{
    g_pKData->cNest = -1;
    NKDbgPrintfW(L"\r\nDouble Fault -- backlink=%4.4x\r\n", DoubleTSS.Backlink);
    DumpTSS(&MainTSS);
    __asm {
        cli
        hlt
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
InitTSSSelector(
    ULONG uSelector,
    PKTSS pTSS
    ) 
{
    PKGDTENTRY pTSSSel = (PKGDTENTRY)(((ULONG)&g_pGDT[0]) + uSelector);
    pTSSSel->LimitLow = sizeof(KTSS)-1;
    pTSSSel->BaseLow = (USHORT)((DWORD)pTSS) & 0xFFFF;
    pTSSSel->HighWord.Bytes.BaseMid = (UCHAR)(((DWORD)pTSS) >> 16) & 0xFF;
    pTSSSel->HighWord.Bytes.BaseHi = (UCHAR)(((DWORD)pTSS) >> 24) & 0xFF;
    pTSSSel->HighWord.Bits.Type = TYPE_TSS;
    pTSSSel->HighWord.Bits.Pres = 1;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
InitializeTaskStates(void) 
{
    ulong pageDir;
    __asm {
        mov eax, CR3
        mov pageDir, eax
    }
    InitTSSSelector(KGDT_MAIN_TSS, &MainTSS);

    MainTSS.Esp0 = 0xB00B00;
    MainTSS.Ss0 = KGDT_R0_DATA;
    MainTSS.Esp1 = (ulong)(SyscallStack+sizeof(SyscallStack));
    MainTSS.Ss1 = KGDT_R1_DATA | 1;

    InitTSSSelector(KGDT_NMI_TSS, &NMITSS);
    NMITSS.Eip = (ULONG)&NonMaskableInterrupt;
    NMITSS.Cs = KGDT_R0_CODE;
    NMITSS.Ss = KGDT_R0_DATA;
    NMITSS.Ds = KGDT_R0_DATA;
    NMITSS.Es = KGDT_R0_DATA;
    NMITSS.Ss0 = KGDT_R0_DATA;
    NMITSS.Esp = NMITSS.Esp0 = ((ULONG)&TASKStack) + sizeof(TASKStack);
    NMITSS.CR3 = pageDir;

    InitTSSSelector(KGDT_DOUBLE_TSS, &DoubleTSS);
    DoubleTSS.Eip = (ULONG)&DoubleFault;
    DoubleTSS.Cs = KGDT_R0_CODE;
    DoubleTSS.Ss = KGDT_R0_DATA;
    DoubleTSS.Ds = KGDT_R0_DATA;
    DoubleTSS.Es = KGDT_R0_DATA;
    DoubleTSS.Ss0 = KGDT_R0_DATA;
    DoubleTSS.Esp = DoubleTSS.Esp0 = ((ULONG)&TASKStack) + sizeof(TASKStack);
    DoubleTSS.CR3 = pageDir;

    {
        // Change the KGDT_EMX87 entry in the global descriptor table to be Ring 3 conforming
        PKGDTENTRY pTSSSel = (PKGDTENTRY)(((ULONG)&g_pGDT[0]) + KGDT_EMX87);
        pTSSSel->HighWord.Bits.Dpl = 3;
    }
    _asm {
        mov     eax, KGDT_MAIN_TSS
        ltr     ax
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
InitIDTEntry(
    int i,
    USHORT usSelector,
    PVOID pFaultHandler,
    USHORT usGateType
    ) 
{
    PKIDTENTRY pCur = &g_aIntDescTable[i];
    pCur->Offset = LOWORD((DWORD)pFaultHandler);
    pCur->ExtendedOffset = HIWORD((DWORD)pFaultHandler);
    pCur->Selector = usSelector;
    pCur->Access = usGateType;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void 
InitializeIDT(void) 
{
    InitIDTEntry(0x00, KGDT_R0_CODE, ZeroDivide, INTERRUPT_GATE);
    InitIDTEntry(0x01, KGDT_R0_CODE, Int1Fault, RING3_INT_GATE);
    InitIDTEntry(0x02, KGDT_NMI_TSS, 0, TASK_GATE);
    InitIDTEntry(0x02, KGDT_R0_CODE, Int2Fault, INTERRUPT_GATE);
    InitIDTEntry(0x03, KGDT_R0_CODE, Int3Fault, RING3_INT_GATE);
    InitIDTEntry(0x06, KGDT_R0_CODE, InvalidOpcode, INTERRUPT_GATE);
    InitIDTEntry(0x08, KGDT_DOUBLE_TSS, 0, TASK_GATE);
    InitIDTEntry(0x0D, KGDT_R0_CODE, GeneralFault, INTERRUPT_GATE);
    InitIDTEntry(0x0E, KGDT_R0_CODE, PageFault, INTERRUPT_GATE);
    _asm { lidt [g_IDTBase] };

    InitIDTEntry(SYSCALL_INT, KGDT_R1_CODE | 1, Int20SyscallHandler(), RING3_INT_GATE);
    InitIDTEntry(KCALL_INT, KGDT_R0_CODE, Int22KCallHandler, RING1_INT_GATE);


}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL 
HookInterrupt(
    int hwInterruptNumber,
    FARPROC pfnHandler
    ) 
{
    if ((DWORD) hwInterruptNumber < ARRAY_SIZE(g_aIntDescTable)) {
        PKIDTENTRY pIDTEntry = &g_aIntDescTable[hwInterruptNumber];
        if (pIDTEntry->Offset == 0 && pIDTEntry->Selector == 0) {
            PINTHOOKSTUB pStub = &g_aIntHookStubs[0];
            for (pStub = &g_aIntHookStubs[0]; pStub <= LAST_ELEMENT(g_aIntHookStubs); pStub++) {
                if (pStub->pfnHandler == NULL) {
                    pStub->PushEax   = 0x50;
                    pStub->Pushad    = 0x60;
                    pStub->MovEsi    = 0xBE;
                    pStub->pfnHandler= pfnHandler;
                    pStub->JmpNear   = 0xE9;
                    pStub->JmpOffset = ((DWORD)&CommonIntDispatch) - ((DWORD)(&pStub->JmpOffset) + 4);
                    InitIDTEntry(hwInterruptNumber, KGDT_R0_CODE, pStub, INTERRUPT_GATE);
                    return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL 
UnhookInterrupt(
    int hwInterruptNumber,
    FARPROC pfnHandler
    ) 
{
    if ((DWORD) hwInterruptNumber < ARRAY_SIZE(g_aIntDescTable)) {
        PKIDTENTRY pIDTEntry = &g_aIntDescTable[hwInterruptNumber];
        PINTHOOKSTUB pStub = (PINTHOOKSTUB)(pIDTEntry->Offset + (pIDTEntry->ExtendedOffset << 16));
        if (pStub >= &g_aIntHookStubs[0] && pStub <= LAST_ELEMENT(g_aIntHookStubs)) {
            InitIDTEntry(hwInterruptNumber, 0, 0, 0);
            pStub->pfnHandler = NULL;
            return(TRUE);
        }
    }
    return(FALSE);
}

static int FindRAMEntry (PADDRMAP pOEMAddressTable)
{
    int i;
    DWORD dwRAMAddr = pTOC->ulRAMFree;
    for (i = 0; pOEMAddressTable[i].dwSize; i ++) {
        if ((dwRAMAddr >= pOEMAddressTable[i].dwVA)
            && (dwRAMAddr < (pOEMAddressTable[i].dwVA + pOEMAddressTable[i].dwSize))) {
            g_dwRAMOfstVA2PA = pOEMAddressTable[i].dwPA - pOEMAddressTable[i].dwVA;
            break;
        }
    }
    return i;
}

void NKStartup (struct KDataStruct *pKData)
{
    PFN_OEMInitGlobals pfnInitGlob;
    PFN_DllMain pfnKitlEntry;
    
    // pickup arguments from the nk loader
    g_pKData   = pKData;
    g_pGDT     = pKData->pGDT;
    pTOC       = (const ROMHDR *) pKData->dwTOCAddr;

    // initialize nk globals
    FirstROM.pTOC = (ROMHDR *) pTOC;
    FirstROM.pNext = 0;
    ROMChain = &FirstROM;
    KInfoTable[KINX_PTOC] = (long)pTOC;

    // o Page directory at pTOC->ulRamFree
    g_ppdirNK = (PPAGEDIRECTORY) pTOC->ulRAMFree;
    g_pOEMAddressTable = pKData->pAddrMap;
    g_pKData->pNk  = g_pNKGlobal;

    // calculate g_dwRAMOfstVA2PA 
    FindRAMEntry (g_pOEMAddressTable);

    pfnInitGlob = (PFN_OEMInitGlobals) pKData->dwOEMInitGlobalsAddr;

    // no checking here, if OAL entry point doesn't exist, we can't continue
    g_pOemGlobal = pfnInitGlob (g_pNKGlobal);
    g_pKData->pOem = g_pOemGlobal;
    g_pOemGlobal->dwMainMemoryEndAddress = pTOC->ulRAMEnd;
    
    // setup globals
    g_pprcNK->ppdir = g_ppdirNK;
    pVMProc         = g_pprcNK;
    pActvProc       = g_pprcNK;
    g_pprcNK->vaFree = VM_NKVM_BASE;    

    // faked current thread id so ECS won't put 0 in OwnerThread
    dwCurThId = 2;

    // initialize Process information
    CEProcessorType = PROCESSOR_INTEL_486;
    CEInstructionSet = PROCESSOR_X86_32BIT_INSTRUCTION;

    g_pNKGlobal->pfnWriteDebugString = g_pOemGlobal->pfnWriteDebugString;

    // try to load KITL if exist
    if ((pfnKitlEntry = (PFN_DllMain) g_pOemGlobal->pfnKITLGlobalInit) ||
        (pfnKitlEntry = (PFN_DllMain) FindROMDllEntry (pTOC, KITLDLL))) {
        (* pfnKitlEntry) (NULL, DLL_PROCESS_ATTACH, (DWORD) NKKernelLibIoControl);
    }

#ifdef DEBUG
    CurMSec = dwPrevReschedTime = (DWORD) -200000;      // ~3 minutes before wrap
#endif

    OEMInitDebugSerial ();

    NKDbgPrintfW(L"\r\nSysInit: GDTBase=%8.8lx IDTBase=%8.8lx KData=%8.8lx\r\n", g_pGDT, &g_aIntDescTable, g_pKData);

    // Initialize the interrupt dispatch tables
    InitializeTaskStates();
    InitializeIDT();

    OEMWriteDebugString (TEXT("Windows CE Kernel for i486 Built on ") TEXT(__DATE__) TEXT(" at ") TEXT(__TIME__) TEXT("\r\n"));

    OEMInit ();

    KernelFindMemory();
    SafeIdentifyCpu ();

    NKDbgPrintfW (TEXT("X86Init done, OEMAddressTable = %8.8lx, RAM mapped = %8.8lx.\r\n"),
        g_pOEMAddressTable, g_pOEMAddressTable[0].dwSize);
    KernelInit ();

    // initialization done. schedule the 1st thread
    __asm {
        xor edi, edi
        mov ebx, dword ptr [g_pKData]
        jmp Reschedule
    }
}

