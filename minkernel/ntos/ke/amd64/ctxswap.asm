        title  "Context Swap"
;++
;
; Copyright (c) 2000  Microsoft Corporation
;
; Module Name:
;
;   ctxswap.asm
;
; Abstract:
;
;   This module implements the code necessary to field the dispatch interrupt
;   and perform context switching.
;
; Author:
;
;   David N. Cutler (davec) 26-Aug-2000
;
; Environment:
;
;    Kernel mode only.
;
;--

include ksamd64.inc

        extern  KeBugCheckEx:proc
        extern  KiDeliverApc:proc
        extern  KeRaiseIrqlToSynchLevel:proc
        extern  KiQuantumEnd:proc
        extern  KiQueueReadyThread:proc
        extern  KiRetireDpcList:proc
        extern  WmiTraceContextSwap:proc
        extern  __imp_HalRequestSoftwareInterrupt:qword

        subttl  "Swap Context"
;++
;
; BOOLEAN
; KiSwapContext (
;    IN PKTHREAD OldThread,
;    IN PKTHREAD NewThread
;    )
;
; Routine Description:
;
;   This function is a small wrapper that marshalls arguments and calls the
;   actual swap context routine.
;
;   N.B. The old thread lock has been acquired and the dispatcher lock dropped
;        before this routine is called.
;
;   N.B. The current thread address and the new thread state has been set
;        before this routine is called.
;
; Arguments:
;
;   OldThread (rcx) - Supplies the address of the old thread.
;
;   NewThread (rdx) - Supplies the address of the old thread.
;
; Return Value:
;
;   If a kernel APC is pending, then a value of TRUE is returned. Otherwise,
;   a value of FALSE is returned.
;
;--

        NESTED_ENTRY KiSwapContext, _TEXT$00

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

        mov     rbx, gs:[PcCurrentPrcb] ; get current PRCB address
        mov     rdi, rcx                ; set old thread address
        mov     rsi, rdx                ; set new thread address
        mov     cl, ThWaitIrql[rdi]     ; set APC interrupt bypass disable
        call    SwapContext             ; swap context

        RESTORE_EXCEPTION_STATE         ; restore exception state/deallocate

        ret                             ; return

        NESTED_END KiSwapContext, _TEXT$00

        subttl  "Dispatch Interrupt"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a software interrupt generated
;   at DISPATCH_LEVEL. Its function is to process the DPC list, and then
;   perform a context switch if a new thread has been selected for execution
;   on the current processor.
;
;   This routine is entered at DISPATCH_LEVEL with the dispatcher database
;   unlocked.
;
; Arguments:
;
;   None
;
; Return Value:
;
;   None.
;
;--

DiFrame struct
        P1Home  dq ?                    ; PRCB address parameter
        Fill    dq ?                    ; fill to 8 mod 16
        SavedRbx dq ?                   ; saved RBX
DiFrame ends

        NESTED_ENTRY KiDispatchInterrupt, _TEXT$00

        push_reg rbx                    ; save nonvolatile register
        alloc_stack (sizeof DiFrame - 8) ; allocate stack frame

        END_PROLOGUE

        mov     rbx, gs:[PcCurrentPrcb] ; get current PRCB address
        and     byte ptr PbDpcInterruptRequested[rbx], 0 ; clear request

;
; Check if the DPC queue has any entries to process.
;

KiDI10: cli                             ; disable interrupts
        mov     eax, PbDpcQueueDepth[rbx] ; get DPC queue depth
        or      rax, PbTimerRequest[rbx] ; merge timer request value

ifndef NT_UP

        or      rax, PbDeferredReadyListHead[rbx] ; merge deferred ready list

endif

        jz      short KiDI20            ; if z, no DPCs to process
        mov     PbSavedRsp[rbx], rsp    ; save current stack pointer
        mov     rsp, PbDpcStack[rbx]    ; set DPC stack pointer
        mov     rcx, rbx                ; set PRCB address parameter
        call    KiRetireDpcList         ; process the DPC list
        mov     rsp, PbSavedRsp[rbx]    ; restore current stack pointer

;
; Check to determine if quantum end is requested.
;

KiDI20: sti                             ; enable interrupts
        cmp     byte ptr PbQuantumEnd[rbx], 0 ; check if quantum end request
        je      short KiDI40            ; if e, quantum end not requested
        and     byte ptr PbQuantumEnd[rbx], 0 ; clear quantum end indicator
        call    KiQuantumEnd            ; process quantum end

;
; Restore nonvolatile registers, deallocate stack frame, and return.
;

KiDI30: add     rsp, sizeof DiFrame - 8 ; deallocate stack frame
        pop     rbx                     ; restore nonvolatile register
        ret                             ; return

