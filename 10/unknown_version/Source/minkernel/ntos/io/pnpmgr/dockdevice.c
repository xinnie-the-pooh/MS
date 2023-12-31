/*++

Copyright (C) Microsoft. All rights reserved.

Module Name:

    PpProfile.c

Abstract:

    Kernel-mode Plug and Play Manager Docking and Hardware Profile Support
    Routines.

Author:

    Adrian J. Oney (AdriaO) June 1998
    Kenneth D. Ray (kenray) June 1998

Revision History:



--*/

#include "pnpmgrp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PpProfileInit)
#pragma alloc_text(PAGE, PpProfileBeginHardwareProfileTransition)
#pragma alloc_text(PAGE, PpProfileIncludeInHardwareProfileTransition)
#pragma alloc_text(PAGE, PpProfileQueryHardwareProfileChange)
#pragma alloc_text(PAGE, PpProfileCommitTransitioningDock)
#pragma alloc_text(PAGE, PpProfileCancelTransitioningDock)
#pragma alloc_text(PAGE, PpProfileCancelHardwareProfileTransition)
#pragma alloc_text(PAGE, PpProfileMarkAllTransitioningDocksEjected)
#pragma alloc_text(PAGE, PpProfileProcessDockDeviceCapability)
#pragma alloc_text(PAGE, PiProfileSendHardwareProfileCommit)
#pragma alloc_text(PAGE, PiProfileSendHardwareProfileCancel)
#pragma alloc_text(PAGE, PiProfileUpdateDeviceTree)
#pragma alloc_text(PAGE, PiProfileUpdateDeviceTreeWorker)
#pragma alloc_text(PAGE, PiProfileUpdateDeviceTreeCallback)
#pragma alloc_text(PAGE, PnpProfileUpdateHardwareProfile)
#endif

//
// List of current dock devices, and the number of dockdevices.
// Must hold PiProfileDeviceListLock to change these values.
//
LIST_ENTRY  PiProfileDeviceListHead;
ULONG       PiProfileDeviceCount;
KGUARDED_MUTEX PiProfileDeviceListLock;
KSEMAPHORE  PiProfileChangeSemaphore;
BOOLEAN     PiProfileChangeCancelRequired;
__volatile LONG        PiProfileDevicesInTransition;


VOID
PpProfileInit(
    VOID
    )
/*++

Routine Description:

    This routine initializes docking support for Win2K.

Arguments:

    None.

Return Value:

    Nope.

--*/
{
    //
    // Initialize the list of dock devices, and its lock.
    //
    InitializeListHead(&PiProfileDeviceListHead);
    KeInitializeGuardedMutex(&PiProfileDeviceListLock);
    PiProfileDeviceCount = 0;
    KeInitializeSemaphore(&PiProfileChangeSemaphore, 1, 1);
}


VOID
PpProfileBeginHardwareProfileTransition(
    __in BOOLEAN SubsumeExistingDeparture
    )
