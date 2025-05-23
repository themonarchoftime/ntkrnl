/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    mi.h

Abstract:

    This module contains the private data structures and procedure
    prototypes for the memory management system.

Author:

    Lou Perazzoli (loup) 20-Mar-1989
    Landy Wang (landyw) 02-Jun-1997

Revision History:

--*/

#ifndef _MI_
#define _MI_

#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4324)   // alignment sensitive to declspec
#pragma warning(disable:4127)   // condition expression is constant
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4232)   // dllimport not static
#pragma warning(disable:4206)   // translation unit empty

#include "ntos.h"
#include "ntimage.h"
#include "ki.h"
#include "fsrtl.h"
#include "zwapi.h"
#include "pool.h"
#include "stdio.h"
#include "string.h"
#include "safeboot.h"
#include "triage.h"
#include "xip.h"

#if defined(_X86_)
#include "..\mm\i386\mi386.h"

#elif defined(_AMD64_)
#include "..\mm\amd64\miamd.h"

#elif defined(_IA64_)
#include "..\mm\ia64\miia64.h"

#else
#error "mm: a target architecture must be defined."
#endif

#if defined (_WIN64)
#define ASSERT32(exp)
#define ASSERT64(exp)   ASSERT(exp)

//
// This macro is used to satisfy the compiler -
// note the assignments are not needed for correctness
// but without it the compiler cannot compile this code
// W4 to check for use of uninitialized variables.
//

#define SATISFY_OVERZEALOUS_COMPILER(x) x
#else
#define ASSERT32(exp)   ASSERT(exp)
#define ASSERT64(exp)
#define SATISFY_OVERZEALOUS_COMPILER(x) x
#endif

//
// Special pool constants
//
#define MI_SPECIAL_POOL_PAGABLE         0x8000
#define MI_SPECIAL_POOL_VERIFIER        0x4000
#define MI_SPECIAL_POOL_IN_SESSION      0x2000
#define MI_SPECIAL_POOL_PTE_PAGABLE     0x0002
#define MI_SPECIAL_POOL_PTE_NONPAGABLE  0x0004


#define _2gb  0x80000000                // 2 gigabytes
#define _3gb  0xC0000000                // 3 gigabytes
#define _4gb 0x100000000                // 4 gigabytes

#define MM_FLUSH_COUNTER_MASK (0xFFFFF)

#define MM_FREE_WSLE_SHIFT 4

#define WSLE_NULL_INDEX ((((WSLE_NUMBER)-1) >> MM_FREE_WSLE_SHIFT))

#define MM_FREE_POOL_SIGNATURE (0x50554F4C)

#define MM_MINIMUM_PAGED_POOL_NTAS ((SIZE_T)(48*1024*1024))

#define MM_ALLOCATION_FILLS_VAD ((PMMPTE)(ULONG_PTR)~3)

#define MM_WORKING_SET_LIST_SEARCH 17

#define MM_FLUID_WORKING_SET 8

#define MM_FLUID_PHYSICAL_PAGES 32  //see MmResidentPages below.

#define MM_USABLE_PAGES_FREE 32

#define X64K (ULONG)65536

#define MM_HIGHEST_VAD_ADDRESS ((PVOID)((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (64 * 1024)))


#define MM_WS_NOT_LISTED    ((PLIST_ENTRY)0)
#define MM_WS_TRIMMING      ((PLIST_ENTRY)1)
#define MM_WS_SWAPPED_OUT   ((PLIST_ENTRY)2)

#if DBG
#define MM_IO_IN_PROGRESS ((PLIST_ENTRY)97)
#endif

#define MM4K_SHIFT    12  //MUST BE LESS THAN OR EQUAL TO PAGE_SHIFT
#define MM4K_MASK  0xfff

#define MMSECTOR_SHIFT 9  //MUST BE LESS THAN OR EQUAL TO PAGE_SHIFT

#define MMSECTOR_MASK 0x1ff

#define MM_LOCK_BY_REFCOUNT 0

#define MM_LOCK_BY_NONPAGE 1

#define MM_MAXIMUM_WRITE_CLUSTER (MM_MAXIMUM_DISK_IO_SIZE / PAGE_SIZE)

//
// Number of PTEs to flush singularly before flushing the entire TB.
//

#define MM_MAXIMUM_FLUSH_COUNT (FLUSH_MULTIPLE_MAXIMUM-1)

//
// Page protections
//

#define MM_ZERO_ACCESS         0  // this value is not used.
#define MM_READONLY            1
#define MM_EXECUTE             2
#define MM_EXECUTE_READ        3
#define MM_READWRITE           4  // bit 2 is set if this is writable.
#define MM_WRITECOPY           5
#define MM_EXECUTE_READWRITE   6
#define MM_EXECUTE_WRITECOPY   7

#define MM_NOCACHE            0x8
#define MM_GUARD_PAGE         0x10
#define MM_DECOMMIT           0x10   //NO_ACCESS, Guard page
#define MM_NOACCESS           0x18   //NO_ACCESS, Guard_page, nocache.
#define MM_UNKNOWN_PROTECTION 0x100  //bigger than 5 bits!
#define MM_LARGE_PAGES        0x111

#define MM_INVALID_PROTECTION ((ULONG)-1)  //bigger than 5 bits!

#define MM_KSTACK_OUTSWAPPED  0x1F   // Denotes outswapped kernel stack pages.

#define MM_PROTECTION_WRITE_MASK     4
#define MM_PROTECTION_COPY_MASK      1
#define MM_PROTECTION_OPERATION_MASK 7 // mask off guard page and nocache.
#define MM_PROTECTION_EXECUTE_MASK   2

#define MM_SECURE_DELETE_CHECK 0x55

#if defined(_X86PAE_)

//
// PAE mode makes most kernel resources executable to improve
// compatibility with existing driver binaries.
//

#define MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE(TempPte)         \
                    ASSERT ((TempPte).u.Hard.Valid == 1);   \
                    ((TempPte).u.Long &= ~MmPaeMask);

#define MI_ADD_EXECUTE_TO_INVALID_PTE_IF_PAE(TempPte)       \
                    ASSERT ((TempPte).u.Hard.Valid == 0);   \
                    ((TempPte).u.Soft.Protection |= MM_EXECUTE);

#else

//
// NT64 drivers derived from 32-bit source have to be recompiled so there's
// no need to make everything executable - drivers can specify it explicitly.
//

#define MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE(TempPte)
#define MI_ADD_EXECUTE_TO_INVALID_PTE_IF_PAE(TempPte)
#endif

//
// Debug flags
//

#define MM_DBG_WRITEFAULT       0x1
#define MM_DBG_PTE_UPDATE       0x2
#define MM_DBG_DUMP_WSL         0x4
#define MM_DBG_PAGEFAULT        0x8
#define MM_DBG_WS_EXPANSION     0x10
#define MM_DBG_MOD_WRITE        0x20
#define MM_DBG_CHECK_PTE        0x40
#define MM_DBG_VAD_CONFLICT     0x80
#define MM_DBG_SECTIONS         0x100
#define MM_DBG_STOP_ON_WOW64_ACCVIO   0x200
#define MM_DBG_SYS_PTES         0x400
#define MM_DBG_CLEAN_PROCESS    0x800
#define MM_DBG_COLLIDED_PAGE    0x1000
#define MM_DBG_DUMP_BOOT_PTES   0x2000
#define MM_DBG_FORK             0x4000
#define MM_DBG_DIR_BASE         0x8000
#define MM_DBG_FLUSH_SECTION    0x10000
#define MM_DBG_PRINTS_MODWRITES 0x20000
#define MM_DBG_PAGE_IN_LIST     0x40000
#define MM_DBG_CHECK_PFN_LOCK   0x80000
#define MM_DBG_PRIVATE_PAGES    0x100000
#define MM_DBG_WALK_VAD_TREE    0x200000
#define MM_DBG_SWAP_PROCESS     0x400000
#define MM_DBG_LOCK_CODE        0x800000
#define MM_DBG_STOP_ON_ACCVIO   0x1000000
#define MM_DBG_PAGE_REF_COUNT   0x2000000
#define MM_DBG_SHOW_FAULTS      0x40000000
#define MM_DBG_SESSIONS         0x80000000

//
// If the PTE.protection & MM_COPY_ON_WRITE_MASK == MM_COPY_ON_WRITE_MASK
// then the PTE is copy on write.
//

#define MM_COPY_ON_WRITE_MASK  5

extern ULONG MmProtectToValue[32];

extern
#if (defined(_WIN64) || defined(_X86PAE_))
ULONGLONG
#else
ULONG
#endif
MmProtectToPteMask[32];
extern ULONG MmMakeProtectNotWriteCopy[32];
extern ACCESS_MASK MmMakeSectionAccess[8];
extern ACCESS_MASK MmMakeFileAccess[8];


//
// Time constants
//

extern const LARGE_INTEGER MmSevenMinutes;
const extern LARGE_INTEGER MmOneSecond;
const extern LARGE_INTEGER MmTwentySeconds;
const extern LARGE_INTEGER MmSeventySeconds;
const extern LARGE_INTEGER MmShortTime;
const extern LARGE_INTEGER MmHalfSecond;
const extern LARGE_INTEGER Mm30Milliseconds;
extern LARGE_INTEGER MmCriticalSectionTimeout;

//
// A month's worth
//

extern ULONG MmCritsectTimeoutSeconds;

//
// this is the csrss process !
//

extern PEPROCESS ExpDefaultErrorPortProcess;

extern SIZE_T MmExtendedCommit;

extern SIZE_T MmTotalProcessCommit;

#if !defined(_WIN64)
extern LIST_ENTRY MmProcessList;
extern PMMPTE MiLargePageHyperPte;
extern PMMPTE MiInitialSystemPageDirectory;
#endif

//
// The total number of pages needed for the loader to successfully hibernate.
//

extern PFN_NUMBER MmHiberPages;

//
//  The counters and reasons to retry IO to protect against verifier induced
//  failures and temporary conditions.
//

extern ULONG MiIoRetryMask;
extern ULONG MiFaultRetryMask;
extern ULONG MiUserFaultRetryMask;

#define MmIsRetryIoStatus(S) (((S) == STATUS_INSUFFICIENT_RESOURCES) || \
                              ((S) == STATUS_WORKING_SET_QUOTA) ||      \
                              ((S) == STATUS_NO_MEMORY))

#if defined (_MI_MORE_THAN_4GB_)

extern PFN_NUMBER MiNoLowMemory;

#if defined (_WIN64)
#define MI_MAGIC_4GB_RECLAIM     0xffffedf0
#else
#define MI_MAGIC_4GB_RECLAIM     0xffedf0
#endif

#define MI_LOWMEM_MAGIC_BIT     (0x80000000)

extern PRTL_BITMAP MiLowMemoryBitMap;
#endif

//
// This is a version of COMPUTE_PAGES_SPANNED that works for 32 and 64 ranges.
//

#define MI_COMPUTE_PAGES_SPANNED(Va, Size) \
    ((((ULONG_PTR)(Va) & (PAGE_SIZE -1)) + (Size) + (PAGE_SIZE - 1)) >> PAGE_SHIFT)

//++
//
// ULONG
// MI_CONVERT_FROM_PTE_PROTECTION (
//     IN ULONG PROTECTION_MASK
//     )
//
// Routine Description:
//
//  This routine converts a PTE protection into a Protect value.
//
// Arguments:
//
//
// Return Value:
//
//     Returns the
//
//--

#define MI_CONVERT_FROM_PTE_PROTECTION(PROTECTION_MASK)      \
                                     (MmProtectToValue[PROTECTION_MASK])

#define MI_IS_PTE_PROTECTION_COPY_WRITE(PROTECTION_MASK)  \
   (((PROTECTION_MASK) & MM_COPY_ON_WRITE_MASK) == MM_COPY_ON_WRITE_MASK)

//++
//
// ULONG
// MI_ROUND_TO_64K (
//     IN ULONG LENGTH
//     )
//
// Routine Description:
//
//
// The ROUND_TO_64k macro takes a LENGTH in bytes and rounds it up to a multiple
// of 64K.
//
// Arguments:
//
//     LENGTH - LENGTH in bytes to round up to 64k.
//
// Return Value:
//
//     Returns the LENGTH rounded up to a multiple of 64k.
//
//--

#define MI_ROUND_TO_64K(LENGTH)  (((LENGTH) + X64K - 1) & ~((ULONG_PTR)X64K - 1))

extern ULONG MiLastVadBit;

//++
//
// ULONG
// MI_ROUND_TO_SIZE (
//     IN ULONG LENGTH,
//     IN ULONG ALIGNMENT
//     )
//
// Routine Description:
//
//
// The ROUND_TO_SIZE macro takes a LENGTH in bytes and rounds it up to a
// multiple of the alignment.
//
// Arguments:
//
//     LENGTH - LENGTH in bytes to round up to.
//
//     ALIGNMENT - alignment to round to, must be a power of 2, e.g, 2**n.
//
// Return Value:
//
//     Returns the LENGTH rounded up to a multiple of the alignment.
//
//--

#define MI_ROUND_TO_SIZE(LENGTH,ALIGNMENT)     \
                    (((LENGTH) + ((ALIGNMENT) - 1)) & ~((ALIGNMENT) - 1))

//++
//
// PVOID
// MI_64K_ALIGN (
//     IN PVOID VA
//     )
//
// Routine Description:
//
//
// The MI_64K_ALIGN macro takes a virtual address and returns a 64k-aligned
// virtual address for that page.
//
// Arguments:
//
//     VA - Virtual address.
//
// Return Value:
//
//     Returns the 64k aligned virtual address.
//
//--

#define MI_64K_ALIGN(VA) ((PVOID)((ULONG_PTR)(VA) & ~((LONG)X64K - 1)))


//++
//
// PVOID
// MI_ALIGN_TO_SIZE (
//     IN PVOID VA
//     IN ULONG ALIGNMENT
//     )
//
// Routine Description:
//
//
// The MI_ALIGN_TO_SIZE macro takes a virtual address and returns a
// virtual address for that page with the specified alignment.
//
// Arguments:
//
//     VA - Virtual address.
//
//     ALIGNMENT - alignment to round to, must be a power of 2, e.g, 2**n.
//
// Return Value:
//
//     Returns the aligned virtual address.
//
//--

#define MI_ALIGN_TO_SIZE(VA,ALIGNMENT) ((PVOID)((ULONG_PTR)(VA) & ~((ULONG_PTR) ALIGNMENT - 1)))

//++
//
// LONGLONG
// MI_STARTING_OFFSET (
//     IN PSUBSECTION SUBSECT
//     IN PMMPTE PTE
//     )
//
// Routine Description:
//
//    This macro takes a pointer to a PTE within a subsection and a pointer
//    to that subsection and calculates the offset for that PTE within the
//    file.
//
// Arguments:
//
//     PTE - PTE within subsection.
//
//     SUBSECT - Subsection
//
// Return Value:
//
//     Offset for issuing I/O from.
//
//--

#define MI_STARTING_OFFSET(SUBSECT,PTE) \
           (((LONGLONG)((ULONG_PTR)((PTE) - ((SUBSECT)->SubsectionBase))) << PAGE_SHIFT) + \
             ((LONGLONG)((SUBSECT)->StartingSector) << MMSECTOR_SHIFT));


// NTSTATUS
// MiFindEmptyAddressRangeDown (
//    IN ULONG_PTR SizeOfRange,
//    IN PVOID HighestAddressToEndAt,
//    IN ULONG_PTR Alignment,
//    OUT PVOID *Base
//    )
//
//
// Routine Description:
//
//    The function examines the virtual address descriptors to locate
//    an unused range of the specified size and returns the starting
//    address of the range.  This routine looks from the top down.
//
// Arguments:
//
//    SizeOfRange - Supplies the size in bytes of the range to locate.
//
//    HighestAddressToEndAt - Supplies the virtual address to begin looking
//                            at.
//
//    Alignment - Supplies the alignment for the address.  Must be
//                 a power of 2 and greater than the page_size.
//
//Return Value:
//
//    Returns the starting address of a suitable range.
//

#define MiFindEmptyAddressRangeDown(Root,SizeOfRange,HighestAddressToEndAt,Alignment,Base) \
               (MiFindEmptyAddressRangeDownTree(                             \
                    (SizeOfRange),                                           \
                    (HighestAddressToEndAt),                                 \
                    (Alignment),                                             \
                    Root,                                                    \
                    (Base)))

// PMMVAD
// MiGetPreviousVad (
//     IN PMMVAD Vad
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically precedes the specified virtual
//     address descriptor.
//
// Arguments:
//
//     Vad - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     next address range, NULL if none.
//
//

#define MiGetPreviousVad(VAD) ((PMMVAD)MiGetPreviousNode((PMMADDRESS_NODE)(VAD)))


// PMMVAD
// MiGetNextVad (
//     IN PMMVAD Vad
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically follows the specified address range.
//
// Arguments:
//
//     VAD - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     next address range, NULL if none.
//

#define MiGetNextVad(VAD) ((PMMVAD)MiGetNextNode((PMMADDRESS_NODE)(VAD)))



// PMMVAD
// MiGetFirstVad (
//     Process
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically is first within the address space.
//
// Arguments:
//
//     Process - Specifies the process in which to locate the VAD.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     first address range, NULL if none.

#define MiGetFirstVad(Process) \
    ((PMMVAD)MiGetFirstNode(&Process->VadRoot))


LOGICAL
MiCheckForConflictingVadExistence (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

// PMMVAD
// MiCheckForConflictingVad (
//     IN PVOID StartingAddress,
//     IN PVOID EndingAddress
//     )
//
// Routine Description:
//
//     The function determines if any addresses between a given starting and
//     ending address is contained within a virtual address descriptor.
//
// Arguments:
//
//     StartingAddress - Supplies the virtual address to locate a containing
//                       descriptor.
//
//     EndingAddress - Supplies the virtual address to locate a containing
//                       descriptor.
//
// Return Value:
//
//     Returns a pointer to the first conflicting virtual address descriptor
//     if one is found, otherwise a NULL value is returned.
//

#define MiCheckForConflictingVad(CurrentProcess,StartingAddress,EndingAddress) \
    ((PMMVAD)MiCheckForConflictingNode(                                   \
                    MI_VA_TO_VPN(StartingAddress),                        \
                    MI_VA_TO_VPN(EndingAddress),                          \
                    &CurrentProcess->VadRoot))

// PMMCLONE_DESCRIPTOR
// MiGetNextClone (
//     IN PMMCLONE_DESCRIPTOR Clone
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically follows the specified address range.
//
// Arguments:
//
//     Clone - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     next address range, NULL if none.
//
//

#define MiGetNextClone(CLONE) \
 ((PMMCLONE_DESCRIPTOR)MiGetNextNode((PMMADDRESS_NODE)(CLONE)))



// PMMCLONE_DESCRIPTOR
// MiGetPreviousClone (
//     IN PMMCLONE_DESCRIPTOR Clone
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically precedes the specified virtual
//     address descriptor.
//
// Arguments:
//
//     Clone - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     next address range, NULL if none.


#define MiGetPreviousClone(CLONE)  \
             ((PMMCLONE_DESCRIPTOR)MiGetPreviousNode((PMMADDRESS_NODE)(CLONE)))



// PMMCLONE_DESCRIPTOR
// MiGetFirstClone (
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically is first within the address space.
//
// Arguments:
//
//     None.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     first address range, NULL if none.
//


#define MiGetFirstClone(_CurrentProcess) \
        (((PMM_AVL_TABLE)(_CurrentProcess->CloneRoot))->NumberGenericTableElements == 0 ? NULL : (PMMCLONE_DESCRIPTOR)MiGetFirstNode((PMM_AVL_TABLE)(_CurrentProcess->CloneRoot)))



// VOID
// MiInsertClone (
//     IN PMMCLONE_DESCRIPTOR Clone
//     )
//
// Routine Description:
//
//     This function inserts a virtual address descriptor into the tree and
//     reorders the splay tree as appropriate.
//
// Arguments:
//
//     Clone - Supplies a pointer to a virtual address descriptor.
//
//
// Return Value:
//
//     None.
//

#define MiInsertClone(_CurrentProcess, CLONE) \
    {                                           \
        ASSERT ((CLONE)->NumberOfPtes != 0);     \
        ASSERT (_CurrentProcess->CloneRoot != NULL); \
        MiInsertNode(((PMMADDRESS_NODE)(CLONE)),(PMM_AVL_TABLE)(_CurrentProcess->CloneRoot)); \
    }




// VOID
// MiRemoveClone (
//     IN PMMCLONE_DESCRIPTOR Clone
//     )
//
// Routine Description:
//
//     This function removes a virtual address descriptor from the tree and
//     reorders the splay tree as appropriate.
//
// Arguments:
//
//     Clone - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     None.
//

#define MiRemoveClone(_CurrentProcess, CLONE) \
    ASSERT (_CurrentProcess->CloneRoot != NULL); \
    ASSERT (((PMM_AVL_TABLE)_CurrentProcess->CloneRoot)->NumberGenericTableElements != 0); \
    MiRemoveNode((PMMADDRESS_NODE)(CLONE),(PMM_AVL_TABLE)(_CurrentProcess->CloneRoot));



// PMMCLONE_DESCRIPTOR
// MiLocateCloneAddress (
//     IN PVOID VirtualAddress
//     )
//
// /*++
//
// Routine Description:
//
//     The function locates the virtual address descriptor which describes
//     a given address.
//
// Arguments:
//
//     VirtualAddress - Supplies the virtual address to locate a descriptor
//                      for.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor which contains
//     the supplied virtual address or NULL if none was located.
//

#define MiLocateCloneAddress(_CurrentProcess, VA)                           \
    (_CurrentProcess->CloneRoot ?                                           \
        ((PMMCLONE_DESCRIPTOR)MiLocateAddressInTree(((ULONG_PTR)VA),        \
                   (PMM_AVL_TABLE)(_CurrentProcess->CloneRoot))) :     \
        NULL)


#define MI_VA_TO_PAGE(va) ((ULONG_PTR)(va) >> PAGE_SHIFT)

#define MI_VA_TO_VPN(va)  ((ULONG_PTR)(va) >> PAGE_SHIFT)

#define MI_VPN_TO_VA(vpn)  (PVOID)((vpn) << PAGE_SHIFT)

#define MI_VPN_TO_VA_ENDING(vpn)  (PVOID)(((vpn) << PAGE_SHIFT) | (PAGE_SIZE - 1))

#define MiGetByteOffset(va) ((ULONG_PTR)(va) & (PAGE_SIZE - 1))

#define MI_PFN_ELEMENT(index) (&MmPfnDatabase[index])

//
// Make a write-copy PTE, only writable.
//

#define MI_MAKE_PROTECT_NOT_WRITE_COPY(PROTECT) \
            (MmMakeProtectNotWriteCopy[PROTECT])

//
// Define macros to lock and unlock the PFN database.
//

#define MiLockPfnDatabase(OldIrql) \
    OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock)

#define MiUnlockPfnDatabase(OldIrql) \
    KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql)

#define MiLockPfnDatabaseAtDpcLevel() \
    KeAcquireQueuedSpinLockAtDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueuePfnLock])

#define MiUnlockPfnDatabaseFromDpcLevel() \
    KeReleaseQueuedSpinLockFromDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueuePfnLock])

#define MiReleasePfnLock() \
    KeReleaseQueuedSpinLockFromDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueuePfnLock])

#define MiLockSystemSpace(OldIrql) \
    OldIrql = KeAcquireQueuedSpinLock(LockQueueSystemSpaceLock)

#define MiUnlockSystemSpace(OldIrql) \
    KeReleaseQueuedSpinLock(LockQueueSystemSpaceLock, OldIrql)

#define MiLockSystemSpaceAtDpcLevel() \
    KeAcquireQueuedSpinLockAtDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueueSystemSpaceLock])

#define MiUnlockSystemSpaceFromDpcLevel() \
    KeReleaseQueuedSpinLockFromDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueueSystemSpaceLock])

#define MI_MAX_PFN_CALLERS  500

typedef struct _MMPFNTIMINGS {
    LARGE_INTEGER HoldTime;     // Low bit is set if another processor waited
    PVOID AcquiredAddress;
    PVOID ReleasedAddress;
} MMPFNTIMINGS, *PMMPFNTIMINGS;

extern ULONG MiPfnTimings;
extern PVOID MiPfnAcquiredAddress;
extern MMPFNTIMINGS MiPfnSorted[];
extern LARGE_INTEGER MiPfnAcquired;
extern LARGE_INTEGER MiPfnReleased;
extern LARGE_INTEGER MiPfnThreshold;

PVOID
MiGetExecutionAddress (
    VOID
    );

LARGE_INTEGER
MiQueryPerformanceCounter (
    IN PLARGE_INTEGER PerformanceFrequency 
    );

VOID
MiAddLockToTable (
    IN PVOID AcquireAddress,
    IN PVOID ReleaseAddress,
    IN LARGE_INTEGER HoldTime
    );

#if defined(_X86_) || defined(_AMD64_)
#define MI_GET_EXECUTION_ADDRESS(varname) varname = MiGetExecutionAddress();
#else
#define MI_GET_EXECUTION_ADDRESS(varname) varname = NULL;
#endif

// #define _MI_INSTRUMENT_PFN 1

#if defined (_MI_INSTRUMENT_PFN)

#define LOCK_PFN_TIMESTAMP()                                \
        {                                                   \
            MiPfnAcquired = MiQueryPerformanceCounter (NULL);\
            MI_GET_EXECUTION_ADDRESS(MiPfnAcquiredAddress); \
        }

#define UNLOCK_PFN_TIMESTAMP()                                  \
        {                                                       \
            PVOID ExecutionAddress;                             \
            LARGE_INTEGER PfnHoldTime;                          \
                                                                \
            MiPfnReleased = MiQueryPerformanceCounter (NULL);   \
            MI_GET_EXECUTION_ADDRESS(ExecutionAddress);         \
            PfnHoldTime.QuadPart = (MiPfnReleased.QuadPart - MiPfnAcquired.QuadPart) & ~0x1; \
            MiAddLockToTable (MiPfnAcquiredAddress, ExecutionAddress, PfnHoldTime); \
        }
#else
#define LOCK_PFN_TIMESTAMP()
#define UNLOCK_PFN_TIMESTAMP()
#endif

#define LOCK_PFN(OLDIRQL) ASSERT (KeGetCurrentIrql() <= APC_LEVEL); \
                          MiLockPfnDatabase(OLDIRQL);               \
                          LOCK_PFN_TIMESTAMP();

#define UNLOCK_PFN(OLDIRQL)                                        \
    ASSERT (OLDIRQL <= APC_LEVEL);                                 \
    UNLOCK_PFN_TIMESTAMP();                                        \
    MiUnlockPfnDatabase(OLDIRQL);                                  \
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

#define LOCK_PFN2(OLDIRQL) ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL); \
                          MiLockPfnDatabase(OLDIRQL);               \
                          LOCK_PFN_TIMESTAMP();

#define UNLOCK_PFN2(OLDIRQL)                                       \
    ASSERT (OLDIRQL <= DISPATCH_LEVEL);                            \
    UNLOCK_PFN_TIMESTAMP();                                        \
    MiUnlockPfnDatabase(OLDIRQL);                                  \
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

#define LOCK_PFN_AT_DPC() ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL); \
                          MiLockPfnDatabaseAtDpcLevel();                 \
                          LOCK_PFN_TIMESTAMP();

#define UNLOCK_PFN_FROM_DPC()                                      \
    UNLOCK_PFN_TIMESTAMP();                                        \
    MiUnlockPfnDatabaseFromDpcLevel();                             \
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

#define UNLOCK_PFN_AND_THEN_WAIT(OLDIRQL)                          \
                {                                                  \
                    KIRQL XXX;                                     \
                    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL); \
                    ASSERT (OLDIRQL <= APC_LEVEL);                 \
                    KiLockDispatcherDatabase (&XXX);               \
                    UNLOCK_PFN_TIMESTAMP();                        \
                    MiReleasePfnLock();                            \
                    (KeGetCurrentThread())->WaitIrql = OLDIRQL;    \
                    (KeGetCurrentThread())->WaitNext = TRUE;       \
                }

extern KMUTANT MmSystemLoadLock;

#if DBG
#define SYSLOAD_LOCK_OWNED_BY_ME()      ASSERT (MmSystemLoadLock.OwnerThread == KeGetCurrentThread())
#else
#define SYSLOAD_LOCK_OWNED_BY_ME()
#endif

#if DBG

#if defined (_MI_COMPRESSION)

extern KIRQL MiCompressionIrql;

#define MM_PFN_LOCK_ASSERT() \
    if (MmDebug & 0x80000) { \
        KIRQL _OldIrql; \
        _OldIrql = KeGetCurrentIrql(); \
        ASSERT ((_OldIrql == DISPATCH_LEVEL) || \
                ((MiCompressionIrql != 0) && (_OldIrql == MiCompressionIrql))); \
    }

#else

#define MM_PFN_LOCK_ASSERT() \
    if (MmDebug & 0x80000) { \
        KIRQL _OldIrql; \
        _OldIrql = KeGetCurrentIrql(); \
        ASSERT (_OldIrql == DISPATCH_LEVEL); \
    }

#endif

extern PETHREAD MiExpansionLockOwner;

#define MM_SET_EXPANSION_OWNER()  ASSERT (MiExpansionLockOwner == NULL); \
                                  MiExpansionLockOwner = PsGetCurrentThread();

#define MM_CLEAR_EXPANSION_OWNER()  ASSERT (MiExpansionLockOwner == PsGetCurrentThread()); \
                                    MiExpansionLockOwner = NULL;

#else
#define MM_PFN_LOCK_ASSERT()
#define MM_SET_EXPANSION_OWNER()
#define MM_CLEAR_EXPANSION_OWNER()
#endif //DBG


#define LOCK_EXPANSION(OLDIRQL)     ASSERT (KeGetCurrentIrql() <= APC_LEVEL); \
                                ExAcquireSpinLock (&MmExpansionLock, &OLDIRQL);\
                                MM_SET_EXPANSION_OWNER ();