;
; Check to determine if a new thread has been selected for execution on this
; processor.
;

KiDI40: cmp     qword ptr PbNextThread[rbx], 0 ; check if new thread selected
        je      short KiDI30            ; if eq, then no new thread

;
; Swap context to a new thread as the result of new thread being scheduled
; by the dispatch interrupt.
;

        add     rsp, sizeof DiFrame - 8 ; deallocate stack frame
        pop     rbx                     ; restore nonvolatile register
        jmp     short KxDispatchInterrupt ;

        NESTED_END KiDispatchInterrupt, _TEXT$00

;
; There is a new thread scheduled for execution and the dispatcher lock
; has been acquired. Context switch to the new thread immediately.
;
; N.B. The following routine is entered by falling through from the above
;      routine.
;
; N.B. The following routine is carefully written as a nested function that
;      appears to have been called directly by the caller of the above
;      function which processes the dispatch interrupt.
;
; Arguments:
;
;   None.
;

        NESTED_ENTRY KxDispatchInterrupt, _TEXT$00

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

        mov     rbx, gs:[PcCurrentPrcb] ; get current PRCB address
        mov     rdi, PbCurrentThread[rbx] ; get old thread address

;
; Raise IRQL to SYNCH level, set context swap busy for the old thread, and
; acquire the current PRCB lock.
;

ifndef NT_UP

        call    KeRaiseIrqlToSynchLevel ; raise IRQL to SYNCH Level
        mov     byte ptr ThSwapbusy[rdi], 1 ; set context swap busy
        lea     r11, PbPrcbLock[rbx]    ; set address of current PRCB

        AcquireSpinLock r11             ; acquire current PRCB lock

endif

;
; Get the next thread address, set the thread state to running, queue the old
; running thread, and swap context to the next thread.
;

        mov     rsi, PbNextThread[rbx]  ; get next thread address
        and     qword ptr PbNextThread[rbx], 0 ; clear next thread address
        mov     PbCurrentThread[rbx], rsi ; set current thread address
        mov     byte ptr ThState[rsi], Running ; set new thread state
        mov     byte ptr ThWaitReason[rdi], WrDispatchInt ; set wait reason
        mov     rcx, rdi                ; set address of old thread
        mov     rdx, rbx                ; set address of current PRCB
        call    KiQueueReadyThread      ; queue ready thread for execution
        mov     cl, APC_LEVEL           ; set APC interrupt bypass disable
        call    SwapContext             ; call context swap routine

        RESTORE_EXCEPTION_STATE         ; restore exception state/deallocate

        ret                             ; return

        NESTED_END KxDispatchInterrupt, _TEXT$00

        subttl  "Swap Context"
;++
;
; Routine Description:
;
;   This routine is called to swap context from one thread to the next. It
;   swaps context, flushes the translation buffer, swaps the process address
;   space if necessary, and returns to its caller.
;
;   N.B. This routine is only called by code within this module and the idle
;        thread code and uses special register calling conventions.
;
; Arguments:
;
;   al - Supplies a boolean value that determines whether the full legacy
;       floating state needs to be saved.
;
;   cl - Supplies the APC interrupt bypass disable IRQL value.
;
;   rbx - Supplies the address of the current PRCB.
;
;   rdi - Supplies the address of previous thread.
;
;   rsi - Supplies the address of next thread.
;
; Return value:
;
;   al - Supplies the kernel APC pending flag.
;
;   rbx - Supplies the address of the current PRCB.
;
;   rsi - Supplies the address of current thread.
;
;--

        NESTED_ENTRY SwapContext, _TEXT$00

        push_reg rbp                    ; save nonvolatile register
        alloc_stack (KSWITCH_FRAME_LENGTH - (2 * 8)) ; allocate stack frame

        END_PROLOGUE

        mov     SwApcBypass[rsp], cl    ; save APC bypass disable

;
; Wait for new thread lock to be dropped.
;
; N.B. It is necessary to wait for the new thread context to be swapped so
;      that any context switch away from the new thread on another processor
;      is completed before attempting to swap context context to the thread.
;

ifndef NT_UP

KiSC00: cmp     byte ptr ThSwapBusy[rsi], 0 ; check if swap busy for new thread
        jne     short KiSC00            ; if ne, context busy for new thread

endif

;
; Increment the number of context switches on this processor.
;
; N.B. This increment is done here is force the cache block containing the
;      context switch count into the cache as write exclusive. There are
;      several other references to this cache block in the following code.
;

        inc     dword ptr (PcContextSwitches - PcPrcb)[rbx] ; processor count

;
; Accumulate the total time spent in a thread.
;