/*++

Routine Description:

    This routine must be called before any dock devnodes can be marked for
    transition (ie arriving or departing). After calling this function,
    PpProfileIncludeInHardwareProfileTransition should be called for each dock
    that is appearing or disappearing.

    Functionally, this code acquires the profile change semaphore. Future
    changes in the life of the added dock devnodes cause it to be released.

Arguments:

    SubsumeExistingDeparture - Set if we are ejecting the parent of a
                               device that is still in the process of
                               ejecting...

Return Value:

    None.

--*/
{
    NTSTATUS status;

    PAGED_CODE();

    if (SubsumeExistingDeparture) {

        //
        // We will already have queried in this case. Also, enumeration is
        // locked right now, so the appropriate devices found cannot disappear.
        // Assert everything is consistant.
        //
        ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
        PNP_ASSERT(PiProfileDevicesInTransition != 0);
        return;
    }

    //
    // Take the profile change semaphore. We do this whenever a dock is
    // in our list, even if no query is going to occur.
    //
    status = KeWaitForSingleObject(
        &PiProfileChangeSemaphore,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    PNP_ASSERT(status == STATUS_SUCCESS);
}


VOID
PpProfileIncludeInHardwareProfileTransition(
    __in  PDEVICE_NODE    DeviceNode,
    __in  PROFILE_STATUS  ChangeInPresence
    )
/*++

Routine Description:

    This routine is called to mark a dock as "in transition", ie it is either
    disappearing or appearing, the results of which determine our final
    hardware profile state. After all the docks that are transitioning have
    been passed into this function, PiProfileQueryHardwareProfileChange is
    called.

Arguments:

    DeviceNode          - The dock devnode that is appearing or disappearing
    ChangeInPresence    - Either DOCK_DEPARTING or DOCK_ARRIVING

Return Value:

    Nope.

--*/
{
    PWCHAR          deviceSerialNumber;
    PDEVICE_OBJECT  deviceObject;
    NTSTATUS        status;

    PAGED_CODE();

    //
    // Verify we are under semaphore, we aren't marking the dock twice, and
    // our parameters are sensable.
    //
    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
    PNP_ASSERT(DeviceNode->DockInfo.DockStatus == DOCK_QUIESCENT);
    PNP_ASSERT((ChangeInPresence == DOCK_DEPARTING)||
           (ChangeInPresence == DOCK_ARRIVING));

    if (ChangeInPresence == DOCK_ARRIVING) {

        //
        // First, ensure this dock is a member of the dock list.
        //
        // ADRIAO N.B. 07/09/2000 -
        //     We should move this into IopProcessNewDeviceNode, or perhaps
        // PipStartPhaseN.
        //
        if (IsListEmpty(&DeviceNode->DockInfo.ListEntry)) {

            //
            // Acquire the lock on the list of dock devices
            //
            KeAcquireGuardedMutex(&PiProfileDeviceListLock);

            //
            // Add this element to the head of the list
            //
            InsertHeadList(&PiProfileDeviceListHead,
                           &DeviceNode->DockInfo.ListEntry);
            PiProfileDeviceCount++;

            //
            // Release the lock on the list of dock devices
            //
            KeReleaseGuardedMutex(&PiProfileDeviceListLock);
        }

        //
        // Retrieve the Serial Number from this dock device. We do this just
        // to test the BIOS today. Later we will be acquiring the information
        // to determine the profile we are *about* to enter.
        //
        deviceObject = DeviceNode->PhysicalDeviceObject;

        status = PnpQuerySerialNumber(DeviceNode,
                                      &deviceSerialNumber);

        if (NT_SUCCESS(status) && (deviceSerialNumber != NULL)) {

            ExFreePool(deviceSerialNumber);
        }

    } else {

        //
        // DOCK_DEPARTING case, we must be a member of the dock list...
        //
        PNP_ASSERT(!IsListEmpty(&DeviceNode->DockInfo.ListEntry));
    }

    InterlockedIncrement(&PiProfileDevicesInTransition);
    DeviceNode->DockInfo.DockStatus = ChangeInPresence;
}


NTSTATUS
PpProfileQueryHardwareProfileChange(
    __in  BOOLEAN                     SubsumingExistingDeparture,
    __in  PROFILE_NOTIFICATION_TIME   InPnpEvent,
    __out PPNP_VETO_TYPE              VetoType,
    __out_opt PUNICODE_STRING             VetoName
    )
/*++

Routine Description:

    This function queries drivers to see if it is OK to exit the current
    hardware profile and enter next one (as determined by which docks have
    been marked). One of two functions should be used subsequently to this
    call:
        PpProfileCommitTransitioningDock
            (call when a dock has been successfully started or has disappeared)
        PpProfileCancelHardwareProfileTransition
            (call to abort a transition, say if a dock failed to start or a
             query returned failure for eject)

Arguments:

    InPnpEvent  - This argument indicates whether an operation is being done
                  within the context of another PnpEvent or not. If not, we
                  will queue such an event and block on it. If so, we cannot
                  queue&block (we'd deadlock), so we do the query manually.
    VetoType    - If this function returns false, this parameter will describe
                  who failed the query profile change. The below optional
                  parameter will contain the name of said vetoer.
    VetoName    - This optional parameter will get the name of the vetoer (ie
                  devinst, service name, application name, etc). If VetoName
                  is supplied, the caller must free the buffer returned.

Return Value:

    NTSTATUS.

--*/
{
    NTSTATUS status;
    BOOLEAN arrivingDockFound;
    PLIST_ENTRY listEntry;
    PDEVICE_NODE devNode;

    PAGED_CODE();

    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);

    //
    // Acquire the lock on the list of dock devices and determine whether any
    // dock devnodes are arriving.
    //
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

    PNP_ASSERT(PiProfileDevicesInTransition);

    arrivingDockFound = FALSE;
    for (listEntry  = PiProfileDeviceListHead.Flink;
        listEntry != &(PiProfileDeviceListHead);
        listEntry  = listEntry->Flink ) {

        devNode = CONTAINING_RECORD(listEntry,
                                    DEVICE_NODE,
                                    DockInfo.ListEntry);

        PNP_ASSERT((devNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE)&&
               (devNode->DockInfo.DockStatus != DOCK_EJECTIRP_COMPLETED));

        if (devNode->DockInfo.DockStatus == DOCK_ARRIVING) {

            arrivingDockFound = TRUE;
        }
    }

    //
    // Release the lock on the list of dock devices
    //
    KeReleaseGuardedMutex(&PiProfileDeviceListLock);

    if (SubsumingExistingDeparture) {

        PNP_ASSERT(PiProfileChangeCancelRequired);
        //
        // We're nesting. Work off the last query, and don't requery.
        //
        return STATUS_SUCCESS;
    }

    if (arrivingDockFound) {

        //
        // We currently don't actually query for hardware profile change on a
        // dock event as the user may have the lid closed. If we ever find a
        // piece of hardware that needs to be updated *prior* to actually
        // switching over, we will have to remove this bit of code.
        //
        PiProfileChangeCancelRequired = FALSE;
        return STATUS_SUCCESS;
    }

    IopDbgPrint((IOP_TRACE_LEVEL,
               "NTOSKRNL: Sending HW profile change [query]\n"));

    status = PnpRequestHwProfileChangeNotification(&GUID_HWPROFILE_QUERY_CHANGE,
                                                   InPnpEvent,
                                                   VetoType,
                                                   VetoName);

    if (NT_SUCCESS(status)) {
        PiProfileChangeCancelRequired = TRUE;
    } else {
        PiProfileChangeCancelRequired = FALSE;
    }
    return status;
}


