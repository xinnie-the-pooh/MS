;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft shared
; source or premium shared source license agreement under which you licensed
; this source code. If you did not accept the terms of the license agreement,
; you are not authorized to use this source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the SOURCE.RTF on your install media or the root of your tools installation.
; THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;
    INCLUDE kxarm.h

; API call defines: cloned from psyscall.h

FIRST_METHOD                EQU     0xF1020000
MAX_METHOD_NUM              EQU     0xF1020400      ; exactly value is 0xF10203FC. 
SYSCALL_RETURN              EQU     FIRST_METHOD-4

METHOD_MASK                 EQU     0x00FF
APISET_MASK                 EQU     0x007F
APISET_SHIFT                EQU     8

NUM_SYS_HANDLES             EQU     32
SYS_HANDLE_BASE             EQU     64
SH_WIN32                    EQU     0
SH_CURTHREAD                EQU     1
SH_CURPROC                  EQU     2

;
; The architecture version is read to decide whether to use the
; new page table format provided in V6 onwards.

; -- ARM Architectures Versions
;
; CP15:r0:bits[19:16]=architecture version
;
ARMv4T                      EQU     0x2
ARMv6                       EQU     0x7

;
; section mapped defs
;
BANK_SHIFT                  EQU     20      ; each section = 1M


; Page table bits
; Cacheable/Bufferable are always at the same bit position
CACHEABLE                   EQU     8
BUFFERABLE                  EQU     4

; Basic page table entry types
PTL1_2Y_TABLE               EQU     1       ; top-level entry points at second-level table
PTL1_SECTION                EQU     2       ; top-level entry maps a 1MB section directly
PTL1_XN                     EQU     0       ; No execution control on pre-V6

PTL2_LARGE_PAGE             EQU     1
PTL2_SMALL_PAGE             EQU     2

; Common access attribute settings
PTL1_KRW                    EQU     0x400   ; bits 10, 11
PTL1_KRW_URO                EQU     0x800
PTL1_RW                     EQU     0xc00
PTL2_KRW                    EQU     0x10
PTL2_KRW_URO                EQU     0x20
PTL2_RW                     EQU     0x30

; Values to use for V6 format tables
; eXecute-Never bits.
ARMV6_MMU_PTL1_XN           EQU     1 <<  4
ARMV6_MMU_PTL2_LARGE_XN     EQU     1 << 15
ARMV6_MMU_PTL2_SMALL_XN     EQU     1 <<  0

; V6 ro settings are explicit rather than using the CP15 R1 R bit.
; Note that these are more restrictive than the non-V6 settings: they deny user-mode access.
ARMV6_MMU_PTL1_KR0          EQU     0x8400  ; bits 11,10=01, APX(b15)=1
ARMV6_MMU_PTL2_KR0          EQU     0x210   ; bits  5, 4=01, APX(b9 )=1

ARMV6_MMU_ASID_ADDR_SHIFT   EQU     25      ; there are 128 slots covering the 32-bit address space.
                                            ; Therefore shift by 32-7=25 to convert between
                                            ; (simple slot number) ASID and process base address

; Values to use for old format page tables
PREARMV6_MMU_PTL1_XN        EQU     0       ; No execution control on pre-V6
PREARMV6_MMU_PTL2_SMALL_XN  EQU     0
PREARMV6_MMU_PTL1_KR0       EQU     0x000   ; bits 10,11=00 : R bit set in CP15
PREARMV6_MMU_PTL2_KR0       EQU     0x00    ; bits  5, 4=00 : R bit set in CP15

;
; V6 specific bits in control register
;
ARMV6_A_BIT                 EQU     1 << 1  ; bit 1 is A bit
ARMV6_U_BIT                 EQU     1 << 22 ; bit 22 is U bit

TTBR_CTRL_BIT_MASK          EQU     0xff    ; bottom 8 bits
INVALID_ASID                EQU     0xff    ; special ASID that is never used by any process

; ARM processor modes
USER_MODE                   EQU     2_10000
FIQ_MODE                    EQU     2_10001
IRQ_MODE                    EQU     2_10010
SVC_MODE                    EQU     2_10011
ABORT_MODE                  EQU     2_10111
UNDEF_MODE                  EQU     2_11011
SYSTEM_MODE                 EQU     2_11111
MODE_MASK                   EQU     2_11111
THUMB_STATE                 EQU     0x0020