#define UNLOCK_EXPANSION(OLDIRQL)    MM_CLEAR_EXPANSION_OWNER (); \
                                ExReleaseSpinLock (&MmExpansionLock, OLDIRQL); \
                                ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

#define LOCK_EXPANSION2(OLDIRQL)     ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL); \
                                ExAcquireSpinLock (&MmExpansionLock, &OLDIRQL);\
                                MM_SET_EXPANSION_OWNER ();

#define UNLOCK_EXPANSION2(OLDIRQL)    MM_CLEAR_EXPANSION_OWNER (); \
                                ExReleaseSpinLock (&MmExpansionLock, OLDIRQL); \
                                ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

// #define _MI_INSTRUMENT_WS 1

#if defined (_MI_INSTRUMENT_WS)

#if defined (_MI_INSTRUMENT_PFN)
error - cannot enable both WS & PFN tracing concurrently yet.
#endif

extern KSPIN_LOCK MiInstrumentationLock;
extern EPROCESS MiSystemCacheDummyProcess;

#define LOCK_WS_TIMESTAMP(PROCESS)                                      \
        if (PROCESS != NULL) {                                          \
            LARGE_INTEGER TimeNow;                                      \
            TimeNow = MiQueryPerformanceCounter (NULL);                 \
            PROCESS->Spare0 = (PVOID) (ULONG_PTR) TimeNow.LowPart;      \
            PROCESS->Spare1 = (PVOID) MiGetExecutionAddress();          \
        }

#define UNLOCK_WS_TIMESTAMP(PROCESS)                                    \
        if (PROCESS != NULL) {                                          \
            PVOID ExecutionAddress;                                     \
            LARGE_INTEGER WsHoldTime;                                   \
            LARGE_INTEGER WsReleased;                                   \
                                                                        \
            WsReleased = MiQueryPerformanceCounter (NULL);              \
            MI_GET_EXECUTION_ADDRESS(ExecutionAddress);                 \
            WsHoldTime.QuadPart = ((ULONG_PTR)WsReleased.LowPart - (ULONG_PTR)PROCESS->Spare0) & ~0x1; \
            MiAddLockToTable (PROCESS->Spare1, ExecutionAddress, WsHoldTime); \
        }

#else
#define LOCK_WS_TIMESTAMP(PROCESS)
#define UNLOCK_WS_TIMESTAMP(PROCESS)
#endif

#define MM_WS_LOCK_ASSERT(WSINFO)                               \
        ASSERT (KeGetCurrentThread () == KeGetOwnerGuardedMutex (&(WSINFO)->WorkingSetMutex))

//
// System working set synchronization definitions.
//

#define MM_SYSTEM_WS_LOCK_TIMESTAMP()                           \
        LOCK_WS_TIMESTAMP(((PEPROCESS)&MiSystemCacheDummyProcess));

#define MM_SYSTEM_WS_UNLOCK_TIMESTAMP()                         \
        UNLOCK_WS_TIMESTAMP(((PEPROCESS)&MiSystemCacheDummyProcess));

#define LOCK_SYSTEM_WS(_Thread)                                         \
            KeAcquireGuardedMutex (&MmSystemCacheWs.WorkingSetMutex);   \
            MM_SYSTEM_WS_LOCK_TIMESTAMP();

#define UNLOCK_SYSTEM_WS()                                              \
            MM_SYSTEM_WS_UNLOCK_TIMESTAMP();                            \
            KeReleaseGuardedMutex (&MmSystemCacheWs.WorkingSetMutex);

//
// Generic working set synchronization definitions.
//

#define LOCK_WORKING_SET(WSINFO)                                        \
            ASSERT (MI_IS_SESSION_ADDRESS(WSINFO) == FALSE);            \
            KeAcquireGuardedMutex (&(WSINFO)->WorkingSetMutex);         \

#define UNLOCK_WORKING_SET(WSINFO)                                      \
            ASSERT (MI_IS_SESSION_ADDRESS(WSINFO) == FALSE);            \
            KeReleaseGuardedMutex (&(WSINFO)->WorkingSetMutex);

//
// Session working set synchronization definitions.
//

#define MM_SESSION_SPACE_WS_LOCK_ASSERT()                               \
            MM_WS_LOCK_ASSERT (&MmSessionSpace->Vm);

//
// Process working set synchronization definitions.
//

#define MI_WS_OWNER(PROCESS) (KeGetOwnerGuardedMutex (&(PROCESS)->Vm.WorkingSetMutex) == KeGetCurrentThread ())
#define MI_NOT_WS_OWNER(PROCESS) (!MI_WS_OWNER(PROCESS))

#define MI_IS_WS_UNSAFE(PROCESS) ((PROCESS)->Vm.Flags.AcquiredUnsafe == 1)

#define LOCK_WS(PROCESS)                                            \
            KeAcquireGuardedMutex (&((PROCESS)->Vm.WorkingSetMutex));      \
            LOCK_WS_TIMESTAMP(PROCESS);                             \
            ASSERT (!MI_IS_WS_UNSAFE(PROCESS));

#define LOCK_WS_UNSAFE(PROCESS)                                     \
            ASSERT (KeAreAllApcsDisabled () == TRUE);               \
            KeAcquireGuardedMutexUnsafe (&((PROCESS)->Vm.WorkingSetMutex));\
            LOCK_WS_TIMESTAMP(PROCESS);                             \
            ASSERT (!MI_IS_WS_UNSAFE(PROCESS));                     \
            (PROCESS)->Vm.Flags.AcquiredUnsafe = 1;

#define MI_MUST_BE_UNSAFE(PROCESS)                                  \
            ASSERT (KeGetCurrentIrql() <= APC_LEVEL);               \
            ASSERT (KeAreAllApcsDisabled () == TRUE);               \
            ASSERT (MI_WS_OWNER(PROCESS));                          \
            ASSERT (MI_IS_WS_UNSAFE(PROCESS));

#define MI_MUST_BE_SAFE(PROCESS)                                    \
            ASSERT (MI_WS_OWNER(PROCESS));                          \
            ASSERT (!MI_IS_WS_UNSAFE(PROCESS));

#define UNLOCK_WS(PROCESS)                                          \
            MI_MUST_BE_SAFE(PROCESS);                               \
            UNLOCK_WS_TIMESTAMP(PROCESS);                           \
            KeReleaseGuardedMutex (&((PROCESS)->Vm.WorkingSetMutex));

#define UNLOCK_WS_UNSAFE(PROCESS)                                   \
            MI_MUST_BE_UNSAFE(PROCESS);                             \
            ASSERT (KeAreAllApcsDisabled () == TRUE);               \
            (PROCESS)->Vm.Flags.AcquiredUnsafe = 0;                 \
            UNLOCK_WS_TIMESTAMP(PROCESS);                           \
            KeReleaseGuardedMutexUnsafe(&((PROCESS)->Vm.WorkingSetMutex)); \
            ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

//
// Address space synchronization definitions.
//

#define LOCK_ADDRESS_SPACE(PROCESS)                                  \
            KeAcquireGuardedMutex (&((PROCESS)->AddressCreationLock));

#define LOCK_WS_AND_ADDRESS_SPACE(PROCESS)                          \
        LOCK_ADDRESS_SPACE(PROCESS);                                \
        LOCK_WS_UNSAFE(PROCESS);

#define UNLOCK_WS_AND_ADDRESS_SPACE(PROCESS)                        \
        UNLOCK_WS_UNSAFE(PROCESS);                                  \
        UNLOCK_ADDRESS_SPACE(PROCESS);

#define UNLOCK_ADDRESS_SPACE(PROCESS)                               \
            KeReleaseGuardedMutex (&((PROCESS)->AddressCreationLock));

//
// The working set lock may have been acquired safely or unsafely.
// Release and reacquire it regardless.
//

#define UNLOCK_WS_REGARDLESS(PROCESS, WSHELDSAFE)                   \
            ASSERT (MI_WS_OWNER (PROCESS));                         \
            if (MI_IS_WS_UNSAFE (PROCESS)) {                        \
                UNLOCK_WS_UNSAFE (PROCESS);                         \
                WSHELDSAFE = FALSE;                                 \
            }                                                       \
            else {                                                  \
                UNLOCK_WS (PROCESS);                                \
                WSHELDSAFE = TRUE;                                  \
            }

#define LOCK_WS_REGARDLESS(PROCESS, WSHELDSAFE)                     \
            if (WSHELDSAFE == TRUE) {                               \
                LOCK_WS (PROCESS);                                  \
            }                                                       \
            else {                                                  \
                LOCK_WS_UNSAFE (PROCESS);                           \
            }

//
// Hyperspace synchronization definitions.
//

#define LOCK_HYPERSPACE(_Process, OLDIRQL)                      \
    ASSERT (_Process == PsGetCurrentProcess ());                \
    ExAcquireSpinLock (&_Process->HyperSpaceLock, OLDIRQL);

#define UNLOCK_HYPERSPACE(_Process, VA, OLDIRQL)                \
    ASSERT (_Process == PsGetCurrentProcess ());                \
    MiGetPteAddress(VA)->u.Long = 0;                            \
    ExReleaseSpinLock (&_Process->HyperSpaceLock, OLDIRQL);

#define LOCK_HYPERSPACE_AT_DPC(_Process)                        \
    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);              \
    ASSERT (_Process == PsGetCurrentProcess ());                \
    ExAcquireSpinLockAtDpcLevel (&_Process->HyperSpaceLock);

#define UNLOCK_HYPERSPACE_FROM_DPC(_Process, VA)                \
    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);              \
    ASSERT (_Process == PsGetCurrentProcess ());                \
    MiGetPteAddress(VA)->u.Long = 0;                            \
    ExReleaseSpinLockFromDpcLevel (&_Process->HyperSpaceLock);

#define MiUnmapPageInHyperSpace(_Process, VA, OLDIRQL) UNLOCK_HYPERSPACE(_Process, VA, OLDIRQL)

#define MiUnmapPageInHyperSpaceFromDpc(_Process, VA) UNLOCK_HYPERSPACE_FROM_DPC(_Process, VA)

#define ZERO_LARGE(LargeInteger)                \
        (LargeInteger).LowPart = 0;             \
        (LargeInteger).HighPart = 0;

#define NO_BITS_FOUND   ((ULONG)-1)

//++
//
// ULONG
// MI_CHECK_BIT (
//     IN PULONG ARRAY
//     IN ULONG BIT
//     )
//
// Routine Description:
//
//     The MI_CHECK_BIT macro checks to see if the specified bit is
//     set within the specified array.
//
// Arguments:
//
//     ARRAY - First element of the array to check.
//
//     BIT - bit number (first bit is 0) to check.
//
// Return Value:
//
//     Returns the value of the bit (0 or 1).
//
//--

#define MI_CHECK_BIT(ARRAY,BIT)  \
        (((ULONG)ARRAY[(BIT) / (sizeof(ULONG)*8)] >> ((BIT) & 0x1F)) & 1)


//++
//
// VOID
// MI_SET_BIT (
//     IN PULONG ARRAY
//     IN ULONG BIT
//     )
//
// Routine Description:
//
//     The MI_SET_BIT macro sets the specified bit within the
//     specified array.
//
// Arguments:
//
//     ARRAY - First element of the array to set.
//
//     BIT - bit number.
//
// Return Value:
//
//     None.
//
//--

#define MI_SET_BIT(ARRAY,BIT)  \
        ARRAY[(BIT) / (sizeof(ULONG)*8)] |= (1 << ((BIT) & 0x1F))


//++
//
// VOID
// MI_CLEAR_BIT (
//     IN PULONG ARRAY
//     IN ULONG BIT
//     )
//
// Routine Description:
//
//     The MI_CLEAR_BIT macro sets the specified bit within the
//     specified array.
//
// Arguments:
//
//     ARRAY - First element of the array to clear.
//
//     BIT - bit number.
//
// Return Value:
//
//     None.
//
//--

#define MI_CLEAR_BIT(ARRAY,BIT)  \
        ARRAY[(BIT) / (sizeof(ULONG)*8)] &= ~(1 << ((BIT) & 0x1F))

//
// These control the mirroring capabilities.
//

extern ULONG MmMirroring;

#define MM_MIRRORING_ENABLED    0x1 // Enable mirroring capabilities.
#define MM_MIRRORING_VERIFYING  0x2 // The HAL wants to verify the copy.

extern PRTL_BITMAP MiMirrorBitMap;
extern PRTL_BITMAP MiMirrorBitMap2;
extern LOGICAL MiMirroringActive;

#if defined (_WIN64)
#define MI_MAGIC_AWE_PTEFRAME   0x3ffedcbffffedcb
#else
#define MI_MAGIC_AWE_PTEFRAME   0x3ffedcb
#endif

#define MI_PFN_IS_AWE(Pfn1)                                     \
        ((Pfn1->u2.ShareCount <= 3) &&                          \
         (Pfn1->u3.e1.PageLocation == ActiveAndValid) &&        \
         (Pfn1->u4.VerifierAllocation == 0) &&                  \
         (Pfn1->u3.e1.LargeSessionAllocation == 0) &&           \
         (Pfn1->u3.e1.StartOfAllocation == 1) &&                \
         (Pfn1->u3.e1.EndOfAllocation == 1) &&                  \
         (Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME))

//
// The cache type definitions are carefully chosen to line up with the
// MEMORY_CACHING_TYPE definitions to ease conversions.  Any changes here must
// be reflected throughout the code.
//

typedef enum _MI_PFN_CACHE_ATTRIBUTE {
    MiNonCached,
    MiCached,
    MiWriteCombined,
    MiNotMapped
} MI_PFN_CACHE_ATTRIBUTE, *PMI_PFN_CACHE_ATTRIBUTE;

//
// This conversion array is unfortunately needed because not all
// hardware platforms support all possible cache values.  Note that
// the first range is for system RAM, the second range is for I/O space.
//

extern MI_PFN_CACHE_ATTRIBUTE MiPlatformCacheAttributes[2 * MmMaximumCacheType];

//++
//
// MI_PFN_CACHE_ATTRIBUTE
// MI_TRANSLATE_CACHETYPE (
//     IN MEMORY_CACHING_TYPE InputCacheType,
//     IN ULONG IoSpace
//     )
//
// Routine Description:
//
//     Returns the hardware supported cache type for the requested cachetype.
//
// Arguments:
//
//     InputCacheType - Supplies the desired cache type.
//
//     IoSpace - Supplies nonzero (not necessarily 1 though) if this is
//               I/O space, zero if it is main memory.
//
// Return Value:
//
//     The actual cache type.
//
//--
FORCEINLINE
MI_PFN_CACHE_ATTRIBUTE
MI_TRANSLATE_CACHETYPE(
    IN MEMORY_CACHING_TYPE InputCacheType,
    IN ULONG IoSpace
    )
{
    ASSERT (InputCacheType <= MmWriteCombined);

    if (IoSpace != 0) {
        IoSpace = MmMaximumCacheType;
    }
    return MiPlatformCacheAttributes[IoSpace + InputCacheType];
}

//++
//
// VOID
// MI_SET_CACHETYPE_TRANSLATION (
//     IN MEMORY_CACHING_TYPE    InputCacheType,
//     IN ULONG                  IoSpace,
//     IN MI_PFN_CACHE_ATTRIBUTE NewAttribute
//     )
//
// Routine Description:
//
//     This function sets the hardware supported cachetype for the
//     specified cachetype.
//
// Arguments:
//
//     InputCacheType - Supplies the desired cache type.
//
//     IoSpace - Supplies nonzero (not necessarily 1 though) if this is
//               I/O space, zero if it is main memory.
//
//     NewAttribute - Supplies the desired attribute.
//
// Return Value:
//
//     None.
//
//--
FORCEINLINE
VOID
MI_SET_CACHETYPE_TRANSLATION(
    IN MEMORY_CACHING_TYPE    InputCacheType,
    IN ULONG                  IoSpace,
    IN MI_PFN_CACHE_ATTRIBUTE NewAttribute
    )
{
    ASSERT (InputCacheType <= MmWriteCombined);

    if (IoSpace != 0) {
        IoSpace = MmMaximumCacheType;
    }

    MiPlatformCacheAttributes[IoSpace + InputCacheType] = NewAttribute;
}

#define MI_MARK_FRAME_AS_KSTACK(_PageFrameIndex) {      \
        PMMPFN _Pfn1;                                   \
        _Pfn1 = MI_PFN_ELEMENT (_PageFrameIndex);       \
        ASSERT (_Pfn1->u4.KernelStack == 0);            \
        _Pfn1->u4.KernelStack = 1;                      \
}

#define MI_UNMARK_PFN_AS_KSTACK(_Pfn1)                  \
        ASSERT (_Pfn1->u4.KernelStack == 1);            \
        _Pfn1->u4.KernelStack = 0;



//
// PFN database element.
//

//
// Define pseudo fields for start and end of allocation.
//

#define StartOfAllocation ReadInProgress

#define EndOfAllocation WriteInProgress

#define LargeSessionAllocation PrototypePte

typedef struct _MMPFNENTRY {
    ULONG Modified : 1;
    ULONG ReadInProgress : 1;
    ULONG WriteInProgress : 1;
    ULONG PrototypePte: 1;
    ULONG PageColor : 4;
    ULONG PageLocation : 3;
    ULONG RemovalRequested : 1;
    ULONG CacheAttribute : 2;
    ULONG Rom : 1;
    ULONG ParityError : 1;
    ULONG DontUse : 16;         // overlays USHORT for reference count field.
} MMPFNENTRY;

#if defined (_X86PAE_)
#pragma pack(1)
#endif

typedef struct _MMPFN {
    union {
        PFN_NUMBER Flink;
        WSLE_NUMBER WsIndex;
        PKEVENT Event;
        NTSTATUS ReadStatus;

        //
        // Note: NextStackPfn is actually used as SLIST_ENTRY, however
        // because of its alignment characteristics, using that type would
        // unnecessarily add padding to this structure.
        //

        SINGLE_LIST_ENTRY NextStackPfn;
    } u1;
    PMMPTE PteAddress;
    union {
        PFN_NUMBER Blink;
        ULONG_PTR ShareCount;
    } u2;
    union {
        MMPFNENTRY e1;
        struct {
            USHORT ShortFlags;
            USHORT ReferenceCount;
        } e2;
    } u3;
#if defined (_WIN64)
    ULONG UsedPageTableEntries;
#endif
    union {
        MMPTE OriginalPte;
        LONG AweReferenceCount;
    };
    union {
        ULONG_PTR EntireFrame;
        struct {
#if defined (_WIN64)
#define MM_NO_PTE_FRAME (0x3FFFFFFFFFFFFFF)
            ULONG_PTR PteFrame: 58;
#else
#define MM_NO_PTE_FRAME (0x3FFFFFF)
            ULONG_PTR PteFrame: 26;
#endif
            ULONG_PTR InPageError : 1;
            ULONG_PTR VerifierAllocation : 1;
            ULONG_PTR AweAllocation : 1;
            ULONG_PTR LockCharged : 1;      // maintained for DBG only
            ULONG_PTR KernelStack : 1;      // only for valid (not trans) pages
            ULONG_PTR MustBeCached : 1;
        };
    } u4;

} MMPFN, *PMMPFN;

#if defined (_X86PAE_)
#pragma pack()
#endif

//
// No multiplier reciprocal needs to be inlined because the compiler (using Oxt)
// automatically computes the correct number, avoiding the expensive divide
// instruction.
//

#define MI_PFN_ELEMENT_TO_INDEX(_Pfn) ((PFN_NUMBER)(((ULONG_PTR)(_Pfn) - (ULONG_PTR)MmPfnDatabase) / sizeof (MMPFN)))

PVOID
MiGetInstructionPointer (
    VOID
    );

// #define _MI_DEBUG_DIRTY 1         // Uncomment this for dirty bit logging

#if defined (_MI_DEBUG_DIRTY)

extern ULONG MiTrackDirtys;

#define MI_DIRTY_BACKTRACE_LENGTH 17

typedef struct _MI_DIRTY_TRACES {

    PETHREAD Thread;
    PEPROCESS Process;
    PMMPFN Pfn;
    PMMPTE PointerPte;
    ULONG_PTR CallerId;
    ULONG_PTR ShareCount;
    USHORT ShortFlags;
    USHORT ReferenceCount;
    PVOID StackTrace [MI_DIRTY_BACKTRACE_LENGTH];

} MI_DIRTY_TRACES, *PMI_DIRTY_TRACES;

extern LONG MiDirtyIndex;

extern PMI_DIRTY_TRACES MiDirtyTraces;