VOID
PpProfileCommitTransitioningDock(
    __in PDEVICE_NODE     DeviceNode,
    __in PROFILE_STATUS   ChangeInPresence
    )
/*++

Routine Description:

    This routine finalized the state the specified device in the list of
    current dock devices and requests a Hardware Profile change.

Arguments:

    DeviceNode - The dock devnode that has finished being started or removed.
    ChangeInPresence - Either DOCK_DEPARTING or DOCK_ARRIVING

Return Value:

    None.

--*/
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PWCHAR deviceSerialNumber;
    BOOLEAN profileChanged;
    LONG remainingDockCount;

    PAGED_CODE();

    //
    // If we are commiting a dock, the transition list should not be empty.
    // all dock devices present, the list should not be empty.
    //
    PNP_ASSERT(!IsListEmpty(&DeviceNode->DockInfo.ListEntry));
    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);

    if (ChangeInPresence == DOCK_DEPARTING) {

        PNP_ASSERT((DeviceNode->DockInfo.DockStatus == DOCK_DEPARTING) ||
               (DeviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED));

        //
        // Free up the serial number
        //
        if (DeviceNode->DockInfo.SerialNumber != NULL) {

            ExFreePool(DeviceNode->DockInfo.SerialNumber);
            DeviceNode->DockInfo.SerialNumber = NULL;
        }

        //
        // Acquire the lock on the list of dock devices
        //
        KeAcquireGuardedMutex(&PiProfileDeviceListLock);

        //
        // Remove the current devnode from the list of docks
        //
        RemoveEntryList(&DeviceNode->DockInfo.ListEntry);
        InitializeListHead(&DeviceNode->DockInfo.ListEntry);
        PiProfileDeviceCount--;

        //
        // Release the lock on the list of dock devices
        //
        KeReleaseGuardedMutex(&PiProfileDeviceListLock);

    } else {

        PNP_ASSERT(DeviceNode->DockInfo.DockStatus == DOCK_ARRIVING);

        //
        // We only add one dock at a time. So this should have been the last!
        //
        PNP_ASSERT(PiProfileDevicesInTransition == 1);

        //
        // Retrieve the Serial Number from this dock device if we don't already
        // have it.
        //
        if (DeviceNode->DockInfo.SerialNumber == NULL) {

            deviceObject = DeviceNode->PhysicalDeviceObject;

            status = PnpQuerySerialNumber(DeviceNode,
                                         &deviceSerialNumber);

            DeviceNode->DockInfo.SerialNumber = deviceSerialNumber;
        }
    }

    DeviceNode->DockInfo.DockStatus = DOCK_QUIESCENT;
    remainingDockCount = InterlockedDecrement(&PiProfileDevicesInTransition);
    PNP_ASSERT(remainingDockCount >= 0);

    if (remainingDockCount) {

        return;
    }

    profileChanged = FALSE;

    if ((ChangeInPresence == DOCK_ARRIVING) &&
        (DeviceNode->DockInfo.SerialNumber == NULL)) {

        //
        // Couldn't get Serial Number for this dock device, or serial number
        // was NULL. We can make this check here as only one dock at a time
        // can currently arrive.
        //
        status = STATUS_UNSUCCESSFUL;
        goto BroadcastAndLeave;
    }

    //
    // Update the current Hardware Profile now that the transition list has
    // been emptied. This routine does two things for us:
    // 1) It determines whether the profile actually changed and updates
    //    the global flag IopProfileChangeOccured appropriately.
    // 2) If the profile changed, this routine updates the registry.
    //
    status = PnpProfileUpdateHardwareProfile(&profileChanged);
    if (!NT_SUCCESS(status)) {

        IopDbgPrint((IOP_TRACE_LEVEL,
                   "PnpProfileUpdateHardwareProfile failed with status == %lx\n", status));
    }