; ID codes for HandleException
ID_RESCHEDULE               EQU     0
ID_UNDEF_INSTR              EQU     1
ID_SWI_INSTR                EQU     2
ID_PREFETCH_ABORT           EQU     3
ID_DATA_ABORT               EQU     4
ID_IRQ                      EQU     5

;*
;* KDataStruct offsets
;*

USER_KPAGE                  EQU     0xFFFFC800
INTERLOCKED_START           EQU     USER_KPAGE+0x380
INTERLOCKED_END             EQU     USER_KPAGE+0x400

;
; CPU independent offsets
;
lpvTls                      EQU     0x000           ; Current thread local storage pointer
ahSys                       EQU     0x004           ; system handle array
dwCurThId                   EQU     0x008           ; SH_CURTHREAD==1
dwCurProcId                 EQU     0x00c           ; SH_CURPROC==2
bResched                    EQU     0x084           ; reschedule flag
cNest                       EQU     0x085           ; kernel exception nesting
bPowerOff                   EQU     0x086
bProfileOn                  EQU     0x087
dwKCRes                     EQU     0x088
pVMPrc                      EQU     0x08c
pCurPrc                     EQU     0x090           ; pointer to current PROCESS structure
pCurThd                     EQU     0x094           ; pointer to current THREAD structure
pAPIReturn                  EQU     0x098
bInDebugger                 EQU     0x0a0
CurFPUOwner                 EQU     0x0b4
PendEvents1                 EQU     0x0b8
PendEvents2                 EQU     0x0bc
CeLogStatus                 EQU     0x19c
alpeIntrEvents              EQU     0x1a0

;
; CPU dependent offsets (range 0x2a0 - 0x300)
;
architectureId              EQU     0x2a0
pAddrMap                    EQU     0x2a4
dwTTBRCtrlBits              EQU     0x2c0

;*
;* Process structure fields
;*
dwPrcID                     EQU     0xC             ; process id

;*
;* Call Stack structure fields
;*
cstk_args                   EQU     0               ; args
cstk_pcstkNext              EQU     0x38            ; pcstkNext
cstk_retAddr                EQU     0x3C            ; retAddr
cstk_dwPrevSP               EQU     0x40            ; dwPrevSP
cstk_dwPrcInfo              EQU     0x44            ; dwPrcInfo
cstk_pprcLast               EQU     0x48            ; pprcLast
cstk_pprcVM                 EQU     0x4C            ; pprcVM
cstk_phd                    EQU     0x50            ; phd
cstk_pOldTls                EQU     0x54            ; pOldTls
cstk_iMethod                EQU     0x58            ; iMethod
cstk_dwNewSP                EQU     0x5C            ; dwNewSP
cstk_regs                   EQU     0x60            ; regs

cstk_sizeof                 EQU     0x90            ; sizeof (CALLSTACK) 

; bit fields of dePrcInfo
CST_MODE_FROM_USER          EQU     0x1

;* Thread structure fields
ThPcstkTop                  EQU     0x1C            ; pcstkTop
ThTlsPtr                    EQU     0x28            ; tlsPtr
ThTlsSecure                 EQU     0x2c            ; tlsSecure
ThTlsNonSecure              EQU     0x30            ; tlsNonSecure

THREAD_CONTEXT_OFFSET       EQU     0x60

TcxPsr                      EQU     THREAD_CONTEXT_OFFSET+0x000
TcxR0                       EQU     THREAD_CONTEXT_OFFSET+0x004
TcxR1                       EQU     THREAD_CONTEXT_OFFSET+0x008
TcxR2                       EQU     THREAD_CONTEXT_OFFSET+0x00c
TcxR3                       EQU     THREAD_CONTEXT_OFFSET+0x010
TcxR4                       EQU     THREAD_CONTEXT_OFFSET+0x014
TcxR5                       EQU     THREAD_CONTEXT_OFFSET+0x018
TcxR6                       EQU     THREAD_CONTEXT_OFFSET+0x01c
TcxR7                       EQU     THREAD_CONTEXT_OFFSET+0x020
TcxR8                       EQU     THREAD_CONTEXT_OFFSET+0x024
TcxR9                       EQU     THREAD_CONTEXT_OFFSET+0x028
TcxR10                      EQU     THREAD_CONTEXT_OFFSET+0x02c
TcxR11                      EQU     THREAD_CONTEXT_OFFSET+0x030
TcxR12                      EQU     THREAD_CONTEXT_OFFSET+0x034
TcxSp                       EQU     THREAD_CONTEXT_OFFSET+0x038
TcxLr                       EQU     THREAD_CONTEXT_OFFSET+0x03c
TcxPc                       EQU     THREAD_CONTEXT_OFFSET+0x040