VOID
FORCEINLINE
MiSnapDirty (
    IN PMMPFN Pfn,
    IN ULONG NewValue,
    IN ULONG CallerId
    )
{
    PEPROCESS Process;
    PMI_DIRTY_TRACES Information;
    ULONG Index;
    ULONG Hash;

    if (MiDirtyTraces == NULL) {
        return;
    }

    Index = InterlockedIncrement (&MiDirtyIndex);
    Index &= (MiTrackDirtys - 1);
    Information = &MiDirtyTraces[Index];

    Process = PsGetCurrentProcess ();

    Information->Thread = PsGetCurrentThread ();
    Information->Process = Process; 
    Information->Pfn = Pfn;
    Information->PointerPte = Pfn->PteAddress;
    Information->CallerId = CallerId;
    Information->ShareCount = Pfn->u2.ShareCount;
    Information->ShortFlags = Pfn->u3.e2.ShortFlags;
    Information->ReferenceCount = Pfn->u3.e2.ReferenceCount;

    if (NewValue != 0) {
        Information->Process = (PEPROCESS) ((ULONG_PTR)Process | 0x1);
    }

    RtlZeroMemory (&Information->StackTrace[0], MI_DIRTY_BACKTRACE_LENGTH * sizeof(PVOID)); 

#if defined (_WIN64)
    if (KeAreAllApcsDisabled () == TRUE) {
        Information->StackTrace[1] = (PVOID) _ReturnAddress ();
        Information->StackTrace[0] = MiGetInstructionPointer ();
    }
    else
#endif
    RtlCaptureStackBackTrace (0, MI_DIRTY_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

#define MI_SNAP_DIRTY(_Pfn, _NewValue, _Callerid) MiSnapDirty(_Pfn, _NewValue, _Callerid)

#else
#define MI_SNAP_DIRTY(_Pfn, _NewValue, _Callerid)
#endif

#if DBG || defined (_MI_DEBUG_ALTPTE)
#define _MI_DEBUG_PTE 1         // Enable PTE change logging
#endif

#if defined (_MI_DEBUG_PTE)

#define MI_PTE_BACKTRACE_LENGTH 7

typedef struct _MI_PTE_TRACES {

    PETHREAD Thread;
    PEPROCESS Process;
    PMMPTE PointerPte;
    MMPTE PteContents;
    MMPTE NewPteContents;
    PVOID StackTrace [MI_PTE_BACKTRACE_LENGTH];

} MI_PTE_TRACES, *PMI_PTE_TRACES;

extern LONG MiPteIndex;

#define MI_PTE_TRACE_SIZE   0x4000

extern MI_PTE_TRACES MiPteTraces[MI_PTE_TRACE_SIZE];

extern LONG MiInDebugger;

VOID
FORCEINLINE
MiSnapPte (
    IN PMMPTE PointerPte,
    IN MMPTE NewValue
    )
{
    ULONG Hash;
    ULONG Index;
    PMI_PTE_TRACES Information;

    Index = InterlockedIncrement (&MiPteIndex);
    Index &= (MI_PTE_TRACE_SIZE - 1);
    Information = &MiPteTraces[Index];

    Information->Thread = PsGetCurrentThread ();
    Information->Process = PsGetCurrentProcess ();
    Information->PteContents = *PointerPte;
    Information->NewPteContents = NewValue;
    Information->PointerPte = PointerPte;

    if ((PointerPte < MiGetPteAddress (MmHighestUserAddress)) &&
        (PointerPte >= MiGetPteAddress (0))) {

        //
        // The current thread must own this process' working set mutex.
        //

        if (MiInDebugger == 0) {
            ASSERT (MI_WS_OWNER (PsGetCurrentProcess()));
        }

#if !defined(_IA64_)
        if ((NewValue.u.Hard.Valid == 1) && (NewValue.u.Hard.LargePage == 1)) {
            DbgPrint ("Marking PTE %p as large %p\n", PointerPte, NewValue.u.Long);
            DbgBreakPoint ();
        }
#endif
    }

    RtlZeroMemory (&Information->StackTrace[0], MI_PTE_BACKTRACE_LENGTH * sizeof(PVOID)); 

#if defined (_WIN64)
    if (KeAreAllApcsDisabled () == TRUE) {
        Information->StackTrace[1] = (PVOID) _ReturnAddress ();
        Information->StackTrace[0] = MiGetInstructionPointer ();
    }
    else
#endif
    RtlCaptureStackBackTrace (0, MI_PTE_BACKTRACE_LENGTH, Information->StackTrace, &Hash);

#if defined (_MI_DEBUG_ALTPTE)
    if (PsGetCurrentProcess()->Wow64Process != NULL) {
        MiLogPteInAltTrace ((PVOID) Information);
    }
#endif
}

#define MI_LOG_PTE_CHANGE(_PointerPte, _PteContents)    MiSnapPte(_PointerPte, _PteContents)

#else
#define MI_LOG_PTE_CHANGE(_PointerPte, _PteContents)
#endif


#define MI_DEBUGGER_WRITE_VALID_PTE_NEW_PROTECTION(_PointerPte, _PteContents) \
            InterlockedIncrement (&MiInDebugger);                             \
            MI_WRITE_VALID_PTE_NEW_PROTECTION(_PointerPte, _PteContents);     \
            InterlockedDecrement (&MiInDebugger);




#if 0
#define MI_STAMP_MODIFIED(Pfn,id)   (Pfn)->u4.Reserved = (id);
#else
#define MI_STAMP_MODIFIED(Pfn,id)
#endif

//++
//
// VOID
// MI_SET_MODIFIED (
//     IN PMMPFN Pfn,
//     IN ULONG NewValue,
//     IN ULONG CallerId
//     )
//
// Routine Description:
//
//     Set (or clear) the modified bit in the PFN database element.
//     The PFN lock must be held.
//
// Arguments:
//
//     Pfn - Supplies the PFN to operate on.
//
//     NewValue - Supplies 1 to set the modified bit, 0 to clear it.
//
//     CallerId - Supplies a caller ID useful for debugging purposes.
//
// Return Value:
//
//     None.
//
//--
//
#define MI_SET_MODIFIED(_Pfn, _NewValue, _CallerId)             \
            ASSERT ((_Pfn)->u3.e1.Rom == 0);                    \
            MI_SNAP_DIRTY (_Pfn, _NewValue, _CallerId);         \
            if ((_NewValue) != 0) {                             \
                MI_STAMP_MODIFIED (_Pfn, _CallerId);            \
            }                                                   \
            (_Pfn)->u3.e1.Modified = (_NewValue);

//
// ccNUMA is supported in multiprocessor PAE and WIN64 systems only.
//

#if (defined(_WIN64) || defined(_X86PAE_)) && !defined(NT_UP)
#define MI_MULTINODE

VOID
MiDetermineNode (
    IN PFN_NUMBER Page,
    IN PMMPFN Pfn
    );

#else

#define MiDetermineNode(x,y)    ((y)->u3.e1.PageColor = 0)

#endif

#if defined (_WIN64)

//
// Note there are some places where these portable macros are not currently
// used because we are not in the correct address space required.
//

#define MI_CAPTURE_USED_PAGETABLE_ENTRIES(PFN) \
        ASSERT ((PFN)->UsedPageTableEntries <= PTE_PER_PAGE); \
        (PFN)->OriginalPte.u.Soft.UsedPageTableEntries = (PFN)->UsedPageTableEntries;

#define MI_RETRIEVE_USED_PAGETABLE_ENTRIES_FROM_PTE(RBL, PTE) \
        ASSERT ((PTE)->u.Soft.UsedPageTableEntries <= PTE_PER_PAGE); \
        (RBL)->UsedPageTableEntries = (ULONG)(((PMMPTE)(PTE))->u.Soft.UsedPageTableEntries);

#define MI_ZERO_USED_PAGETABLE_ENTRIES_IN_INPAGE_SUPPORT(INPAGE_SUPPORT) \
            (INPAGE_SUPPORT)->UsedPageTableEntries = 0;

#define MI_ZERO_USED_PAGETABLE_ENTRIES_IN_PFN(PFN) (PFN)->UsedPageTableEntries = 0;

#define MI_INSERT_USED_PAGETABLE_ENTRIES_IN_PFN(PFN, INPAGE_SUPPORT) \
        ASSERT ((INPAGE_SUPPORT)->UsedPageTableEntries <= PTE_PER_PAGE); \
        (PFN)->UsedPageTableEntries = (INPAGE_SUPPORT)->UsedPageTableEntries;

#define MI_ZERO_USED_PAGETABLE_ENTRIES(PFN) \
        (PFN)->UsedPageTableEntries = 0;

#define MI_CHECK_USED_PTES_HANDLE(VA) \
        ASSERT (MiGetPdeAddress(VA)->u.Hard.Valid == 1);

#define MI_GET_USED_PTES_HANDLE(VA) \
        ((PVOID)MI_PFN_ELEMENT((PFN_NUMBER)MiGetPdeAddress(VA)->u.Hard.PageFrameNumber))

#define MI_GET_USED_PTES_FROM_HANDLE(PFN) \
        ((ULONG)(((PMMPFN)(PFN))->UsedPageTableEntries))

#define MI_INCREMENT_USED_PTES_BY_HANDLE(PFN) \
        (((PMMPFN)(PFN))->UsedPageTableEntries += 1); \
        ASSERT (((PMMPFN)(PFN))->UsedPageTableEntries <= PTE_PER_PAGE)

#define MI_INCREMENT_USED_PTES_BY_HANDLE_CLUSTER(PFN,INCR) \
        (((PMMPFN)(PFN))->UsedPageTableEntries += (ULONG)(INCR)); \
        ASSERT (((PMMPFN)(PFN))->UsedPageTableEntries <= PTE_PER_PAGE)

#define MI_DECREMENT_USED_PTES_BY_HANDLE(PFN) \
        (((PMMPFN)(PFN))->UsedPageTableEntries -= 1); \
        ASSERT (((PMMPFN)(PFN))->UsedPageTableEntries < PTE_PER_PAGE)

#else

#define MI_CAPTURE_USED_PAGETABLE_ENTRIES(PFN)
#define MI_RETRIEVE_USED_PAGETABLE_ENTRIES_FROM_PTE(RBL, PTE)
#define MI_ZERO_USED_PAGETABLE_ENTRIES_IN_INPAGE_SUPPORT(INPAGE_SUPPORT)
#define MI_ZERO_USED_PAGETABLE_ENTRIES_IN_PFN(PFN)

#define MI_INSERT_USED_PAGETABLE_ENTRIES_IN_PFN(PFN, INPAGE_SUPPORT)

#define MI_CHECK_USED_PTES_HANDLE(VA)

#define MI_GET_USED_PTES_HANDLE(VA) ((PVOID)&MmWorkingSetList->UsedPageTableEntries[MiGetPdeIndex(VA)])

#define MI_GET_USED_PTES_FROM_HANDLE(PDSHORT) ((ULONG)(*(PUSHORT)(PDSHORT)))

#define MI_INCREMENT_USED_PTES_BY_HANDLE(PDSHORT) \
    ((*(PUSHORT)(PDSHORT)) += 1); \
    ASSERT (((*(PUSHORT)(PDSHORT)) <= PTE_PER_PAGE))

#define MI_INCREMENT_USED_PTES_BY_HANDLE_CLUSTER(PDSHORT,INCR) \
    (*(PUSHORT)(PDSHORT)) = (USHORT)((*(PUSHORT)(PDSHORT)) + (INCR)); \
    ASSERT (((*(PUSHORT)(PDSHORT)) <= PTE_PER_PAGE))

#define MI_DECREMENT_USED_PTES_BY_HANDLE(PDSHORT) \
    ((*(PUSHORT)(PDSHORT)) -= 1); \
    ASSERT (((*(PUSHORT)(PDSHORT)) < PTE_PER_PAGE))

#endif

extern PFN_NUMBER MmDynamicPfn;

extern KGUARDED_MUTEX MmDynamicMemoryMutex;

//
// Cache attribute tracking for I/O space mappings.
//

#define MI_IO_BACKTRACE_LENGTH  6

typedef struct _MMIO_TRACKER {
    LIST_ENTRY ListEntry;
    PVOID BaseVa;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    PVOID StackTrace[MI_IO_BACKTRACE_LENGTH];
} MMIO_TRACKER, *PMMIO_TRACKER;

extern KSPIN_LOCK MmIoTrackerLock;
extern LIST_ENTRY MmIoHeader;

extern PCHAR MiCacheStrings[];

MI_PFN_CACHE_ATTRIBUTE
MiInsertIoSpaceMap (
    IN PVOID BaseVa,
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    );

//
// Total number of committed pages.
//

extern SIZE_T MmTotalCommittedPages;

extern SIZE_T MmTotalCommitLimit;

extern SIZE_T MmSharedCommit;

#if DBG

extern SPFN_NUMBER MiLockedCommit;

#define MI_INCREMENT_LOCKED_COMMIT()        \
            MiLockedCommit += 1;

#define MI_DECREMENT_LOCKED_COMMIT()        \
            ASSERT (MiLockedCommit > 0);    \
            MiLockedCommit -= 1;

#else

#define MI_INCREMENT_LOCKED_COMMIT()
#define MI_DECREMENT_LOCKED_COMMIT()

#endif

#if defined(_WIN64)

#define MiChargeCommitmentRegardless() \
    MI_INCREMENT_LOCKED_COMMIT(); \
    InterlockedIncrement64 ((PLONGLONG) &MmTotalCommittedPages);

#define MiReturnCommitmentRegardless() \
    MI_DECREMENT_LOCKED_COMMIT(); \
    InterlockedDecrement64 ((PLONGLONG) &MmTotalCommittedPages);

#else

#define MiChargeCommitmentRegardless() \
    MI_INCREMENT_LOCKED_COMMIT(); \
    InterlockedIncrement ((PLONG) &MmTotalCommittedPages);

#define MiReturnCommitmentRegardless() \
    MI_DECREMENT_LOCKED_COMMIT(); \
    InterlockedDecrement ((PLONG) &MmTotalCommittedPages);

#endif

extern ULONG MiChargeCommitmentFailures[3];   // referenced also in mi.h macros.

FORCEINLINE
LOGICAL
MiChargeCommitmentPfnLockHeld (
    IN LOGICAL Force
    )

/*++

Routine Description:

    This routine charges the specified commitment without attempting
    to expand paging files and waiting for the expansion.

Arguments:

    Force - Supplies TRUE if the lock is short-term and should be forced through
            if necessary.

Return Value:

    TRUE if the commitment was permitted, FALSE if not.

Environment:

    Kernel mode, PFN lock is held.

--*/

{
    if (MmTotalCommittedPages > MmTotalCommitLimit - 64) {

        if ((Force == 0) && (PsGetCurrentThread()->MemoryMaker == 0)) {
            MiChargeCommitmentFailures[2] += 1;
            return FALSE;
        }
    }

    //
    // No need to do an InterlockedCompareExchange for this, keep it fast.
    //

    MiChargeCommitmentRegardless ();
                                                             
    return TRUE;
}

extern PFN_NUMBER MmSystemLockPagesCount;

#if DBG

#define MI_LOCK_ID_COUNTER_MAX 64
extern ULONG MiLockIds[MI_LOCK_ID_COUNTER_MAX];

#define MI_MARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId)      \
         ASSERT (Pfn->u4.LockCharged == 0);          \
         ASSERT (CallerId < MI_LOCK_ID_COUNTER_MAX);    \
         MiLockIds[CallerId] += 1;                      \
         Pfn->u4.LockCharged = 1;

#define MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId)    \
         ASSERT (Pfn->u4.LockCharged == 1);          \
         ASSERT (CallerId < MI_LOCK_ID_COUNTER_MAX);    \
         MiLockIds[CallerId] += 1;                      \
         Pfn->u4.LockCharged = 0;

#else
#define MI_MARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId)
#define MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId)
#endif

FORCEINLINE
LOGICAL
MI_ADD_LOCKED_PAGE_CHARGE (
    IN PMMPFN Pfn1,
    IN LOGICAL Force,
    IN ULONG CallerId
    )

/*++

Routine Description:

    Charge the systemwide count of locked pages if this is the initial
    lock for this page (multiple concurrent locks are only charged once).

Arguments:

    Pfn - Supplies the PFN index to operate on.

    Force - Supplies TRUE if the lock is short-term and should be forced through
            if necessary.

    CallerId - Supplies the ID of the caller, only used in debug builds.

Return Value:

    TRUE if the charge succeeded, FALSE if not.

Environment:

    Kernel mode.  PFN lock held.

--*/

{
#if !DBG
    UNREFERENCED_PARAMETER (CallerId);
#endif

    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

    if (Pfn1->u3.e2.ReferenceCount == 1) {

        if (Pfn1->u2.ShareCount != 0) {

            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);

            if ((Pfn1->u3.e1.PrototypePte == 1) &&
                (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

                //
                // This is a filesystem-backed page - charge commit for
                // it as we have no way to tell when the caller will
                // unlock the page.
                //

                if (MiChargeCommitmentPfnLockHeld (Force) == FALSE) {
                    return FALSE;
                }
            }

            MI_MARK_PFN_AS_LOCK_CHARGED (Pfn1, CallerId);

            MmSystemLockPagesCount += 1;
        }
        else {
            ASSERT (Pfn1->u4.LockCharged == 1);
        }
    }

    return TRUE;
}

FORCEINLINE
LOGICAL
MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (
    IN PMMPFN Pfn1,
    IN LOGICAL Force,
    IN ULONG CallerId
    )

/*++

Routine Description:

    Charge the systemwide count of locked pages if this is the initial
    lock for this page (multiple concurrent locks are only charged once).

Arguments:

    Pfn - the PFN index to operate on.

    Force - Supplies TRUE if the lock is short-term and should be forced through
            if necessary.

    CallerId - the ID of the caller, only used in debug builds.

Return Value:

    TRUE if the charge succeeded, FALSE if not.

Environment:

    Kernel mode.  PFN lock held.

--*/

{
#if !DBG
    UNREFERENCED_PARAMETER (CallerId);
#endif

    ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);
    ASSERT (Pfn1->u2.ShareCount == 0);

    if (Pfn1->u3.e2.ReferenceCount == 0) {

        if ((Pfn1->u3.e1.PrototypePte == 1) &&
            (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

            //
            // This is a filesystem-backed page - charge commit for
            // it as we have no way to tell when the caller will
            // unlock the page.
            //

            if (MiChargeCommitmentPfnLockHeld (Force) == FALSE) {
                return FALSE;
            }
        }

        MI_MARK_PFN_AS_LOCK_CHARGED(Pfn1, CallerId);

        MmSystemLockPagesCount += 1;
    }

    return TRUE;
}

//++
//
// VOID
// MI_REMOVE_LOCKED_PAGE_CHARGE (
//     IN PMMPFN Pfn
//     )
//
// Routine Description:
//
//     Remove the charge from the systemwide count of locked pages if this
//     is the last lock for this page (multiple concurrent locks are only
//     charged once).
//
//     The PFN reference checks are carefully ordered so the most common case
//     is handled first, the next most common case next, etc.  The case of
//     a reference count of 2 occurs more than 1000x (yes, 3 orders of
//     magnitude) more than a reference count of 1.  And reference counts of >2
//     occur 3 orders of magnitude more frequently than reference counts of
//     exactly 1.
//
// Arguments:
//
//     Pfn - the PFN index to operate on.
//
// Return Value:
//
//     None.
//
//--
//
#define MI_REMOVE_LOCKED_PAGE_CHARGE(Pfn, CallerId)                         \
        ASSERT (Pfn->u3.e2.ReferenceCount != 0);                            \
        if (Pfn->u3.e2.ReferenceCount == 2) {                               \
            if (Pfn->u2.ShareCount >= 1) {                                  \
                ASSERT (Pfn->u3.e1.PageLocation == ActiveAndValid);         \
                MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId);               \
                if ((Pfn->u3.e1.PrototypePte == 1) &&                       \
                    (Pfn->OriginalPte.u.Soft.Prototype == 1)) {             \
                    MiReturnCommitmentRegardless();                         \
                }                                                           \
                MmSystemLockPagesCount -= 1;                                \
            }                                                               \
            else {                                                          \
                /*                                                          \
                 * There are multiple referencers to this page and the      \
                 * page is no longer valid in any process address space.    \
                 * The systemwide lock count can only be decremented        \
                 * by the last dereference.                                 \
                 */                                                         \
                NOTHING;                                                    \
            }                                                               \
        }                                                                   \
        else if (Pfn->u3.e2.ReferenceCount != 1) {                          \
            /*                                                              \
             * There are still multiple referencers to this page (it may    \
             * or may not be resident in any process address space).        \
             * Since the systemwide lock count can only be decremented      \
             * by the last dereference (and this is not it), no action      \
             * is taken here.                                               \
             */                                                             \
            ASSERT (Pfn->u3.e2.ReferenceCount > 2);                         \
            NOTHING;                                                        \
        }                                                                   \
        else {                                                              \
            /*                                                              \
             * This page has already been deleted from all process address  \
             * spaces.  It is sitting in limbo (not on any list) awaiting   \
             * this last dereference.                                       \
             */                                                             \
            ASSERT (Pfn->u3.e2.ReferenceCount == 1);                        \
            ASSERT (Pfn->u3.e1.PageLocation != ActiveAndValid);             \
            ASSERT (Pfn->u2.ShareCount == 0);                               \
            MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId);                   \
            if ((Pfn->u3.e1.PrototypePte == 1) &&                           \
                (Pfn->OriginalPte.u.Soft.Prototype == 1)) {                 \
                MiReturnCommitmentRegardless();                             \
            }                                                               \
            MmSystemLockPagesCount -= 1;                                    \
        }

//++
//
// VOID
// MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (
//     IN PMMPFN Pfn
//     )
//
// Routine Description:
//
//     Remove the charge from the systemwide count of locked pages if this
//     is the last lock for this page (multiple concurrent locks are only
//     charged once).
//
//     The PFN reference checks are carefully ordered so the most common case
//     is handled first, the next most common case next, etc.  The case of
//     a reference count of 2 occurs more than 1000x (yes, 3 orders of
//     magnitude) more than a reference count of 1.  And reference counts of >2
//     occur 3 orders of magnitude more frequently than reference counts of
//     exactly 1.
//
//     The PFN reference count is then decremented.
//
// Arguments:
//
//     Pfn - the PFN index to operate on.
//
// Return Value:
//
//     None.
//
//--
//
#define MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn, CallerId)              \
        ASSERT (Pfn->u3.e2.ReferenceCount != 0);                            \
        if (Pfn->u3.e2.ReferenceCount == 2) {                               \
            if (Pfn->u2.ShareCount >= 1) {                                  \
                ASSERT (Pfn->u3.e1.PageLocation == ActiveAndValid);         \
                MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId);               \
                if ((Pfn->u3.e1.PrototypePte == 1) &&                       \
                    (Pfn->OriginalPte.u.Soft.Prototype == 1)) {             \
                    MiReturnCommitmentRegardless();                         \
                }                                                           \
                MmSystemLockPagesCount -= 1;                                \
            }                                                               \
            else {                                                          \
                /*                                                          \
                 * There are multiple referencers to this page and the      \
                 * page is no longer valid in any process address space.    \
                 * The systemwide lock count can only be decremented        \
                 * by the last dereference.                                 \
                 */                                                         \
                NOTHING;                                                    \
            }                                                               \
            Pfn->u3.e2.ReferenceCount -= 1;                                 \
        }                                                                   \
        else if (Pfn->u3.e2.ReferenceCount != 1) {                          \
            /*                                                              \
             * There are still multiple referencers to this page (it may    \
             * or may not be resident in any process address space).        \
             * Since the systemwide lock count can only be decremented      \
             * by the last dereference (and this is not it), no action      \
             * is taken here.                                               \
             */                                                             \
            ASSERT (Pfn->u3.e2.ReferenceCount > 2);                         \
            Pfn->u3.e2.ReferenceCount -= 1;                                 \
        }                                                                   \
        else {                                                              \
            /*                                                              \
             * This page has already been deleted from all process address  \
             * spaces.  It is sitting in limbo (not on any list) awaiting   \
             * this last dereference.                                       \
             */                                                             \
            PFN_NUMBER _PageFrameIndex;                                     \
            ASSERT (Pfn->u3.e2.ReferenceCount == 1);                        \
            ASSERT (Pfn->u3.e1.PageLocation != ActiveAndValid);             \
            ASSERT (Pfn->u2.ShareCount == 0);                               \
            MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId);                   \
            if ((Pfn->u3.e1.PrototypePte == 1) &&                           \
                (Pfn->OriginalPte.u.Soft.Prototype == 1)) {                 \
                MiReturnCommitmentRegardless();                             \
            }                                                               \
            MmSystemLockPagesCount -= 1;                                    \
            _PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX(Pfn);                 \
            MiDecrementReferenceCount (Pfn, _PageFrameIndex);               \
        }

//++
//
// VOID
// MI_ZERO_WSINDEX (
//     IN PMMPFN Pfn
//     )
//
// Routine Description:
//
//     Zero the Working Set Index field of the argument PFN entry.
//     There is a subtlety here on systems where the WsIndex ULONG is
//     overlaid with an Event pointer and sizeof(ULONG) != sizeof(PKEVENT).
//     Note this will need to be updated if we ever decide to allocate bodies of
//     thread objects on 4GB boundaries.
//
// Arguments:
//
//     Pfn - the PFN index to operate on.
//
// Return Value:
//
//     None.
//
//--
//
#define MI_ZERO_WSINDEX(Pfn) \
    Pfn->u1.Event = NULL;

typedef enum _MMSHARE_TYPE {
    Normal,
    ShareCountOnly,
    AndValid
} MMSHARE_TYPE;

typedef struct _MMWSLE_HASH {
    PVOID Key;
    WSLE_NUMBER Index;
} MMWSLE_HASH, *PMMWSLE_HASH;

//++
//
// WSLE_NUMBER
// MI_WSLE_HASH (
//     IN ULONG_PTR VirtualAddress,
//     IN PMMWSL WorkingSetList
//     )
//
// Routine Description:
//
//     Hash the address
//
// Arguments:
//
//     VirtualAddress - the address to hash.
//
//     WorkingSetList - the working set to hash the address into.
//
// Return Value:
//
//     The hash key.
//
//--
//
#define MI_WSLE_HASH(Address, Wsl) \
    ((WSLE_NUMBER)(((ULONG_PTR)PAGE_ALIGN(Address) >> (PAGE_SHIFT - 2)) % \
        ((Wsl)->HashTableSize - 1)))

//
// Working Set List Entry.
//

typedef struct _MMWSLENTRY {
    ULONG_PTR Valid : 1;
    ULONG_PTR LockedInWs : 1;
    ULONG_PTR LockedInMemory : 1;
    ULONG_PTR Protection : 5;
    ULONG_PTR SameProtectAsProto : 1;
    ULONG_PTR Direct : 1;
    ULONG_PTR Age : 2;
#if MM_VIRTUAL_PAGE_FILLER
    ULONG_PTR Filler : MM_VIRTUAL_PAGE_FILLER;
#endif
    ULONG_PTR VirtualPageNumber : MM_VIRTUAL_PAGE_SIZE;
} MMWSLENTRY;

typedef struct _MMWSLE {
    union {
        PVOID VirtualAddress;
        ULONG_PTR Long;
        MMWSLENTRY e1;
    } u1;
} MMWSLE;

#define MI_GENERATE_VALID_WSLE(Wsle)                   \
        ((PVOID)(ULONG_PTR)((Wsle)->u1.Long & (~(PAGE_SIZE - 1) | 0x1)))

#define MI_GET_PROTECTION_FROM_WSLE(Wsl) ((Wsl)->u1.e1.Protection)

typedef MMWSLE *PMMWSLE;

//
// Working Set List.  Must be quadword sized.
//

typedef struct _MMWSL {
    WSLE_NUMBER FirstFree;
    WSLE_NUMBER FirstDynamic;
    WSLE_NUMBER LastEntry;
    WSLE_NUMBER NextSlot;               // The next slot to trim
    PMMWSLE Wsle;
    WSLE_NUMBER LastInitializedWsle;
    WSLE_NUMBER NonDirectCount;
    PMMWSLE_HASH HashTable;
    ULONG HashTableSize;
    ULONG NumberOfCommittedPageTables;
    PVOID HashTableStart;
    PVOID HighestPermittedHashAddress;
    ULONG NumberOfImageWaiters;
    ULONG VadBitMapHint;

#if _WIN64
    PVOID HighestUserAddress;           // Maintained for wow64 processes only
#endif

#if defined(_MIALT4K_)
    PMMPTE HighestUserPte;
    PMMPTE HighestAltPte;
#endif

#if (_MI_PAGING_LEVELS >= 4)
    PULONG CommittedPageTables;

    ULONG NumberOfCommittedPageDirectories;
    PULONG CommittedPageDirectories;

    ULONG NumberOfCommittedPageDirectoryParents;
    ULONG CommittedPageDirectoryParents[(MM_USER_PAGE_DIRECTORY_PARENT_PAGES + sizeof(ULONG)*8-1)/(sizeof(ULONG)*8)];

#elif (_MI_PAGING_LEVELS >= 3)
    PULONG CommittedPageTables;

    ULONG NumberOfCommittedPageDirectories;
    ULONG CommittedPageDirectories[(MM_USER_PAGE_DIRECTORY_PAGES + sizeof(ULONG)*8-1)/(sizeof(ULONG)*8)];

#else

    //
    // This must be at the end.
    // Not used in system cache or session working set lists.
    //

    USHORT UsedPageTableEntries[MM_USER_PAGE_TABLE_PAGES];

    ULONG CommittedPageTables[MM_USER_PAGE_TABLE_PAGES/(sizeof(ULONG)*8)];
#endif

} MMWSL, *PMMWSL;

// #define _MI_DEBUG_WSLE 1         // Enable WSLE change logging

#if defined (_MI_DEBUG_WSLE)

#define MI_WSLE_BACKTRACE_LENGTH 8

typedef struct _MI_WSLE_TRACES {

    PETHREAD Thread;
    PVOID Pad0;
    PMMWSLE Wsle;
    MMWSLE WsleContents;

    MMWSLE NewWsleContents;
    WSLE_NUMBER WorkingSetSize;
    WSLE_NUMBER Quota;
    WSLE_NUMBER LastInitializedWsle;

    PVOID StackTrace [MI_WSLE_BACKTRACE_LENGTH];

} MI_WSLE_TRACES, *PMI_WSLE_TRACES;

extern LONG MiWsleIndex;

#define MI_WSLE_TRACE_SIZE   0x1000

extern MI_WSLE_TRACES MiWsleTraces[MI_WSLE_TRACE_SIZE];

#if defined(_X86_)
extern PMMWSL MmWorkingSetList;
#endif