BroadcastAndLeave:

    //
    // Clean up
    //
    if (NT_SUCCESS(status) && profileChanged) {

        PiProfileSendHardwareProfileCommit();
        PiProfileUpdateDeviceTree();

    } else if (PiProfileChangeCancelRequired) {

        PiProfileSendHardwareProfileCancel();
    }

    KeReleaseSemaphore(
        &PiProfileChangeSemaphore,
        IO_NO_INCREMENT,
        1,
        FALSE
        );

    return;
}


VOID
PpProfileCancelTransitioningDock(
    __in PDEVICE_NODE     DeviceNode,
    __in PROFILE_STATUS   ChangeInPresence
    )
/*++

Routine Description:

    This routine is called when a dock that was marked to disappear didn't (ie,
    after the eject, the dock device still enumerated). We remove it from the
    transition list and complete/cancel the HW profile change as appropriate.
    See PpProfileMarkAllTransitioningDocksEjected.

Arguments:

    DeviceNode - The dock devnode that either didn't start or didn't disappear.
    ChangeInPresence - Either DOCK_DEPARTING or DOCK_ARRIVING

    N.B. - Currently only DOCK_DEPARTING is supported.

Return Value:

    None.

--*/
{
    NTSTATUS status;
    BOOLEAN  profileChanged;
    LONG     remainingDockCount;

#if !DBG
    UNREFERENCED_PARAMETER (ChangeInPresence);
#endif

    PAGED_CODE();

    PNP_ASSERT(ChangeInPresence == DOCK_DEPARTING);

    //
    // Acquire the lock on the list of dock devices
    //
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

    //
    // Since we are about to remove this dock device from the list of
    // all dock devices present, the list should not be empty.
    //
    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
    PNP_ASSERT(DeviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED);
    PNP_ASSERT(!IsListEmpty(&DeviceNode->DockInfo.ListEntry));

    DeviceNode->DockInfo.DockStatus = DOCK_QUIESCENT;
    remainingDockCount = InterlockedDecrement(&PiProfileDevicesInTransition);
    PNP_ASSERT(remainingDockCount >= 0);

    //
    // Release the lock on the list of dock devices
    //
    KeReleaseGuardedMutex(&PiProfileDeviceListLock);

    if (remainingDockCount) {

        return;
    }

    //
    // Update the current Hardware Profile after removing this device.
    //
    status = PnpProfileUpdateHardwareProfile(&profileChanged);

    if (!NT_SUCCESS(status)) {

        //
        // So we're there physically, but not mentally? Too bad, where broadcasting
        // change either way.
        //
        IopDbgPrint((IOP_TRACE_LEVEL,
                   "PnpProfileUpdateHardwareProfile failed with status == %lx\n", status));

        PNP_ASSERT(NT_SUCCESS(status));
    }

    if (NT_SUCCESS(status) && profileChanged) {

        PiProfileSendHardwareProfileCommit();
        PiProfileUpdateDeviceTree();

    } else {

        PNP_ASSERT(PiProfileChangeCancelRequired);
        PiProfileSendHardwareProfileCancel();
    }

    KeReleaseSemaphore(
        &PiProfileChangeSemaphore,
        IO_NO_INCREMENT,
        1,
        FALSE
        );

    return;
}


