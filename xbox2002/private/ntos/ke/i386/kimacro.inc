;++
;
;   Copyright (c) 1989  Microsoft Corporation
;
;   Module Name:
;
;       kimacro.inc
;
;   Abstract:
;
;       This module contains the macros used by kernel assembler code.
;       It includes macros to manipulate interrupts, support system
;       entry and exit for syscalls, faults, and interrupts, and
;       manipulate floating point state.
;
;   Author:
;
;       Shie-Lin (shielint) 24-Jan-1990
;
;   Revision History:
;
;       BryanWi 17-Aug-90
;           Replace GENERATE_MACHINE... and RESTORE... with ENTER_...
;           and EXIT_ALL macros.
;
;--

;++
;
;   These constants are used by the fpo directives in this file.
;   This directive causes the assembler to output a .debug$f segment
;   in the obj file.  The segment will contain 1 fpo record for each
;   directive present during assembly.
;
;   Although the assembler will accept all valid values, the value of 7
;   in the FPO_REGS field indicates to the debugger that a trap frame is
;   generated by the function.  The value of 7 can be used because the
;   C/C++ compiler puts a maximum value of 3 in the field.
;
FPO_LOCALS      equ     0         ; 32 bits, size of locals in dwords
FPO_PARAMS      equ     0         ; 32 bits, size of parameters in dwords
FPO_PROLOG      equ     0         ; 12 bits, 0-4095, # of bytes in prolog
FPO_REGS        equ     0         ; 3 bits, 0-7, # regs saved in prolog
FPO_USE_EBP     equ     0         ; 1 bit, 0-1, is ebp used?
FPO_TRAPFRAME   equ     1         ; 2 bits, 0=fpo, 1=trap frame, 2=tss
;
;--


;++
;
;   POLL_DEBUGGER
;
;   Macro Description:
;
;       Call the debugger so it can check for control-c.  If it finds
;       it, it will report our iret address as address of break-in.
;
;       N.B. This macro should be used when all the caller's registers
;            have been restored. (Otherwise, the kernel debugger register
;            dump will not have correct state.)  The only exception is
;            fs.  This is because Kd may need to access PCR or PRCB.
;
;   Arguments:
;
;       There MUST be an iret frame on the stack when this macro
;       is invoked.
;
;   Exit:
;
;       Debugger will iret for us, so we don't usually return from
;       this macro, but remember that it generates nothing for non-DEVL
;       kernels.
;--

POLL_DEBUGGER   macro
local   a, b, c_

ifdef DEVKIT
        EXTRNP  _KdPollBreakIn,0
        EXTRNP  _DbgBreakPointWithStatus,1
        stdCall _KdPollBreakIn
        or      al,al
        jz      short c_
        stdCall _DbgBreakPointWithStatus,<DBG_STATUS_CONTROL_C>
c_:
endif
endm

;++
;
;   ASSERT_FS
;
;   Try to catch funky condition wherein we get FS=r3 value while
;   running in kernel mode.
;
;--

ASSERT_FS   macro
local   a,b

if DBG
        EXTRNP   _KeBugCheck,1

        cmp     dword ptr fs:[0], 0
        jne     short b

a:
        stdCall   _KeBugCheck,<-1>
align 4
b:
endif
endm



;++
;
;
;   Copy data from various places into base of TrapFrame, net effect
;   is to allow dbg KB command to trace accross trap frame, and to
;   allow user to find arguments to system calls.
;
;   USE ebx and edi.
;--

SET_DEBUG_DATA  macro

ife FPO

;
; This macro is used by ENTER_SYSTEM_CALL, ENTER_TRAP and ENTER_INTERRUPT
; and is used at the end of above macros.  It is safe to destroy ebx, edi.
;

        mov     ebx,[ebp]+TsEbp
        mov     edi,[ebp]+TsEip
        mov     [ebp]+TsDbgArgPointer,edx
        mov     [ebp]+TsDbgArgMark,0BADB0D00h
        mov     [ebp]+TsDbgEbp,ebx
        mov     [ebp]+TsDbgEip,edi