VOID
FORCEINLINE
MiSnapWsle (
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER WorkingSetIndex,
    IN MMWSLE WsleValue
    )
{
    PMMSUPPORT WsInfo;
    ULONG Hash;
    ULONG Index;
    PMI_WSLE_TRACES Information;
    PVOID MatchVa;
    WSLE_NUMBER j;
    PMMWSLE Wsle;
    PEPROCESS Process;

    if (WorkingSetList != MmWorkingSetList) {
        return;
    }

    Process = PsGetCurrentProcess ();
    Information = (PMI_WSLE_TRACES) Process->Spare3[0];
    if (Information == NULL) {
        return;
    }

    Wsle = WorkingSetList->Wsle;

    if (WsleValue.u1.e1.Valid == 1) {
        MatchVa = PAGE_ALIGN (WsleValue.u1.VirtualAddress);

        for (j = 0; j <= WorkingSetList->LastInitializedWsle; j += 1) {

            if ((Wsle->u1.e1.Valid == 1) &&
                (PAGE_ALIGN (Wsle->u1.VirtualAddress) == MatchVa) &&
                (j != WorkingSetIndex)) {

                DbgPrint ("MMWSLE2: DUP %p %x %x\n", WsleValue, WorkingSetIndex, j);
                DbgBreakPoint ();
            }
            Wsle += 1;
        }
    }

    WsInfo = &Process->Vm;

    Index = InterlockedIncrement ((PLONG)&Process->Spare3[1]);
    Index &= (MI_WSLE_TRACE_SIZE - 1);
    Information += Index;

    Information->Thread = PsGetCurrentThread ();
    Information->Wsle = &WorkingSetList->Wsle[WorkingSetIndex];
    Information->WsleContents = WorkingSetList->Wsle[WorkingSetIndex];
    Information->NewWsleContents = WsleValue;

    Information->WorkingSetSize = WsInfo->WorkingSetSize;
    Information->Quota = WorkingSetList->Quota;
    Information->LastInitializedWsle = WorkingSetList->LastInitializedWsle;

    if ((PointerPte < MiGetPteAddress (MmHighestUserAddress)) &&
        (PointerPte >= MiGetPteAddress (0))) {


        //
        // The current thread must own this process' working set mutex.
        //

        ASSERT (MI_WS_OWNER (PsGetCurrentProcess()));
    }

    RtlZeroMemory (&Information->StackTrace[0], MI_WSLE_BACKTRACE_LENGTH * sizeof(PVOID)); 

#if defined (_WIN64)
    if (KeAreAllApcsDisabled () == TRUE) {
        Information->StackTrace[1] = (PVOID) _ReturnAddress ();
        Information->StackTrace[0] = MiGetInstructionPointer ();
    }
    else
#endif
    RtlCaptureStackBackTrace (0, MI_WSLE_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

#define MI_LOG_WSLE_CHANGE(_WorkingSetList, _WorkingSetIndex, _WsleValue)    MiSnapWsle(_WorkingSetList, _WorkingSetIndex, _WsleValue)

#else
#define MI_LOG_WSLE_CHANGE(_WorkingSetList, _WorkingSetIndex, _WsleValue)
#endif

#if defined(_X86_)
extern PMMWSL MmWorkingSetList;
#endif

extern PKEVENT MiHighMemoryEvent;
extern PKEVENT MiLowMemoryEvent;

//
// The claim estimate of unused pages in a working set is limited
// to grow by this amount per estimation period.
//

#define MI_CLAIM_INCR 30

//
// The maximum number of different ages a page can be.
//

#define MI_USE_AGE_COUNT 4
#define MI_USE_AGE_MAX (MI_USE_AGE_COUNT - 1)

//
// If more than this "percentage" of the working set is estimated to
// be used then allow it to grow freely.
//

#define MI_REPLACEMENT_FREE_GROWTH_SHIFT 5

//
// If more than this "percentage" of the working set has been claimed
// then force replacement in low memory.
//

#define MI_REPLACEMENT_CLAIM_THRESHOLD_SHIFT 3

//
// If more than this "percentage" of the working set is estimated to
// be available then force replacement in low memory.
//

#define MI_REPLACEMENT_EAVAIL_THRESHOLD_SHIFT 3

//
// If while doing replacement a page is found of this age or older then
// replace it.  Otherwise the oldest is selected.
//

#define MI_IMMEDIATE_REPLACEMENT_AGE 2

//
// When trimming, use these ages for different passes.
//

#define MI_MAX_TRIM_PASSES 4
#define MI_PASS0_TRIM_AGE 2
#define MI_PASS1_TRIM_AGE 1
#define MI_PASS2_TRIM_AGE 1
#define MI_PASS3_TRIM_AGE 1
#define MI_PASS4_TRIM_AGE 0

//
// If not a forced trim, trim pages older than this age.
//

#define MI_TRIM_AGE_THRESHOLD 2

//
// This "percentage" of a claim is up for grabs in a foreground process.
//

#define MI_FOREGROUND_CLAIM_AVAILABLE_SHIFT 3

//
// This "percentage" of a claim is up for grabs in a background process.
//

#define MI_BACKGROUND_CLAIM_AVAILABLE_SHIFT 1

//++
//
// DWORD
// MI_CALC_NEXT_VALID_ESTIMATION_SLOT (
//     DWORD Previous,
//     DWORD Minimum,
//     DWORD Maximum,
//     MI_NEXT_ESTIMATION_SLOT_CONST NextEstimationSlotConst,
//     PMMWSLE Wsle
//     )
//
// Routine Description:
//
//      We iterate through the working set array in a non-sequential
//      manner so that the sample is independent of any aging or trimming.
//
//      This algorithm walks through the working set with a stride of
//      2^MiEstimationShift elements.
//
// Arguments:
//
//      Previous - Last slot used
//
//      Minimum - Minimum acceptable slot (ie. the first dynamic one)
//
//      Maximum - max slot number + 1
//
//      NextEstimationSlotConst - for this algorithm it contains the stride
//
//      Wsle - the working set array
//
// Return Value:
//
//      Next slot.
//
// Environment:
//
//      Kernel mode, APCs disabled, working set lock held and PFN lock held.
//
//--

typedef struct _MI_NEXT_ESTIMATION_SLOT_CONST {
    WSLE_NUMBER Stride;
} MI_NEXT_ESTIMATION_SLOT_CONST;


#define MI_CALC_NEXT_ESTIMATION_SLOT_CONST(NextEstimationSlotConst, WorkingSetList) \
    (NextEstimationSlotConst).Stride = 1 << MiEstimationShift;

#define MI_NEXT_VALID_ESTIMATION_SLOT(Previous, StartEntry, Minimum, Maximum, NextEstimationSlotConst, Wsle) \
    ASSERT(((Previous) >= Minimum) && ((Previous) <= Maximum)); \
    ASSERT(((StartEntry) >= Minimum) && ((StartEntry) <= Maximum)); \
    do { \
        (Previous) += (NextEstimationSlotConst).Stride; \
        if ((Previous) > Maximum) { \
            (Previous) = Minimum + ((Previous + 1) & (NextEstimationSlotConst.Stride - 1)); \
            StartEntry += 1; \
            (Previous) = StartEntry; \
        } \
        if ((Previous) > Maximum || (Previous) < Minimum) { \
            StartEntry = Minimum; \
            (Previous) = StartEntry; \
        } \
    } while (Wsle[Previous].u1.e1.Valid == 0);

//++
//
// WSLE_NUMBER
// MI_NEXT_VALID_AGING_SLOT (
//     DWORD Previous,
//     DWORD Minimum,
//     DWORD Maximum,
//     PMMWSLE Wsle
//     )
//
// Routine Description:
//
//      This finds the next slot to valid slot to age.  It walks
//      through the slots sequentialy.
//
// Arguments:
//
//      Previous - Last slot used
//
//      Minimum - Minimum acceptable slot (ie. the first dynamic one)
//
//      Maximum - Max slot number + 1
//
//      Wsle - the working set array
//
// Return Value:
//
//      None.
//
// Environment:
//
//      Kernel mode, APCs disabled, working set lock held and PFN lock held.
//
//--

#define MI_NEXT_VALID_AGING_SLOT(Previous, Minimum, Maximum, Wsle) \
    ASSERT(((Previous) >= Minimum) && ((Previous) <= Maximum)); \
    do { \
        (Previous) += 1; \
        if ((Previous) > Maximum) { \
            Previous = Minimum; \
        } \
    } while ((Wsle[Previous].u1.e1.Valid == 0));

//++
//
// ULONG
// MI_CALCULATE_USAGE_ESTIMATE (
//     IN PULONG SampledAgeCounts.
//     IN ULONG CounterShift
//     )
//
// Routine Description:
//
//      In Usage Estimation, we count the number of pages of each age in
//      a sample.  The function turns the SampledAgeCounts into an
//      estimate of the unused pages.
//
// Arguments:
//
//      SampledAgeCounts - counts of pages of each different age in the sample
//
//      CounterShift - shift necessary to apply sample to entire WS
//
// Return Value:
//
//      The number of pages to walk in the working set to get a good
//      estimate of the number available.
//
//--

#define MI_CALCULATE_USAGE_ESTIMATE(SampledAgeCounts, CounterShift) \
                (((SampledAgeCounts)[1] + \
                    (SampledAgeCounts)[2] + (SampledAgeCounts)[3]) \
                    << (CounterShift))

//++
//
// VOID
// MI_RESET_WSLE_AGE (
//     IN PMMPTE PointerPte,
//     IN PMMWSLE Wsle
//     )
//
// Routine Description:
//
//      Clear the age counter for the working set entry.
//
// Arguments:
//
//      PointerPte - pointer to the working set list entry's PTE.
//
//      Wsle - pointer to the working set list entry.
//
// Return Value:
//
//      None.
//
//--
#define MI_RESET_WSLE_AGE(PointerPte, Wsle) \
    (Wsle)->u1.e1.Age = 0;

//++
//
// ULONG
// MI_GET_WSLE_AGE (
//     IN PMMPTE PointerPte,
//     IN PMMWSLE Wsle
//     )
//
// Routine Description:
//
//      Clear the age counter for the working set entry.
//
// Arguments:
//
//      PointerPte - pointer to the working set list entry's PTE
//      Wsle - pointer to the working set list entry
//
// Return Value:
//
//      Age group of the working set entry
//
//--
#define MI_GET_WSLE_AGE(PointerPte, Wsle) \
    ((ULONG)((Wsle)->u1.e1.Age))

//++
//
// VOID
// MI_INC_WSLE_AGE (
//     IN PMMPTE PointerPte,
//     IN PMMWSLE Wsle,
//     )
//
// Routine Description:
//
//      Increment the age counter for the working set entry.
//
// Arguments:
//
//      PointerPte - pointer to the working set list entry's PTE.
//
//      Wsle - pointer to the working set list entry.
//
// Return Value:
//
//      None
//
//--

#define MI_INC_WSLE_AGE(PointerPte, Wsle) \
    if ((Wsle)->u1.e1.Age < 3) { \
        (Wsle)->u1.e1.Age += 1; \
    }

//++
//
// VOID
// MI_UPDATE_USE_ESTIMATE (
//     IN PMMPTE PointerPte,
//     IN PMMWSLE Wsle,
//     IN ULONG *SampledAgeCounts
//     )
//
// Routine Description:
//
//      Update the sampled age counts.
//
// Arguments:
//
//      PointerPte - pointer to the working set list entry's PTE.
//
//      Wsle - pointer to the working set list entry.
//
//      SampledAgeCounts - array of age counts to be updated.
//
// Return Value:
//
//      None
//
//--

#define MI_UPDATE_USE_ESTIMATE(PointerPte, Wsle, SampledAgeCounts) \
    (SampledAgeCounts)[(Wsle)->u1.e1.Age] += 1;

//++
//
// BOOLEAN
// MI_WS_GROWING_TOO_FAST (
//     IN PMMSUPPORT VmSupport
//     )
//
// Routine Description:
//
//      Limit the growth rate of processes as the
//      available memory approaches zero.  Note the caller must ensure that
//      MmAvailablePages is low enough so this calculation does not wrap.
//
// Arguments:
//
//      VmSupport - a working set.
//
// Return Value:
//
//      TRUE if the growth rate is too fast, FALSE otherwise.
//
//--

#define MI_WS_GROWING_TOO_FAST(VmSupport) \
    ((VmSupport)->GrowthSinceLastEstimate > \
        (((MI_CLAIM_INCR * (MmAvailablePages*MmAvailablePages)) / (64*64)) + 1))

#define SECTION_BASE_ADDRESS(_NtSection) \
    (*((PVOID *)&(_NtSection)->PointerToRelocations))

#define SECTION_LOCK_COUNT_POINTER(_NtSection) \
    ((PLONG)&(_NtSection)->NumberOfRelocations)

//
// Memory Management Object structures.
//

typedef enum _SECTION_CHECK_TYPE {
    CheckDataSection,
    CheckImageSection,
    CheckUserDataSection,
    CheckBothSection
} SECTION_CHECK_TYPE;

typedef struct _MMEXTEND_INFO {
    UINT64 CommittedSize;
    ULONG ReferenceCount;
} MMEXTEND_INFO, *PMMEXTEND_INFO;

typedef struct _SEGMENT_FLAGS {
    ULONG_PTR TotalNumberOfPtes4132 : 10;
    ULONG_PTR ExtraSharedWowSubsections : 1;
#if defined (_WIN64)
    ULONG_PTR Spare : 53;
#else
    ULONG_PTR Spare : 21;
#endif
} SEGMENT_FLAGS, *PSEGMENT_FLAGS;

typedef struct _SEGMENT {
    struct _CONTROL_AREA *ControlArea;
    ULONG TotalNumberOfPtes;
    ULONG NonExtendedPtes;
    ULONG WritableUserReferences;

    UINT64 SizeOfSegment;
    MMPTE SegmentPteTemplate;

    SIZE_T NumberOfCommittedPages;
    PMMEXTEND_INFO ExtendInfo;

    SEGMENT_FLAGS SegmentFlags;
    PVOID BasedAddress;

    //
    // The fields below are for image & pagefile-backed sections only.
    // Common fields are above and new common entries must be added to
    // both the SEGMENT and MAPPED_FILE_SEGMENT declarations.
    //

    union {
        SIZE_T ImageCommitment;     // for image-backed sections only
        PEPROCESS CreatingProcess;  // for pagefile-backed sections only
    } u1;

    union {
        PSECTION_IMAGE_INFORMATION ImageInformation;    // for images only
        PVOID FirstMappedVa;        // for pagefile-backed sections only
    } u2;

    PMMPTE PrototypePte;
    MMPTE ThePtes[MM_PROTO_PTE_ALIGNMENT / PAGE_SIZE];

} SEGMENT, *PSEGMENT;

typedef struct _MAPPED_FILE_SEGMENT {
    struct _CONTROL_AREA *ControlArea;
    ULONG TotalNumberOfPtes;
    ULONG NonExtendedPtes;
    ULONG WritableUserReferences;

    UINT64 SizeOfSegment;
    MMPTE SegmentPteTemplate;

    SIZE_T NumberOfCommittedPages;
    PMMEXTEND_INFO ExtendInfo;

    SEGMENT_FLAGS SegmentFlags;
    PVOID BasedAddress;

    struct _MSUBSECTION *LastSubsectionHint;

} MAPPED_FILE_SEGMENT, *PMAPPED_FILE_SEGMENT;

typedef struct _EVENT_COUNTER {
    SLIST_ENTRY ListEntry;
    ULONG RefCount;
    KEVENT Event;
} EVENT_COUNTER, *PEVENT_COUNTER;

typedef struct _MMSECTION_FLAGS {
    unsigned BeingDeleted : 1;
    unsigned BeingCreated : 1;
    unsigned BeingPurged : 1;
    unsigned NoModifiedWriting : 1;

    unsigned FailAllIo : 1;
    unsigned Image : 1;
    unsigned Based : 1;
    unsigned File : 1;

    unsigned Networked : 1;
    unsigned NoCache : 1;
    unsigned PhysicalMemory : 1;
    unsigned CopyOnWrite : 1;

    unsigned Reserve : 1;  // not a spare bit!
    unsigned Commit : 1;
    unsigned FloppyMedia : 1;
    unsigned WasPurged : 1;

    unsigned UserReference : 1;
    unsigned GlobalMemory : 1;
    unsigned DeleteOnClose : 1;
    unsigned FilePointerNull : 1;

    unsigned DebugSymbolsLoaded : 1;
    unsigned SetMappedFileIoComplete : 1;
    unsigned CollidedFlush : 1;
    unsigned NoChange : 1;

    unsigned HadUserReference : 1;
    unsigned ImageMappedInSystemSpace : 1;
    unsigned UserWritable : 1;
    unsigned Accessed : 1;

    unsigned GlobalOnlyPerSession : 1;
    unsigned Rom : 1;
    unsigned filler : 2;
} MMSECTION_FLAGS;

typedef struct _CONTROL_AREA {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    ULONG NumberOfSectionReferences;    // All section refs & image flushes
    ULONG NumberOfPfnReferences;        // valid + transition prototype PTEs
    ULONG NumberOfMappedViews;          // total # mapped views, including
                                        // system cache & system space views
    ULONG NumberOfSystemCacheViews;     // system cache views only
    ULONG NumberOfUserReferences;       // user section & view references
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    PFILE_OBJECT FilePointer;
    PEVENT_COUNTER WaitingForDeletion;
    USHORT ModifiedWriteCount;
    USHORT FlushInProgressCount;
} CONTROL_AREA, *PCONTROL_AREA;

typedef struct _LARGE_CONTROL_AREA {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    ULONG NumberOfSectionReferences;
    ULONG NumberOfPfnReferences;
    ULONG NumberOfMappedViews;
    USHORT NumberOfSubsections;
    USHORT FlushInProgressCount;
    ULONG NumberOfUserReferences;
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    PFILE_OBJECT FilePointer;
    PEVENT_COUNTER WaitingForDeletion;
    USHORT ModifiedWriteCount;
    USHORT NumberOfSystemCacheViews;
    PFN_NUMBER StartingFrame;       // only used if Flags.Rom == 1.
    LIST_ENTRY UserGlobalList;
    ULONG SessionId;
} LARGE_CONTROL_AREA, *PLARGE_CONTROL_AREA;

typedef struct _MMSUBSECTION_FLAGS {
    unsigned ReadOnly : 1;
    unsigned ReadWrite : 1;
    unsigned SubsectionStatic : 1;
    unsigned GlobalMemory: 1;
    unsigned Protection : 5;
    unsigned Spare : 1;
    unsigned StartingSector4132 : 10;   // 2 ** (42+12) == 4MB*4GB == 16K TB
    unsigned SectorEndOffset : 12;
} MMSUBSECTION_FLAGS;

typedef struct _SUBSECTION { // Must start on quadword boundary and be quad sized
    PCONTROL_AREA ControlArea;
    union {
        ULONG LongFlags;
        MMSUBSECTION_FLAGS SubsectionFlags;
    } u;
    ULONG StartingSector;
    ULONG NumberOfFullSectors;  // (4GB-1) * 4K == 16TB-4K limit per subsection
#if defined(_MIALT4K_)
    ULONG LastSplitPageProtection;   // Protection of last split page in this
                                     // subsection.  This must be saved here
                                     // because the final subsection may end
                                     // on a split (merged) page and so we
                                     // cannot just look forward to the next
                                     // subsection in that case to obtain
                                     // the correct permissions.
#endif
    PMMPTE SubsectionBase;
    ULONG UnusedPtes;
    ULONG PtesInSubsection;
    struct _SUBSECTION *NextSubsection;
} SUBSECTION, *PSUBSECTION;

extern const ULONG MMSECT;

//
// Accesses to MMSUBSECTION_FLAGS2 are synchronized via the PFN lock
// (unlike MMSUBSECTION_FLAGS access which is not lock protected at all).
//

typedef struct _MMSUBSECTION_FLAGS2 {
    unsigned SubsectionAccessed : 1;
    unsigned SubsectionConverted : 1;       // only needed for debug
    unsigned Reserved : 30;
} MMSUBSECTION_FLAGS2;

//
// Mapped data file subsection structure.  Not used for images
// or pagefile-backed shared memory.
//

typedef struct _MSUBSECTION { // Must start on quadword boundary and be quad sized
    PCONTROL_AREA ControlArea;
    union {
        ULONG LongFlags;
        MMSUBSECTION_FLAGS SubsectionFlags;
    } u;
    ULONG StartingSector;
    ULONG NumberOfFullSectors;  // (4GB-1) * 4K == 16TB-4K limit per subsection
    PMMPTE SubsectionBase;
    ULONG UnusedPtes;
    ULONG PtesInSubsection;
    struct _SUBSECTION *NextSubsection;
    LIST_ENTRY DereferenceList;
    ULONG_PTR NumberOfMappedViews;
    union {
        ULONG LongFlags2;
        MMSUBSECTION_FLAGS2 SubsectionFlags2;
    } u2;
} MSUBSECTION, *PMSUBSECTION;

#define MI_MAXIMUM_SECTION_SIZE ((UINT64)16*1024*1024*1024*1024*1024 - ((UINT64)1<<MM4K_SHIFT))

VOID
MiDecrementSubsections (
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection OPTIONAL
    );

NTSTATUS
MiAddViewsForSectionWithPfn (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

NTSTATUS
MiAddViewsForSection (
    IN PMSUBSECTION MappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    );

LOGICAL
MiReferenceSubsection (
    IN PMSUBSECTION MappedSubsection
    );

VOID
MiRemoveViewsFromSection (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

VOID
MiRemoveViewsFromSectionWithPfn (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

VOID
MiSubsectionConsistent(
    IN PSUBSECTION Subsection
    );

#if DBG
#define MI_CHECK_SUBSECTION(_subsection) MiSubsectionConsistent((PSUBSECTION)(_subsection))
#else
#define MI_CHECK_SUBSECTION(_subsection)
#endif

//++
//ULONG
//Mi4KStartForSubsection (
//    IN PLARGE_INTEGER address,
//    IN OUT PSUBSECTION subsection
//    );
//
// Routine Description:
//
//    This macro sets into the specified subsection the supplied information
//    indicating the start address (in 4K units) of this portion of the file.
//
// Arguments
//
//    address - Supplies the 64-bit address (in 4K units) of the start of this
//              portion of the file.
//
//    subsection - Supplies the subsection address to store the address in.
//
// Return Value:
//
//    None.
//
//--

#define Mi4KStartForSubsection(address, subsection)  \
   subsection->StartingSector = ((PLARGE_INTEGER)address)->LowPart; \
   subsection->u.SubsectionFlags.StartingSector4132 = \
        (((PLARGE_INTEGER)(address))->HighPart & 0x3ff);

//++
//ULONG
//Mi4KStartFromSubsection (
//    IN OUT PLARGE_INTEGER address,
//    IN PSUBSECTION subsection
//    );
//
// Routine Description:
//
//    This macro gets the start 4K offset from the specified subsection.
//
// Arguments
//
//    address - Supplies the 64-bit address (in 4K units) to place the
//              start of this subsection into.
//
//    subsection - Supplies the subsection address to get the address from.
//
// Return Value:
//
//    None.
//
//--

#define Mi4KStartFromSubsection(address, subsection)  \
   ((PLARGE_INTEGER)address)->LowPart = subsection->StartingSector; \
   ((PLARGE_INTEGER)address)->HighPart = subsection->u.SubsectionFlags.StartingSector4132;

typedef struct _MMDEREFERENCE_SEGMENT_HEADER {
    KSPIN_LOCK Lock;
    KSEMAPHORE Semaphore;
    LIST_ENTRY ListHead;
} MMDEREFERENCE_SEGMENT_HEADER;

//
// This entry is used for calling the segment dereference thread
// to perform page file expansion.  It has a similar structure
// to a control area to allow either a control area or a page file
// expansion entry to be placed on the list.  Note that for a control
// area the segment pointer is valid whereas for page file expansion
// it is null.
//

typedef struct _MMPAGE_FILE_EXPANSION {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    SIZE_T RequestedExpansionSize;
    SIZE_T ActualExpansion;
    KEVENT Event;
    LONG InProgress;
    ULONG PageFileNumber;
} MMPAGE_FILE_EXPANSION, *PMMPAGE_FILE_EXPANSION;

#define MI_EXTEND_ANY_PAGEFILE      ((ULONG)-1)
#define MI_CONTRACT_PAGEFILES       ((SIZE_T)-1)

typedef struct _MMWORKING_SET_EXPANSION_HEAD {
    LIST_ENTRY ListHead;
} MMWORKING_SET_EXPANSION_HEAD;

#define SUBSECTION_READ_ONLY      1L
#define SUBSECTION_READ_WRITE     2L
#define SUBSECTION_COPY_ON_WRITE  4L
#define SUBSECTION_SHARE_ALLOW    8L

//
// The MMINPAGE_FLAGS relies on the fact that a pool allocation is always
// QUADWORD aligned so the low 3 bits are always available.
//

typedef struct _MMINPAGE_FLAGS {
    ULONG_PTR Completed : 1;
    ULONG_PTR Available1 : 1;
    ULONG_PTR Available2 : 1;
#if defined (_WIN64)
    ULONG_PTR PrefetchMdlHighBits : 61;
#else
    ULONG_PTR PrefetchMdlHighBits : 29;
#endif
} MMINPAGE_FLAGS, *PMMINPAGE_FLAGS;

#define MI_EXTRACT_PREFETCH_MDL(_Support) ((PMDL)((ULONG_PTR)(_Support->u1.PrefetchMdl) & ~(sizeof(QUAD) - 1)))

typedef struct _MMINPAGE_SUPPORT {
    KEVENT Event;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER ReadOffset;
    LONG WaitCount;
#if defined (_WIN64)
    ULONG UsedPageTableEntries;
#endif
    PETHREAD Thread;
    PFILE_OBJECT FilePointer;
    PMMPTE BasePte;
    PMMPFN Pfn;
    union {
        MMINPAGE_FLAGS e1;
        ULONG_PTR LongFlags;
        PMDL PrefetchMdl;       // Only used under _PREFETCH_
    } u1;
    MDL Mdl;
    PFN_NUMBER Page[MM_MAXIMUM_READ_CLUSTER_SIZE + 1];
    SINGLE_LIST_ENTRY ListEntry;
} MMINPAGE_SUPPORT, *PMMINPAGE_SUPPORT;

#define MI_PF_DUMMY_PAGE_PTE ((PMMPTE)0x23452345)   // Only used by _PREFETCH_

//
// Section support.
//

typedef struct _SECTION {
    MMADDRESS_NODE Address;
    PSEGMENT Segment;
    LARGE_INTEGER SizeOfSection;
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    ULONG InitialPageProtection;
} SECTION, *PSECTION;

//
// Banked memory descriptor.  Pointed to by VAD which has
// the PhysicalMemory flags set and the Banked pointer field as
// non-NULL.
//

typedef struct _MMBANKED_SECTION {
    PFN_NUMBER BasePhysicalPage;
    PMMPTE BasedPte;
    ULONG BankSize;
    ULONG BankShift; //shift for PTEs to calculate bank number
    PBANKED_SECTION_ROUTINE BankedRoutine;
    PVOID Context;
    PMMPTE CurrentMappedPte;
    MMPTE BankTemplate[1];
} MMBANKED_SECTION, *PMMBANKED_SECTION;


//
// Virtual address descriptor
//
// ***** NOTE **********
//  The first part of a virtual address descriptor is a MMADDRESS_NODE!!!
//

#if defined (_WIN64)

#define COMMIT_SIZE 51

#if ((COMMIT_SIZE + PAGE_SHIFT) < 63)
#error COMMIT_SIZE too small
#endif

#else
#define COMMIT_SIZE 19

#if ((COMMIT_SIZE + PAGE_SHIFT) < 31)
#error COMMIT_SIZE too small
#endif
#endif

#define MM_MAX_COMMIT (((ULONG_PTR) 1 << COMMIT_SIZE) - 1)

#define MM_VIEW_UNMAP 0
#define MM_VIEW_SHARE 1

typedef struct _MMVAD_FLAGS {
    ULONG_PTR CommitCharge : COMMIT_SIZE; //limits system to 4k pages or bigger!
    ULONG_PTR PhysicalMapping : 1;      // Device\PhysicalMemory
    ULONG_PTR ImageMap : 1;
    ULONG_PTR UserPhysicalPages : 1;    // AWE
    ULONG_PTR NoChange : 1;
    ULONG_PTR WriteWatch : 1;
    ULONG_PTR Protection : 5;
    ULONG_PTR LargePages : 1;
    ULONG_PTR MemCommit: 1;
    ULONG_PTR PrivateMemory : 1;    //used to tell VAD from VAD_SHORT
} MMVAD_FLAGS;

typedef struct _MMVAD_FLAGS2 {
    unsigned FileOffset : 24;       // number of 64k units into file
    unsigned SecNoChange : 1;       // set if SEC_NOCHANGE specified
    unsigned OneSecured : 1;        // set if u3 field is a range
    unsigned MultipleSecured : 1;   // set if u3 field is a list head
    unsigned ReadOnly : 1;          // protected as ReadOnly
    unsigned LongVad : 1;           // set if VAD is a long VAD
    unsigned ExtendableFile : 1;
    unsigned Inherit : 1;           //1 = ViewShare, 0 = ViewUnmap
    unsigned CopyOnWrite : 1;
} MMVAD_FLAGS2;

typedef struct _MMADDRESS_LIST {
    ULONG_PTR StartVpn;
    ULONG_PTR EndVpn;
} MMADDRESS_LIST, *PMMADDRESS_LIST;

typedef struct _MMSECURE_ENTRY {
    union {
        ULONG_PTR LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
    ULONG_PTR StartVpn;
    ULONG_PTR EndVpn;
    LIST_ENTRY List;
} MMSECURE_ENTRY, *PMMSECURE_ENTRY;

typedef struct _ALIAS_VAD_INFO {
    KAPC Apc;
    ULONG NumberOfEntries;
    ULONG MaximumEntries;
} ALIAS_VAD_INFO, *PALIAS_VAD_INFO;

typedef struct _ALIAS_VAD_INFO2 {
    ULONG BaseAddress;
    HANDLE SecureHandle;
} ALIAS_VAD_INFO2, *PALIAS_VAD_INFO2;

typedef struct _MMVAD {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
    PCONTROL_AREA ControlArea;
    PMMPTE FirstPrototypePte;
    PMMPTE LastContiguousPte;
    union {
        ULONG LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
} MMVAD, *PMMVAD;

typedef struct _MMVAD_LONG {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
    PCONTROL_AREA ControlArea;
    PMMPTE FirstPrototypePte;
    PMMPTE LastContiguousPte;
    union {
        ULONG LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
    union {
        LIST_ENTRY List;
        MMADDRESS_LIST Secured;
    } u3;
    union {
        PMMBANKED_SECTION Banked;
        PMMEXTEND_INFO ExtendedInfo;
    } u4;
#if defined(_MIALT4K_)
    PALIAS_VAD_INFO AliasInformation;
#endif
} MMVAD_LONG, *PMMVAD_LONG;

typedef struct _MMVAD_SHORT {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
} MMVAD_SHORT, *PMMVAD_SHORT;

#define MI_GET_PROTECTION_FROM_VAD(_Vad) ((ULONG)(_Vad)->u.VadFlags.Protection)

#define MI_PHYSICAL_VIEW_AWE    0x1         // AWE region
#define MI_PHYSICAL_VIEW_PHYS   0x2         // Device\PhysicalMemory region
#define MI_PHYSICAL_VIEW_LARGE  0x4         // Large page region

typedef struct _MI_PHYSICAL_VIEW {
    union {
        LONG_PTR Balance : 2;
        struct _MMADDRESS_NODE *Parent;
    } u1;
    struct _MMADDRESS_NODE *LeftChild;
    struct _MMADDRESS_NODE *RightChild;
    ULONG_PTR StartingVpn;      // Actually a virtual address, not a VPN
    ULONG_PTR EndingVpn;        // Actually a virtual address, not a VPN
    PMMVAD Vad;
    union {
        ULONG_PTR LongFlags;    // physical, AWE or largepage Vad identification
        PRTL_BITMAP BitMap;     // only if Vad->u.VadFlags.WriteWatch == 1
    } u;
} MI_PHYSICAL_VIEW, *PMI_PHYSICAL_VIEW;

#define MI_PHYSICAL_VIEW_ROOT_KEY   'rpmM'
#define MI_PHYSICAL_VIEW_KEY        'vpmM'
#define MI_WRITEWATCH_VIEW_KEY      'wWmM'

//
// Stuff for support of Write Watch.
//

VOID
MiCaptureWriteWatchDirtyBit (
    IN PEPROCESS Process,
    IN PVOID VirtualAddress
    );

//
// Stuff for support of AWE (Address Windowing Extensions).
//

typedef struct _AWEINFO {
    PRTL_BITMAP VadPhysicalPagesBitMap;
    ULONG_PTR VadPhysicalPages;
    ULONG_PTR VadPhysicalPagesLimit;

    //
    // The PushLock is used to allow most of the NtMapUserPhysicalPages{Scatter}
    // to execute in parallel as this is acquired shared for these calls.
    // Exclusive acquisitions are used to protect maps against frees of the
    // pages as well as to protect updates to the AweVadList.  Collisions
    // should be rare because the exclusive acquisitions should be rare.
    //

    PEX_PUSH_LOCK_CACHE_AWARE PushLock;
    PMI_PHYSICAL_VIEW PhysicalViewHint[MAXIMUM_PROCESSORS];

    MM_AVL_TABLE AweVadRoot;
} AWEINFO, *PAWEINFO;

VOID
MiAweViewInserter (
    IN PEPROCESS Process,
    IN PMI_PHYSICAL_VIEW PhysicalView
    );

VOID
MiAweViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    );

//
// Stuff for support of POSIX Fork.
//

typedef struct _MMCLONE_BLOCK {
    MMPTE ProtoPte;
    LONG CloneRefCount;
} MMCLONE_BLOCK, *PMMCLONE_BLOCK;

typedef struct _MMCLONE_HEADER {
    ULONG NumberOfPtes;
    LONG NumberOfProcessReferences;
    PMMCLONE_BLOCK ClonePtes;
} MMCLONE_HEADER, *PMMCLONE_HEADER;

typedef struct _MMCLONE_DESCRIPTOR {
    union {
        LONG_PTR Balance : 2;
        struct _MMADDRESS_NODE *Parent;
    } u1;
    struct _MMADDRESS_NODE *LeftChild;
    struct _MMADDRESS_NODE *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;
    ULONG NumberOfPtes;
    PMMCLONE_HEADER CloneHeader;
    LONG NumberOfReferences;
    LONG FinalNumberOfReferences;
    SIZE_T PagedPoolQuotaCharge;
} MMCLONE_DESCRIPTOR, *PMMCLONE_DESCRIPTOR;

//
// The following macro allocates and initializes a bitmap from the
// specified pool of the specified size.
//
//      VOID
//      MiCreateBitMap (
//          OUT PRTL_BITMAP *BitMapHeader,
//          IN SIZE_T SizeOfBitMap,
//          IN POOL_TYPE PoolType
//          );
//

#define MiCreateBitMap(BMH,S,P) {                          \
    ULONG _S;                                              \
    ASSERT ((ULONG64)(S) < _4gb);                          \
    _S = sizeof(RTL_BITMAP) + (ULONG)((((S) + 31) / 32) * 4);         \
    *(BMH) = (PRTL_BITMAP)ExAllocatePoolWithTag( (P), _S, '  mM');       \
    if (*(BMH)) { \
        RtlInitializeBitMap( *(BMH), (PULONG)((*(BMH))+1), (ULONG)(S)); \
    }                                                          \
}

#define MiRemoveBitMap(BMH)     {                          \
    ExFreePool(*(BMH));                                    \
    *(BMH) = NULL;                                         \
}

#define MI_INITIALIZE_ZERO_MDL(MDL) { \
    MDL->Next = (PMDL) NULL; \
    MDL->MdlFlags = 0; \
    MDL->StartVa = NULL; \
    MDL->ByteOffset = 0; \
    MDL->ByteCount = 0; \
    }

//
// Page File structures.
//

typedef struct _MMMOD_WRITER_LISTHEAD {
    LIST_ENTRY ListHead;
    KEVENT Event;
} MMMOD_WRITER_LISTHEAD, *PMMMOD_WRITER_LISTHEAD;

typedef struct _MMMOD_WRITER_MDL_ENTRY {
    LIST_ENTRY Links;
    LARGE_INTEGER WriteOffset;
    union {
        IO_STATUS_BLOCK IoStatus;
        LARGE_INTEGER LastByte;
    } u;
    PIRP Irp;
    ULONG_PTR LastPageToWrite;
    PMMMOD_WRITER_LISTHEAD PagingListHead;
    PLIST_ENTRY CurrentList;
    struct _MMPAGING_FILE *PagingFile;
    PFILE_OBJECT File;
    PCONTROL_AREA ControlArea;
    PERESOURCE FileResource;
    LARGE_INTEGER IssueTime;
    MDL Mdl;
    PFN_NUMBER Page[1];
} MMMOD_WRITER_MDL_ENTRY, *PMMMOD_WRITER_MDL_ENTRY;


#define MM_PAGING_FILE_MDLS 2

typedef struct _MMPAGING_FILE {
    PFN_NUMBER Size;
    PFN_NUMBER MaximumSize;
    PFN_NUMBER MinimumSize;
    PFN_NUMBER FreeSpace;
    PFN_NUMBER CurrentUsage;
    PFN_NUMBER PeakUsage;
    PFN_NUMBER HighestPage;
    PFILE_OBJECT File;
    PMMMOD_WRITER_MDL_ENTRY Entry[MM_PAGING_FILE_MDLS];
    UNICODE_STRING PageFileName;
    PRTL_BITMAP Bitmap;
    struct {
        ULONG PageFileNumber :  4;
        ULONG ReferenceCount :  4;      // really only need 1 bit for this.
        ULONG BootPartition  :  1;
        ULONG Reserved       : 23;
    };
    HANDLE FileHandle;
} MMPAGING_FILE, *PMMPAGING_FILE;

//
// System PTE structures.
//

typedef struct _MMFREE_POOL_ENTRY {
    LIST_ENTRY List;        // maintained free&chk, 1st entry only
    PFN_NUMBER Size;        // maintained free&chk, 1st entry only
    ULONG Signature;        // maintained chk only, all entries
    struct _MMFREE_POOL_ENTRY *Owner; // maintained free&chk, all entries
} MMFREE_POOL_ENTRY, *PMMFREE_POOL_ENTRY;


typedef struct _MMLOCK_CONFLICT {
    LIST_ENTRY List;
    PETHREAD Thread;
} MMLOCK_CONFLICT, *PMMLOCK_CONFLICT;

//
// System view structures
//

typedef struct _MMVIEW {
    ULONG_PTR Entry;
    PCONTROL_AREA ControlArea;
} MMVIEW, *PMMVIEW;

//
// The MMSESSION structure represents kernel memory that is only valid on a
// per-session basis, thus the calling thread must be in the proper session
// to access this structure.
//

typedef struct _MMSESSION {

    //
    // Never refer to the SystemSpaceViewLock directly - always use the pointer
    // following it or you will break support for multiple concurrent sessions.
    //

    KGUARDED_MUTEX SystemSpaceViewLock;

    //
    // This points to the mutex above and is needed because the MMSESSION
    // is mapped in session space and the mutex needs to be globally
    // visible for proper KeWaitForSingleObject & KeSetEvent operation.
    //

    PKGUARDED_MUTEX SystemSpaceViewLockPointer;
    PCHAR SystemSpaceViewStart;
    PMMVIEW SystemSpaceViewTable;
    ULONG SystemSpaceHashSize;
    ULONG SystemSpaceHashEntries;
    ULONG SystemSpaceHashKey;
    ULONG BitmapFailures;
    PRTL_BITMAP SystemSpaceBitMap;

} MMSESSION, *PMMSESSION;

extern MMSESSION   MmSession;

#define LOCK_SYSTEM_VIEW_SPACE(_Session) \
            KeAcquireGuardedMutex (_Session->SystemSpaceViewLockPointer)

#define UNLOCK_SYSTEM_VIEW_SPACE(_Session) \
            KeReleaseGuardedMutex (_Session->SystemSpaceViewLockPointer)

//
// List for flushing TBs singularly.
//

typedef struct _MMPTE_FLUSH_LIST {
    ULONG Count;
    PVOID FlushVa[MM_MAXIMUM_FLUSH_COUNT];
} MMPTE_FLUSH_LIST, *PMMPTE_FLUSH_LIST;

//
// List for flushing WSLEs and TBs singularly.
//

typedef struct _MMWSLE_FLUSH_LIST {
    ULONG Count;
    WSLE_NUMBER FlushIndex[MM_MAXIMUM_FLUSH_COUNT];
} MMWSLE_FLUSH_LIST, *PMMWSLE_FLUSH_LIST;

typedef struct _LOCK_TRACKER {
    LIST_ENTRY ListEntry;
    PMDL Mdl;
    PVOID StartVa;
    PFN_NUMBER Count;
    ULONG Offset;
    ULONG Length;
    PFN_NUMBER Page;
    PVOID CallingAddress;
    PVOID CallersCaller;
    ULONG Who;
    PEPROCESS Process;
} LOCK_TRACKER, *PLOCK_TRACKER;

extern LOGICAL  MmTrackLockedPages;
extern BOOLEAN  MiTrackingAborted;

typedef struct _LOCK_HEADER {
    LIST_ENTRY ListHead;
    PFN_NUMBER Count;
    KSPIN_LOCK Lock;
    LOGICAL Valid;
} LOCK_HEADER, *PLOCK_HEADER;

extern LOGICAL MmSnapUnloads;

#define MI_UNLOADED_DRIVERS 50

extern ULONG MmLastUnloadedDriver;
extern PUNLOADED_DRIVERS MmUnloadedDrivers;


VOID
MiInitMachineDependent (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiAddHalIoMappings (
    VOID
    );

VOID
MiReportPhysicalMemory (
    VOID
    );

extern PFN_NUMBER MiNumberOfCompressionPages;

NTSTATUS
MiArmCompressionInterrupt (
    VOID
    );

VOID
MiBuildPagedPool (
    VOID
    );

VOID
MiInitializeNonPagedPool (
    VOID
    );

VOID
MiInitializePoolEvents (
    VOID
    );

VOID
MiInitializeNonPagedPoolThresholds (
    VOID
    );

LOGICAL
MiInitializeSystemSpaceMap (
    PVOID Session OPTIONAL
    );

VOID
MiFindInitializationCode (
    OUT PVOID *StartVa,
    OUT PVOID *EndVa
    );

VOID
MiFreeInitializationCode (
    IN PVOID StartVa,
    IN PVOID EndVa
    );

extern ULONG MiNonCachedCollisions;

//
// If /NOLOWMEM is used, this is set to the boundary PFN (pages below this
// value are not used whenever possible).
//

extern PFN_NUMBER MiNoLowMemory;

PVOID
MiAllocateLowMemory (
    IN SIZE_T NumberOfBytes,
    IN PFN_NUMBER LowestAcceptablePfn,
    IN PFN_NUMBER HighestAcceptablePfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PVOID CallingAddress,
    IN MEMORY_CACHING_TYPE CacheType,
    IN ULONG Tag
    );

LOGICAL
MiFreeLowMemory (
    IN PVOID BaseAddress,
    IN ULONG Tag
    );

//
// Move drivers out of the low 16mb that ntldr placed them at - this makes more
// memory below 16mb available for ISA-type drivers that cannot run without it.
//

extern LOGICAL MmMakeLowMemory;

VOID
MiRemoveLowPages (
    IN ULONG RemovePhase
    );

ULONG
MiSectionInitialization (
    VOID
    );

#define MI_MAX_DEREFERENCE_CHUNK (64 * 1024 / PAGE_SIZE)

typedef struct _MI_PFN_DEREFERENCE_CHUNK {
    SINGLE_LIST_ENTRY ListEntry;
    CSHORT Flags;
    USHORT NumberOfPages;
    PFN_NUMBER Pfns[MI_MAX_DEREFERENCE_CHUNK];
} MI_PFN_DEREFERENCE_CHUNK, *PMI_PFN_DEREFERENCE_CHUNK;

extern SLIST_HEADER MmPfnDereferenceSListHead;
extern PSLIST_ENTRY MmPfnDeferredList;

#define MI_DEFER_PFN_HELD               0x1
#define MI_DEFER_DRAIN_LOCAL_ONLY       0x2

VOID
MiDeferredUnlockPages (
     ULONG Flags
     );

LOGICAL
MiFreeAllExpansionNonPagedPool (
    VOID
    );

VOID
MiDecrementReferenceCountForAwePage (
    IN PMMPFN Pfn1,
    IN LOGICAL PfnHeld
    );

VOID
FASTCALL
MiDecrementReferenceCount (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex
    );

//++
//VOID
//MiDecrementReferenceCountInline (
//    IN PMMPFN PFN
//    IN PFN_NUMBER FRAME
//    );
//
// Routine Description:
//
//    MiDecrementReferenceCountInline decrements the reference count inline,
//    only calling MiDecrementReferenceCount if the count would go to zero
//    which would cause the page to be released.
//
// Arguments:
//
//    PFN - Supplies the PFN to decrement.
//
//    FRAME - Supplies the frame matching the above PFN.
//
// Return Value:
//
//    None.
//
// Environment:
//
//    PFN lock held.
//
//--

#define MiDecrementReferenceCountInline(PFN, FRAME)                     \
            MM_PFN_LOCK_ASSERT();                                       \
            ASSERT (MI_PFN_ELEMENT(FRAME) == (PFN));                    \
            ASSERT ((FRAME) <= MmHighestPhysicalPage);                  \
            ASSERT ((PFN)->u3.e2.ReferenceCount != 0);                  \
            if ((PFN)->u3.e2.ReferenceCount != 1) {                     \
                (PFN)->u3.e2.ReferenceCount -= 1;                       \
            }                                                           \
            else {                                                      \
                MiDecrementReferenceCount (PFN, FRAME);                 \
            }

VOID
FASTCALL
MiDecrementShareCount (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex
    );

//++
//VOID
//MiDecrementShareCountInline (
//    IN PMMPFN PFN,
//    IN PFN_NUMBER FRAME
//    );
//
// Routine Description:
//
//    MiDecrementShareCountInline decrements the share count inline,
//    only calling MiDecrementShareCount if the count would go to zero
//    which would cause the page to be released.
//
// Arguments:
//
//    PFN - Supplies the PFN to decrement.
//
//    FRAME - Supplies the frame matching the above PFN.
//
// Return Value:
//
//    None.
//
// Environment:
//
//    PFN lock held.
//
//--

#define MiDecrementShareCountInline(PFN, FRAME)                         \
            MM_PFN_LOCK_ASSERT();                                       \
            ASSERT (((FRAME) <= MmHighestPhysicalPage) && ((FRAME) > 0));   \
            ASSERT (MI_PFN_ELEMENT(FRAME) == (PFN));                    \
            ASSERT ((PFN)->u2.ShareCount != 0);                         \
            if ((PFN)->u3.e1.PageLocation != ActiveAndValid && (PFN)->u3.e1.PageLocation != StandbyPageList) {                                            \
                KeBugCheckEx (PFN_LIST_CORRUPT, 0x99, FRAME, (PFN)->u3.e1.PageLocation, 0);                                                             \
            }                                                           \
            if ((PFN)->u2.ShareCount != 1) {                            \
                (PFN)->u2.ShareCount -= 1;                              \
                PERFINFO_DECREFCNT((PFN), PERF_SOFT_TRIM, PERFINFO_LOG_TYPE_DECSHARCNT); \
                ASSERT ((PFN)->u2.ShareCount < 0xF000000);              \
            }                                                           \
            else {                                                      \
                MiDecrementShareCount (PFN, FRAME);                     \
            }

//
// Routines which operate on the Page Frame Database Lists
//

VOID
FASTCALL
MiInsertPageInList (
    IN PMMPFNLIST ListHead,
    IN PFN_NUMBER PageFrameIndex
    );

VOID
FASTCALL
MiInsertPageInFreeList (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
FASTCALL
MiInsertStandbyListAtFront (
    IN PFN_NUMBER PageFrameIndex
    );

PFN_NUMBER
FASTCALL
MiRemovePageFromList (
    IN PMMPFNLIST ListHead
    );

VOID
FASTCALL
MiUnlinkPageFromList (
    IN PMMPFN Pfn
    );

VOID
MiUnlinkFreeOrZeroedPage (
    IN PMMPFN Pfn
    );

VOID
FASTCALL
MiInsertFrontModifiedNoWrite (
    IN PFN_NUMBER PageFrameIndex
    );

//
// These are the thresholds for handing out an available page.
//

#define MM_LOW_LIMIT                2

#define MM_MEDIUM_LIMIT            32

#define MM_HIGH_LIMIT             128

//
// These are thresholds for enabling various optimizations.
//

#define MM_TIGHT_LIMIT            256

#define MM_PLENTY_FREE_LIMIT     1024

#define MM_VERY_HIGH_LIMIT      10000

#define MM_ENORMOUS_LIMIT       20000

ULONG
FASTCALL
MiEnsureAvailablePageOrWait (
    IN PEPROCESS Process,
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    );

PFN_NUMBER
MiAllocatePfn (
    IN PMMPTE PointerPte,
    IN ULONG Protection
    );

PFN_NUMBER
FASTCALL
MiRemoveAnyPage (
    IN ULONG PageColor
    );

PFN_NUMBER
FASTCALL
MiRemoveZeroPage (
    IN ULONG PageColor
    );

typedef struct _COLORED_PAGE_INFO {
    union {
        PFN_NUMBER PagesLeftToScan;
#if defined(MI_MULTINODE) 
        KAFFINITY Affinity;
#endif
    };
    PFN_COUNT PagesQueued;
    SCHAR BasePriority;
    PMMPFN PfnAllocation;
    KEVENT Event;
} COLORED_PAGE_INFO, *PCOLORED_PAGE_INFO;

VOID
MiZeroInParallel (
    IN PCOLORED_PAGE_INFO ColoredPageInfoBase
    );

VOID
MiStartZeroPageWorkers (
    VOID
    );

VOID
MiPurgeTransitionList (
    VOID
    );

typedef struct _MM_LDW_WORK_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    PFILE_OBJECT FileObject;
} MM_LDW_WORK_CONTEXT, *PMM_LDW_WORK_CONTEXT;

VOID
MiLdwPopupWorker (
    IN PVOID Context
    );

LOGICAL
MiDereferenceLastChanceLdw (
    IN PMM_LDW_WORK_CONTEXT LdwContext
    );

#define MI_LARGE_PAGE_DRIVER_BUFFER_LENGTH 512

extern WCHAR MmLargePageDriverBuffer[];
extern ULONG MmLargePageDriverBufferLength;

VOID
MiInitializeDriverLargePageList (
    VOID
    );

VOID
MiInitializeLargePageSupport (
    VOID
    );

NTSTATUS
MiAllocateLargePages (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

PFN_NUMBER
MiFindLargePageMemory (
    IN PCOLORED_PAGE_INFO ColoredPageInfo,
    IN PFN_NUMBER SizeInPages,
    OUT PPFN_NUMBER OutZeroCount
    );

VOID
MiFreeLargePageMemory (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    );

VOID
MiFreeLargePages (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

PVOID
MiMapWithLargePages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Protection,
    IN MEMORY_CACHING_TYPE CacheType
    );

VOID
MiUnmapLargePages (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );




LOGICAL
MiMustFrameBeCached (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiSyncCachedRanges (
    VOID
    );

LOGICAL
MiAddCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    );

VOID
MiRemoveCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    );

#define MI_PAGE_FRAME_INDEX_MUST_BE_CACHED(PageFrameIndex)                  \
            MiMustFrameBeCached(PageFrameIndex)



VOID
MiFreeContiguousPages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    );

PFN_NUMBER
MiFindContiguousPages (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType
    );

PVOID
MiFindContiguousMemory (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType,
    IN PVOID CallingAddress
    );

PVOID
MiCheckForContiguousMemory (
    IN PVOID BaseAddress,
    IN PFN_NUMBER BaseAddressPages,
    IN PFN_NUMBER SizeInPages,
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    );

//
// Routines which operate on the page frame database entry.
//

VOID
MiInitializePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN ULONG ModifiedState
    );

VOID
MiInitializePfnForOtherProcess (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN PFN_NUMBER ContainingPageFrame
    );

VOID
MiInitializeCopyOnWritePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN WSLE_NUMBER WorkingSetIndex,
    IN PVOID SessionSpace
    );

VOID
MiInitializeTransitionPfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte
    );

extern SLIST_HEADER MmInPageSupportSListHead;

VOID
MiFreeInPageSupportBlock (
    IN PMMINPAGE_SUPPORT Support
    );

PMMINPAGE_SUPPORT
MiGetInPageSupportBlock (
    IN KIRQL OldIrql,
    IN PNTSTATUS Status
    );

//
// Routines which require a physical page to be mapped into hyperspace
// within the current process.
//

VOID
FASTCALL
MiZeroPhysicalPage (
    IN PFN_NUMBER PageFrameIndex,
    IN ULONG Color
    );

VOID
FASTCALL
MiRestoreTransitionPte (
    IN PMMPFN Pfn1
    );

PSUBSECTION
MiGetSubsectionAndProtoFromPte (
    IN PMMPTE PointerPte,
    IN PMMPTE *ProtoPte
    );

PVOID
MiMapPageInHyperSpace (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex,
    OUT PKIRQL OldIrql
    );

PVOID
MiMapPageInHyperSpaceAtDpc (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiUnmapPagesInZeroSpace (
    IN PVOID VirtualAddress,
    IN PFN_COUNT NumberOfPages
    );

PVOID
MiMapImageHeaderInHyperSpace (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiUnmapImageHeaderInHyperSpace (
    VOID
    );

PFN_NUMBER
MiGetPageForHeader (
    LOGICAL ZeroPage
    );

VOID
MiRemoveImageHeaderPage (
    IN PFN_NUMBER PageFrameNumber
    );

PVOID
MiMapPagesToZeroInHyperSpace (
    IN PMMPFN PfnAllocation,
    IN PFN_COUNT NumberOfPages
    );


//
// Routines to obtain and release system PTEs.
//

PMMPTE
MiReserveSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

PMMPTE
MiReserveAlignedSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType,
    IN ULONG Alignment
    );

VOID
MiReleaseSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

VOID
MiReleaseSplitSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

VOID
MiIncrementSystemPtes (
    IN ULONG  NumberOfPtes
    );

VOID
MiIssueNoPtesBugcheck (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

VOID
MiInitializeSystemPtes (
    IN PMMPTE StartingPte,
    IN PFN_NUMBER NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

NTSTATUS
MiAddMappedPtes (
    IN PMMPTE FirstPte,
    IN PFN_NUMBER NumberOfPtes,
    IN PCONTROL_AREA ControlArea
    );

VOID
MiInitializeIoTrackers (
    VOID
    );

PVOID
MiMapSinglePage (
     IN PVOID VirtualAddress OPTIONAL,
     IN PFN_NUMBER PageFrameIndex,
     IN MEMORY_CACHING_TYPE CacheType,
     IN MM_PAGE_PRIORITY Priority
     );

VOID
MiUnmapSinglePage (
     IN PVOID BaseAddress
     );

typedef struct _MM_PTE_MAPPING {
    LIST_ENTRY ListEntry;
    PVOID SystemVa;
    PVOID SystemEndVa;
    ULONG Protection;
} MM_PTE_MAPPING, *PMM_PTE_MAPPING;

extern LIST_ENTRY MmProtectedPteList;

extern KSPIN_LOCK MmProtectedPteLock;

LOGICAL
MiCheckSystemPteProtection (
    IN ULONG_PTR StoreInstruction,
    IN PVOID VirtualAddress
    );

//
// Access Fault routines.
//

#define STATUS_ISSUE_PAGING_IO (0xC0033333)

#define MM_NOIRQL (HIGH_LEVEL + 2)

NTSTATUS
MiDispatchFault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAdress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN LOGICAL RecheckAccess,
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    OUT PLOGICAL ApcNeeded
    );

NTSTATUS
MiResolveDemandZeroFault (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    );

BOOLEAN
MiIsAddressValid (
    IN PVOID VirtualAddress,
    IN LOGICAL UseForceIfPossible
    );

VOID
MiAllowWorkingSetExpansion (
    IN PMMSUPPORT WsInfo
    );

WSLE_NUMBER
MiAddValidPageToWorkingSet (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG WsleMask
    );

VOID
MiTrimPte (
    IN PVOID VirtualAddress,
    IN PMMPTE ReadPte,
    IN PMMPFN Pfn1,
    IN PEPROCESS CurrentProcess,
    IN MMPTE NewPteContents
    );

NTSTATUS
MiWaitForInPageComplete (
    IN PMMPFN Pfn,
    IN PMMPTE PointerPte,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPteContents,
    IN PMMINPAGE_SUPPORT InPageSupport,
    IN PEPROCESS CurrentProcess
    );

LOGICAL
FASTCALL
MiCopyOnWrite (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte
    );

VOID
MiSetDirtyBit (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN ULONG PfnHeld
    );

VOID
MiSetModifyBit (
    IN PMMPFN Pfn
    );

PMMPTE
MiFindActualFaultingPte (
    IN PVOID FaultingAddress
    );

VOID
MiInitializeReadInProgressSinglePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    );

VOID
MiInitializeReadInProgressPfn (
    IN PMDL Mdl,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    );

NTSTATUS
MiAccessCheck (
    IN PMMPTE PointerPte,
    IN ULONG_PTR WriteOperation,
    IN KPROCESSOR_MODE PreviousMode,
    IN ULONG Protection,
    IN BOOLEAN CallerHoldsPfnLock
    );

NTSTATUS
FASTCALL
MiCheckForUserStackOverflow (
    IN PVOID FaultingAddress
    );

PMMPTE
MiCheckVirtualAddress (
    IN PVOID VirtualAddress,
    OUT PULONG ProtectCode,
    OUT PMMVAD *VadOut
    );

NTSTATUS
FASTCALL
MiCheckPdeForPagedPool (
    IN PVOID VirtualAddress
    );

#if defined (_WIN64)
#define MI_IS_WOW64_PROCESS(PROCESS) (PROCESS->Wow64Process)
#else
#define MI_IS_WOW64_PROCESS(PROCESS) NULL
#endif

#if DBG || defined (_MI_DEBUG_ALTPTE)
#define MI_BREAK_ON_AV(VirtualAddress, Id)                                  \
        if (MmDebug & MM_DBG_STOP_ON_ACCVIO) {                              \
            DbgPrint ("MM:access violation - %p %u\n", VirtualAddress, Id); \
            DbgBreakPoint ();                                               \
        }                                                                   \
        if (MmDebug & MM_DBG_STOP_ON_WOW64_ACCVIO) {                        \
            if (MI_IS_WOW64_PROCESS(PsGetCurrentProcess())) {               \
                DbgPrint ("MM:wow64 access violation - %p %u\n", VirtualAddress, Id); \
                DbgBreakPoint ();                                           \
            }                                                               \
        }
#else
#define MI_BREAK_ON_AV(VirtualAddress, Id)
#endif

//
// Routines which operate on an address tree.
//

PMMADDRESS_NODE
FASTCALL
MiGetNextNode (
    IN PMMADDRESS_NODE Node
    );

PMMADDRESS_NODE
FASTCALL
MiGetPreviousNode (
    IN PMMADDRESS_NODE Node
    );


PMMADDRESS_NODE
FASTCALL
MiGetFirstNode (
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
MiGetLastNode (
    IN PMM_AVL_TABLE Root
    );

VOID
FASTCALL
MiInsertNode (
    IN PMMADDRESS_NODE Node,
    IN PMM_AVL_TABLE Root
    );

VOID
FASTCALL
MiRemoveNode (
    IN PMMADDRESS_NODE Node,
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
FASTCALL
MiLocateAddressInTree (
    IN ULONG_PTR Vpn,
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
MiCheckForConflictingNode (
    IN ULONG_PTR StartVpn,
    IN ULONG_PTR EndVpn,
    IN PMM_AVL_TABLE Root
    );

NTSTATUS
MiFindEmptyAddressRangeInTree (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PMMADDRESS_NODE *PreviousVad,
    OUT PVOID *Base
    );

NTSTATUS
MiFindEmptyAddressRangeDownTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PVOID *Base
    );

NTSTATUS
MiFindEmptyAddressRangeDownBasedTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PVOID *Base
    );

PVOID
MiEnumerateGenericTableWithoutSplayingAvl (
    IN PMM_AVL_TABLE Table,
    IN PVOID *RestartKey
    );

VOID
NodeTreeWalk (
    PMMADDRESS_NODE Start
    );

TABLE_SEARCH_RESULT
MiFindNodeOrParent (
    IN PMM_AVL_TABLE Table,
    IN ULONG_PTR StartingVpn,
    OUT PMMADDRESS_NODE *NodeOrParent
    );

//
// Routines which operate on the tree of virtual address descriptors.
//

NTSTATUS
MiInsertVad (
    IN PMMVAD Vad
    );

VOID
MiRemoveVad (
    IN PMMVAD Vad
    );

PMMVAD
FASTCALL
MiLocateAddress (
    IN PVOID VirtualAddress
    );

NTSTATUS
MiFindEmptyAddressRange (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN ULONG QuickCheck,
    IN PVOID *Base
    );

//
// Routines which operate on the clone tree structure.
//


NTSTATUS
MiCloneProcessAddressSpace (
    IN PEPROCESS ProcessToClone,
    IN PEPROCESS ProcessToInitialize
    );


ULONG
MiDecrementCloneBlockReference (
    IN PMMCLONE_DESCRIPTOR CloneDescriptor,
    IN PMMCLONE_BLOCK CloneBlock,
    IN PEPROCESS CurrentProcess,
    IN KIRQL OldIrql
    );

LOGICAL
MiWaitForForkToComplete (
    IN PEPROCESS CurrentProcess
    );

//
// Routines which operate on the working set list.
//

WSLE_NUMBER
MiAllocateWsle (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG_PTR WsleMask
    );

VOID
MiReleaseWsle (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMSUPPORT WsInfo
    );

VOID
MiInitializeWorkingSetList (
    IN PEPROCESS CurrentProcess
    );

VOID
MiGrowWsleHash (
    IN PMMSUPPORT WsInfo
    );

WSLE_NUMBER
MiTrimWorkingSet (
    IN WSLE_NUMBER Reduction,
    IN PMMSUPPORT WsInfo,
    IN ULONG TrimAge
    );

#if defined(_AMD64_)
#define MM_PROCESS_COMMIT_CHARGE 6
#elif defined(_IA64_)
#define MM_PROCESS_COMMIT_CHARGE 5
#elif defined (_X86PAE_)
#define MM_PROCESS_COMMIT_CHARGE 8
#else
#define MM_PROCESS_COMMIT_CHARGE 4
#endif

#define MI_SYSTEM_GLOBAL    0
#define MI_USER_LOCAL       1
#define MI_SESSION_LOCAL    2

LOGICAL
MiTrimAllSystemPagableMemory (
    IN ULONG MemoryType,
    IN LOGICAL PurgeTransition
    );

VOID
MiRemoveWorkingSetPages (
    IN PMMSUPPORT WsInfo
    );

VOID
MiAgeAndEstimateAvailableInWorkingSet (
    IN PMMSUPPORT VmSupport,
    IN LOGICAL DoAging,
    IN PWSLE_NUMBER WslesScanned,
    IN OUT PPFN_NUMBER TotalClaim,
    IN OUT PPFN_NUMBER TotalEstimatedAvailable
    );

VOID
FASTCALL
MiInsertWsleHash (
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo
    );

VOID
FASTCALL
MiRemoveWsle (
    IN WSLE_NUMBER Entry,
    IN PMMWSL WorkingSetList
    );

WSLE_NUMBER
FASTCALL
MiLocateWsle (
    IN PVOID VirtualAddress,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER WsPfnIndex
    );

ULONG
MiFreeWsle (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte
    );

WSLE_NUMBER
MiFreeWsleList (
    IN PMMSUPPORT WsInfo,
    IN PMMWSLE_FLUSH_LIST WsleFlushList
    );

VOID
MiSwapWslEntries (
    IN WSLE_NUMBER SwapEntry,
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo,
    IN LOGICAL EntryNotInHash
    );

VOID
MiRemoveWsleFromFreeList (
    IN WSLE_NUMBER Entry,
    IN PMMWSLE Wsle,
    IN PMMWSL WorkingSetList
    );

ULONG
MiRemovePageFromWorkingSet (
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN PMMSUPPORT WsInfo
    );

PFN_NUMBER
MiDeleteSystemPagableVm (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER NumberOfPtes,
    IN MMPTE NewPteValue,
    IN LOGICAL SessionAllocation,
    OUT PPFN_NUMBER ResidentPages OPTIONAL
    );

VOID
MiLockCode (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG LockType
    );

PKLDR_DATA_TABLE_ENTRY
MiLookupDataTableEntry (
    IN PVOID AddressWithinSection,
    IN ULONG ResourceHeld
    );

//
// Routines which perform working set management.
//

VOID
MiObtainFreePages (
    VOID
    );

VOID
MiModifiedPageWriter (
    IN PVOID StartContext
    );

VOID
MiMappedPageWriter (
    IN PVOID StartContext
    );

LOGICAL
MiIssuePageExtendRequest (
    IN PMMPAGE_FILE_EXPANSION PageExtend
    );

VOID
MiIssuePageExtendRequestNoWait (
    IN PFN_NUMBER SizeInPages
    );

SIZE_T
MiExtendPagingFiles (
    IN PMMPAGE_FILE_EXPANSION PageExpand
    );

VOID
MiContractPagingFiles (
    VOID
    );

VOID
MiAttemptPageFileReduction (
    VOID
    );

LOGICAL
MiCancelWriteOfMappedPfn (
    IN PFN_NUMBER PageToStop,
    IN KIRQL OldIrql
    );

//
// Routines to delete address space.
//

VOID
MiDeletePteRange (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN LOGICAL AddressSpaceDeletion
    );

VOID
MiDeleteVirtualAddresses (
    IN PUCHAR StartingAddress,
    IN PUCHAR EndingAddress,
    IN PMMVAD Vad
    );

ULONG
MiDeletePte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN ULONG AddressSpaceDeletion,
    IN PEPROCESS CurrentProcess,
    IN PMMPTE PrototypePte,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL,
    IN KIRQL OldIrql
    );

VOID
MiDeleteValidSystemPte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL
    );

VOID
MiDeletePageTablesForPhysicalRange (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

VOID
MiFlushPteList (
    IN PMMPTE_FLUSH_LIST PteFlushList,
    IN ULONG AllProcessors
    );

ULONG
FASTCALL
MiReleasePageFileSpace (
    IN MMPTE PteContents
    );

VOID
FASTCALL
MiUpdateModifiedWriterMdls (
    IN ULONG PageFileNumber
    );

PVOID
MiAllocateAweInfo (
    VOID
    );

VOID
MiRemoveUserPhysicalPagesVad (
    IN PMMVAD_SHORT FoundVad
    );

VOID
MiCleanPhysicalProcessPages (
    IN PEPROCESS Process
    );

VOID
MiInsertPhysicalVadRoot (
    IN PEPROCESS Process,
    IN PMM_AVL_TABLE PhysicalVadRoot
    );

VOID
MiPhysicalViewInserter (
    IN PEPROCESS Process,
    IN PMI_PHYSICAL_VIEW PhysicalView
    );

VOID
MiPhysicalViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    );

VOID
MiPhysicalViewAdjuster (
    IN PEPROCESS Process,
    IN PMMVAD OldVad,
    IN PMMVAD NewVad
    );

//
// MM_SYSTEM_PAGE_COLOR - MmSystemPageColor
//
// This variable is updated frequently, on MP systems we keep
// a separate system color per processor to avoid cache line
// thrashing.
//

#if defined(NT_UP)

#define MI_SYSTEM_PAGE_COLOR    MmSystemPageColor

#else

#define MI_SYSTEM_PAGE_COLOR    (KeGetCurrentPrcb()->PageColor)

#endif

#if defined(MI_MULTINODE)

extern PKNODE KeNodeBlock[];

#define MI_NODE_FROM_COLOR(c)                                               \
        (KeNodeBlock[(c) >> MmSecondaryColorNodeShift])

#define MI_GET_COLOR_FROM_LIST_ENTRY(index,pfn)                             \
    ((ULONG)(((pfn)->u3.e1.PageColor << MmSecondaryColorNodeShift) |        \
         MI_GET_SECONDARY_COLOR((index),(pfn))))

#define MI_ADJUST_COLOR_FOR_NODE(c,n)   ((c) | (n)->Color)
#define MI_CURRENT_NODE_COLOR           (KeGetCurrentNode()->MmShiftedColor)

#define MiRemoveZeroPageIfAny(COLOR)                                        \
        KeNumberNodes > 1 ? (KeGetCurrentNode()->FreeCount[ZeroedPageList] != 0) ? MiRemoveZeroPage(COLOR) : 0 :                                           \
    (MmFreePagesByColor[ZeroedPageList][COLOR].Flink != MM_EMPTY_LIST) ?    \
                       MiRemoveZeroPage(COLOR) : 0

#define MI_GET_PAGE_COLOR_NODE(n)                                           \
        (((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask) |                \
         KeNodeBlock[n]->MmShiftedColor)

#else

#define MI_NODE_FROM_COLOR(c)

#define MI_GET_COLOR_FROM_LIST_ENTRY(index,pfn)                             \
         ((ULONG)MI_GET_SECONDARY_COLOR((index),(pfn)))

#define MI_ADJUST_COLOR_FOR_NODE(c,n)   (c)
#define MI_CURRENT_NODE_COLOR           0

#define MiRemoveZeroPageIfAny(COLOR)   \
    (MmFreePagesByColor[ZeroedPageList][COLOR].Flink != MM_EMPTY_LIST) ? \
                       MiRemoveZeroPage(COLOR) : 0

#define MI_GET_PAGE_COLOR_NODE(n)                                           \
        ((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask)

#endif

FORCEINLINE
PFN_NUMBER
MiRemoveZeroPageMayReleaseLocks (
    IN ULONG Color,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This routine returns a zeroed page.
    
    It may release and reacquire the PFN lock to do so, as well as mapping
    the page in hyperspace to perform the actual zeroing if necessary.

Environment:

    Kernel mode.  PFN lock held, hyperspace lock NOT held.

--*/

{
    PFN_NUMBER PageFrameIndex;

    PageFrameIndex = MiRemoveZeroPageIfAny (Color);

    if (PageFrameIndex == 0) {
        PageFrameIndex = MiRemoveAnyPage (Color);
        UNLOCK_PFN (OldIrql);
        MiZeroPhysicalPage (PageFrameIndex, Color);
        LOCK_PFN (OldIrql);
    }

    return PageFrameIndex;
}

//
// General support routines.
//

#if (_MI_PAGING_LEVELS <= 3)

//++
//PMMPTE
//MiGetPxeAddress (
//    IN PVOID va
//    );
//
// Routine Description:
//
//    MiGetPxeAddress returns the address of the extended page directory parent
//    entry which maps the given virtual address.  This is one level above the
//    page parent directory.
//
// Arguments
//
//    Va - Supplies the virtual address to locate the PXE for.
//
// Return Value:
//
//    The address of the PXE.
//
//--

#define MiGetPxeAddress(va)   ((PMMPTE)0)

//++
//LOGICAL
//MiIsPteOnPxeBoundary (
//    IN PVOID PTE
//    );
//
// Routine Description:
//
//    MiIsPteOnPxeBoundary returns TRUE if the PTE is
//    on an extended page directory parent entry boundary.
//
// Arguments
//
//    PTE - Supplies the PTE to check.
//
// Return Value:
//
//    TRUE if on a boundary, FALSE if not.
//
//--

#define MiIsPteOnPxeBoundary(PTE) (FALSE)

#endif

#if (_MI_PAGING_LEVELS <= 2)

//++
//PMMPTE
//MiGetPpeAddress (
//    IN PVOID va
//    );
//
// Routine Description:
//
//    MiGetPpeAddress returns the address of the page directory parent entry
//    which maps the given virtual address.  This is one level above the
//    page directory.
//
// Arguments
//
//    Va - Supplies the virtual address to locate the PPE for.
//
// Return Value:
//
//    The address of the PPE.
//
//--

#define MiGetPpeAddress(va)  ((PMMPTE)0)

//++
//LOGICAL
//MiIsPteOnPpeBoundary (
//    IN PVOID VA
//    );
//
// Routine Description:
//
//    MiIsPteOnPpeBoundary returns TRUE if the PTE is
//    on a page directory parent entry boundary.
//
// Arguments
//
//    VA - Supplies the virtual address to check.
//
// Return Value:
//
//    TRUE if on a boundary, FALSE if not.
//
//--

#define MiIsPteOnPpeBoundary(PTE) (FALSE)

#endif

ULONG
MiDoesPdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    );

#if (_MI_PAGING_LEVELS >= 3)
#define MiDoesPpeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED) \
            MiDoesPdeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED)
#else
#define MiDoesPpeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED) 1
#endif

#if (_MI_PAGING_LEVELS >= 4)
#define MiDoesPxeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED) \
            MiDoesPdeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED)
#else
#define MiDoesPxeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED) 1
#endif

VOID
MiMakePdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValid (
    IN PVOID VirtualAddress,
    IN PEPROCESS CurrentProcess
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfnWs (
    IN PVOID VirtualAddress,
    IN PEPROCESS CurrentProcess OPTIONAL,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfnSystemWs (
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfn (
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    );

VOID
FASTCALL
MiLockPagedAddress (
    IN PVOID VirtualAddress
    );

VOID
FASTCALL
MiUnlockPagedAddress (
    IN PVOID VirtualAddress,
    IN ULONG PfnLockHeld
    );

ULONG
FASTCALL
MiIsPteDecommittedPage (
    IN PMMPTE PointerPte
    );

ULONG
FASTCALL
MiIsProtectionCompatible (
    IN ULONG OldProtect,
    IN ULONG NewProtect
    );

ULONG
FASTCALL
MiIsPteProtectionCompatible (
    IN ULONG OldPteProtection,
    IN ULONG NewProtect
    );

ULONG
FASTCALL
MiMakeProtectionMask (
    IN ULONG Protect
    );

ULONG
MiIsEntireRangeCommitted (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

ULONG
MiIsEntireRangeDecommitted (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

LOGICAL
MiCheckProtoPtePageState (
    IN PMMPTE PrototypePte,
    IN KIRQL OldIrql,
    OUT PLOGICAL DroppedPfnLock
    );

//++
//PMMPTE
//MiGetProtoPteAddress (
//    IN PMMPTE VAD,
//    IN PVOID VA
//    );
//
// Routine Description:
//
//    MiGetProtoPteAddress returns a pointer to the prototype PTE which
//    is mapped by the given virtual address descriptor and address within
//    the virtual address descriptor.
//
// Arguments
//
//    VAD - Supplies a pointer to the virtual address descriptor that contains
//          the VA.
//
//    VPN - Supplies the virtual page number.
//
// Return Value:
//
//    A pointer to the proto PTE which corresponds to the VA.
//
//--


#define MiGetProtoPteAddress(VAD,VPN)                                        \
    ((((((VPN) - (VAD)->StartingVpn) << PTE_SHIFT) +                         \
      (ULONG_PTR)(VAD)->FirstPrototypePte) <= (ULONG_PTR)(VAD)->LastContiguousPte) ? \
    ((PMMPTE)(((((VPN) - (VAD)->StartingVpn) << PTE_SHIFT) +                 \
        (ULONG_PTR)(VAD)->FirstPrototypePte))) :                                  \
        MiGetProtoPteAddressExtended ((VAD),(VPN)))

PMMPTE
FASTCALL
MiGetProtoPteAddressExtended (
    IN PMMVAD Vad,
    IN ULONG_PTR Vpn
    );

PSUBSECTION
FASTCALL
MiLocateSubsection (
    IN PMMVAD Vad,
    IN ULONG_PTR Vpn
    );

VOID
MiInitializeSystemCache (
    IN ULONG MinimumWorkingSet,
    IN ULONG MaximumWorkingSet
    );

VOID
MiAdjustWorkingSetManagerParameters(
    IN LOGICAL WorkStation
    );

#if defined (_MI_COMPRESSION)
VOID
MiNotifyMemoryEvents (
    VOID
    );
#endif

extern PFN_NUMBER MmLowMemoryThreshold;
extern PFN_NUMBER MmHighMemoryThreshold;

extern PFN_NUMBER MiLowPagedPoolThreshold;
extern PFN_NUMBER MiHighPagedPoolThreshold;

extern PFN_NUMBER MiLowNonPagedPoolThreshold;
extern PFN_NUMBER MiHighNonPagedPoolThreshold;

extern PKEVENT MiLowPagedPoolEvent;
extern PKEVENT MiHighPagedPoolEvent;

extern PKEVENT MiLowNonPagedPoolEvent;
extern PKEVENT MiHighNonPagedPoolEvent;

//
// Section support
//

VOID
FASTCALL
MiInsertBasedSection (
    IN PSECTION Section
    );

NTSTATUS
MiMapViewOfPhysicalSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN ULONG ProtectionMask,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType,
    IN LOGICAL WriteCombined
    );

NTSTATUS
MiMapViewOfDataSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN PSECTION Section,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG ProtectionMask,
    IN SIZE_T CommitSize,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType
    );

NTSTATUS
MiUnmapViewOfSection (
    IN PEPROCESS Process,
    IN PVOID BaseAddress,
    IN LOGICAL AddressSpaceMutexHeld
    );

VOID
MiRemoveImageSectionObject(
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA ControlArea
    );

VOID
MiAddSystemPtes(
    IN PMMPTE StartingPte,
    IN ULONG  NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

VOID
MiRemoveMappedView (
    IN PEPROCESS CurrentProcess,
    IN PMMVAD Vad
    );

VOID
MiSegmentDelete (
    PSEGMENT Segment
    );

VOID
MiSectionDelete (
    IN PVOID Object
    );

VOID
MiDereferenceSegmentThread (
    IN PVOID StartContext
    );

NTSTATUS
MiCreateImageFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment
    );

NTSTATUS
MiCreateDataFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN ULONG IgnoreFileSizing
    );

NTSTATUS
MiCreatePagingFileMap (
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG ProtectionMask,
    IN ULONG AllocationAttributes
    );

VOID
MiPurgeSubsectionInternal (
    IN PSUBSECTION Subsection,
    IN ULONG PteOffset
    );

VOID
MiPurgeImageSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process OPTIONAL,
    IN KIRQL OldIrql
    );

VOID
MiCleanSection (
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL DirtyDataPagesOk
    );

VOID
MiDereferenceControlArea (
    IN PCONTROL_AREA ControlArea
    );

VOID
MiCheckControlArea (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS CurrentProcess,
    IN KIRQL PreviousIrql
    );

NTSTATUS
MiCheckPurgeAndUpMapCount (
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL FailIfSystemViews
    );

VOID
MiCheckForControlAreaDeletion (
    IN PCONTROL_AREA ControlArea
    );

LOGICAL
MiCheckControlAreaStatus (
    IN SECTION_CHECK_TYPE SectionCheckType,
    IN PSECTION_OBJECT_POINTERS SectionObjectPointers,
    IN ULONG DelayClose,
    OUT PCONTROL_AREA *ControlArea,
    OUT PKIRQL OldIrql
    );

extern SLIST_HEADER MmEventCountSListHead;

PEVENT_COUNTER
MiGetEventCounter (
    VOID
    );

VOID
MiFreeEventCounter (
    IN PEVENT_COUNTER Support
    );

ULONG
MiCanFileBeTruncatedInternal (
    IN PSECTION_OBJECT_POINTERS SectionPointer,
    IN PLARGE_INTEGER NewFileSize OPTIONAL,
    IN LOGICAL BlockNewViews,
    OUT PKIRQL PreviousIrql
    );

#define STATUS_MAPPED_WRITER_COLLISION (0xC0033333)

NTSTATUS
MiFlushSectionInternal (
    IN PMMPTE StartingPte,
    IN PMMPTE FinalPte,
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection,
    IN ULONG Synchronize,
    IN LOGICAL WriteInProgressOk,
    OUT PIO_STATUS_BLOCK IoStatus
    );

//
// protection stuff...
//

NTSTATUS
MiProtectVirtualMemory (
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PSIZE_T CapturedRegionSize,
    IN ULONG Protect,
    IN PULONG LastProtect
    );

ULONG
MiGetPageProtection (
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN LOGICAL PteCapturedToLocalStack
    );

NTSTATUS
MiSetProtectionOnSection (
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN ULONG NewProtect,
    OUT PULONG CapturedOldProtect,
    IN ULONG DontCharge,
    OUT PULONG Locked
    );

NTSTATUS
MiCheckSecuredVad (
    IN PMMVAD Vad,
    IN PVOID Base,
    IN ULONG_PTR Size,
    IN ULONG ProtectionMask
    );

HANDLE
MiSecureVirtualMemory (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG ProbeMode,
    IN LOGICAL AddressSpaceMutexHeld
    );

VOID
MiUnsecureVirtualMemory (
    IN HANDLE SecureHandle,
    IN LOGICAL AddressSpaceMutexHeld
    );

ULONG
MiChangeNoAccessForkPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    );

//
// Routines for charging quota and commitment.
//

VOID
MiTrimSegmentCache (
    VOID
    );

VOID
MiInitializeCommitment (
    VOID
    );

LOGICAL
FASTCALL
MiChargeCommitment (
    IN SIZE_T QuotaCharge,
    IN PEPROCESS Process OPTIONAL
    );

LOGICAL
FASTCALL
MiChargeCommitmentCantExpand (
    IN SIZE_T QuotaCharge,
    IN ULONG MustSucceed
    );

LOGICAL
FASTCALL
MiChargeTemporaryCommitmentForReduction (
    IN SIZE_T QuotaCharge
    );

#if defined (_MI_DEBUG_COMMIT_LEAKS)

VOID
FASTCALL
MiReturnCommitment (
    IN SIZE_T QuotaCharge
    );

#else

#define MiReturnCommitment(_QuotaCharge)                                \
            ASSERT ((SSIZE_T)(_QuotaCharge) >= 0);                      \
            ASSERT (MmTotalCommittedPages >= (_QuotaCharge));           \
            InterlockedExchangeAddSizeT (&MmTotalCommittedPages, 0-((SIZE_T)(_QuotaCharge))); \
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_NORMAL, (_QuotaCharge));