VOID
PpProfileCancelHardwareProfileTransition(
    VOID
    )
/*++

Routine Description:

    This routine unmarks any marked devnodes (ie, sets them to no change,
    appearing or disappearing), and sends the CancelQueryProfileChange as
    appropriate. Once called, other profile changes can occur.

Arguments:

    None.

Return Value:

    Nodda.

--*/
{
    PLIST_ENTRY  listEntry;
    PDEVICE_NODE devNode;

    PAGED_CODE();

    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);

    //
    // Acquire the lock on the list of dock devices
    //
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

    for (listEntry  = PiProfileDeviceListHead.Flink;
        listEntry != &(PiProfileDeviceListHead);
        listEntry  = listEntry->Flink ) {

        devNode = CONTAINING_RECORD(listEntry,
                                    DEVICE_NODE,
                                    DockInfo.ListEntry);

        PNP_ASSERT((devNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE)&&
               (devNode->DockInfo.DockStatus != DOCK_EJECTIRP_COMPLETED));
        if (devNode->DockInfo.DockStatus != DOCK_QUIESCENT) {

            InterlockedDecrement(&PiProfileDevicesInTransition);
            devNode->DockInfo.DockStatus = DOCK_QUIESCENT;
        }
    }

    PNP_ASSERT(!PiProfileDevicesInTransition);

    //
    // Release the lock on the list of dock devices
    //
    KeReleaseGuardedMutex(&PiProfileDeviceListLock);

    if (PiProfileChangeCancelRequired) {

        PiProfileSendHardwareProfileCancel();
    }

    KeReleaseSemaphore(
        &PiProfileChangeSemaphore,
        IO_NO_INCREMENT,
        1,
        FALSE
        );
}


VOID
PpProfileMarkAllTransitioningDocksEjected(
    VOID
    )
/*++

Routine Description:

    This routine moves any departing devnodes to the ejected state. If any
    subsequent enumeration lists the device as present, we know the eject
    failed and we appropriately cancel that piece of the profile change.
    PpProfileCancelTransitioningDock can only be called after this function
    is called.

Arguments:

    None.

Return Value:

    Nodda.

--*/
{
    PLIST_ENTRY  listEntry;
    PDEVICE_NODE devNode;

    PAGED_CODE();

    //
    // The semaphore might not be signalled if the dock was resurrected before
    // the eject completed. This can happen in warm undock scenarios where the
    // machine is resumed inside the dock instead of being detached.
    //
    //ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);

    //
    // Acquire the lock on the list of dock devices
    //
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

    for (listEntry  = PiProfileDeviceListHead.Flink;
        listEntry != &(PiProfileDeviceListHead);
        listEntry  = listEntry->Flink ) {

        devNode = CONTAINING_RECORD(listEntry,
                                    DEVICE_NODE,
                                    DockInfo.ListEntry);

        PNP_ASSERT((devNode->DockInfo.DockStatus == DOCK_QUIESCENT)||
               (devNode->DockInfo.DockStatus == DOCK_DEPARTING));
        if (devNode->DockInfo.DockStatus != DOCK_QUIESCENT) {

            devNode->DockInfo.DockStatus = DOCK_EJECTIRP_COMPLETED;
        }
    }

    //
    // Release the lock on the list of dock devices
    //
    KeReleaseGuardedMutex(&PiProfileDeviceListLock);
}