ifdef PERF_DATA

        rdtsc                           ; read cycle counter
        sub     eax, PbThreadStartCount + 0[rbx] ; sub out thread start time
        sbb     edx, PbThreadStartCount + 4[rbx] ;
        add     EtPerformanceCountLow[rdi], eax ; accumlate thread run time
        adc     EtPerformanceCountHigh[rdi], edx ;
        add     PbThreadStartCount + 4[rbx], eax ; set new thread start time
        adc     PbThreadStartCount + 8[rbx], edx ;

endif

;
; Check for context swap logging.
;

        cmp     qword ptr (PcPerfGlobalGroupMask - PcPrcb)[rbx], 0 ; check if logging enable
        je      short KiSC05            ; if eq, logging not enabled
        mov     rax, (PcPerfGlobalGroupMask - PcPrcb)[rbx] ; get global mask address
        mov     rdx, rdi                ; set address of old thread
        mov     rcx, rsi                ; set address of new thread
        test    dword ptr PERF_CONTEXTSWAP_OFFSET[rax], PERF_CONTEXTSWAP_FLAG ; check flag
        jz      short KiSC05            ; if z, context swap events not enabled
        call    WmiTraceContextSwap     ; call trace routine

;
; Save the kernel mode XMM control/status register. If the current thread
; NPX state is switch, then save the legacy floating point state.
;

KiSC05: stmxcsr SwMxCsr[rsp]            ; save kernel mode XMM control/status
        cmp     byte ptr ThNpxState[rdi], LEGACY_STATE_SWITCH ; check if switched
        jne     short KiSC10            ; if ne, legacy state not switched
        mov     rbp, ThInitialStack[rdi] ; get previous thread initial stack
        fnsave  [ebp]                   ; save full legacy floating point state

;
; Switch kernel stacks.
;

KiSC10: mov     ThKernelStack[rdi], rsp ; save old kernel stack pointer
        mov     rsp, ThKernelStack[rsi] ; get new kernel stack pointer

;
; Swap the process address space if the new process is not the same as the
; previous process.
;

        mov     r14, ThApcState + AsProcess[rsi] ; get new process address
        cmp     r14, ThApcState + AsProcess[rdi] ; check if process match
        je      short KiSC20            ; if e, process addresses match

;
; Clear the processor bit in the old process.
;

ifndef NT_UP

        mov     rax, ThApcState + AsProcess[rdi] ; get old process address
        mov     rcx, (PcSetMember - PcPrcb)[rbx] ; get processor set member
   lock xor     PrActiveProcessors[rax], rcx ; clear bit in previous set

if DBG

        test    PrActiveProcessors[rax], rcx ; test if bit clear in previous set
        jz      short @f                ; if z, bit clear in previous set
        int     3                       ; debug break - incorrect active mask
@@:                                     ; reference label

endif

endif


;
; Set the processor bit in the new process.
;

ifndef NT_UP

   lock xor     PrActiveProcessors[r14], rcx ; set bit in new set

if DBG

        test    PrActiveProcessors[r14], rcx ; test if bit set in new set
        jnz     short @f                ; if nz, bit set in new set
        int     3                       ; debug break - incorrect active mask
@@:                                     ; reference label

endif

endif

;
; Load new CR3 value which will flush the TB.
;

        mov     rax, PrDirectoryTableBase[r14] ; get new directory base
        mov     cr3, rax                ; flush TLB and set new directory base

;
; Set context swap idle for the old thread lock.
;

KiSc20:                                 ;

ifndef NT_UP

        and     byte ptr ThSwapBusy[rdi], 0  ; set context swap idle

endif

;
; Set the new kernel stack base in the TSS.
;

        mov     r15, (PcTss - PcPrcb)[rbx] ; get processor TSS address
        mov     rbp, ThInitialStack[rsi] ; get new stack base address
        mov     TssRsp0[r15], rbp       ; set stack base address in TSS

;
; If the new thread executes in user mode, then restore the legacy floating
; state, load the compatibility mode TEB address, load the native user mode
; TEB address, and reload the segment registers if needed.
;
; N.B. The upper 32-bits of the compatibility mode TEB address are always
;      zero.
;

        cmp     byte ptr ThNpxState[rsi], LEGACY_STATE_UNUSED ; check if kernel thread
        je      KiSC30                  ; if e, kernel thread
        mov     cx, LfControlWord[rbp]  ; save current control word
        mov     word ptr LfControlWord[rbp], 03fh ; set to mask all exceptions
        frstor  [ebp]                   ; restore legacy floating point state
        mov     LfControlWord[rbp], cx  ; restore control word
        fldcw   word ptr LfControlWord[rbp] ; load legacy control word