#endif

VOID
MiCauseOverCommitPopup (
    VOID
    );


extern SIZE_T MmTotalCommitLimitMaximum;

SIZE_T
MiCalculatePageCommitment (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

VOID
MiReturnPageTablePageCommitment (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PEPROCESS CurrentProcess,
    IN PMMVAD PreviousVad,
    IN PMMVAD NextVad
    );

VOID
MiFlushAllPages (
    VOID
    );

VOID
MiModifiedPageWriterTimerDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

LONGLONG
MiStartingOffset(
    IN PSUBSECTION Subsection,
    IN PMMPTE PteAddress
    );

LARGE_INTEGER
MiEndingOffset(
    IN PSUBSECTION Subsection
    );

VOID
MiReloadBootLoadedDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiSetSystemCodeProtection (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG ProtectionMask
    );

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    );

VOID
MiSetIATProtect (
    IN PVOID DllBase,
    IN ULONG Protection
    );

VOID
MiMakeEntireImageCopyOnWrite (
    IN PSUBSECTION Subsection
    );

LOGICAL
MiInitializeLoadedModuleList (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#define UNICODE_TAB               0x0009
#define UNICODE_LF                0x000A
#define UNICODE_CR                0x000D
#define UNICODE_SPACE             0x0020
#define UNICODE_CJK_SPACE         0x3000

#define UNICODE_WHITESPACE(_ch)     (((_ch) == UNICODE_TAB) || \
                                     ((_ch) == UNICODE_LF) || \
                                     ((_ch) == UNICODE_CR) || \
                                     ((_ch) == UNICODE_SPACE) || \
                                     ((_ch) == UNICODE_CJK_SPACE) || \
                                     ((_ch) == UNICODE_NULL))