VOID
PiProfileSendHardwareProfileCommit(
    VOID
    )
/*++

Routine Description:

    This routine (internal to ppdock.c) simply sends the change complete message.
    We do not wait for this, as it is asynchronous...

Arguments:

    None.

Return Value:

    Nodda.

--*/
{
    PAGED_CODE();

    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
    IopDbgPrint((IOP_TRACE_LEVEL,
               "NTOSKRNL: Sending HW profile change [commit]\n"));

    PnpRequestHwProfileChangeNotification(&GUID_HWPROFILE_CHANGE_COMPLETE,
                                          PROFILE_PERHAPS_IN_PNPEVENT,
                                          NULL,
                                          NULL);
}


VOID
PiProfileSendHardwareProfileCancel(
    VOID
    )
/*++

Routine Description:

    This routine (internal to ppdock.c) simply sends the cancel.

Arguments:

    None.

Return Value:

    Nodda.

--*/
{
    PAGED_CODE();

    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
    IopDbgPrint((IOP_TRACE_LEVEL,
               "NTOSKRNL: Sending HW profile change [cancel]\n"));

    PnpRequestHwProfileChangeNotification(&GUID_HWPROFILE_CHANGE_CANCELLED,
                                          PROFILE_PERHAPS_IN_PNPEVENT,
                                          NULL,
                                          NULL);
}


NTSTATUS
PnpProfileUpdateHardwareProfile(
    __out BOOLEAN     *ProfileChanged
    )