; floating point registers
TcxFpscr                    EQU     THREAD_CONTEXT_OFFSET+0x044
TcxFpExc                    EQU     THREAD_CONTEXT_OFFSET+0x048
TcxRegs                     EQU     THREAD_CONTEXT_OFFSET+0x04c

TcxSizeof                   EQU     THREAD_CONTEXT_OFFSET+0x0f0   ; TcxRegs + (NUM_VFP_REGS+NUM_ExTRA_CONTROL_REGS+1) * 4

; size of exception record, MUST MATCH sizeof(EXCEPTION_RECORD)
ExrSizeof                   EQU     0x50

; Dispatcher Context Structure Offset Definitions

DcControlPc                 EQU     0x0
DcFunctionEntry             EQU     0x4
DcEstablisherFrame          EQU     0x8
DcContextRecord             EQU     0xc

; Exception Record Offset, Flag, and Enumerated Type Definitions

EXCEPTION_NONCONTINUABLE    EQU     0x1
EXCEPTION_UNWINDING         EQU     0x2
EXCEPTION_EXIT_UNWIND       EQU     0x4
EXCEPTION_STACK_INVALID     EQU     0x8
EXCEPTION_NESTED_CALL       EQU     0x10
EXCEPTION_TARGET_UNWIND     EQU     0x20
EXCEPTION_COLLIDED_UNWIND   EQU     0x40
EXCEPTION_UNWIND            EQU     0x66

ExceptionContinueExecution  EQU     0x0
ExceptionContinueSearch     EQU     0x1
ExceptionNestedException    EQU     0x2
ExceptionCollidedUnwind     EQU     0x3

ErExceptionCode             EQU     0x0
ErExceptionFlags            EQU     0x4
ErExceptionRecord           EQU     0x8
ErExceptionAddress          EQU     0xc
ErNumberParameters          EQU     0x10
ErExceptionInformation      EQU     0x14
ExceptionRecordLength       EQU     0x50

;
; Context Frame Offset and Flag Definitions
;

CONTEXT_FULL                EQU     0x047
CONTEXT_CONTROL             EQU     0x041
CONTEXT_INTEGER             EQU     0x042
CONTEXT_FLOATING_POINT      EQU     0x044

CtxFlags                    EQU     0x000
CtxR0                       EQU     0x004
CtxR1                       EQU     0x008
CtxR2                       EQU     0x00c
CtxR3                       EQU     0x010
CtxR4                       EQU     0x014
CtxR5                       EQU     0x018
CtxR6                       EQU     0x01c
CtxR7                       EQU     0x020
CtxR8                       EQU     0x024
CtxR9                       EQU     0x028
CtxR10                      EQU     0x02c
CtxR11                      EQU     0x030
CtxR12                      EQU     0x034
CtxSp                       EQU     0x038
CtxLr                       EQU     0x03c
CtxPc                       EQU     0x040
CtxPsr                      EQU     0x044

; floating point registers
CtxFpscr                    EQU    0x048
CtxFpExc                    EQU    0x04c
CtxRegs                     EQU    0x050

CtxSizeof                   EQU   0x0f4       ; CtxRegs + (NUM_VFP_REGS+NUM_ExTRA_CONTROL_REGS+1) * 4

;* NK interrupt defines

SYSINTR_NOP                 EQU     0
SYSINTR_RESCHED             EQU     1
SYSINTR_BREAK               EQU     2
SYSINTR_DEVICES             EQU     8
SYSINTR_MAX_DEVICES         EQU     64

SYSINTR_FIRMWARE            EQU     SYSINTR_DEVICES+16
SYSINTR_MAXIMUM             EQU     SYSINTR_DEVICES+SYSINTR_MAX_DEVICES

    END