extern ULONG MmSpecialPoolTag;
extern PVOID MmSpecialPoolStart;
extern PVOID MmSpecialPoolEnd;
extern PVOID MmSessionSpecialPoolStart;
extern PVOID MmSessionSpecialPoolEnd;

LOGICAL
MiInitializeSpecialPool (
    IN POOL_TYPE PoolType
    );

LOGICAL
MiIsSpecialPoolAddressNonPaged (
    IN PVOID VirtualAddress
    );

#if defined (_WIN64)
LOGICAL
MiInitializeSessionSpecialPool (
    VOID
    );

VOID
MiDeleteSessionSpecialPool (
    VOID
    );
#endif

#if defined (_X86_)
LOGICAL
MiRecoverSpecialPtes (
    IN ULONG NumberOfPtes
    );
#endif

VOID
MiEnableRandomSpecialPool (
    IN LOGICAL Enable
    );

LOGICAL
MiTriageSystem (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiTriageAddDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiTriageVerifyDriver (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

extern ULONG MmTriageActionTaken;

#if defined (_WIN64)
#define MM_SPECIAL_POOL_PTES ((1024 * 1024) / sizeof (MMPTE))
#else
#define MM_SPECIAL_POOL_PTES (24 * PTE_PER_PAGE)
#endif

#define MI_SUSPECT_DRIVER_BUFFER_LENGTH 512

extern WCHAR MmVerifyDriverBuffer[];
extern ULONG MmVerifyDriverBufferLength;
extern ULONG MmVerifyDriverLevel;

extern LOGICAL MmSnapUnloads;
extern LOGICAL MmProtectFreedNonPagedPool;
extern ULONG MmEnforceWriteProtection;
extern LOGICAL MmTrackLockedPages;
extern ULONG MmTrackPtes;

#define VI_POOL_FREELIST_END  ((ULONG_PTR)-1)

#define VI_POOL_PAGE_HEADER_SIGNATURE 0x21321345

typedef struct _VI_POOL_PAGE_HEADER {
    PSLIST_ENTRY NextPage;
    PVOID VerifierEntry;
    ULONG_PTR Signature;
} VI_POOL_PAGE_HEADER, *PVI_POOL_PAGE_HEADER;

typedef struct _VI_POOL_ENTRY_INUSE {
    PVOID VirtualAddress;
    PVOID CallingAddress;
    SIZE_T NumberOfBytes;
    ULONG_PTR Tag;
} VI_POOL_ENTRY_INUSE, *PVI_POOL_ENTRY_INUSE;

typedef struct _VI_POOL_ENTRY {
    union {
        VI_POOL_PAGE_HEADER PageHeader;
        VI_POOL_ENTRY_INUSE InUse;
        PSLIST_ENTRY NextFree;
    };
} VI_POOL_ENTRY, *PVI_POOL_ENTRY;

#define MI_VERIFIER_ENTRY_SIGNATURE            0x98761940

typedef struct _MI_VERIFIER_DRIVER_ENTRY {
    LIST_ENTRY Links;
    ULONG Loads;
    ULONG Unloads;

    UNICODE_STRING BaseName;
    PVOID StartAddress;
    PVOID EndAddress;

#define VI_VERIFYING_DIRECTLY   0x1
#define VI_VERIFYING_INVERSELY  0x2
#define VI_DISABLE_VERIFICATION 0x4

    ULONG Flags;
    ULONG_PTR Signature;

    SLIST_HEADER PoolPageHeaders;
    SLIST_HEADER PoolTrackers;

    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;
    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedBytes;
    SIZE_T NonPagedBytes;
    SIZE_T PeakPagedBytes;
    SIZE_T PeakNonPagedBytes;

} MI_VERIFIER_DRIVER_ENTRY, *PMI_VERIFIER_DRIVER_ENTRY;

typedef struct _MI_VERIFIER_POOL_HEADER {
    PVI_POOL_ENTRY VerifierPoolEntry;
} MI_VERIFIER_POOL_HEADER, *PMI_VERIFIER_POOL_HEADER;

typedef struct _MM_DRIVER_VERIFIER_DATA {
    ULONG Level;
    ULONG RaiseIrqls;
    ULONG AcquireSpinLocks;
    ULONG SynchronizeExecutions;

    ULONG AllocationsAttempted;
    ULONG AllocationsSucceeded;
    ULONG AllocationsSucceededSpecialPool;
    ULONG AllocationsWithNoTag;

    ULONG TrimRequests;
    ULONG Trims;
    ULONG AllocationsFailed;
    ULONG AllocationsFailedDeliberately;

    ULONG Loads;
    ULONG Unloads;
    ULONG UnTrackedPool;
    ULONG UserTrims;

    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;
    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedBytes;
    SIZE_T NonPagedBytes;
    SIZE_T PeakPagedBytes;
    SIZE_T PeakNonPagedBytes;

    ULONG BurstAllocationsFailedDeliberately;
    ULONG SessionTrims;
    ULONG Reserved[2];

} MM_DRIVER_VERIFIER_DATA, *PMM_DRIVER_VERIFIER_DATA;

VOID
MiInitializeDriverVerifierList (
    VOID
    );

LOGICAL
MiInitializeVerifyingComponents (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiApplyDriverVerifier (
    IN PKLDR_DATA_TABLE_ENTRY,
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier
    );

VOID
MiReApplyVerifierToLoadedModules(
    IN PLIST_ENTRY ModuleListHead
    );

VOID
MiVerifyingDriverUnloading (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

VOID
MiVerifierCheckThunks (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

extern ULONG MiActiveVerifierThunks;
extern LIST_ENTRY MiSuspectDriverList;

extern ULONG MiVerifierThunksAdded;

VOID
MiEnableKernelVerifier (
    VOID
    );

extern LOGICAL KernelVerifier;

extern MM_DRIVER_VERIFIER_DATA MmVerifierData;

#define MI_FREED_SPECIAL_POOL_SIGNATURE 0x98764321

#define MI_STACK_BYTES 1024

typedef struct _MI_FREED_SPECIAL_POOL {
    POOL_HEADER OverlaidPoolHeader;
    MI_VERIFIER_POOL_HEADER OverlaidVerifierPoolHeader;

    ULONG Signature;
    ULONG TickCount;
    ULONG NumberOfBytesRequested;
    ULONG Pagable;

    PVOID VirtualAddress;
    PVOID StackPointer;
    ULONG StackBytes;
    PETHREAD Thread;

    UCHAR StackData[MI_STACK_BYTES];
} MI_FREED_SPECIAL_POOL, *PMI_FREED_SPECIAL_POOL;

#define MM_DBG_COMMIT_NONPAGED_POOL_EXPANSION           0
#define MM_DBG_COMMIT_PAGED_POOL_PAGETABLE              1
#define MM_DBG_COMMIT_PAGED_POOL_PAGES                  2
#define MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES          3
#define MM_DBG_COMMIT_ALLOCVM1                          4
#define MM_DBG_COMMIT_ALLOCVM_SEGMENT                   5
#define MM_DBG_COMMIT_IMAGE                             6
#define MM_DBG_COMMIT_PAGEFILE_BACKED_SHMEM             7
#define MM_DBG_COMMIT_INDEPENDENT_PAGES                 8
#define MM_DBG_COMMIT_CONTIGUOUS_PAGES                  9
#define MM_DBG_COMMIT_MDL_PAGES                         0xA
#define MM_DBG_COMMIT_NONCACHED_PAGES                   0xB
#define MM_DBG_COMMIT_MAPVIEW_DATA                      0xC
#define MM_DBG_COMMIT_FILL_SYSTEM_DIRECTORY             0xD
#define MM_DBG_COMMIT_EXTRA_SYSTEM_PTES                 0xE
#define MM_DBG_COMMIT_DRIVER_PAGING_AT_INIT             0xF
#define MM_DBG_COMMIT_PAGEFILE_FULL                     0x10
#define MM_DBG_COMMIT_PROCESS_CREATE                    0x11
#define MM_DBG_COMMIT_KERNEL_STACK_CREATE               0x12
#define MM_DBG_COMMIT_SET_PROTECTION                    0x13
#define MM_DBG_COMMIT_SESSION_CREATE                    0x14
#define MM_DBG_COMMIT_SESSION_IMAGE_PAGES               0x15
#define MM_DBG_COMMIT_SESSION_PAGETABLE_PAGES           0x16
#define MM_DBG_COMMIT_SESSION_SHARED_IMAGE              0x17
#define MM_DBG_COMMIT_DRIVER_PAGES                      0x18
#define MM_DBG_COMMIT_INSERT_VAD                        0x19
#define MM_DBG_COMMIT_SESSION_WS_INIT                   0x1A
#define MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_PAGES       0x1B
#define MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_HASHPAGES   0x1C
#define MM_DBG_COMMIT_SPECIAL_POOL_PAGES                0x1D
#define MM_DBG_COMMIT_SPECIAL_POOL_MAPPING_PAGES        0x1E
#define MM_DBG_COMMIT_SMALL                             0x1F
#define MM_DBG_COMMIT_EXTRA_WS_PAGES                    0x20
#define MM_DBG_COMMIT_EXTRA_INITIAL_SESSION_WS_PAGES    0x21
#define MM_DBG_COMMIT_ALLOCVM_PROCESS                   0x22
#define MM_DBG_COMMIT_INSERT_VAD_PT                     0x23
#define MM_DBG_COMMIT_ALLOCVM_PROCESS2                  0x24
#define MM_DBG_COMMIT_CHARGE_NORMAL                     0x25
#define MM_DBG_COMMIT_CHARGE_CAUSE_POPUP                0x26
#define MM_DBG_COMMIT_CHARGE_CANT_EXPAND                0x27
#define MM_DBG_COMMIT_LARGE_VA_PAGES                    0x28
#define MM_DBG_COMMIT_LOAD_SYSTEM_IMAGE_TEMP            0x29

#define MM_DBG_COMMIT_RETURN_NONPAGED_POOL_EXPANSION    0x40
#define MM_DBG_COMMIT_RETURN_PAGED_POOL_PAGES           0x41
#define MM_DBG_COMMIT_RETURN_SESSION_DATAPAGE           0x42
#define MM_DBG_COMMIT_RETURN_ALLOCVM_SEGMENT            0x43
#define MM_DBG_COMMIT_RETURN_ALLOCVM2                   0x44

#define MM_DBG_COMMIT_RETURN_IMAGE_NO_LARGE_CA          0x46
#define MM_DBG_COMMIT_RETURN_PTE_RANGE                  0x47
#define MM_DBG_COMMIT_RETURN_NTFREEVM1                  0x48
#define MM_DBG_COMMIT_RETURN_NTFREEVM2                  0x49
#define MM_DBG_COMMIT_RETURN_INDEPENDENT_PAGES          0x4A
#define MM_DBG_COMMIT_RETURN_AWE_EXCESS                 0x4B
#define MM_DBG_COMMIT_RETURN_MDL_PAGES                  0x4C
#define MM_DBG_COMMIT_RETURN_NONCACHED_PAGES            0x4D
#define MM_DBG_COMMIT_RETURN_SESSION_CREATE_FAILURE     0x4E
#define MM_DBG_COMMIT_RETURN_PAGETABLES                 0x4F
#define MM_DBG_COMMIT_RETURN_PROTECTION                 0x50
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE1            0x51
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE2            0x52
#define MM_DBG_COMMIT_RETURN_PAGEFILE_FULL              0x53
#define MM_DBG_COMMIT_RETURN_SESSION_DEREFERENCE        0x54
#define MM_DBG_COMMIT_RETURN_VAD                        0x55
#define MM_DBG_COMMIT_RETURN_PROCESS_CREATE_FAILURE1    0x56
#define MM_DBG_COMMIT_RETURN_PROCESS_DELETE             0x57
#define MM_DBG_COMMIT_RETURN_PROCESS_CLEAN_PAGETABLES   0x58
#define MM_DBG_COMMIT_RETURN_KERNEL_STACK_DELETE        0x59
#define MM_DBG_COMMIT_RETURN_SESSION_DRIVER_LOAD_FAILURE1 0x5A
#define MM_DBG_COMMIT_RETURN_DRIVER_INIT_CODE           0x5B
#define MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD              0x5C
#define MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1             0x5D
#define MM_DBG_COMMIT_RETURN_NORMAL                     0x5E
#define MM_DBG_COMMIT_RETURN_PF_FULL_EXTEND             0x5F
#define MM_DBG_COMMIT_RETURN_EXTENDED                   0x60
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE3            0x61
#define MM_DBG_COMMIT_CHARGE_LARGE_PAGES                0x62
#define MM_DBG_COMMIT_RETURN_LARGE_PAGES                0x63

#if 0

#define MM_COMMIT_COUNTER_MAX 0x80

#define MM_TRACK_COMMIT(_index, bump) \
    if (_index >= MM_COMMIT_COUNTER_MAX) { \
        DbgPrint("Mm: Invalid commit counter %d %d\n", _index, MM_COMMIT_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    else { \
        InterlockedExchangeAddSizeT (&MmTrackCommit[_index], bump); \
    }

#define MM_TRACK_COMMIT_REDUCTION(_index, bump) \
    if (_index >= MM_COMMIT_COUNTER_MAX) { \
        DbgPrint("Mm: Invalid commit counter %d %d\n", _index, MM_COMMIT_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    else { \
        InterlockedExchangeAddSizeT (&MmTrackCommit[_index], 0 - (bump)); \
    }

extern SIZE_T MmTrackCommit[MM_COMMIT_COUNTER_MAX];

#define MI_INCREMENT_TOTAL_PROCESS_COMMIT(_charge) InterlockedExchangeAddSizeT (&MmTotalProcessCommit, (_charge));

#else

#define MM_TRACK_COMMIT(_index, bump)
#define MM_TRACK_COMMIT_REDUCTION(_index, bump)
#define MI_INCREMENT_TOTAL_PROCESS_COMMIT(_charge)

#endif

//
// Types of resident available page charges.
//

#define MM_RESAVAIL_ALLOCATE_ZERO_PAGE_CLUSTERS          0
#define MM_RESAVAIL_ALLOCATE_PAGETABLES_FOR_PAGED_POOL   1
#define MM_RESAVAIL_ALLOCATE_GROW_BSTORE                 2
#define MM_RESAVAIL_ALLOCATE_CONTIGUOUS                  3
#define MM_RESAVAIL_FREE_OUTPAGE_BSTORE                  4
#define MM_RESAVAIL_FREE_PAGE_DRIVER                     5
#define MM_RESAVAIL_ALLOCATE_CREATE_PROCESS              6
#define MM_RESAVAIL_FREE_DELETE_PROCESS                  7
#define MM_RESAVAIL_FREE_CLEAN_PROCESS2                  8
#define MM_RESAVAIL_ALLOCATE_CREATE_STACK                9

#define MM_RESAVAIL_FREE_DELETE_STACK                   10
#define MM_RESAVAIL_ALLOCATE_GROW_STACK                 11
#define MM_RESAVAIL_FREE_OUTPAGE_STACK                  12
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE_EXCESS       13
#define MM_RESAVAIL_ALLOCATE_LOAD_SYSTEM_IMAGE          14
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE1             15
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE2             16
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE3             17
#define MM_RESAVAIL_FREE_DRIVER_INITIALIZATION          18
#define MM_RESAVAIL_FREE_SET_DRIVER_PAGING              19

#define MM_RESAVAIL_FREE_CONTIGUOUS2                    20
#define MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1           21
#define MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE            22
#define MM_RESAVAIL_FREE_EXPANSION_NONPAGED_POOL        23
#define MM_RESAVAIL_ALLOCATE_EXPANSION_NONPAGED_POOL    24
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE1                 25
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE3                 26
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST            27
#define MM_RESAVAIL_ALLOCATE_INDEPENDENT                28
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE2                 29

#define MM_RESAVAIL_FREE_INDEPENDENT                    30
#define MM_RESAVAIL_ALLOCATE_NONPAGED_SPECIAL_POOL      31
#define MM_RESAVAIL_FREE_CONTIGUOUS                     32
#define MM_RESAVAIL_ALLOCATE_SPECIAL_POOL_EXPANSION     33
#define MM_RESAVAIL_ALLOCATE_FOR_MDL                    34
#define MM_RESAVAIL_FREE_FROM_MDL                       35
#define MM_RESAVAIL_FREE_AWE                            36
#define MM_RESAVAIL_FREE_NONPAGED_SPECIAL_POOL          37
#define MM_RESAVAIL_FREE_FOR_MDL_EXCESS                 38
#define MM_RESAVAIL_ALLOCATE_HOTADD_PFNDB               39

#define MM_RESAVAIL_ALLOCATE_CREATE_SESSION             40
#define MM_RESAVAIL_FREE_CLEAN_PROCESS1                 41
#define MM_RESAVAIL_ALLOCATE_SINGLE_PFN                 42
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST1           43
#define MM_RESAVAIL_ALLOCATE_SESSION_PAGE_TABLES        44
#define MM_RESAVAIL_ALLOCATE_SESSION_IMAGE              45
#define MM_RESAVAIL_ALLOCATE_BUILDMDL                   46
#define MM_RESAVAIL_FREE_BUILDMDL_EXCESS                47
#define MM_RESAVAIL_ALLOCATE_ADD_WS_PAGE                48
#define MM_RESAVAIL_FREE_CREATE_SESSION                 49

#define MM_RESAVAIL_ALLOCATE_INIT_SESSION_WS            50
#define MM_RESAVAIL_FREE_SESSION_PAGE_TABLE             51
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION            52
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION_PAGES      53
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST2           54
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST3           55
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION_WS         56
#define MM_RESAVAIL_FREE_LOAD_SESSION_IMAGE1            57
#define MM_RESAVAIL_ALLOCATE_USER_PAGE_TABLE            58
#define MM_RESAVAIL_FREE_USER_PAGE_TABLE                59

#define MM_RESAVAIL_FREE_HOTADD_MEMORY                  60
#define MM_RESAVAIL_ALLOCATE_HOTREMOVE_MEMORY           61
#define MM_RESAVAIL_FREE_HOTREMOVE_MEMORY1              62
#define MM_RESAVAIL_FREE_HOTREMOVE_FAILED               63
#define MM_RESAVAIL_FREE_HOTADD_ECC                     64
#define MM_RESAVAIL_ALLOCATE_COMPRESSION                65
#define MM_RESAVAIL_FREE_COMPRESSION                    66
#define MM_RESAVAIL_ALLOCATE_LARGE_PAGES                67
#define MM_RESAVAIL_FREE_LARGE_PAGES                    68
#define MM_RESAVAIL_ALLOCATE_LOAD_SYSTEM_IMAGE_TEMP     69

#define MM_RESAVAIL_ALLOCATE_WSLE_HASH                  70
#define MM_RESAVAIL_FREE_WSLE_HASH                      71
#define MM_RESAVAIL_FREE_CLEAN_PROCESS_WS               72
#define MM_RESAVAIL_FREE_SESSION_PAGE_TABLES_EXCESS     73

#define MM_BUMP_COUNTER_MAX 74

extern SIZE_T MmResTrack[MM_BUMP_COUNTER_MAX];

#define MI_INCREMENT_RESIDENT_AVAILABLE(bump, _index)                        \
    InterlockedExchangeAddSizeT (&MmResidentAvailablePages, (SIZE_T)(bump)); \
    ASSERT (_index < MM_BUMP_COUNTER_MAX);                                   \
    InterlockedExchangeAddSizeT (&MmResTrack[_index], (SIZE_T)(bump));

#define MI_DECREMENT_RESIDENT_AVAILABLE(bump, _index)                          \
    InterlockedExchangeAddSizeT (&MmResidentAvailablePages, 0-(SIZE_T)(bump)); \
    ASSERT (_index < MM_BUMP_COUNTER_MAX);                                     \
    InterlockedExchangeAddSizeT (&MmResTrack[_index], (SIZE_T)(bump));

//++
//PFN_NUMBER
//MI_NONPAGABLE_MEMORY_AVAILABLE(
//    VOID
//    );
//
// Routine Description:
//
//    This routine lets callers know how many pages can be charged against
//    the resident available, factoring in earlier Mm promises that
//    may not have been redeemed at this point (ie: nonpaged pool expansion,
//    etc, that must be honored at a later point if requested).
//
// Arguments
//
//    None.
//
// Return Value:
//
//    The number of currently available pages in the resident available.
//
//    N.B.  This is a signed quantity and can be negative.
//
//--
#define MI_NONPAGABLE_MEMORY_AVAILABLE()                                    \
        ((SPFN_NUMBER)                                                      \
            (MmResidentAvailablePages -                                     \
             MmSystemLockPagesCount))

extern ULONG MmLargePageMinimum;

//
// hack stuff for testing.
//

VOID
MiDumpValidAddresses (
    VOID
    );

VOID
MiDumpPfn ( VOID );

VOID
MiDumpWsl ( VOID );


VOID
MiFormatPte (
    IN PMMPTE PointerPte
    );

VOID
MiCheckPfn ( VOID );

VOID
MiCheckPte ( VOID );

VOID
MiFormatPfn (
    IN PMMPFN PointerPfn
    );




extern const MMPTE ZeroPte;

extern const MMPTE ZeroKernelPte;

extern const MMPTE ValidKernelPteLocal;

extern MMPTE ValidKernelPte;

extern MMPTE ValidKernelPde;

extern const MMPTE ValidKernelPdeLocal;

extern const MMPTE ValidUserPte;

extern const MMPTE ValidPtePte;

extern const MMPTE ValidPdePde;

extern MMPTE DemandZeroPde;

extern const MMPTE DemandZeroPte;

extern MMPTE KernelPrototypePte;

extern const MMPTE TransitionPde;

extern MMPTE PrototypePte;

extern const MMPTE NoAccessPte;

extern ULONG_PTR MmSubsectionBase;

extern ULONG_PTR MmSubsectionTopPage;

extern ULONG ExpMultiUserTS;

//
// Virtual alignment for PTEs (machine specific) minimum value is
// 4k maximum value is 64k.  The maximum value can be raised by
// changing the MM_PROTO_PTE_ALIGNMENT constant and adding more
// reserved mapping PTEs in hyperspace.
//

//
// Total number of physical pages on the system.
//

extern PFN_COUNT MmNumberOfPhysicalPages;

//
// Highest possible physical page number in the system.
//

extern PFN_NUMBER MmHighestPossiblePhysicalPage;

#if defined (_WIN64)

#define MI_DTC_MAX_PAGES ((PFN_NUMBER)(((ULONG64)512 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DTC_BOOTED_3GB_MAX_PAGES     MI_DTC_MAX_PAGES

#define MI_ADS_MAX_PAGES ((PFN_NUMBER)(((ULONG64)64 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DEFAULT_MAX_PAGES ((PFN_NUMBER)(((ULONG64)16 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#else

#define MI_DTC_MAX_PAGES ((PFN_NUMBER)(((ULONG64)128 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DTC_BOOTED_3GB_MAX_PAGES ((PFN_NUMBER)(((ULONG64)16 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_ADS_MAX_PAGES ((PFN_NUMBER)(((ULONG64)32 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DEFAULT_MAX_PAGES ((PFN_NUMBER)(((ULONG64)4 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#endif

#define MI_BLADE_MAX_PAGES ((PFN_NUMBER)(((ULONG64)2 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

extern RTL_BITMAP MiPfnBitMap;


FORCEINLINE
LOGICAL
MI_IS_PFN (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    Check if a given address is backed by RAM or IO space.

Arguments:

    PageFrameIndex - Supplies a page frame number to check.

Return Value:

    TRUE - If the address is backed by RAM.

    FALSE - If the address is IO mapped memory.

Environment:

    Kernel mode.  PFN lock or dynamic memory mutex may be held.

--*/

{
    if (PageFrameIndex > MmHighestPossiblePhysicalPage) {
        return FALSE;
    }

    return MI_CHECK_BIT (MiPfnBitMap.Buffer, PageFrameIndex);
}

//
// Total number of available pages on the system.  This
// is the sum of the pages on the zeroed, free and standby lists.
//

extern PFN_NUMBER MmAvailablePages;

//
// Total number physical pages which would be usable if every process
// was at it's minimum working set size.  This value is initialized
// at system initialization to MmAvailablePages - MM_FLUID_PHYSICAL_PAGES.
// Everytime a thread is created, the kernel stack is subtracted from
// this and every time a process is created, the minimum working set
// is subtracted from this.  If the value would become negative, the
// operation (create process/kernel stack/ adjust working set) fails.
// The PFN LOCK must be owned to manipulate this value.
//

extern SPFN_NUMBER MmResidentAvailablePages;

//
// The total number of pages which would be removed from working sets
// if every working set was at its minimum.
//

extern PFN_NUMBER MmPagesAboveWsMinimum;

//
// If memory is becoming short and MmPagesAboveWsMinimum is
// greater than MmPagesAboveWsThreshold, trim working sets.
//

extern PFN_NUMBER MmPlentyFreePages;

extern PFN_NUMBER MmPagesAboveWsThreshold;

extern LONG MiDelayPageFaults;

extern PMMPFN MmPfnDatabase;

extern MMPFNLIST MmZeroedPageListHead;

extern MMPFNLIST MmFreePageListHead;

extern MMPFNLIST MmStandbyPageListHead;

extern MMPFNLIST MmRomPageListHead;

extern MMPFNLIST MmModifiedPageListHead;

extern MMPFNLIST MmModifiedNoWritePageListHead;

extern MMPFNLIST MmBadPageListHead;

extern PMMPFNLIST MmPageLocationList[NUMBER_OF_PAGE_LISTS];

extern MMPFNLIST MmModifiedPageListByColor[MM_MAXIMUM_NUMBER_OF_COLORS];

//
// Mask for isolating secondary color from physical page number.
//

extern ULONG MmSecondaryColorMask;

//
// Mask for isolating node color from combined node and secondary
// color.
//

extern ULONG MmSecondaryColorNodeMask;

//
// Width of MmSecondaryColorMask in bits.   In multi node systems,
// the node number is combined with the secondary color to make up
// the page color.
//

extern UCHAR MmSecondaryColorNodeShift;

//
// Events for available pages, set means pages are available.
//

extern KEVENT MmAvailablePagesEvent;

extern KEVENT MmAvailablePagesEventMedium;

extern KEVENT MmAvailablePagesEventHigh;

//
// Event for the zeroing page thread.
//

extern KEVENT MmZeroingPageEvent;

//
// Boolean to indicate if the zeroing page thread is currently
// active.  This is set to true when the zeroing page event is
// set and set to false when the zeroing page thread is done
// zeroing all the pages on the free list.
//

extern BOOLEAN MmZeroingPageThreadActive;

//
// Minimum number of free pages before zeroing page thread starts.
//

extern PFN_NUMBER MmMinimumFreePagesToZero;

//
// Global event to synchronize mapped writing with cleaning segments.
//

extern KEVENT MmMappedFileIoComplete;

//
// Hyper space items.
//

extern PMMPTE MmFirstReservedMappingPte;

extern PMMPTE MmLastReservedMappingPte;

//
// System space sizes - MmNonPagedSystemStart to MM_NON_PAGED_SYSTEM_END
// defines the ranges of PDEs which must be copied into a new process's
// address space.
//

extern PVOID MmNonPagedSystemStart;

extern PCHAR MmSystemSpaceViewStart;

extern LOGICAL MmProtectFreedNonPagedPool;

//
// Pool sizes.
//

extern SIZE_T MmSizeOfNonPagedPoolInBytes;

extern SIZE_T MmMinimumNonPagedPoolSize;

extern SIZE_T MmDefaultMaximumNonPagedPool;

extern ULONG MmMaximumNonPagedPoolPercent;

extern ULONG MmMinAdditionNonPagedPoolPerMb;

extern ULONG MmMaxAdditionNonPagedPoolPerMb;

extern SIZE_T MmSizeOfPagedPoolInBytes;
extern PFN_NUMBER MmSizeOfPagedPoolInPages;

extern SIZE_T MmMaximumNonPagedPoolInBytes;

extern PFN_NUMBER MmMaximumNonPagedPoolInPages;

extern PFN_NUMBER MmAllocatedNonPagedPool;

extern PVOID MmNonPagedPoolExpansionStart;

extern ULONG MmExpandedPoolBitPosition;

extern PFN_NUMBER MmNumberOfFreeNonPagedPool;

extern PFN_NUMBER MmNumberOfSystemPtes;

extern ULONG MiRequestedSystemPtes;

extern PMMPTE MmSystemPagePtes;

extern ULONG MmSystemPageDirectory[];

extern SIZE_T MmHeapSegmentReserve;

extern SIZE_T MmHeapSegmentCommit;

extern SIZE_T MmHeapDeCommitTotalFreeThreshold;

extern SIZE_T MmHeapDeCommitFreeBlockThreshold;

#define MI_MAX_FREE_LIST_HEADS  4

extern LIST_ENTRY MmNonPagedPoolFreeListHead[MI_MAX_FREE_LIST_HEADS];

//
// Counter for flushes of the entire TB.
//

extern ULONG MmFlushCounter;

//
// Pool start and end.
//

extern PVOID MmNonPagedPoolStart;

extern PVOID MmNonPagedPoolEnd;

extern PVOID MmPagedPoolStart;

extern PVOID MmPagedPoolEnd;

//
// Pool bit maps and other related structures.
//

typedef struct _MM_PAGED_POOL_INFO {

    PRTL_BITMAP PagedPoolAllocationMap;
    PRTL_BITMAP EndOfPagedPoolBitmap;
    PMMPTE FirstPteForPagedPool;
    PMMPTE LastPteForPagedPool;
    PMMPTE NextPdeForPagedPoolExpansion;
    ULONG PagedPoolHint;
    SIZE_T PagedPoolCommit;
    SIZE_T AllocatedPagedPool;

} MM_PAGED_POOL_INFO, *PMM_PAGED_POOL_INFO;

extern MM_PAGED_POOL_INFO MmPagedPoolInfo;

extern PVOID MmPageAlignedPoolBase[2];

extern PRTL_BITMAP VerifierLargePagedPoolMap;

//
// MmFirstFreeSystemPte contains the offset from the
// Nonpaged system base to the first free system PTE.
// Note, that an offset of zero indicates an empty list.
//

extern MMPTE MmFirstFreeSystemPte[MaximumPtePoolTypes];

extern ULONG_PTR MiSystemViewStart;

//
// System cache sizes.
//

extern PMMWSL MmSystemCacheWorkingSetList;

extern PMMWSLE MmSystemCacheWsle;

extern PVOID MmSystemCacheStart;

extern PVOID MmSystemCacheEnd;

extern PFN_NUMBER MmSystemCacheWsMinimum;

extern PFN_NUMBER MmSystemCacheWsMaximum;

//
// Virtual alignment for PTEs (machine specific) minimum value is
// 0 (no alignment) maximum value is 64k.  The maximum value can be raised by
// changing the MM_PROTO_PTE_ALIGNMENT constant and adding more
// reserved mapping PTEs in hyperspace.
//

extern ULONG MmAliasAlignment;

//
// Mask to AND with virtual address to get an offset to go
// with the alignment.  This value is page aligned.
//

extern ULONG MmAliasAlignmentOffset;

//
// Mask to and with PTEs to determine if the alias mapping is compatible.
// This value is usually (MmAliasAlignment - 1)
//

extern ULONG MmAliasAlignmentMask;

//
// Cells to track unused thread kernel stacks to avoid TB flushes
// every time a thread terminates.
//

extern ULONG MmMaximumDeadKernelStacks;
extern SLIST_HEADER MmDeadStackSListHead;

//
// MmSystemPteBase contains the address of 1 PTE before
// the first free system PTE (zero indicates an empty list).
// The value of this field does not change once set.
//

extern PMMPTE MmSystemPteBase;

//
// Root of system space virtual address descriptors.  These define
// the pagable portion of the system.
//

extern PMMVAD MmVirtualAddressDescriptorRoot;

extern MM_AVL_TABLE MmSectionBasedRoot;

extern PVOID MmHighSectionBase;

//
// Section commit mutex.
//

extern KGUARDED_MUTEX MmSectionCommitMutex;

//
// Section base address mutex.
//

extern KGUARDED_MUTEX MmSectionBasedMutex;

//
// Resource for section extension.
//

extern ERESOURCE MmSectionExtendResource;
extern ERESOURCE MmSectionExtendSetResource;

//
// Inpage cluster sizes for executable pages (set based on memory size).
//

extern ULONG MmDataClusterSize;

extern ULONG MmCodeClusterSize;

//
// Pagefile creation mutex.
//

extern KGUARDED_MUTEX MmPageFileCreationLock;

//
// Event to set when first paging file is created.
//

extern PKEVENT MmPagingFileCreated;

//
// Paging file debug information.
//

extern ULONG_PTR MmPagingFileDebug[];

//
// Fast mutex which guards the working set list for the system shared
// address space (paged pool, system cache, pagable drivers).
//

extern FAST_MUTEX MmSystemWsLock;

//
// Spin lock for allowing working set expansion.
//

extern KSPIN_LOCK MmExpansionLock;

//
// To prevent optimizations.
//

extern MMPTE GlobalPte;

//
// Page color for system working set.
//

extern ULONG MmSystemPageColor;

extern ULONG MmSecondaryColors;

extern ULONG MmProcessColorSeed;

//
// Set from ntos\config\CMDAT3.C  Used by customers to disable paging
// of executive on machines with lots of memory.  Worth a few TPS on a
// data base server.
//

#define MM_SYSTEM_CODE_LOCKED_DOWN 0x1
#define MM_PAGED_POOL_LOCKED_DOWN  0x2

extern ULONG MmDisablePagingExecutive;


//
// For debugging.


#if DBG
extern ULONG MmDebug;
#endif

//
// Unused segment management
//

extern MMDEREFERENCE_SEGMENT_HEADER MmDereferenceSegmentHeader;

extern LIST_ENTRY MmUnusedSegmentList;

extern LIST_ENTRY MmUnusedSubsectionList;

extern KEVENT MmUnusedSegmentCleanup;

extern ULONG MmConsumedPoolPercentage;

extern ULONG MmUnusedSegmentCount;

extern ULONG MmUnusedSubsectionCount;

extern ULONG MmUnusedSubsectionCountPeak;

extern SIZE_T MiUnusedSubsectionPagedPool;

extern SIZE_T MiUnusedSubsectionPagedPoolPeak;

#define MI_UNUSED_SUBSECTIONS_COUNT_INSERT(_MappedSubsection) \
        MmUnusedSubsectionCount += 1; \
        if (MmUnusedSubsectionCount > MmUnusedSubsectionCountPeak) { \
            MmUnusedSubsectionCountPeak = MmUnusedSubsectionCount; \
        } \
        MiUnusedSubsectionPagedPool += EX_REAL_POOL_USAGE((_MappedSubsection->PtesInSubsection + _MappedSubsection->UnusedPtes) * sizeof (MMPTE)); \
        if (MiUnusedSubsectionPagedPool > MiUnusedSubsectionPagedPoolPeak) { \
            MiUnusedSubsectionPagedPoolPeak = MiUnusedSubsectionPagedPool; \
        } \

#define MI_UNUSED_SUBSECTIONS_COUNT_REMOVE(_MappedSubsection) \
        MmUnusedSubsectionCount -= 1; \
        MiUnusedSubsectionPagedPool -= EX_REAL_POOL_USAGE((_MappedSubsection->PtesInSubsection + _MappedSubsection->UnusedPtes) * sizeof (MMPTE));

#define MI_FILESYSTEM_NONPAGED_POOL_CHARGE 150

#define MI_FILESYSTEM_PAGED_POOL_CHARGE 1024

//++
//LOGICAL
//MI_UNUSED_SEGMENTS_SURPLUS (
//    IN PVOID va
//    );
//
// Routine Description:
//
//    This routine determines whether a surplus of unused
//    segments exist.  If so, the caller can initiate a trim to free pool.
//
// Arguments
//
//    None.
//
// Return Value:
//
//    TRUE if unused segment trimming should be initiated, FALSE if not.
//
//--
#define MI_UNUSED_SEGMENTS_SURPLUS()                                    \
        (((ULONG)((MmPagedPoolInfo.AllocatedPagedPool * 100) / (MmSizeOfPagedPoolInBytes >> PAGE_SHIFT)) > MmConsumedPoolPercentage) || \
        ((ULONG)((MmAllocatedNonPagedPool * 100) / MmMaximumNonPagedPoolInPages) > MmConsumedPoolPercentage))

VOID
MiConvertStaticSubsections (
    IN PCONTROL_AREA ControlArea
    );

//++
//VOID
//MI_INSERT_UNUSED_SEGMENT (
//    IN PCONTROL_AREA _ControlArea
//    );
//
// Routine Description:
//
//    This routine inserts a control area into the unused segment list,
//    also managing the associated pool charges.
//
// Arguments
//
//    _ControlArea - Supplies the control area to obtain the pool charges from.
//
// Return Value:
//
//    None.
//
//--
#define MI_INSERT_UNUSED_SEGMENT(_ControlArea)                               \
        {                                                                    \
           MM_PFN_LOCK_ASSERT();                                             \
           if ((_ControlArea->u.Flags.Image == 0) &&                         \
               (_ControlArea->FilePointer != NULL) &&                        \
               (_ControlArea->u.Flags.PhysicalMemory == 0)) {                \
               MiConvertStaticSubsections(_ControlArea);                     \
           }                                                                 \
           InsertTailList (&MmUnusedSegmentList, &_ControlArea->DereferenceList); \
           MmUnusedSegmentCount += 1; \
        }

//++
//VOID
//MI_UNUSED_SEGMENTS_REMOVE_CHARGE (
//    IN PCONTROL_AREA _ControlArea
//    );
//
// Routine Description:
//
//    This routine manages pool charges during removals of segments from
//    the unused segment list.
//
// Arguments
//
//    _ControlArea - Supplies the control area to obtain the pool charges from.
//
// Return Value:
//
//    None.
//
//--
#define MI_UNUSED_SEGMENTS_REMOVE_CHARGE(_ControlArea)                       \
        {                                                                    \
           MM_PFN_LOCK_ASSERT();                                             \
           MmUnusedSegmentCount -= 1; \
        }

//
// List heads
//

extern MMWORKING_SET_EXPANSION_HEAD MmWorkingSetExpansionHead;

extern MMPAGE_FILE_EXPANSION MmAttemptForCantExtend;

//
// Paging files
//

extern MMMOD_WRITER_LISTHEAD MmPagingFileHeader;

extern MMMOD_WRITER_LISTHEAD MmMappedFileHeader;

extern PMMPAGING_FILE MmPagingFile[MAX_PAGE_FILES];

extern LIST_ENTRY MmFreePagingSpaceLow;

extern ULONG MmNumberOfActiveMdlEntries;

extern ULONG MmNumberOfPagingFiles;

extern KEVENT MmModifiedPageWriterEvent;

extern KEVENT MmCollidedFlushEvent;

extern KEVENT MmCollidedLockEvent;

// #define _MI_DEBUG_DATA 1         // Uncomment this for data logging

#if defined (_MI_DEBUG_DATA)

#define MI_DATA_BACKTRACE_LENGTH 8

typedef struct _MI_DATA_TRACES {

    PETHREAD Thread;
    PMMPFN Pfn;
    PMMPTE PointerPte;
    MMPFN PfnData;
    ULONG CallerId;
    ULONG DataInThePage[2];
    PVOID StackTrace [MI_DATA_BACKTRACE_LENGTH];

} MI_DATA_TRACES, *PMI_DATA_TRACES;

extern LONG MiDataIndex;

extern ULONG MiTrackData;

extern PMI_DATA_TRACES MiDataTraces;

VOID
FORCEINLINE
MiSnapData (
    IN PMMPFN Pfn,
    IN PMMPTE PointerPte,
    IN ULONG CallerId
    )
{
    KIRQL OldIrql;
    PVOID Va;
    PMI_DATA_TRACES Information;
    ULONG Index;
    ULONG Hash;
    PEPROCESS CurrentProcess;

    if (MiDataTraces == NULL) {
        return;
    }

    Index = InterlockedIncrement (&MiDataIndex);
    Index &= (MiTrackData - 1);
    Information = &MiDataTraces[Index];

    Information->Thread = PsGetCurrentThread ();
    Information->Pfn = Pfn;
    Information->PointerPte = PointerPte;
    Information->PfnData = *Pfn;
    Information->CallerId = CallerId;

    CurrentProcess = PsGetCurrentProcess ();
    Va = MiMapPageInHyperSpace (CurrentProcess, MI_PFN_ELEMENT_TO_INDEX (Pfn), &OldIrql);

    RtlCopyMemory (&Information->DataInThePage[0],
                   Va,
                   sizeof (Information->DataInThePage));

    MiUnmapPageInHyperSpace (CurrentProcess, Va, OldIrql);

    RtlZeroMemory (&Information->StackTrace[0], MI_DATA_BACKTRACE_LENGTH * sizeof(PVOID));                                                 \

#if defined (_WIN64)
    if (KeAreAllApcsDisabled () == TRUE) {
        Information->StackTrace[1] = (PVOID) _ReturnAddress ();
        Information->StackTrace[0] = MiGetInstructionPointer ();
    }
    else
#endif
    RtlCaptureStackBackTrace (0, MI_DATA_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

#define MI_SNAP_DATA(_Pfn, _Pte, _CallerId) MiSnapData(_Pfn, _Pte, _CallerId)

#else
#define MI_SNAP_DATA(_Pfn, _Pte, _CallerId)
#endif


//
// Modified page writer.
//


VOID
FORCEINLINE
MiReleaseConfirmedPageFileSpace (
    IN MMPTE PteContents
    )

/*++

Routine Description:

    This routine frees the paging file allocated to the specified PTE.

Arguments:

    PteContents - Supplies the PTE which is in page file format.

Return Value:

    Returns TRUE if any paging file space was deallocated.

Environment:

    Kernel mode, APCs disabled, PFN lock held.

--*/

{
    ULONG FreeBit;
    ULONG PageFileNumber;
    PMMPAGING_FILE PageFile;

    MM_PFN_LOCK_ASSERT();

    ASSERT (PteContents.u.Soft.Prototype == 0);

    FreeBit = GET_PAGING_FILE_OFFSET (PteContents);

    ASSERT ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED));

    PageFileNumber = GET_PAGING_FILE_NUMBER (PteContents);

    PageFile = MmPagingFile[PageFileNumber];

    ASSERT (RtlCheckBit( PageFile->Bitmap, FreeBit) == 1);

#if DBG
    if ((FreeBit < 8192) && (PageFileNumber == 0)) {
        ASSERT ((MmPagingFileDebug[FreeBit] & 1) != 0);
        MmPagingFileDebug[FreeBit] ^= 1;
    }
#endif

    MI_CLEAR_BIT (PageFile->Bitmap->Buffer, FreeBit);

    PageFile->FreeSpace += 1;
    PageFile->CurrentUsage -= 1;

    //
    // Check to see if we should move some MDL entries for the
    // modified page writer now that more free space is available.
    //

    if ((MmNumberOfActiveMdlEntries == 0) ||
        (PageFile->FreeSpace == MM_USABLE_PAGES_FREE)) {

        MiUpdateModifiedWriterMdls (PageFileNumber);
    }
}

extern PFN_NUMBER MmMinimumFreePages;

extern PFN_NUMBER MmFreeGoal;

extern PFN_NUMBER MmModifiedPageMaximum;

extern ULONG MmModifiedWriteClusterSize;

extern ULONG MmMinimumFreeDiskSpace;

extern ULONG MmPageFileExtension;

extern ULONG MmMinimumPageFileReduction;

extern LARGE_INTEGER MiModifiedPageLife;

extern BOOLEAN MiTimerPending;

extern KEVENT MiMappedPagesTooOldEvent;

extern KDPC MiModifiedPageWriterTimerDpc;

extern KTIMER MiModifiedPageWriterTimer;

//
// System process working set sizes.
//

extern PFN_NUMBER MmSystemProcessWorkingSetMin;

extern PFN_NUMBER MmSystemProcessWorkingSetMax;

extern PFN_NUMBER MmMinimumWorkingSetSize;

//
// Support for debugger's mapping physical memory.
//

extern PMMPTE MmDebugPte;

extern PMMPTE MmCrashDumpPte;

extern ULONG MiOverCommitCallCount;

//
// Event tracing routines
//

extern PPAGE_FAULT_NOTIFY_ROUTINE MmPageFaultNotifyRoutine;

extern SIZE_T MmSystemViewSize;

VOID
FASTCALL
MiIdentifyPfn (
    IN PMMPFN Pfn1,
    OUT PMMPFN_IDENTITY PfnIdentity
    );

#if defined (_WIN64)
#define InterlockedExchangeAddSizeT(a, b) InterlockedExchangeAdd64((PLONGLONG)a, b)
#else
#define InterlockedExchangeAddSizeT(a, b) InterlockedExchangeAdd((PLONG)(a), b)
#endif

//
// This is a special value loaded into an EPROCESS pointer to indicate that
// the action underway is for a Hydra session, not really the current process.
// Any value could have been used here that is not a valid system pointer
// or NULL - 1 was chosen because it simplifies checks for both NULL &
// HydraProcess by comparing for greater than HydraProcess.
//

#define HYDRA_PROCESS   ((PEPROCESS)1)

#define PREFETCH_PROCESS   ((PEPROCESS)2)

#define MI_SESSION_SPACE_STRUCT_SIZE MM_ALLOCATION_GRANULARITY

#if defined (_WIN64)

/*++

  Virtual memory layout of session space when loaded down from
  0x2000.0002.0000.0000 (IA64) or FFFF.F980.0000.0000 (AMD64) :

  Note that the sizes of mapped views, paged pool & images are registry tunable.

                        +------------------------------------+
    2000.0002.0000.0000 |                                    |
                        |   win32k.sys & video drivers       |
                        |             (16MB)                 |
                        |                                    |
                        +------------------------------------+
    2000.0001.FF00.0000 |                                    |
                        |   MM_SESSION_SPACE & Session WSLs  |
                        |              (16MB)                |
                        |                                    |
    2000.0001.FEFF.0000 +------------------------------------+
                        |                                    |
                        |              ...                   |
                        |                                    |
                        +------------------------------------+
    2000.0001.FE80.0000 |                                    |
                        |   Mapped Views for this session    |
                        |              (104MB)               |
                        |                                    |
                        +------------------------------------+
    2000.0001.F800.0000 |                                    |
                        |   Paged Pool for this session      |
                        |              (64MB)                |
                        |                                    |
    2000.0001.F400.0000 +------------------------------------+
                        |   Special Pool for this session    |
                        |              (64MB)                |
                        |                                    |
    2000.0000.0000.0000 +------------------------------------+

--*/

#define MI_SESSION_SPACE_WS_SIZE  ((ULONG_PTR)(16*1024*1024) - MI_SESSION_SPACE_STRUCT_SIZE)

#define MI_SESSION_DEFAULT_IMAGE_SIZE     ((ULONG_PTR)(16*1024*1024))

#define MI_SESSION_DEFAULT_VIEW_SIZE      ((ULONG_PTR)(104*1024*1024))

#define MI_SESSION_DEFAULT_POOL_SIZE      ((ULONG_PTR)(64*1024*1024))

#define MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE (MM_VA_MAPPED_BY_PPE)

#else

/*++

  Virtual memory layout of session space when loaded down from 0xC0000000.

  Note that the sizes of mapped views, paged pool and images are registry
  tunable on 32-bit systems (if NOT booted /3GB, as 3GB has very limited
  address space).

                 +------------------------------------+
        C0000000 |                                    |
                 | win32k.sys, video drivers and any  |
                 | rebased NT4 printer drivers.       |
                 |                                    |
                 |             (8MB)                  |
                 |                                    |
                 +------------------------------------+
        BF800000 |                                    |
                 |   MM_SESSION_SPACE & Session WSLs  |
                 |              (4MB)                 |
                 |                                    |
                 +------------------------------------+
        BF400000 |                                    |
                 |   Mapped views for this session    |
                 |     (20MB by default, but is       |
                 |      registry configurable)        |
                 |                                    |
                 +------------------------------------+
        BE000000 |                                    |
                 |   Paged pool for this session      |
                 |     (16MB by default, but is       |
                 |      registry configurable)        |
                 |                                    |
        BD000000 +------------------------------------+

--*/

#define MI_SESSION_SPACE_WS_SIZE  (4*1024*1024 - MI_SESSION_SPACE_STRUCT_SIZE)

#define MI_SESSION_DEFAULT_IMAGE_SIZE      (8*1024*1024)

#define MI_SESSION_DEFAULT_VIEW_SIZE      (20*1024*1024)

#define MI_SESSION_DEFAULT_POOL_SIZE      (16*1024*1024)

#define MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE \
            (MM_SYSTEM_CACHE_END_EXTRA - MM_KSEG2_BASE)

#endif



#define MI_SESSION_SPACE_DEFAULT_TOTAL_SIZE \
            (MI_SESSION_DEFAULT_IMAGE_SIZE + \
             MI_SESSION_SPACE_STRUCT_SIZE + \
             MI_SESSION_SPACE_WS_SIZE + \
             MI_SESSION_DEFAULT_VIEW_SIZE + \
             MI_SESSION_DEFAULT_POOL_SIZE)

extern ULONG_PTR MmSessionBase;
extern PMMPTE MiSessionBasePte;
extern PMMPTE MiSessionLastPte;

extern ULONG_PTR MiSessionSpaceWs;

extern ULONG_PTR MiSessionViewStart;
extern SIZE_T MmSessionViewSize;

extern ULONG_PTR MiSessionImageStart;
extern ULONG_PTR MiSessionImageEnd;
extern SIZE_T MmSessionImageSize;

extern PMMPTE MiSessionImagePteStart;
extern PMMPTE MiSessionImagePteEnd;

extern ULONG_PTR MiSessionPoolStart;
extern ULONG_PTR MiSessionPoolEnd;
extern SIZE_T MmSessionPoolSize;

extern ULONG_PTR MiSessionSpaceEnd;

extern ULONG MiSessionSpacePageTables;

//
// The number of page table pages required to map all of session space.
//

#define MI_SESSION_SPACE_MAXIMUM_PAGE_TABLES \
            (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE / MM_VA_MAPPED_BY_PDE)

extern SIZE_T MmSessionSize;        // size of the entire session space.

//
// Macros to determine if a given address lies in the specified session range.
//

#define MI_IS_SESSION_IMAGE_ADDRESS(VirtualAddress) \
        ((PVOID)(VirtualAddress) >= (PVOID)MiSessionImageStart && (PVOID)(VirtualAddress) < (PVOID)(MiSessionImageEnd))

#define MI_IS_SESSION_POOL_ADDRESS(VirtualAddress) \
        ((PVOID)(VirtualAddress) >= (PVOID)MiSessionPoolStart && (PVOID)(VirtualAddress) < (PVOID)MiSessionPoolEnd)

#define MI_IS_SESSION_ADDRESS(_VirtualAddress) \
        ((PVOID)(_VirtualAddress) >= (PVOID)MmSessionBase && (PVOID)(_VirtualAddress) < (PVOID)(MiSessionSpaceEnd))

#define MI_IS_SESSION_PTE(_Pte) \
        ((PMMPTE)(_Pte) >= MiSessionBasePte && (PMMPTE)(_Pte) < MiSessionLastPte)

#define MI_IS_SESSION_IMAGE_PTE(_Pte) \
        ((PMMPTE)(_Pte) >= MiSessionImagePteStart && (PMMPTE)(_Pte) < MiSessionImagePteEnd)

#define SESSION_GLOBAL(_Session)    (_Session->GlobalVirtualAddress)

#define MM_DBG_SESSION_INITIAL_PAGETABLE_ALLOC          0
#define MM_DBG_SESSION_INITIAL_PAGETABLE_FREE_RACE      1
#define MM_DBG_SESSION_INITIAL_PAGE_ALLOC               2
#define MM_DBG_SESSION_INITIAL_PAGE_FREE_FAIL1          3
#define MM_DBG_SESSION_INITIAL_PAGETABLE_FREE_FAIL1     4
#define MM_DBG_SESSION_WS_PAGE_FREE                     5
#define MM_DBG_SESSION_PAGETABLE_ALLOC                  6
#define MM_DBG_SESSION_SYSMAPPED_PAGES_ALLOC            7
#define MM_DBG_SESSION_WS_PAGETABLE_ALLOC               8
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC        9
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_FREE_FAIL1   10
#define MM_DBG_SESSION_WS_PAGE_ALLOC                    11
#define MM_DBG_SESSION_WS_PAGE_ALLOC_GROWTH             12
#define MM_DBG_SESSION_INITIAL_PAGE_FREE                13
#define MM_DBG_SESSION_PAGETABLE_FREE                   14
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC1       15
#define MM_DBG_SESSION_DRIVER_PAGES_LOCKED              16
#define MM_DBG_SESSION_DRIVER_PAGES_UNLOCKED            17
#define MM_DBG_SESSION_WS_HASHPAGE_ALLOC                18
#define MM_DBG_SESSION_SYSMAPPED_PAGES_COMMITTED        19

#define MM_DBG_SESSION_COMMIT_PAGEDPOOL_PAGES           30
#define MM_DBG_SESSION_COMMIT_DELETE_VM_RETURN          31
#define MM_DBG_SESSION_COMMIT_POOL_FREED                32
#define MM_DBG_SESSION_COMMIT_IMAGE_UNLOAD              33
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_FAILED1         34
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_FAILED2         35
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_NOACCESS        36

#define MM_DBG_SESSION_NP_LOCK_CODE1                    38
#define MM_DBG_SESSION_NP_LOCK_CODE2                    39
#define MM_DBG_SESSION_NP_SESSION_CREATE                40
#define MM_DBG_SESSION_NP_PAGETABLE_ALLOC               41
#define MM_DBG_SESSION_NP_POOL_CREATE                   42
#define MM_DBG_SESSION_NP_COMMIT_IMAGE                  43
#define MM_DBG_SESSION_NP_COMMIT_IMAGE_PT               44
#define MM_DBG_SESSION_NP_INIT_WS                       45
#define MM_DBG_SESSION_NP_WS_GROW                       46
#define MM_DBG_SESSION_NP_HASH_GROW                     47

#define MM_DBG_SESSION_NP_PAGE_DRIVER                   48
#define MM_DBG_SESSION_NP_POOL_CREATE_FAILED            49
#define MM_DBG_SESSION_NP_WS_PAGE_FREE                  50
#define MM_DBG_SESSION_NP_SESSION_DESTROY               51
#define MM_DBG_SESSION_NP_SESSION_PTDESTROY             52
#define MM_DBG_SESSION_NP_DELVA                         53
#define MM_DBG_SESSION_NP_HASH_SHRINK                   54
#define MM_DBG_SESSION_WS_HASHPAGE_FREE                 55

#if DBG
#define MM_SESS_COUNTER_MAX 56

#define MM_BUMP_SESS_COUNTER(_index, bump) \
    if (_index >= MM_SESS_COUNTER_MAX) { \
        DbgPrint("Mm: Invalid bump counter %d %d\n", _index, MM_SESS_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    MmSessionSpace->Debug[_index] += (bump);

typedef struct _MM_SESSION_MEMORY_COUNTERS {
    SIZE_T NonPagablePages;
    SIZE_T CommittedPages;
} MM_SESSION_MEMORY_COUNTERS, *PMM_SESSION_MEMORY_COUNTERS;

#define MM_SESS_MEMORY_COUNTER_MAX  8

#define MM_SNAP_SESS_MEMORY_COUNTERS(_index) \
    if (_index >= MM_SESS_MEMORY_COUNTER_MAX) { \
        DbgPrint("Mm: Invalid session mem counter %d %d\n", _index, MM_SESS_MEMORY_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    else { \
        MmSessionSpace->Debug2[_index].NonPagablePages = MmSessionSpace->NonPagablePages; \
        MmSessionSpace->Debug2[_index].CommittedPages = MmSessionSpace->CommittedPages; \
    }

#else
#define MM_BUMP_SESS_COUNTER(_index, bump)
#define MM_SNAP_SESS_MEMORY_COUNTERS(_index)
#endif


#define MM_SESSION_FAILURE_NO_IDS                   0
#define MM_SESSION_FAILURE_NO_COMMIT                1
#define MM_SESSION_FAILURE_NO_RESIDENT              2
#define MM_SESSION_FAILURE_RACE_DETECTED            3
#define MM_SESSION_FAILURE_NO_SYSPTES               4
#define MM_SESSION_FAILURE_NO_PAGED_POOL            5
#define MM_SESSION_FAILURE_NO_NONPAGED_POOL         6
#define MM_SESSION_FAILURE_NO_IMAGE_VA_SPACE        7
#define MM_SESSION_FAILURE_NO_SESSION_PAGED_POOL    8
#define MM_SESSION_FAILURE_NO_AVAILABLE             9
#define MM_SESSION_FAILURE_IMAGE_ZOMBIE             10

#define MM_SESSION_FAILURE_CAUSES                   11

extern ULONG MmSessionFailureCauses[MM_SESSION_FAILURE_CAUSES];

#define MM_BUMP_SESSION_FAILURES(_index) MmSessionFailureCauses[_index] += 1;

typedef struct _MM_SESSION_SPACE_FLAGS {
    ULONG Initialized : 1;
    ULONG DeletePending : 1;
    ULONG Filler : 30;
} MM_SESSION_SPACE_FLAGS;

//
// The value of SESSION_POOL_SMALL_LISTS is very carefully chosen for each
// architecture to avoid spilling over into an additional session data page.
//

#if defined(_AMD64_)
#define SESSION_POOL_SMALL_LISTS        21
#elif defined(_IA64_)
#define SESSION_POOL_SMALL_LISTS        53
#elif defined(_X86_)
#define SESSION_POOL_SMALL_LISTS        26
#else
#error "no target architecture"
#endif

//
// The session space data structure - allocated per session and only visible at
// MM_SESSION_SPACE_BASE when in the context of a process from the session.
// This virtual address space is rotated at context switch time when switching
// from a process in session A to a process in session B.  This rotation is
// useful for things like providing paged pool per session so many sessions
// won't exhaust the VA space which backs the system global pool.
//
// A kernel PTE is also allocated to double map this page so that global
// pointers can be maintained to provide system access from any process context.
// This is needed for things like mutexes and WSL chains.
//

typedef struct _MM_SESSION_SPACE {

    //
    // This is a pointer in global system address space, used to make various
    // fields that can be referenced from any process visible from any process
    // context.  This is for things like mutexes, WSL chains, etc.
    //

    struct _MM_SESSION_SPACE *GlobalVirtualAddress;

    ULONG ReferenceCount;

    union {
        ULONG LongFlags;
        MM_SESSION_SPACE_FLAGS Flags;
    } u;

    ULONG SessionId;

    //
    // This is the list of the processes in this group that have
    // session space entries.
    //

    LIST_ENTRY ProcessList;

    LARGE_INTEGER LastProcessSwappedOutTime;

    //
    // All the page tables for session space use this as their parent.
    // Note that it's not really a page directory - it's really a page
    // table page itself (the one used to map this very structure).
    //
    // This provides a reference to something that won't go away and
    // is relevant regardless of which process within the session is current.
    //

    PFN_NUMBER SessionPageDirectoryIndex;

    //
    // This is the count of non paged allocations to support this session
    // space.  This includes the session structure page table and data pages,
    // WSL page table and data pages, session pool page table pages and session
    // image page table pages.  These are all charged against
    // MmResidentAvailable.
    //

    SIZE_T NonPagablePages;

    //
    // This is the count of pages in this session that have been charged against
    // the systemwide commit.  This includes all the NonPagablePages plus the
    // data pages they typically map.
    //

    SIZE_T CommittedPages;

    //
    // Start of session paged pool virtual space.
    //

    PVOID PagedPoolStart;

    //
    // Current end of pool virtual space. Can be extended to the
    // end of the session space.
    //

    PVOID PagedPoolEnd;

    //
    // PTE pointers for pool.
    //

    PMMPTE PagedPoolBasePde;

    ULONG Color;

    ULONG ProcessOutSwapCount;

    ULONG SessionPoolAllocationFailures[4];

    //
    // This is the list of system images currently valid in
    // this session space.  This information is in addition
    // to the module global information in PsLoadedModuleList.
    //

    LIST_ENTRY ImageList;

    LCID LocaleId;

    //
    // The count of "known attachers and the associated event.
    //

    ULONG AttachCount;

    KEVENT AttachEvent;

    PEPROCESS LastProcess;

    //
    // This is generally decremented in process delete (not clean) so that
    // the session data page and mapping PTE can finally be freed when this
    // reaches zero.  smss is the only process that decrements it in other
    // places as smss never exits.
    //

    LONG ProcessReferenceToSession;

    //
    // This chain is in global system addresses (not session VAs) and can
    // be walked from any system context, ie: for WSL trimming.
    //

    LIST_ENTRY WsListEntry;

    //
    // Session lookasides for fast pool allocation/freeing.
    //

    GENERAL_LOOKASIDE Lookaside[SESSION_POOL_SMALL_LISTS];

    //
    // Support for mapping system views into session space.  Each desktop
    // allocates a 3MB heap and the global system view space is only 48M
    // total.  This would limit us to only 20-30 users - rotating the
    // system view space with each session removes this limitation.
    //

    MMSESSION Session;

    //
    // Session space paged pool support.
    //

    KGUARDED_MUTEX PagedPoolMutex;

    MM_PAGED_POOL_INFO PagedPoolInfo;

    //
    // Working set information.
    //

    MMSUPPORT  Vm;
    PMMWSLE    Wsle;

    PDRIVER_UNLOAD Win32KDriverUnload;

    //
    // Pool descriptor for less than 1 page allocations.
    //

    POOL_DESCRIPTOR PagedPool;

#if (_MI_PAGING_LEVELS >= 3)

    //
    // The page directory that maps session space is saved here so
    // trimmers can attach.
    //

    MMPTE PageDirectory;

#else

    //
    // The second level page tables that map session space are shared
    // by all processes in the session.
    //

    PMMPTE PageTables;

#endif

#if defined (_WIN64)

    //
    // NT64 has enough virtual address space to support per-session special
    // pool.
    //

    PMMPTE SpecialPoolFirstPte;
    PMMPTE SpecialPoolLastPte;
    PMMPTE NextPdeForSpecialPoolExpansion;
    PMMPTE LastPdeForSpecialPoolExpansion;
    PFN_NUMBER SpecialPagesInUse;
#endif

#if defined(_IA64_)
    REGION_MAP_INFO SessionMapInfo;
    PFN_NUMBER PageDirectoryParentPage;
#endif

    LONG ImageLoadingCount;

#if DBG
    ULONG Debug[MM_SESS_COUNTER_MAX];

    MM_SESSION_MEMORY_COUNTERS Debug2[MM_SESS_MEMORY_COUNTER_MAX];
#endif

} MM_SESSION_SPACE, *PMM_SESSION_SPACE;

extern PMM_SESSION_SPACE MmSessionSpace;

extern ULONG MiSessionCount;

//
// This flushes just the non-global TB entries.
//

#define MI_FLUSH_SESSION_TB() KeFlushProcessTb (TRUE);

//
// The default number of pages for the session working set minimum & maximum.
//

#define MI_SESSION_SPACE_WORKING_SET_MINIMUM 20

#define MI_SESSION_SPACE_WORKING_SET_MAXIMUM 384

NTSTATUS
MiSessionInsertImage (
    IN PVOID BaseAddress
    );

NTSTATUS
MiSessionCommitPageTables (
    PVOID StartVa,
    PVOID EndVa
    );

NTSTATUS
MiInitializeAndChargePfn (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PFN_NUMBER ContainingPageFrame,
    IN LOGICAL SessionAllocation
    );

VOID
MiSessionPageTableRelease (
    IN PFN_NUMBER PageFrameIndex
    );

NTSTATUS
MiInitializeSessionPool (
    VOID
    );

VOID
MiCheckSessionPoolAllocations (
    VOID
    );

VOID
MiFreeSessionPoolBitMaps (
    VOID
    );

VOID
MiDetachSession (
    VOID
    );

VOID
MiAttachSession (
    IN PMM_SESSION_SPACE SessionGlobal
    );

VOID
MiReleaseProcessReferenceToSessionDataPage (
    PMM_SESSION_SPACE SessionGlobal
    );

extern PMMPTE MiHighestUserPte;
extern PMMPTE MiHighestUserPde;
#if (_MI_PAGING_LEVELS >= 4)
extern PMMPTE MiHighestUserPpe;
extern PMMPTE MiHighestUserPxe;
#endif

NTSTATUS
MiEmptyWorkingSet (
    IN PMMSUPPORT WsInfo,
    IN LOGICAL NeedLock
    );

//++
//ULONG
//MiGetPdeSessionIndex (
//    IN PVOID va
//    );
//
// Routine Description:
//
//    MiGetPdeSessionIndex returns the session structure index for the PDE
//    will (or does) map the given virtual address.
//
// Arguments
//
//    Va - Supplies the virtual address to locate the PDE index for.
//
// Return Value:
//
//    The index of the PDE entry.
//
//--

#define MiGetPdeSessionIndex(va)  ((ULONG)(((ULONG_PTR)(va) - (ULONG_PTR)MmSessionBase) >> PDI_SHIFT))

//
// Session space contains the image loader and tracker, virtual
// address allocator, paged pool allocator, system view image mappings,
// and working set for kernel mode virtual addresses that are instanced
// for groups of processes in a Session process group. This
// process group is identified by a SessionId.
//
// Each Session process group's loaded kernel modules, paged pool
// allocations, working set, and mapped system views are separate from
// other Session process groups, even though they have the same
// virtual addresses.
//
// This is to support the Hydra multi-user Windows NT system by
// replicating WIN32K.SYS, and its complement of video and printer drivers,
// desktop heaps, memory allocations, etc.
//

//
// Structure linked into a session space structure to describe
// which system images in PsLoadedModuleTable and
// SESSION_DRIVER_GLOBAL_LOAD_ADDRESS's
// have been allocated for the current session space.
//
// The reference count tracks the number of loads of this image within
// this session.
//

typedef struct _SESSION_GLOBAL_SUBSECTION_INFO {
    ULONG_PTR PteIndex;
    ULONG PteCount;
    ULONG Protection;
} SESSION_GLOBAL_SUBSECTION_INFO, *PSESSION_GLOBAL_SUBSECTION_INFO;

typedef struct _IMAGE_ENTRY_IN_SESSION {
    LIST_ENTRY Link;
    PVOID Address;
    PVOID LastAddress;
    ULONG ImageCountInThisSession;
    LOGICAL ImageLoading;  // Mods to this field protected by system load mutant
    PMMPTE PrototypePtes;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PSESSION_GLOBAL_SUBSECTION_INFO GlobalSubs;
} IMAGE_ENTRY_IN_SESSION, *PIMAGE_ENTRY_IN_SESSION;

extern LIST_ENTRY MiSessionWsList;

NTSTATUS
FASTCALL
MiCheckPdeForSessionSpace(
    IN PVOID VirtualAddress
    );

NTSTATUS
MiShareSessionImage (
    IN PVOID BaseAddress,
    IN PSECTION Section
    );

VOID
MiSessionWideInitializeAddresses (
    VOID
    );

NTSTATUS
MiSessionWideReserveImageAddress (
    IN PSECTION Section,
    OUT PVOID *AssignedAddress,
    OUT PSECTION *NewSectionPointer
    );

VOID
MiInitializeSessionIds (
    VOID
    );

VOID
MiInitializeSessionWsSupport(
    VOID
    );

VOID
MiSessionAddProcess (
    IN PEPROCESS NewProcess
    );

VOID
MiSessionRemoveProcess (
    VOID
    );

VOID
MiRemoveImageSessionWide (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry OPTIONAL,
    IN PVOID BaseAddress,
    IN ULONG_PTR NumberOfBytes
    );

NTSTATUS
MiDeleteSessionVirtualAddresses(
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes
    );

NTSTATUS
MiUnloadSessionImageByForce (
    IN SIZE_T NumberOfPtes,
    IN PVOID ImageBase
    );

PIMAGE_ENTRY_IN_SESSION
MiSessionLookupImage (
    IN PVOID BaseAddress
    );

VOID
MiSessionUnloadAllImages (
    VOID
    );

VOID
MiFreeSessionSpaceMap (
    VOID
    );

NTSTATUS
MiSessionInitializeWorkingSetList (
    VOID
    );

VOID
MiSessionUnlinkWorkingSet (
    VOID
    );

VOID
MiSessionOutSwapProcess (
    IN PEPROCESS Process
    );

VOID
MiSessionInSwapProcess (
    IN PEPROCESS Process
    );

#if !defined (_X86PAE_)

#define MI_GET_DIRECTORY_FRAME_FROM_PROCESS(_Process) \
        MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)(&((_Process)->Pcb.DirectoryTableBase[0])))

#define MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS(_Process) \
        MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)(&((_Process)->Pcb.DirectoryTableBase[1])))

#else

#define MI_GET_DIRECTORY_FRAME_FROM_PROCESS(_Process) \
        (MI_GET_PAGE_FRAME_FROM_PTE(((PMMPTE)((_Process)->PaeTop)) + PD_PER_SYSTEM - 1))

#define MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS(_Process) \
        ((PFN_NUMBER)((_Process)->Pcb.DirectoryTableBase[1]))

#endif

#if defined(_MIALT4K_)
NTSTATUS
MiSetCopyPagesFor4kPage (
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    IN OUT PVOID StartingAddress,
    IN OUT PVOID EndingAddress,
    IN ULONG ProtectionMask,
    OUT PMMVAD *CallerNewVad
    );

VOID
MiRemoveAliasedVads (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    );

PVOID
MiDuplicateAliasVadList (
    IN PMMVAD Vad
    );
#endif

//
// The LDR_DATA_TABLE_ENTRY->LoadedImports is used as a list of imported DLLs.
//
// This field is zero if the module was loaded at boot time and the
// import information was never filled in.
//
// This field is -1 if no imports are defined by the module.
//
// This field contains a valid paged pool PLDR_DATA_TABLE_ENTRY pointer
// with a low-order (bit 0) tag of 1 if there is only 1 usable import needed
// by this driver.
//
// This field will contain a valid paged pool PLOAD_IMPORTS pointer in all
// other cases (ie: where at least 2 imports exist).
//

typedef struct _LOAD_IMPORTS {
    SIZE_T                  Count;
    PKLDR_DATA_TABLE_ENTRY   Entry[1];
} LOAD_IMPORTS, *PLOAD_IMPORTS;

#define LOADED_AT_BOOT  ((PLOAD_IMPORTS)0)
#define NO_IMPORTS_USED ((PLOAD_IMPORTS)-2)

#define SINGLE_ENTRY(ImportVoid)    ((ULONG)((ULONG_PTR)(ImportVoid) & 0x1))

#define SINGLE_ENTRY_TO_POINTER(ImportVoid)    ((PKLDR_DATA_TABLE_ENTRY)((ULONG_PTR)(ImportVoid) & ~0x1))

#define POINTER_TO_SINGLE_ENTRY(Pointer)    ((PKLDR_DATA_TABLE_ENTRY)((ULONG_PTR)(Pointer) | 0x1))

// #define _MI_DEBUG_RONLY 1     // Uncomment this for session readonly tracking

#if _MI_DEBUG_RONLY

VOID
MiAssertNotSessionData (
    IN PMMPTE PointerPte
    );

VOID
MiLogSessionDataStart (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

#define MI_ASSERT_NOT_SESSION_DATA(PTE) MiAssertNotSessionData(PTE)
#define MI_LOG_SESSION_DATA_START(DataTableEntry) MiLogSessionDataStart(DataTableEntry)
#else
#define MI_ASSERT_NOT_SESSION_DATA(PTE)
#define MI_LOG_SESSION_DATA_START(DataTableEntry)
#endif

//
// This tracks driver-specified individual verifier thunks.
//

typedef struct _DRIVER_SPECIFIED_VERIFIER_THUNKS {
    LIST_ENTRY ListEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG NumberOfThunks;
} DRIVER_SPECIFIED_VERIFIER_THUNKS, *PDRIVER_SPECIFIED_VERIFIER_THUNKS;

// #define _MI_DEBUG_SUB 1         // Uncomment this for subsection logging

#if defined (_MI_DEBUG_SUB)

extern ULONG MiTrackSubs;

#define MI_SUB_BACKTRACE_LENGTH 8

typedef struct _MI_SUB_TRACES {

    PETHREAD Thread;
    PMSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    ULONG_PTR CallerId;
    PVOID StackTrace [MI_SUB_BACKTRACE_LENGTH];

    MSUBSECTION SubsectionContents;
    CONTROL_AREA ControlAreaContents;

} MI_SUB_TRACES, *PMI_SUB_TRACES;

extern LONG MiSubsectionIndex;

extern PMI_SUB_TRACES MiSubsectionTraces;

VOID
FORCEINLINE
MiSnapSubsection (
    IN PMSUBSECTION Subsection,
    IN ULONG CallerId
    )
{
    PMI_SUB_TRACES Information;
    PCONTROL_AREA ControlArea;
    ULONG Index;
    ULONG Hash;

    if (MiSubsectionTraces == NULL) {
        return;
    }

    ControlArea = Subsection->ControlArea;

    Index = InterlockedIncrement (&MiSubsectionIndex);
    Index &= (MiTrackSubs - 1);
    Information = &MiSubsectionTraces[Index];

    Information->Subsection = Subsection;
    Information->ControlArea = ControlArea;
    *(PMSUBSECTION)&Information->SubsectionContents = *Subsection;
    *(PCONTROL_AREA)&Information->ControlAreaContents = *ControlArea;
    Information->Thread = PsGetCurrentThread();
    Information->CallerId = CallerId;

#if defined (_WIN64)
    if (KeAreAllApcsDisabled () == TRUE) {
        Information->StackTrace[1] = (PVOID) _ReturnAddress ();
        Information->StackTrace[0] = MiGetInstructionPointer ();
    }
    else
#endif
    RtlCaptureStackBackTrace (0, MI_SUB_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

#define MI_SNAP_SUB(_Sub, callerid) MiSnapSubsection(_Sub, callerid)

#else
#define MI_SNAP_SUB(_Sub, callerid)
#endif

//
//  Hot-patching private definitions
//

extern LIST_ENTRY MiHotPatchList;

#endif  // MI