/*++

Routine Description:

    This routine scans the list of current dock devices, builds a list of serial
    numbers from those devices, and calls for the Hardware Profile to be
    changed, based on that list.

Arguments:

    ProfileChanged - Supplies a variable to receive TRUE if the current hardware
                     profile changes as a result of calling this routine.

Return Value:

    NTSTATUS code.

--*/
{
    NTSTATUS status = STATUS_SUCCESS;
    PLIST_ENTRY  listEntry;
    PDEVICE_NODE devNode;
    PWCHAR  *profileSerialNumbers, *p;
    HANDLE  hProfileKey=NULL;
    ULONG   len, numProfiles;
    HANDLE  hCurrent, hIDConfigDB;
    UNICODE_STRING unicodeName;

    PAGED_CODE();

    //
    // Acquire the lock on the list of dock devices
    //
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

    //
    // Update the flag for Ejectable Docks (flag is the count of docks)
    //
    PiWstrToUnicodeString(&unicodeName, CM_HARDWARE_PROFILE_STR_DATABASE);
    if(NT_SUCCESS(IopOpenRegistryKeyEx(&hIDConfigDB,
                                       NULL,
                                       &unicodeName,
                                       KEY_READ) )) {

        PiWstrToUnicodeString(&unicodeName, CM_HARDWARE_PROFILE_STR_CURRENT_DOCK_INFO);
        if(NT_SUCCESS(IopOpenRegistryKeyEx(&hCurrent,
                                           hIDConfigDB,
                                           &unicodeName,
                                           KEY_READ | KEY_WRITE) )) {

            PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_EJECTABLE_DOCKS);
            ZwSetValueKey(hCurrent,
                          &unicodeName,
                          0,
                          REG_DWORD,
                          &PiProfileDeviceCount,
                          sizeof(PiProfileDeviceCount));
            ZwClose(hCurrent);
        }
        ZwClose(hIDConfigDB);
    }

    if (PiProfileDeviceCount == 0) {
        //
        // if there are no dock devices, the list should
        // contain a single null entry, in addition to the null
        // termination.
        //
        numProfiles = 1;
        PNP_ASSERT(IsListEmpty(&PiProfileDeviceListHead));
    } else {
        numProfiles = PiProfileDeviceCount;
        PNP_ASSERT(!IsListEmpty(&PiProfileDeviceListHead));
    }

    //
    // Allocate space for a null-terminated list of SerialNumber lists.
    //
    len = (numProfiles+1)*sizeof(PWCHAR);
    profileSerialNumbers = ExAllocatePool(NonPagedPoolNx, len);

    if (profileSerialNumbers) {

        p = profileSerialNumbers;

        //
        // Create the list of Serial Numbers
        //
        for (listEntry  = PiProfileDeviceListHead.Flink;
             listEntry != &(PiProfileDeviceListHead);
             listEntry  = listEntry->Flink ) {

            devNode = CONTAINING_RECORD(listEntry,
                                        DEVICE_NODE,
                                        DockInfo.ListEntry);

            PNP_ASSERT(devNode->DockInfo.DockStatus == DOCK_QUIESCENT);
            if (devNode->DockInfo.SerialNumber) {
                PNP_ASSERT((ULONG)(p - profileSerialNumbers) <= numProfiles);
                __analysis_assume((ULONG)(p - profileSerialNumbers) <= numProfiles);

                *p = devNode->DockInfo.SerialNumber;
                p++;
            }
        }

        KeReleaseGuardedMutex(&PiProfileDeviceListLock);

        if (p == profileSerialNumbers) {
            //
            // Set a single list entry to NULL if we look to be in an "undocked"
            // profile
            //
            *p = NULL;
            p++;
        }

        //
        // Null-terminate the list
        //
        *p = NULL;

        numProfiles = (ULONG)(p - profileSerialNumbers);

        //
        // Change the current Hardware Profile based on the new Dock State
        // and perform notification that the Hardware Profile has changed
        //
        status = IopExecuteHardwareProfileChange(HardwareProfileBusTypeACPI,
                                                 profileSerialNumbers,
                                                 numProfiles,
                                                 &hProfileKey,
                                                 ProfileChanged);
        if (hProfileKey) {
            ZwClose(hProfileKey);
        }
        ExFreePool (profileSerialNumbers);

    } else {

        KeReleaseGuardedMutex(&PiProfileDeviceListLock);

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


NTSTATUS
PiProfileUpdateDeviceTree(
    VOID
    )
/*++

Routine Description:

    This function is called after the system has transitioned into a new
    hardware profile. The thread from which it is called may be holding an
    enumeration lock. Calling this function does two tasks:

    1) If a disabled devnode in the tree should be enabled in this new hardware
       profile state, it will be started.

    2) If an enabled devnode in the tree should be disabled in this new hardware
       profile state, it will be (surprise) removed.

    ADRIAO N.B. 02/19/1999 -
        Why surprise remove? There are four cases to be handled:
        a) Dock disappearing, need to enable device in new profile
        b) Dock appearing, need to enable device in new profile
        c) Dock disappearing, need to disable device in new profile
        d) Dock appearing, need to disable device in new profile

        a) and b) are trivial. c) involves treating the appropriate devices as
        if they were in the removal relation lists for the dock. d) is another
        matter altogether as we need to query-remove/remove devices before
        starting another. NT5's PnP state machine cannot handle this, so for
        this release we cleanup rather hastily after the profile change.

Parameters:

    NONE.

Return Value:

    NTSTATUS.

--*/
{
    PWORK_QUEUE_ITEM workQueueItem;

    PAGED_CODE();

    workQueueItem = (PWORK_QUEUE_ITEM) ExAllocatePool(
        NonPagedPoolNx,
        sizeof(WORK_QUEUE_ITEM)
        );

    if (workQueueItem) {

        //
        // Queue this up so we can walk the tree outside of the enumeration lock.
        //
        ExInitializeWorkItem(
            workQueueItem,
            PiProfileUpdateDeviceTreeWorker,
            workQueueItem
            );

        ExQueueWorkItem(
            workQueueItem,
            CriticalWorkQueue
            );

        return STATUS_SUCCESS;

    } else {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
}


VOID
PiProfileUpdateDeviceTreeWorker(
    __in PVOID Context
    )
/*++

Routine Description:

    This function is called on a work thread by PiProfileUpdateDeviceTree
    when the system has transitioned to a new hardware profile.

Parameters:

    NONE.

Return Value:

    NONE.

--*/
{
    PAGED_CODE();

    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);
    PipForAllDeviceNodes(PiProfileUpdateDeviceTreeCallback, NULL);
    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

    ExFreePool(Context);
}


NTSTATUS
PiProfileUpdateDeviceTreeCallback(
    __in PDEVICE_NODE DeviceNode,
    __in PVOID Context
    )
/*++

Routine Description:

    This function is called for each devnode after the system has transitioned
    hardware profile states.

Parameters:

    NONE.

Return Value:

    NONE.

--*/
{
    PDEVICE_NODE parentDevNode;

    UNREFERENCED_PARAMETER( Context );

    PAGED_CODE();

    if (DeviceNode->State == DeviceNodeStarted) {

        //
        // Calling this function will disable the device if it is appropriate
        // to do so.
        //

        if (!PnpIsDeviceInstanceEnabled(NULL, PnpGetDeviceInstancePath(DeviceNode), FALSE)) {
            PnpRequestDeviceRemoval(DeviceNode,
                                    FALSE,
                                    CM_PROB_DISABLED,
                                    STATUS_SUCCESS);
        }

    } else if (((DeviceNode->State == DeviceNodeInitialized) ||
                (DeviceNode->State == DeviceNodeRemoved)) &&
               PipIsDevNodeProblem(DeviceNode, CM_PROB_DISABLED)) {

        //
        // We might be turning on the device. So we will clear the problem
        // flags iff the device problem was CM_PROB_DISABLED. We must clear the
        // problem code or otherwise PnpIsDeviceInstanceEnabled will ignore us.
        //
        PipClearDevNodeProblem(DeviceNode);

        //
        // Make sure the device stays down iff appropriate.
        //
        if (PnpIsDeviceInstanceEnabled(NULL, PnpGetDeviceInstancePath(DeviceNode), FALSE)) {

            //
            // This device should come back online. Bring it out of the
            // removed state and queue up an enumeration at the parent level
            // to resurrect him.
            //
            PnpRestartDeviceNode(DeviceNode);

            parentDevNode = DeviceNode->Parent;

            IoInvalidateDeviceRelations(
                parentDevNode->PhysicalDeviceObject,
                BusRelations
                );

        } else {

            //
            // Restore the problem code.
            //
            PipSetDevNodeProblem(DeviceNode, CM_PROB_DISABLED, STATUS_SUCCESS);
        }

    } else {

         PNP_ASSERT((!PipIsDevNodeProblem(DeviceNode, CM_PROB_DISABLED)) ||
                ((DeviceNode->State == DeviceNodeAwaitingQueuedRemoval) ||
                 (DeviceNode->State == DeviceNodeAwaitingQueuedDeletion)));
    }

    return STATUS_SUCCESS;
}

VOID
PpProfileProcessDockDeviceCapability(
    __in PDEVICE_NODE DeviceNode,
    __in PDEVICE_CAPABILITIES Capabilities
    )
{
    PAGED_CODE();

    if (Capabilities->DockDevice) {

        if (DeviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED) {

            PNP_ASSERT(DeviceNode->DockInfo.DockStatus != DOCK_EJECTIRP_COMPLETED);
            PpProfileCancelTransitioningDock(DeviceNode, DOCK_DEPARTING);
        }
        DeviceNode->DockInfo.DockStatus = DOCK_QUIESCENT;
    } else {

        DeviceNode->DockInfo.DockStatus = DOCK_NOTDOCKDEVICE;
    }
}