endif

endm

;++
;
;   ENTER_SYSCALL
;
;   Macro Description:
;
;       Build the frame and set registers needed by a system call.
;
;       Save:
;           Errorpad,
;           Non-volatile regs,
;           FS,
;           ExceptionList,
;           PreviousMode
;
;       Don't Save:
;           Volatile regs
;           Seg regs
;           Floating point state
;
;       Set:
;           FS,
;           ExceptionList,
;           PreviousMode,
;           Direction
;
;   Arguments:
;       None.
;
;   Exit-conditions:
;       Interrupts match input state (this routine doesn't change IEF)
;       (esp)->base of trap frame
;       (ebp)->base of trap frame
;       Preserves entry eax, edx
;
;   Note:
;       The DS: reference to PreviousMode is *required* for correct
;       functioning of lazy selector loads.  If you remove this use
;       of DS:, put a DS: override on something.
;
;--

ENTER_SYSCALL macro

.FPO ( FPO_LOCALS, FPO_PARAMS, FPO_PROLOG, FPO_REGS, FPO_USE_EBP, FPO_TRAPFRAME )

ifdef KERNELONLY

;
; Construct trap frame.
;
; N.B. The initial part of the trap frame is constructed by pushing values
;      on the stack. If the format of the trap frame is changed, then the
;      following code must alos be changed.
;

        push    0                       ; put pad dword for error on stack
        push    ebp                     ; save the non-volatile registers
        push    ebx                     ;
        push    esi                     ;
        push    edi                     ;

;
; Save the old exception list in trap frame and initialize a new empty
; exception list.
;

        push    PCR[PcExceptionList]    ; save old exception list
        mov     PCR[PcExceptionList],EXCEPTION_CHAIN_END ; set new empty list

;
; Allocate the remainder of trap frame,
;

        sub     esp,TsExceptionList
        mov     ebp,esp                 ; set trap frame address
        cld                             ; make sure direction is forward

        SET_DEBUG_DATA                  ; Note this destroys edi

        sti                             ; enable interrupts

else
        %out    ENTER_SYSCAL outside of kernel
        .err
endif
        endm

;++
;
;   ENTER_INTERRUPT  PassParm
;
;   Macro Description:
;
;       Build the frame and set registers needed by an interrupt.
;
;       Save:
;           Errorpad,
;           Non-volatile regs,
;           FS,
;           ExceptionList,
;           PreviousMode
;           Volatile regs
;           Seg regs from V86 mode
;           DS, ES, GS
;
;       Don't Save:
;           Floating point state
;
;       Set:
;           FS,
;           ExceptionList,
;           Direction,
;           DS, ES
;
;       Don't Set:
;           PreviousMode
;
;   Arguments:
;       PassParm -
;
;   Exit-conditions:
;       Interrupts match input state (this routine doesn't change IEF)
;       (esp)->base of trap frame
;       (ebp)->base of trap frame
;       Preserves entry eax, ecx, edx
;
;--

ENTER_INTERRUPT macro   PassParm
        local b

.FPO ( FPO_LOCALS+2, FPO_PARAMS, FPO_PROLOG, FPO_REGS, FPO_USE_EBP, FPO_TRAPFRAME )

;
;   Fill in parts of frame we care about
;

ifb <PassParm>
        push    esp                 ; put pad dword for error on stack
endif
        push    ebp                 ; Save the non-volatile registers
        push    ebx
        push    esi
        push    edi

        sub     esp, TsEdi
        mov     ebp,esp

        mov     [esp]+TsEax, eax    ; Save volatile registers
        mov     [esp]+TsEcx, ecx
        mov     [esp]+TsEdx, edx

        mov     ebx, PCR[PcExceptionList] ;Save, set ExceptionList
        mov     PCR[PcExceptionList],EXCEPTION_CHAIN_END
        mov     [esp]+TsExceptionList, ebx

        cld

        SET_DEBUG_DATA

        endm

;++
;
;   ENTER_TRAP
;
;   Macro Description:
;
;       Build the frame and set registers needed by a trap or exception.
;
;       Save:
;           Non-volatile regs,
;           FS,
;           ExceptionList,
;           PreviousMode,
;           Volatile regs
;           Seg Regs from V86 mode
;           DS, ES, GS
;
;       Don't Save:
;           Floating point state
;
;       Set:
;           FS,
;           Direction,
;           DS, ES
;
;       Don't Set:
;           PreviousMode,
;           ExceptionList
;
;   Arguments:
;       None.
;
;   Exit-conditions:
;       Interrupts match input state (this routine doesn't change IEF)
;       (esp)->base of trap frame
;       (ebp)->base of trap frame
;       Preserves entry eax
;
;--

ENTER_TRAP macro
        local b

.FPO ( FPO_LOCALS, FPO_PARAMS, FPO_PROLOG, FPO_REGS, FPO_USE_EBP, FPO_TRAPFRAME )

;
;   Fill in parts of frame we care about
;

        mov     word ptr [esp+2], 0 ; Clear upper word of ErrorCode

        push    ebp                 ; Save the non-volatile registers
        push    ebx
        push    esi
        push    edi

        mov     ebx, PCR[PcExceptionList] ;Save ExceptionList
        push    ebx
        push    eax                 ; Save the volatile registers
        push    ecx
        push    edx

;
;   Skip allocate reset of trap frame.
;

        sub     esp,TsEdx

        mov     ebp,esp

        cld
        SET_DEBUG_DATA

        endm
;++
;
;   EXIT_ALL    NoRestoreSegs, NoRestoreVolatiles, NoPreviousMode
;
;   Macro Description:
;
;       Load a syscall frame back into the machine.
;
;       Restore:
;           Volatile regs, IF NoRestoreVolatiles blank
;           NoPreviousMode,
;           ExceptionList,
;           FS,
;           Non-volatile regs
;
;       If the frame is a kernel mode frame, AND esp has been edited,
;       then TsSegCs will have a special value.  Test for that value
;       and execute special code for that case.
;
;       N.B. This macro generates an IRET!  (i.e. It exits!)
;
;   Arguments:
;
;       NoRestoreSegs - non-blank if DS, ES, GS are NOT to be restored
;
;       NoRestoreVolatiles - non-blank if Volatile regs are NOT to be restored
;
;       NoPreviousMode - if nb pop ThPreviousMode
;
;   Entry-conditions:
;
;       (esp)->base of trap frame
;       (ebp)->Base of trap frame
;
;   Exit-conditions:
;
;       Does not exit, returns.
;       Preserves eax, ecx, edx, IFF NoRestoreVolatiles is set
;
;--

?adjesp = 0
?RestoreAll = 1

EXIT_ALL macro  NoRestoreSegs, NoRestoreVolatiles, NoPreviousMode
local   a, b, f, x
local   Db_NotATrapFrame, Db_A, Db_NotValidEntry
local   DbgHalt, DbgNoHalt, DbgDoneHalt

;
; Sanity check some values and setup globals for macro
;

?adjesp = TsEdx
?RestoreAll = 1

ifnb <NoRestoreSegs>
    ?RestoreAll = 0
endif

ifnb <NoRestoreVolatiles>
    if ?RestoreAll eq 1
        %out "EXIT_ALL NoRestoreVolatiles requires NoRestoreSegs"
        .err
    endif
    ?adjesp = ?adjesp + 12
endif

ifb <NoPreviousMode>
ifndef KERNELONLY
        %out    EXIT_ALL can not restore previousmode outside kernel
        .err
endif
endif

; All callers are responsible for getting here with interrupts disabled.

if DBG
        pushfd
        pop     edx

        test    edx, EFLAGS_INTERRUPT_MASK
        jnz     Db_NotValidEntry

        cmp     esp, ebp                    ; make sure esp = ebp
        jne     Db_NotValidEntry

; Make sure BADB0D00 sig is present.  If not this isn't a trap frame!
Db_A:   sub     [esp]+TsDbgArgMark,0BADB0D00h
        jne     Db_NotATrapFrame
endif

        ASSERT_FS

        mov     edx, [esp]+TsExceptionList
if DBG
        or      edx, edx
        jnz     short @f
    int 3
@@:
endif
        mov     PCR[PcExceptionList], edx   ; Restore ExceptionList

        test    word ptr [esp]+TsSegCs,FRAME_EDITED
        jz      b                           ; Edited frame pop out.

ifb <NoRestoreVolatiles>
ifb <NoRestoreSegs>                         ; must restore eax before any
        mov     eax, [esp].TsEax            ; selectors! (see trap0e handler)
endif
endif

ifb <NoRestoreVolatiles>
        mov     edx, [ebp]+TsEdx            ; Restore volitales
        mov     ecx, [ebp]+TsEcx
ifb <NoRestoreSegs>
else
        mov     eax, [ebp]+TsEax
endif
endif   ; NoRestoreVolatiles

        lea     esp, [ebp]+TsEdi            ; Skip PreMode, ExceptList and fs

ifdef DEVKIT
        ; check for requested halt
        mov     edi, PCR[PcPrcbData+PbDebugHaltThread]
        test    edi, edi
        jnz     short DbgHalt
DbgNoHalt:
        ; EBP is toast here, so we'd better not need it
endif

        pop     edi                         ; restore non-volatiles
        pop     esi
        pop     ebx
        pop     ebp

;
; Esp MUST point to the Error Code on the stack.  Because we use it to
; store the entering esp.
;

        add     esp, 4              ; remove error code from trap frame
        iretd                       ; return

ifdef DEVKIT
DbgHalt:
        ; There is a requested halt, so let's see if now's the time to halt
        push    eax
        mov     eax, PCR[PcIrql]
        push    ecx
        push    edx
        test    eax, eax            ; only break if irql = 0
        jne     DbgDoneHalt
        call    edi
        test    al, al
        je      DbgDoneHalt
        sti
        mov     ebp, [esp+24]       ; load up the saved EBP so the debugger
        int     3h                  ; shows a reasonable trace
        cli
DbgDoneHalt:
        pop     edx
        pop     ecx
        pop     eax
        jmp     DbgNoHalt
endif

if DBG
Db_NotATrapFrame:
        add     [esp]+TsDbgArgMark,0BADB0D00h   ; put back the orig value
Db_NotValidEntry:
        int 3
        jmp     Db_A
endif

;
;   TsSegCs contains the special value that means the frame was edited
;   in a way that affected esp, AND it's a kernel mode frame.
;   (Special value is null selector except for RPL.)
;
;   Put back the real CS.
;   push eflags, eip onto target stack
;   restore
;   switch to target stack
;   iret
;

b:      mov     ebx,[esp]+TsTempSegCs
        mov     [esp]+TsSegCs,ebx

;
;   There is no instruction that will load esp with an arbitrary value
;   (i.e. one out of a frame) and do a return, if no privledge transition
;   is occuring.  Therefore, if we are returning to kernel mode, and
;   esp has been edited, we must "emulate" a kind of iretd.
;
;   We do this by logically pushing the eip,cs,eflags onto the new
;   logical stack, loading that stack, and doing an iretd.  This
;   requires that the new logical stack is at least 1 dword higher
;   than the unedited esp would have been.  (i.e.  It is not legal
;   to edit esp to have a new value < the old value.)
;
;   KeContextToKframes enforces this rule.
;

;
;   Compute new logical stack address
;

        mov     ebx,[esp]+TsTempEsp
        sub     ebx,12
        mov     [esp]+TsErrCode,ebx

;
;   Copy eip,cs,eflags to new stack.  note we do this high to low
;

        mov     esi,[esp]+TsEflags
        mov     [ebx+8],esi
        mov     esi,[esp]+TsSegCs
        mov     [ebx+4],esi
        mov     esi,[esp]+TsEip
        mov     [ebx],esi

;
;   Do a standard restore sequence.
;
;   Observe that RestoreVolatiles is honored.  Editing a volatile
;   register has no effect when returning from a system call.
;
ifb     <NoRestoreVolatiles>
        mov     eax,[esp].TsEax
endif
;        add     esp,TsSegGs
;
;ifb     <NoRestoreSegs>
;        pop     gs
;        pop     es
;        pop     ds
;else
;        add     esp,12
;endif

ifb     <NoRestoreVolatiles>
        mov     edx, [esp]+TsEdx
        mov     ecx, [esp]+TsEcx
endif

;ifnb <NoPreviousMode>
;        add     esp, 4              ; Skip previous mode
;else
;        pop     ebx                 ; Restore PreviousMode
;        mov     esi,fs:[PcPrcbData+PbCurrentThread]
;        mov     ss:[esi]+ThPreviousMode,bl
;endif
;
;       pop     ebx
;
;       mov     fs:[PcExceptionList], ebx ;Restore ExceptionList
;       pop     fs

        add     esp, TsEdi
        pop     edi                 ; restore non-volatiles
        pop     esi
        pop     ebx
        pop     ebp

;
;   (esp)->TsErrCode, where we saved the new esp
;

        mov     esp,[esp]           ; Do move not push to avoid increment
        iretd

        endm


;++
;
;   INTERRUPT_EXIT
;
;   Macro Description:
;
;       This macro is executed on return from an interrupt vector service
;       service routine.  Its function is to restore privileged processor
;       state, and continue thread execution. If control is returning to
;       user mode and there is a user  APC pending, then APC level interupt
;       will be requested and control is transfered to the user APC delivery
;       routine, if no higher level interrupt pending.
;
;   Arguments:
;
;       (TOS)   = previous irql
;       (TOS+4 ...) = machine_state frame
;       (ebp)-> machine state frame (trap frame)
;
;--

INTERRUPT_EXIT     macro
local   a

if DBG                                          ; save current eip for
a:      mov     esi, offset a                   ; debugging bad trap frames
endif

        cli
        pop     ecx
        jmp     @HalEndSystemInterrupt@4

endm


;++
;
;   LEVEL_INTERRUPT_EXIT
;
;   Macro Description:
;
;       This macro is executed on return from an interrupt vector service
;       service routine.  Its function is to restore privileged processor
;       state, and continue thread execution. If control is returning to
;       user mode and there is a user  APC pending, then APC level interupt
;       will be requested and control is transfered to the user APC delivery
;       routine, if no higher level interrupt pending.
;
;   Arguments:
;
;       (eax)-> bus interrupt level
;       (TOS)   = previous irql
;       (TOS+4 ...) = machine_state frame
;       (ebp)-> machine state frame (trap frame)
;
;--

LEVEL_INTERRUPT_EXIT     macro
local   a

if DBG                                          ; save current eip for
a:      mov     esi, offset a                   ; debugging bad trap frames
endif

        cli
        pop     ecx
        jmp     @HalEndSystemLevelInterrupt@4

endm


;++
;
;   SPURIOUS_INTERRUPT_EXIT
;
;   Macro Description:
;
;       To exit an interrupt without performing the EOI.
;
;   Arguments:
;
;       (TOS) = machine_state frame
;       (ebp)-> machine state frame (trap frame)
;
;--

SPURIOUS_INTERRUPT_EXIT  macro
local   a
if DBG                                          ; save current eip for
a:      mov     esi, offset a                   ; debugging bad trap frames
endif
        jmp     Kei386EoiHelper@0
endm