;
; Set base of compatibility mode TEB.
;

        mov     eax, ThTeb[rsi]         ; compute compatibility mode TEB address
        add     eax, CmThreadEnvironmentBlockOffset ;
        mov     rcx, (PcGdt - PcPrcb)[rbx] ; get GDT base address
        mov     KgdtBaseLow + KGDT64_R3_CMTEB[rcx], ax ; set CMTEB base address
        shr     eax, 16                 ;
        mov     KgdtBaseMiddle + KGDT64_R3_CMTEB[rcx], al ;
        mov     KgdtBaseHigh + KGDT64_R3_CMTEB[rcx], ah   ;

;
; If the user segment selectors have been changed, then reload them with
; their cannonical values.
;
; N.B. The following code depends on the values defined in ntamd64.w that
;      can be loaded in ds, es, fs, and gs. In particular an "and" operation
;      is used for the below comparison.
;

        mov     ax, ds                  ; compute sum of segment selectors
        mov     cx, es                  ;
        and     ax, cx                  ;
        mov     cx, gs                  ;
        and     ax, cx                  ;
        cmp     ax, (KGDT64_R3_DATA or RPL_MASK) ; check if sum matches
        je      short KiSC25            ; if e, sum matches expected value
        mov     cx, KGDT64_R3_DATA or RPL_MASK ; reload user segment selectors
        mov     ds, cx                  ;
        mov     es, cx                  ;

;
; N.B. The following reload of the GS selector destroys the system MSR_GS_BASE
;      register. Thus this sequence must be done with interrupt off.
;

        mov     eax, (PcSelf - PcPrcb)[rbx] ; get current PCR address
        mov     edx, (PcSelf - PcPrcb + 4)[rbx] ;
        cli                             ; disable interrupts
        mov     gs, cx                  ; reload GS segment selector
        mov     ecx, MSR_GS_BASE        ; get GS base MSR number
        wrmsr                           ; write system PCR base address
        sti                             ; enable interrupts
KiSC25: mov     ax, KGDT64_R3_CMTEB or RPL_MASK ; reload FS segment selector
        mov     fs, ax                  ;
        mov     eax, ThTeb[rsi]         ; get low part of user TEB address
        mov     edx, ThTeb + 4[rsi]     ; get high part of user TEB address
        mov     (PcTeb - PcPrcb)[rbx], eax ; set user TEB address in PCR
        mov     (PcTeb - PcPrcb + 4)[rbx], edx ;
        mov     ecx, MSR_GS_SWAP        ; get GS base swap MSR number
        wrmsr                           ; write user TEB base address

;
; Restore kernel mode XMM control/status and update context switch counters.
;

KiSC30: ldmxcsr SwMxCsr[rsp]            ; kernel mode XMM control/status
        inc     dword ptr ThContextSwitches[rsi] ; thread count

;
; Check if an attempt is being made to context switch while in a DPC routine.
;

        cmp     word ptr PbDpcRoutineActive[rbx], 0 ; check if DPC active
        jne     short KiSC50            ; if ne, DPC is active

;
; If the new thread has a kernel mode APC pending, then request an APC
; interrupt if APC bypass is disabled.
;

        cmp     byte ptr ThApcState + AsKernelApcPending[rsi], TRUE ; check if APC pending
        jne     short KiSC40            ; if ne, kernel APC not pending
        cmp     word ptr ThSpecialApcDisable[rsi], 0 ; check if special APC disable
        jne     short KiSC40            ; if ne, special APC disable
        cmp     byte ptr SwApcBypass[rsp], PASSIVE_LEVEL ; check if APC bypass enabled
        je      short KiSC40            ; if e, APC bypass enabled
        mov     cl, APC_LEVEL           ; request APC interrupt
        call    __imp_HalRequestSoftwareInterrupt ;
        or      rax, rsp                ; clear ZF flag
KiSC40: setz    al                      ; set return value
        add     rsp, KSWITCH_FRAME_LENGTH - (2 * 8) ; deallocate stack frame
        pop     rbp                     ; restore nonvolatile register
        ret                             ; return

;
; An attempt is being made to context switch while in a DPC routine. This is
; most likely caused by a DPC routine calling one of the wait functions.
;

KiSC50: xor     r9, r9                  ; clear register
        mov     SwP5Home[rsp], r9       ; set parameter 5
        mov     r8, rsi                 ; set new thread address
        mov     rdx, rdi                ; set old thread address
        mov     ecx, ATTEMPTED_SWITCH_FROM_DPC ; set bug check code
        call    KeBugCheckEx            ; bug check system - no return
        ret                             ; return

        NESTED_END SwapContext, _TEXT$00

        end
