fic System Integrity functionality.

Environment:

    Boot

--*/

//
// ------------------------------------------------------------------- Includes
//

#include "osloader.h"
#include <fvetpmtypes.h>
#include <strsafe.h>
#include <tpmapi.h>
#include <wbcl.h>
#include <sipaapi.h>
#include <sipap.h>
#include <blsi.h>

//
// ------------------------------------------------------------------- Defines
//

#define MAX_PATH 260

#define DRIVER_LOAD_POLICY_DEFAULT 0x1

#define REG_KEY_NAME_FILESYSTEM L"FileSystem"
#define REG_KEY_NAME_POLICIES L"Policies"

//
// The following names for registry values are kept the same as in ntfsinit.c
// so these will hopefully be discovered upon any changes to page file encryption
// in ntfs.
//

#define ENCRYPT_PAGINGFILE_GUARDED_HOSTS_VALUE_NAME L"NtfsEncryptPagingFileForGuardedHosts"

#define DISABLE_ENCRYPTION_VALUE_NAME L"NtfsDisableEncryption"

#define ENCRYPT_PAGINGFILE_VALUE_NAME L"NtfsEncryptPagingFile"

//
// ------------------------------------------------------------------- Extern
//

extern const PCWSTR OslControlString;

//
// -------------------------------------------------------Forward declarations
//

NTSTATUS
OslpIsPagefileEncryptionEnabled(
    _In_ ULONG SystemHiveHandle,
    _Out_ PBOOLEAN PagefileEncryptionEnabled
    );

//
// ------------------------------------------------------------------ Functions
//

NTSTATUS
OslMeasureEarlyLaunchHive (
    __in ULONG ElamHiveHandle
    )

/*++

Routine Description:

    This routine measures the early launch anti-malware registry hive. The
    Measurement is best effort. If the data is not measured the machine is
    not trustworthy.

Arguments:

    ElamHiveHandle - Supplies a handle to the early launch anti-malware
        registry hive.

Return Value:

    NT status code.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PCM_KEY_NODE elamRootNode = NULL;
    PCM_KEY_NODE avKey = NULL;
    ULONG n = 0;
    PWSTR szName = NULL;
    ULONG cchName = 0;
    PBYTE pbConfig = NULL;
    ULONG cbConfig = 0;
    PBYTE pbPolicy = NULL;
    ULONG cbPolicy = 0;
    PBYTE pbMeasured = NULL;
    ULONG cbMeasured = 0;
    A_SHA_CTX shaContext = {0};
    BYTE digest[A_SHA_DIGEST_LEN] = {0};
    PTCGEventQueue ElamQueue = NULL;
    PTCGEventQueue Event = NULL;
    size_t cbAggregation = 0;
    PBYTE pbAggregation = NULL;

    //
    // Ensure that there is a consumer of the generated information
    //

    if(BlpSiEnvironmentReady() == FALSE) {
        goto OslMeasureEarlyLaunchHiveEnd;
    }

    // Get the root node from the ELAM hive
    if((elamRootNode = OslGetRootCell(ElamHiveHandle)) == NULL)
    {
        goto OslMeasureEarlyLaunchHiveEnd;
    }

    // Create the elam Queue
    if(SipapCreateQueue(0, 0, &ElamQueue) != S_OK)
    {
        goto OslMeasureEarlyLaunchHiveEnd;
    }

    // Iterate through the entries in the hive
    while(NT_SUCCESS(Status = OslGetSubkeyByNumber(ElamHiveHandle,
                                                   elamRootNode,
                                                   n,
                                                   &avKey,
                                                   &szName)))
    {
        // Obtain the key name
        if(szName == NULL)
        {
            // If this key has no name, we are going to ignore it
            goto OslMeasureEarlyLaunchHiveIterationEnd;
        }
        cchName = (ULONG)wcslen(szName);

        // Obtain the Config value
        OslGetBinaryValue(ElamHiveHandle,
                          avKey,
                          L"Config",
                          &pbConfig,
                          &cbConfig);

        // Obtain the Policy value
        OslGetBinaryValue(ElamHiveHandle,
                          avKey,
                          L"Policy",
                          &pbPolicy,
                          &cbPolicy);

        // Obtain the measured value
        OslGetBinaryValue(ElamHiveHandle,
                          avKey,
                          L"Measured",
                          &pbMeasured,
                          &cbMeasured);

        // Do we have data to record?
        if(((pbConfig == NULL) || (cbConfig == 0)) &&
           ((pbPolicy == NULL) || (cbPolicy == 0)) &&
           ((pbMeasured == NULL) || (cbMeasured == 0)))
        {
            // No data recorded
            goto OslMeasureEarlyLaunchHiveIterationEnd;
        }

        // Create a Queue that will group the key specific data
        if(SipapCreateQueue(0, 0, &Event) != S_OK)
        {
            goto OslMeasureEarlyLaunchHiveIterationEnd;
        }

        // Measure the collected data
        if(SipaQueueConfigEntryToQueue(SIPAEVENT_ELAM_KEYNAME,
                                       (UINT32)(cchName + 1) * sizeof(WCHAR),
                                       (PBYTE)szName,
                                       &Event) != S_OK)
        {
            goto OslMeasureEarlyLaunchHiveIterationEnd;
        }
        if((pbConfig != NULL) && (cbConfig != 0))
        {
            A_SHAInit(&shaContext);
            A_SHAUpdate(&shaContext, pbConfig, cbConfig);
            A_SHAFinal(&shaContext, digest);
            if(SipaQueueConfigEntryToQueue(SIPAEVENT_ELAM_CONFIGURATION,
                                           sizeof(digest),
                                           digest,
                                           &Event) != S_OK)
            {
                goto OslMeasureEarlyLaunchHiveIterationEnd;
            }
        }
        if((pbPolicy != NULL) && (cbPolicy != 0))
        {
            A_SHAInit(&shaContext);
            A_SHAUpdate(&shaContext, pbPolicy, cbPolicy);
            A_SHAFinal(&shaContext, digest);
            if(SipaQueueConfigEntryToQueue(SIPAEVENT_ELAM_POLICY,
                                           sizeof(digest),
                                           digest,
                                           &Event) != S_OK)
            {
                goto OslMeasureEarlyLaunchHiveIterationEnd;
            }
        }
        if((pbMeasured != NULL) && (cbMeasured != 0))
        {
            A_SHAInit(&shaContext);
            A_SHAUpdate(&shaContext, pbMeasured, cbMeasured);
            A_SHAFinal(&shaContext, digest);
            if(SipaQueueConfigEntryToQueue(SIPAEVENT_ELAM_MEASURED,
                                           sizeof(digest),
                                           digest,
                                           &Event) != S_OK)
            {
                goto OslMeasureEarlyLaunchHiveIterationEnd;
            }
        }

        // Close the bag for this key and append to the ELAM aggregation
        cbAggregation = Event->QueueLength;
        if((pbAggregation = BlMmAllocateHeap(cbAggregation)) == NULL)
        {
            goto OslMeasureEarlyLaunchHiveIterationEnd;
        }
        memcpy(pbAggregation, Event->Log, cbAggregation);
        if(SipaQueueConfigEntryToQueue(SIPAEVENT_ELAM_AGGREGATION,
                                       (UINT32)cbAggregation,
                                       pbAggregation,
                                       &ElamQueue) != S_OK)
        {
            goto OslMeasureEarlyLaunchHiveIterationEnd;
        }

OslMeasureEarlyLaunchHiveIterationEnd:

        // Release all resources acquired for this iteration
        if(szName != NULL)
        {
            BlMmFreeHeap(szName);
            szName = NULL;
        }
        cchName = 0;
        if(pbConfig != NULL)
        {
            BlMmFreeHeap(pbConfig);
            pbConfig = NULL;
        }
        cbConfig = 0;
        if(pbPolicy != NULL)
        {
            BlMmFreeHeap(pbPolicy);
            pbPolicy = NULL;
        }
        cbPolicy = 0;
        if(pbMeasured != NULL)
        {
            BlMmFreeHeap(pbMeasured);
            pbMeasured = NULL;
        }
        cbMeasured = 0;
        if(pbAggregation != NULL)
        {
            BlMmFreeHeap(pbAggregation);
            pbAggregation = NULL;
        }
        cbAggregation = 0;
        if(Event != NULL)
        {
            BlMmFreeHeap(Event);
            Event = NULL;
        }
        n++;
    }
    Status = STATUS_SUCCESS;

    // Close the bag for the whole ELAM aggregation and append to global queue
    if(n > 0)
    {
        // If there is something to report, wrap it up
        cbAggregation = ElamQueue->QueueLength;
        if((pbAggregation = (PBYTE)BlMmAllocateHeap(cbAggregation)) == NULL)
        {
            goto OslMeasureEarlyLaunchHiveEnd;
        }
        memcpy(pbAggregation, ElamQueue->Log, cbAggregation);
    }
    if(n == 1)
    {
        // A single entry is just being added to the queue
        if(SipapAppendEntry((UINT32)cbAggregation,
                            pbAggregation,
                            &SipaDataPointers.PendingEvents) != S_OK)
        {
            goto OslMeasureEarlyLaunchHiveEnd;
        }
    }
    else if(n > 1)
    {
        // Multiple entries are packaged in a container before adding them to the queue
        if(SipaQueueConfigEntryToQueue(SIPAEVENT_ELAM_AGGREGATION,
                                       (UINT32)cbAggregation,
                                       pbAggregation,
                                       &SipaDataPointers.PendingEvents) != S_OK)
        {
            goto OslMeasureEarlyLaunchHiveEnd;
        }
    }

OslMeasureEarlyLaunchHiveEnd:
    if(szName != NULL)
    {
        BlMmFreeHeap(szName);
    }
    if(pbConfig != NULL)
    {
        BlMmFreeHeap(pbConfig);
    }
    if(pbPolicy != NULL)
    {
        BlMmFreeHeap(pbPolicy);
    }
    if(pbMeasured != NULL)
    {
        BlMmFreeHeap(pbMeasured);
    }
    if(pbAggregation != NULL)
    {
        BlMmFreeHeap(pbAggregation);
    }
    if(Event != NULL)
    {
        BlMmFreeHeap(Event);
    }
    if(ElamQueue != NULL)
    {
        BlMmFreeHeap(ElamQueue);
    }
    return Status;
}

VOID
OslReportKernelLaunch (
    __in PBOOT_ENTRY_OPTION OsLoaderOptions,
    __in ULONG SystemHiveHandle
    )

/*++

Routine Description:

    This routine reports boot loader and kernel policy to the system integrity
    boot library.

    N.B. Changes to the boot options can not be made after this routine is
         called.

Arguments:

    OsLoaderOptions - Supplies the boot options for the Operating System.

    SystemHiveHandle - Supplies a handle to the system registry hive.

Return Value:

    None.

--*/

{

    size_t cchSystemRoot;
    ULONG DriverLoadPolicy;
    BOOLEAN FlightSigningEnabled;
    ULONGLONG HVLaunchType;
    BOOLEAN IntegrityChecksDisabled;
    BOOLEAN KernelDebuggingEnabled;
    ULONGLONG NxPolicy;
    PBOOT_ENVIRONMENT_DEVICE OSDevice;
    UINT32 OSDeviceCode;
    ULONGLONG PAEPolicy;
    BOOLEAN PagefileEncryptionEnabled;
    BOOLEAN SafeModeBoot;
    ULONGLONG SafeModeState;
    NTSTATUS Status;
    PWSTR SystemRoot = NULL;
    BOOLEAN TestSigningEnabled;
    ULONGLONG VsmLaunchType;
    BOOLEAN WinPE;
#if defined(NT_IUM)
    ULONGLONG HvciPolicy;
    BOOLEAN True;
    VSM_POLICY VsmPolicy;
#endif

    //
    // Make sure that the data being reported into the boot configuration log
    // is of the length expected in the log format.
    //

    C_ASSERT((sizeof(KernelDebuggingEnabled) == 1) &&
             (sizeof(WinPE) == 1) &&
             (sizeof(SafeModeBoot) == 1) &&
             (sizeof(PagefileEncryptionEnabled) == 1));

    //
    // Ensure that there is a consumer of the generated information
    //

    if(BlpSiEnvironmentReady() == FALSE) {
        return;
    }

    //
    // Extract system integrity relevant information from the boot option list.
    //

    Status = BlGetBootOptionBoolean(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_KERNEL_DEBUGGER_ENABLED,
                                    &KernelDebuggingEnabled);

    if (Status == STATUS_NOT_FOUND) {
        KernelDebuggingEnabled = FALSE;

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting kernel debugging boot option");

        return;
    }

    Status = BlGetBootOptionInteger(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_NX_POLICY,
                                    &NxPolicy);

    if (Status == STATUS_NOT_FOUND) {
        NxPolicy = NxPolicyOptIn;

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting NX policy boot option");

        return;

    }

    Status = BlGetBootOptionDevice(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_OS_DEVICE,
                                    &OSDevice,
                                    NULL);

    if (Status == STATUS_NOT_FOUND) {
        RtlZeroMemory(&OSDevice, sizeof(OSDevice));

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting OS device boot option");

        return;

    }
    else
    {
        switch(OSDevice->DeviceType)
        {
            case DEVICE_BLOCK_IO:
                switch(OSDevice->u.BlockIo.BlockIoType)
                {
                    case BLOCK_IO_HARD_DISK:
                        OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_HARDDISK;
                        break;
                    case BLOCK_IO_REMOVABLE_DISK:
                        OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_REMOVABLEDISK;
                        break;
                    case BLOCK_IO_CDROM:
                        OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_CDROM;
                        break;
                    case BLOCK_IO_RAMDISK:
                        OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_RAMDISK;
                        break;
                    case BLOCK_IO_PARTITION:
                        OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_PARTITION;
                        break;
                    case BLOCK_IO_FILE:
                        OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_FILE;
                        break;
                    case BLOCK_IO_VIRTUAL_HARD_DISK:
                        OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_VIRTUALHARDDISK;
                        break;
                    default:
                        OSDeviceCode = OSDEVICE_TYPE_UNKNOWN;
                        break;
                }
                break;
            case DEVICE_PARTITION:
                OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_PARTITION;
                break;
            case DEVICE_SERIAL_PORT:
                OSDeviceCode = OSDEVICE_TYPE_SERIAL;
                break;
            case DEVICE_UDP:
                OSDeviceCode = OSDEVICE_TYPE_UDP;
                break;
            case DEVICE_PARTITION_EX:
                OSDeviceCode = OSDEVICE_TYPE_BLOCKIO_PARTITION;
                break;
            default:
                OSDeviceCode = OSDEVICE_TYPE_UNKNOWN;
                break;
        }
    }

    Status = BlGetBootOptionString(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_SYSTEM_ROOT,
                                    &SystemRoot);

    if (Status == STATUS_NOT_FOUND) {
        SystemRoot = NULL;
        cchSystemRoot = 0;

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting system root boot option");
        if(SystemRoot != NULL) {
            BlMmFreeHeap(SystemRoot);
            SystemRoot = NULL;
        }

        return;

    } else if(StringCchLengthW(SystemRoot, MAX_PATH, &cchSystemRoot) != S_OK)
    {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error processing system root boot option");
        if(SystemRoot != NULL) {
            BlMmFreeHeap(SystemRoot);
            SystemRoot = NULL;
        }

        return;

    }

    Status = BlGetBootOptionBoolean(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_WINPE,
                                    &WinPE);

    if (Status == STATUS_NOT_FOUND) {
        WinPE = FALSE;

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting WinPE boot option");

        return;
    }

    Status = BlGetBootOptionInteger(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_PAE_POLICY,
                                    &PAEPolicy);

    if (Status == STATUS_NOT_FOUND) {
        PAEPolicy = PaePolicyDefault;

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting PAE policy boot option");

        return;

    }

    Status = BlGetBootOptionInteger(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_HYPERVISOR_LAUNCH_TYPE,
                                    &HVLaunchType);

    if (Status == STATUS_NOT_FOUND) {
        HVLaunchType = HypervisorLaunchOff;

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting HV launch type boot option");

        return;

    }

    Status = BlGetBootOptionInteger(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_VSM_LAUNCH_TYPE,
                                    &VsmLaunchType);

    if (Status == STATUS_NOT_FOUND) {
        VsmLaunchType = VsmLaunchOff;

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting VSM launch type boot option");

        return;
    }

#if defined(NT_IUM)
    BlVsmGetSystemPolicy(&VsmPolicy);
#endif

    //
    // For system integrity purposes, treat all types of safe mode equivalently.
    //

    Status = BlGetBootOptionInteger(OsLoaderOptions,
                                    BCDE_OSLOADER_TYPE_SAFEBOOT,
                                    &SafeModeState);

    if (Status == STATUS_NOT_FOUND) {
        SafeModeBoot = FALSE;

    } else if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting safe mode boot option");

        return;

    } else {
        SafeModeBoot = TRUE;
    }

    Status = OslpIsPagefileEncryptionEnabled(SystemHiveHandle,
                                             &PagefileEncryptionEnabled);
    
    if (!NT_SUCCESS(Status)) {
        BlSiEnterInsecureState(SiInsecureUnexpectedLibraryError,
                               "Error getting pagefile encryption setting");

        return;
    }

    //
    // Log the values for the System Integrity-relevant boot policy.
    //

    BlSiPaRecordEvent(SIPAEVENT_OSKERNELDEBUG,
                      sizeof(KernelDebuggingEnabled),
                      &KernelDebuggingEnabled);

    BlSiPaRecordEvent(SIPAEVENT_OSDEVICE,
                      sizeof(OSDeviceCode),
                      &OSDeviceCode);

    BlSiPaRecordEvent(SIPAEVENT_SYSTEMROOT,
                      (UINT32)(cchSystemRoot + 1) * sizeof(WCHAR),
                      SystemRoot);

    if (SystemRoot != NULL) {
        BlMmFreeHeap(SystemRoot);
        SystemRoot = NULL;
    }

    BlSiPaRecordEvent(SIPAEVENT_DATAEXECUTIONPREVENTION,
                      sizeof(NxPolicy),
                      &NxPolicy);

    BlSiPaRecordEvent(SIPAEVENT_PHYSICALADDRESSEXTENSION,
                      sizeof(PAEPolicy),
                      &PAEPolicy);

    BlSiPaRecordEvent(SIPAEVENT_SAFEMODE,
                      sizeof(SafeModeBoot),
                      &SafeModeBoot);

    BlSiPaRecordEvent(SIPAEVENT_WINPE,
                      sizeof(WinPE),
                      &WinPE);

    BlSiPaRecordEvent(SIPAEVENT_HYPERVISOR_LAUNCH_TYPE,
                      sizeof(HVLaunchType),
                      &HVLaunchType);

    BlSiPaRecordEvent(SIPAEVENT_VSM_LAUNCH_TYPE,
                      sizeof(VsmLaunchType),
                      &VsmLaunchType);

#if NTDDI_VERSION >= NTDDI_WIN10_RS1 

    BlSiPaRecordEvent(SIPAEVENT_PAGEFILE_ENCRYPTION_ENABLED,
                      sizeof(PagefileEncryptionEnabled),
                      &PagefileEncryptionEnabled);

#endif

    //
    // Log the VSM policy state.
    //

#if defined(NT_IUM)
    if (VsmPolicy.AllOptions != 0) {

        True = TRUE;

        if (VsmPolicy.Options.Mandatory != 0) {
            BlSiPaRecordEvent(SIPAEVENT_VBS_MANDATORY_ENFORCEMENT,
                              sizeof(True),
                              &True);
        }

        if (VsmPolicy.Options.VsmRequired != 0) {
            BlSiPaRecordEvent(SIPAEVENT_VBS_VSM_REQUIRED,
                              sizeof(True),
                              &True);
        }

        if (VsmPolicy.Options.SecureBootRequired != 0) {
            BlSiPaRecordEvent(SIPAEVENT_VBS_SECUREBOOT_REQUIRED,
                              sizeof(True),
                              &True);
        }

        if (VsmPolicy.Options.IommuProtectionRequired != 0) {
            BlSiPaRecordEvent(SIPAEVENT_VBS_IOMMU_REQUIRED,
                              sizeof(True),
                              &True);
        }

        if (VsmPolicy.Options.MmioNxRequired != 0) {
            BlSiPaRecordEvent(SIPAEVENT_VBS_MMIO_NX_REQUIRED,
                              sizeof(True),
                              &True);
        }

        if (VsmPolicy.Options.StrongMsrFiltering != 0) {
            BlSiPaRecordEvent(SIPAEVENT_VBS_MSR_FILTERING_REQUIRED,
                              sizeof(True),
                              &True);
        }

        if (VsmPolicy.Options.Hvci != 0) {

            HvciPolicy = VsmPolicy.Options.Hvci;
            BlSiPaRecordEvent(SIPAEVENT_VBS_HVCI_POLICY,
                              sizeof(HvciPolicy),
                              &HvciPolicy);
        }

        if (VsmPolicy.Options.RequireMicrosoftSignedBootChain != FALSE) {
            BlSiPaRecordEvent(SIPAEVENT_VBS_MICROSOFT_BOOT_CHAIN_REQUIRED,
                              sizeof(True),
                              &True);
        }
    }
#endif

    //
    // Log driver load policy state.
    //

    Status = OslHiveGetDriverLoadPolicy(SystemHiveHandle, &DriverLoadPolicy);
    if (!NT_SUCCESS(Status)) {
        DriverLoadPolicy = DRIVER_LOAD_POLICY_DEFAULT;
    }

    BlSiPaRecordEvent(SIPAEVENT_DRIVER_LOAD_POLICY,
                      sizeof(DriverLoadPolicy),
                      &DriverLoadPolicy);

    //
    // If code integrity is disabled in the kernel, report that a non-validated
    // image has been loaded.  This isn't accurate, but from a security
    // perspective, disabling integrity checks is equivalent to loading an
    // unchecked image.
    //

    //
    // Also check whether test signing is enabled. Test signing is initially
    // checked when an application is loaded.  This check handles the F10
    // scenario where the boot option was added during the OS loader's
    // execution.
    //

    BlImgQueryCodeIntegrityBootOptions(BlGetApplicationEntry(),
                                       &IntegrityChecksDisabled,
                                       &TestSigningEnabled,
                                       &FlightSigningEnabled);

    if (IntegrityChecksDisabled != FALSE) {
        BlEnNotifyEvent(EVENT_NVPE_IMAGE_LOADED, NULL);
    }

    if (TestSigningEnabled != FALSE) {
        BlEnNotifyEvent(EVENT_TEST_SIGNING_ENABLED, NULL);
    }

    if (FlightSigningEnabled != FALSE) {
        BlEnNotifyEvent(EVENT_FLIGHT_SIGNING_ENABLED, NULL);
    }

    //
    // Flush any pending measurements and complete the aggregation event for winload.
    // Keep the SI library open though as we may need to measure VSM IDK.
    //
    BlSiFlushCurrentMeasurements();
    return;
}

VOID
OslCloseSipaReporting ()

/*++

Routine Description:

    This routine closes the SIPA measuring / reporting sub-system.
    No measured boot actions are possible after this method has been called.

Arguments:

    None.

Return Value:

    None.

--*/

{
    BlSiCloseEnvironment();
}

BOOLEAN
OslSwitchTpmPcrBanks (
    __in PBOOT_ENVIRONMENT_DEVICE BootDevice
    )

/*++

Routine Description:

    This routine tries to switch the TPM PCR banks to the OS preferred
    banks. Before doing so, it checks if Bitlocker is active.

Arguments:

    BootDevice - the boot device for the OS.

Return Value:

    FALSE - if no further action is required.

    TRUE - if a reboot is required.

--*/

{
    NTSTATUS Status;
    ULONG FveStaticFlags;
    ULONG FvePcrBitmap;

    Status = STATUS_SUCCESS;

    if (BootDevice != NULL) {

        Status = BlFveQueryStaticDeviceStatus(BootDevice,
                                              &FveStaticFlags,
                                              &FvePcrBitmap);

        if (NT_SUCCESS(Status)) {

            //
            // Check if Bitlocker is on, TPM protector is enabled, and is not suspended.
            // If so, don't touch PCR banks.
            //

            if ((BlFveBitlockerEnabled(FveStaticFlags) == TRUE) &&
                (BlFveBitlockerTpmProtected(FveStaticFlags) == TRUE) &&
                (BlFveBitlockerSuspended(FveStaticFlags) == FALSE)) {

                return FALSE;
            }
        }
    }

    //
    // Change the active PCR bank.
    //
    // At this point Bitlocker is off or suspended.
    //

    Status = BlTpmSwitchPcrBanks();

    if (Status == STATUS_RESTART_BOOT_APPLICATION) {

        return TRUE;
    }

    //
    // ignore all other return values
    //

    return FALSE;
}

NTSTATUS
OslpIsPagefileEncryptionEnabled(
    _In_ ULONG SystemHiveHandle,
    _Out_ PBOOLEAN PagefileEncryptionEnabled
    )

/*++

Routine Description:

    This helper routine determines if page file encryption is enabled
    by checking the same set of registry key triggers as done by the ntfs
    driver (see ntfsinit.c::DriverEntry).

    This routine must be updated if the above conditon in the ntfs driver 
    is ever changed.

Arguments:

    SystemHiveHandle - Supplies a handle to the system registry hive.

    PagefileEncryptionEnabled - Supplies a pointer to a variable to receive 
        a flag indicating whether page file encryption is enabled.

Return Value:

    NTSTATUS code.

--*/

{
    NTSTATUS Status;
    PCM_KEY_NODE CurrentControlSetCell;
    PCM_KEY_NODE ControlCell;
    PCM_KEY_NODE FileSystemCell;
    PCM_KEY_NODE PoliciesCell;
    DWORD RegValue;

    Status = OslGetSubkey(SystemHiveHandle,
                          NULL,
                          NULL,
                          &CurrentControlSetCell);

    if (!NT_SUCCESS(Status)) {
        goto IsPagefileEncryptionEnabledEnd;
    }

    Status = OslGetSubkey(SystemHiveHandle,
                          CurrentControlSetCell,
                          OslControlString,
                          &ControlCell);

    if (!NT_SUCCESS(Status)) {
        goto IsPagefileEncryptionEnabledEnd;
    }

    //
    // Condition #1.
    //
    // If COMPATIBILITY_MODE_KEY_NAME::ENCRYPT_PAGINGFILE_GUARDED_HOSTS_VALUE_NAME(DWORD) 
    // is present and set to 1 then page file encryption is enabled.
    //
    // The registry path above is:
    // \\CurrentControlSet\\Control\\FileSystem::NtfsEncryptPagingFileForGuardedHosts
    //

    Status = OslGetSubkey(SystemHiveHandle,
                          ControlCell,
                          REG_KEY_NAME_FILESYSTEM,
                          &FileSystemCell);

    if (!NT_SUCCESS(Status)) {
        goto IsPagefileEncryptionEnabledEnd;
    }

    Status = OslGetDWordValue(SystemHiveHandle,
                              FileSystemCell,
                              ENCRYPT_PAGINGFILE_GUARDED_HOSTS_VALUE_NAME,
                              &RegValue);

    if ((Status != STATUS_NOT_FOUND) && 
        (Status != STATUS_OBJECT_TYPE_MISMATCH)) {

        if (!NT_SUCCESS(Status)) {
            goto IsPagefileEncryptionEnabledEnd;
        }

        if (RegValue == 1) {
            *(PagefileEncryptionEnabled) = TRUE;

            goto IsPagefileEncryptionEnabledEnd;
        }
    }

    //
    // Condition #2. 
    //
    // If either GROUP_POLICY_KEY_NAME::DISABLE_ENCRYPTION_VALUE_NAME or
    // COMPATIBILITY_MODE_KEY_NAME::DISABLE_ENCRYPTION_VALUE_NAME exist and 
    // are set to (DWORD)1 then page file encryption is disabled for good.
    //
    // The registry path above is:
    // \\CurrentControlSet\\Policies::NtfsDisableEncryption and
    // \\CurrentControlSet\\Control\\FileSystem::NtfsDisableEncryption
    //

    Status = OslGetSubkey(SystemHiveHandle,
                          CurrentControlSetCell,
                          REG_KEY_NAME_POLICIES,
                          &PoliciesCell);

    if (!NT_SUCCESS(Status)) {
        goto IsPagefileEncryptionEnabledEnd;
    }

    Status = OslGetDWordValue(SystemHiveHandle,
                              PoliciesCell,
                              DISABLE_ENCRYPTION_VALUE_NAME,
                              &RegValue);

    if ((Status != STATUS_NOT_FOUND) && 
        (Status != STATUS_OBJECT_TYPE_MISMATCH)) {

        if (!NT_SUCCESS(Status)) {
            goto IsPagefileEncryptionEnabledEnd;
        }

        if (RegValue == 1) {
            *(PagefileEncryptionEnabled) = FALSE;

            goto IsPagefileEncryptionEnabledEnd;
        }
    }

    Status = OslGetDWordValue(SystemHiveHandle,
                              FileSystemCell,
                              DISABLE_ENCRYPTION_VALUE_NAME,
                              &RegValue);

    if ((Status != STATUS_NOT_FOUND) && 
        (Status != STATUS_OBJECT_TYPE_MISMATCH)) {

        if (!NT_SUCCESS(Status)) {
            goto IsPagefileEncryptionEnabledEnd;
        }

        if (RegValue == 1) {
            *(PagefileEncryptionEnabled) = FALSE;

            goto IsPagefileEncryptionEnabledEnd;
        }
    }

    //
    // Condition #3 (requires Condition #2 above to be false). 
    //
    // If either GROUP_POLICY_KEY_NAME::ENCRYPT_PAGINGFILE_VALUE_NAME or
    // COMPATIBILITY_MODE_KEY_NAME::ENCRYPT_PAGINGFILE_VALUE_NAME exist and 
    // are set to (DWORD)1 then page file encryption is enabled.
    //
    // The registry path above is:
    // \\CurrentControlSet\\Policies::NtfsEncryptPagingFile and
    // \\CurrentControlSet\\Control\\FileSystem::NtfsEncryptPagingFile
    //

    Status = OslGetDWordValue(SystemHiveHandle,
                              PoliciesCell,
                              ENCRYPT_PAGINGFILE_VALUE_NAME,
                              &RegValue);

    if ((Status != STATUS_NOT_FOUND) && 
        (Status != STATUS_OBJECT_TYPE_MISMATCH)) {

        if (!NT_SUCCESS(Status)) {
            goto IsPagefileEncryptionEnabledEnd;
        }

        if (RegValue == 1) {
            *(PagefileEncryptionEnabled) = TRUE;

            goto IsPagefileEncryptionEnabledEnd;
        }
    }

    Status = OslGetDWordValue(SystemHiveHandle,
                              FileSystemCell,
                              ENCRYPT_PAGINGFILE_VALUE_NAME,
                              &RegValue);

    if ((Status != STATUS_NOT_FOUND) && 
        (Status != STATUS_OBJECT_TYPE_MISMATCH)) {

        if (!NT_SUCCESS(Status)) {
            goto IsPagefileEncryptionEnabledEnd;
        }

        if (RegValue == 1) {
            *(PagefileEncryptionEnabled) = TRUE;

            goto IsPagefileEncryptionEnabledEnd;
        }
    }

    Status = STATUS_SUCCESS;
    *(PagefileEncryptionEnabled) = FALSE;

IsPagefileEncryptionEnabledEnd:
    return Status;
}

/*++
Copyright (c) Microsoft Corporation

Module Name:

    roothub.c

Abstract:

    USB Controller Extension Driver

--*/
#include "pch.h"

#include "roothub.tmh"

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL RootHub_EvtIoDeviceControl;

VOID
RootHub_EvtIoDeviceControl(
    WDFQUEUE    WdfQueue,
    WDFREQUEST  WdfRequest,
    size_t      OutputBufferLength,
    size_t      InputBufferLength,
    ULONG       IoControlCode
    )
/*++

Routine Description:

    This function is called when a request is received on the default queue.

--*/
{
    UNREFERENCED_PARAMETER(WdfQueue);
    UNREFERENCED_PARAMETER(IoControlCode);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    TRY {

        LogError(FlagRootHub, "User mode ioctl Ioctl %x not supported", IoControlCode);
        WdfRequestComplete(WdfRequest, STATUS_INVALID_DEVICE_REQUEST);

    } FINALLY {

    }

    return;
}


_Must_inspect_result_
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
RootHub_StaticPdoSetup(
    __in 
        UCXCONTROLLER      UcxController,
    __in 
        WDFDEVICE          WdfDevice,
    __in 
        UCXROOTHUB         UcxRootHub
    )
/*++

Routine Description:

    This function sets up a static PDO for the UcxRootHub

Arguments:

    UcxController - Handle to the controller object

    WdfDevice - Handle to the WdfDevice Object.

    UcxRootHub - Handle to the roothub object

Return Value:

    NT_SUCCESS status if a Static Pdo was successfully associated

Commnets:
    This routine is called by the UCX's EvtDriverDeviceAdd routine.

--*/
{
    NTSTATUS                        status;
    PNP_BUS_INFORMATION             busInformation;
    PWDFDEVICE_INIT                 wdfDeviceInit = NULL;
    WDF_OBJECT_ATTRIBUTES           objectAttributes;
    WDFDEVICE                       rootHubPdo = NULL;
    BOOLEAN                         deletePdoOnError = TRUE;
    PWDF_DEVICE_PNP_CAPABILITIES    pnpCaps;
    PWDF_DEVICE_POWER_CAPABILITIES  powerCaps;
    WDF_PDO_EVENT_CALLBACKS         pdoEventCallbacks;
    SYSTEM_POWER_STATE              oldRhSystemWake;
    SYSTEM_POWER_STATE              s;
    PUCXROOTHUB_PRIVCONTEXT         ucxRootHubContext;
    PUCXCONTROLLER_PRIVCONTEXT      ucxControllerContext;
    PRHPDO_CONTEXT                  rhPdoContext;
    UCHAR                           minorFunctionArray[1];
    PDEVICE_CAPABILITIES            hcCaps;
    WDF_PNPPOWER_EVENT_CALLBACKS    wdfPnpPowerEvtCallbacks;
    KIRQL                           irql;
    ULONG                           index;
    WDF_IO_QUEUE_CONFIG             wdfIoQueueConfig;
    WDFQUEUE                        wdfQueue;
    PCUNICODE_STRING                hardwareId;

    DECLARE_CONST_UNICODE_STRING(instanceId, L"0\0");
    DECLARE_CONST_UNICODE_STRING(deviceId, ROOTHUB_DEVICE_ID);
    DECLARE_UNICODE_STRING_SIZE(pciHardwareId, sizeof(ROOTHUB_PCI_HARDWARE_ID));
    DECLARE_UNICODE_STRING_SIZE(acpiHardwareId, sizeof(ROOTHUB_ACPI_HARDWARE_ID));
    DECLARE_UNICODE_STRING_SIZE(pdoName, sizeof(L"\\Device\\USBPDO-") + 32*sizeof(WCHAR));

    TRY {

        ucxRootHubContext = UcxRootHubGetContext(UcxRootHub);
        ucxControllerContext = ucxRootHubContext->UcxControllerContext;

        //
        // Add bus information for this Device
        //

        RtlZeroMemory(&busInformation, sizeof(busInformation));

        busInformation.BusTypeGuid = GUID_BUS_TYPE_USB;
        busInformation.LegacyBusType = PNPBus;
        busInformation.BusNumber = 0;

        WdfDeviceSetBusInformationForChildren(WdfDevice, &busInformation);

        //
        // TASK: Setup a Static PDO for the RootHub
        //

        //
        // Allocate a WDFDEVICE_INIT structure
        //
        wdfDeviceInit = WdfPdoInitAllocate(WdfDevice);

        if (NULL == wdfDeviceInit) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            LogError(FlagRootHub, "WdfPdoInitAllcoate Failed");
            LEAVE;
        }

        //
        // Assign an instance ID
        //

        status = WdfPdoInitAssignInstanceID(wdfDeviceInit, &instanceId);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfPdoInitAssignInstanceID for RH child pdo Failed %!STATUS!", status);
            LEAVE;
        }

        switch (ucxControllerContext->Config.ParentBusType) {

        case UcxControllerParentBusTypePci:

            RtlUnicodeStringPrintf(&pciHardwareId,
                                   ROOTHUB_PCI_HARDWARE_ID_FORMAT_STRING,
                                   ucxControllerContext->Config.PciDeviceInfo.VendorId,
                                   ucxControllerContext->Config.PciDeviceInfo.DeviceId,
                                   ucxControllerContext->Config.PciDeviceInfo.RevisionId,
                                   ucxControllerContext->Config.PciDeviceInfo.VendorId,
                                   ucxControllerContext->Config.PciDeviceInfo.DeviceId);
            
            //
            // Replace the '#' char's with '\0' chars in the string. 
            //

            ((PWCHAR)pciHardwareId.Buffer)[ROOTHUB_PCI_HARDWARE_ID_NULL_LOCATION_1] = L'\0';
            ((PWCHAR)pciHardwareId.Buffer)[ROOTHUB_PCI_HARDWARE_ID_NULL_LOCATION_2] = L'\0';
            ((PWCHAR)pciHardwareId.Buffer)[ROOTHUB_PCI_HARDWARE_ID_NULL_LOCATION_3] = L'\0';

            hardwareId = &pciHardwareId;
            break;

        case UcxControllerParentBusTypeAcpi:

            RtlUnicodeStringPrintf(&acpiHardwareId,
                                   ROOTHUB_ACPI_HARDWARE_ID_FORMAT_STRING,
                                   ucxControllerContext->Config.AcpiDeviceInfo.VendorId,
                                   ucxControllerContext->Config.AcpiDeviceInfo.DeviceId,
                                   ucxControllerContext->Config.AcpiDeviceInfo.RevisionId,
                                   ucxControllerContext->Config.AcpiDeviceInfo.VendorId,
                                   ucxControllerContext->Config.AcpiDeviceInfo.DeviceId);

            ((PWCHAR)acpiHardwareId.Buffer)[ROOTHUB_ACPI_HARDWARE_ID_NULL_LOCATION_1] = L'\0';
            ((PWCHAR)acpiHardwareId.Buffer)[ROOTHUB_ACPI_HARDWARE_ID_NULL_LOCATION_2] = L'\0';
            ((PWCHAR)acpiHardwareId.Buffer)[ROOTHUB_ACPI_HARDWARE_ID_NULL_LOCATION_3] = L'\0';

            hardwareId = &acpiHardwareId;
            break;

        default:

            hardwareId = &deviceId;
            break;
        }

        //
        // Assign a Hardware ID
        //
        status = WdfPdoInitAddHardwareID(wdfDeviceInit, hardwareId);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfPdoInitAddHardwareID for RH child pdo Failed %!STATUS!", status);
            LEAVE;
        }

        //
        // Assign a Device ID
        //
        status = WdfPdoInitAssignDeviceID(wdfDeviceInit, &deviceId);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfPdoInitAssignDeviceID for RH child pdo Failed %!STATUS!", status);
            LEAVE;
        }

        //
        // Allow forwarding to parent queue
        //
        WdfPdoInitAllowForwardingRequestToParent(wdfDeviceInit);

        status = WdfDeviceInitAssignWdmIrpPreprocessCallback(
            wdfDeviceInit,
            RootHub_Pdo_EvtInternalDeviceControlIrpPreprocessCallback,
            IRP_MJ_INTERNAL_DEVICE_CONTROL,
            NULL, 0);

        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfDeviceInitAssignWdmIrpPreprocessCallback Failed %!STATUS!", status);
            LEAVE;
        }

        //
        // Due WDF not handling different QueryInterface versions for 1 GUID (USB is the only bus
        // using such versioning), we need to install a Irp Preprocess Callback to handle the USBDI
        // interface.
        //
        minorFunctionArray[0] =  IRP_MN_QUERY_INTERFACE;

        status = WdfDeviceInitAssignWdmIrpPreprocessCallback(
            wdfDeviceInit,
            RootHub_Pdo_EvtQueryInterfaceIrpPreprocessCallback,
            IRP_MJ_PNP,
            minorFunctionArray,
            1);

        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfDeviceInitAssignWdmIrpPreprocessCallback Failed For Query Interface %!STATUS!", status);
            LEAVE;
        }

        //
        // For Compat, we need to get information about the LastSystemSleepState. 
        // Wdf does not provide that information, thus we need a pre-processor routine. 
        //
        minorFunctionArray[0] =  IRP_MN_SET_POWER;

        status = WdfDeviceInitAssignWdmIrpPreprocessCallback(
            wdfDeviceInit,
            RootHub_Pdo_EvtSetPowerIrpPreprocessCallback,
            IRP_MJ_POWER,
            minorFunctionArray,
            1);

        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfDeviceInitAssignWdmIrpPreprocessCallback Failed For Set Power %!STATUS!", status);
            LEAVE;
        }

        //
        // Set the attributes for WDF requests.
        //
        WdfDeviceInitSetRequestAttributes(wdfDeviceInit,
                                          &ucxRootHubContext->Config.WdfRequestAttributes);

        WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&objectAttributes, RHPDO_CONTEXT);

        //
        // Set Pnp & Power Callbacks.
        //
        WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&wdfPnpPowerEvtCallbacks);
        wdfPnpPowerEvtCallbacks.EvtDevicePrepareHardware = RootHub_Pdo_EvtPrepareHardware;
        wdfPnpPowerEvtCallbacks.EvtDeviceReleaseHardware = RootHub_Pdo_EvtReleaseHardware;
        wdfPnpPowerEvtCallbacks.EvtDeviceD0Entry = RootHub_Pdo_EvtD0Entry;
        wdfPnpPowerEvtCallbacks.EvtDeviceD0Exit = RootHub_Pdo_EvtD0Exit;

        WdfDeviceInitSetPnpPowerEventCallbacks(wdfDeviceInit, &wdfPnpPowerEvtCallbacks);

        WDF_PDO_EVENT_CALLBACKS_INIT(&pdoEventCallbacks);
        pdoEventCallbacks.EvtDeviceEnableWakeAtBus  = RootHub_Pdo_EvtDeviceEnableWakeAtBus;
        pdoEventCallbacks.EvtDeviceDisableWakeAtBus = RootHub_Pdo_EvtDeviceDisableWakeAtBus;

        WdfPdoInitSetEventCallbacks(wdfDeviceInit, &pdoEventCallbacks);
        
        index = 0;

        do {

            //
            // Create the PDO name
            //

            status = RtlUnicodeStringPrintf(&pdoName, L"\\Device\\USBPDO-%d", index);

            if (!NT_SUCCESS(status)) {
                LogError(FlagRootHub, "RtlUnicodeStringPrintf, index %d, %!STATUS!", index, status);
                LEAVE;
            }

            status = WdfDeviceInitAssignName(wdfDeviceInit, &pdoName);

            if (!NT_SUCCESS(status)) {
                LogError(FlagRootHub, "WdfDeviceInitAssignName Failed, index %d, %!STATUS!", index, status);
                LEAVE;
            }
            
            status = WdfDeviceInitAssignSDDLString(wdfDeviceInit, 
                                                   &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);

            if (!NT_SUCCESS(status)) {
                LogError(FlagRootHub, "WdfDeviceInitAssignSDDLString Failed, index %d, %!STATUS!", index, status);
                LEAVE;
            }

            status = WdfDeviceCreate(&wdfDeviceInit, &objectAttributes, &rootHubPdo);
            
            index++;

        } while (status == STATUS_OBJECT_NAME_COLLISION);
                
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfDeviceCreate for RH child pdo Failed %!STATUS!", status);
            LEAVE;
        }

        WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&wdfIoQueueConfig, WdfIoQueueDispatchParallel);
        wdfIoQueueConfig.EvtIoDeviceControl = RootHub_EvtIoDeviceControl;

        status = WdfIoQueueCreate(rootHubPdo,
                                  &wdfIoQueueConfig,
                                  WDF_NO_OBJECT_ATTRIBUTES,
                                  &wdfQueue);

        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfIoQueueCreate for RH child pdo Failed %!STATUS!", status);
            LEAVE;
        }

        //
        // Indicate special file support
        //
        WdfDeviceSetSpecialFileSupport(rootHubPdo,
                                       WdfSpecialFilePaging,
                                       TRUE);
        WdfDeviceSetSpecialFileSupport(rootHubPdo,
                                       WdfSpecialFileHibernation,
                                       TRUE);
        WdfDeviceSetSpecialFileSupport(rootHubPdo,
                                       WdfSpecialFileDump,
                                       TRUE);
        WdfDeviceSetSpecialFileSupport(rootHubPdo,
                                       WdfSpecialFileBoot,
                                       TRUE);

        //
        // set the Pnp and Power properties for this device
        //
        pnpCaps = &ucxRootHubContext->PnpCaps;
        WDF_DEVICE_PNP_CAPABILITIES_INIT(pnpCaps);
        pnpCaps->Removable = WdfFalse;
        pnpCaps->Address = 0;
        pnpCaps->UniqueID = WdfFalse;
        WdfDeviceSetPnpCapabilities(rootHubPdo, pnpCaps);

        //
        // Build the Power Capabilities for RootHub Pdo. Please Note that this code
        // has been adapted from USBPORT to preserve compat. So certain things may 
        // seem unusual / voilate WDM rules, however preseving COMPAT is most 
        // important here. 
        // Please note that in USB: 
        //    D2 implies armed for wake, and, 
        //    D3 implies not armed for wake
        //
        powerCaps = &ucxRootHubContext->PowerCaps;
        WDF_DEVICE_POWER_CAPABILITIES_INIT(powerCaps);
        powerCaps->DeviceWake = PowerDeviceD2;
        powerCaps->WakeFromD0 = WdfTrue;
        powerCaps->WakeFromD1 = WdfFalse;
        powerCaps->WakeFromD2 = WdfTrue;
        powerCaps->WakeFromD3 = WdfFalse;
        powerCaps->DeviceD2 = WdfTrue;
        powerCaps->DeviceD1 = WdfFalse;

        //
        // generate the root hub power capabilities from the
        // HC Power Attributes plus a little magic
        //

        //
        // Get the capabililites of the HCD
        //
        hcCaps = &ucxRootHubContext->UcxControllerContext->HcCaps;

        if (hcCaps->SystemWake == PowerSystemUnspecified) {
            powerCaps->SystemWake = PowerSystemWorking;
        } else {
            powerCaps->SystemWake = hcCaps->SystemWake;
        }

        //
        // We may update powerCaps->SystemWake below, store a 
        // copy original value into a local variable
        //
        oldRhSystemWake = powerCaps->SystemWake;

        for (s = PowerSystemSleeping1; s <= PowerSystemHibernate; s++) {

            if (s <= hcCaps->SystemWake) {

                //
                // s is a system state from which HCD can wake the system up.
                //
                if (hcCaps->DeviceState[s] != PowerDeviceUnspecified) {

                    //
                    // RootHub is allowed to wake the system up only from D2.
                    // Thus explicitly specify PowerDeviceD2 here. This might be against the 
                    // WDM rules, however is necessary to preserve compat
                    //
                    powerCaps->DeviceState[s] = PowerDeviceD2;

                    if (oldRhSystemWake == PowerSystemShutdown) {
                        //
                        // If the HCD Devnode specified that it can wake from PowerSystemShutdown, 
                        // update the SystemWake to the power-state lowest valid SystemPower
                        // state from which the device can be woken up.
                        //
                        powerCaps->SystemWake = s;
                    }

                } else {

                    //
                    // This is an invalid combination information recieved from the HCD devnode. 
                    // Thus set PowerDeviceD3 essentially indicating that we cannot wake 
                    // from this state.
                    //
                    powerCaps->DeviceState[s] = PowerDeviceD3;

                }
                
            } else {

                //
                // s is a system state from which HCD can not wake the system up.
                // Thus specify DeviceState as PowerDeviceD3
                //
                powerCaps->DeviceState[s] = PowerDeviceD3;

            }

        }

        //
        // TODO: Pending input from Kernel Team 
        // May need to also incorporate Hiber-boot, and how that will impact these 
        // settings (if at all)
        //

        // 
        // This code is for hybrid-sleep.  In this case, most drivers are notified
        // of S4, but the system really goes to S3.  If the bus supports wake from S3
        // but not wake from S4, then on hybrid-sleep we want to arm the bus for wake.
        // In order to trick the client drivers into arming themselves, we need to lie to 
        // them on the wake capabilities of the bus.
        // 
        // TODO: Pending input from Kernel Team? 
        // Re-verify that we should preserve this.  See what else we need
        // to do for the RhS3mappedToS4 (like not arming for wake if going to 
        // TRUE S4 if the controller isn't capable)
        // 
        // recompute root hub caps if FastS4MapS3toS4 is indicated.  This will cause the 
        // root hub to report it is wake capable from S4.
        // 
        if (powerCaps->DeviceState[PowerSystemSleeping3] == PowerDeviceD2 &&
            powerCaps->DeviceState[PowerSystemHibernate] != PowerDeviceD2) {
            // can wake from S3
            powerCaps->DeviceState[PowerSystemHibernate] = PowerDeviceD2;
            powerCaps->SystemWake = PowerSystemHibernate;
        } 
        
        
        WdfDeviceSetPowerCapabilities(rootHubPdo, powerCaps);

        rhPdoContext = RHPdoGetContext(rootHubPdo);
        rhPdoContext->Sig = SigRHPdoContext;
        rhPdoContext->UcxRootHubContext = ucxRootHubContext;
        rhPdoContext->UcxControllerContext = UcxControllerGetContext(UcxController);
        rhPdoContext->UcxRootHubContext->SystemPowerAction = PowerActionNone;

        status = WdfDeviceCreateDeviceInterface(rootHubPdo, &GUID_CLASS_USBHUB, NULL);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, 
                     "WdfDeviceCreateDeviceInterface failed WdfDevice 0x%p, Status %!STATUS!",
                     rootHubPdo, status);
            LEAVE;
        }

        //
        // Store WDFDEVICE, InterfaceString in our UCXROOTHUB Context
        //
        KeAcquireSpinLock(&ucxRootHubContext->RHPdoInfoLock, &irql); 
        ucxRootHubContext->RootHubPdo = rootHubPdo;
        KeReleaseSpinLock(&ucxRootHubContext->RHPdoInfoLock, irql);


        //
        // Tell WDF About this PDO
        //
        status = WdfFdoAddStaticChild(WdfDevice, rootHubPdo);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfFdoAddStaticChild for RH child pdo Failed %!STATUS!", status);
            LEAVE;
        }

        //
        // The rootHubPdo should not be deleted anymore.
        //
        deletePdoOnError = FALSE;

        //
        // Add a QueryInterface to interract with the Hub
        //
        status = RootHub_Pdo_QueryInterfaceAdd(rootHubPdo, UcxRootHub);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "RHPdoQueryInterfaceAdd for RH child pdo Failed %!STATUS!", status);
            LEAVE;
        }

    } FINALLY {

        if (!NT_SUCCESS(status)) {

            if (wdfDeviceInit != NULL) {
                WdfDeviceInitFree(wdfDeviceInit);
            }

            if ((rootHubPdo != NULL) && deletePdoOnError) {
                WdfObjectDelete(rootHubPdo);
            }

        }

        LogVerbose(FlagRootHub, "Completing %!STATUS!", status);
    }

    return status;
}

_Must_inspect_result_
NTSTATUS
RootHub_AddUsbDeviceContext(
    __in 
        UCXCONTROLLER      UcxController,
    __in 
        UCXROOTHUB         UcxRootHub
   )
/*++

Routine Description:

    This function adds necessary plumbing to the UcxRootHub so that it
    can also be treated as a UcxUsbDevice.

Arguments:

    UcxController - Handle to the controller object

    UcxRootHub - Handle to the roothub object

    DefaultEndpointQueue - The handle to the Queue of the default endpoint.

Return Value:

    NT_SUCCESS status if the operation is successful.

Commnets:
    Since WDF objects can have multiple contexts associated, so all we do is attach
    a context of type UCXUSBDEVICE_PRIVCONTEXT to this object. Now this UCXROOTHUB
    can be casted to UCXUSBDEVICE and all our code will still function.


--*/
{
    NTSTATUS                          status;
    WDF_OBJECT_ATTRIBUTES             objectAttributes;
    PUCXUSBDEVICE_PRIVCONTEXT         ucxUsbDeviceContext;
    PUCXCONTROLLER_PRIVCONTEXT        ucxControllerContext;
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext;

    TRY {
        WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&objectAttributes,
                                                UCXUSBDEVICE_PRIVCONTEXT);

        status = WdfObjectAllocateContext(UcxRootHub,
                                          &objectAttributes,
                                          &ucxUsbDeviceContext);
        if (! NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfObjectAllocateContext Failed %!STATUS!", status);
            LEAVE;
        }

        ucxControllerContext = UcxControllerGetContext(UcxController);

        ucxUsbDeviceContext->Sig = SigUcxUsbDeviceContext;
        ucxUsbDeviceContext->UcxUsbDevice = UCXROOTHUB_TO_UCXUSBDEVICE(UcxRootHub);
        ucxUsbDeviceContext->UcxControllerContext = ucxControllerContext;
        ucxUsbDeviceContext->UsbDeviceType = UsbDeviceTypeHub | UsbDeviceTypeRootHub;

        //
        // AMD Filter driver calls IsDeviceHighSpeed for the controller itself. 
        // Thus populating fake speed information for Roothub. (Windows 8 Bug 328163)
        //
        ucxUsbDeviceContext->UcxUsbDeviceInfo.DeviceSpeed = UsbSuperSpeed;

        InitializeListHead(&ucxUsbDeviceContext->EndpointListHead);
        InitializeListHead(&ucxUsbDeviceContext->StaleEndpointListHead);
        InitializeListHead(&ucxUsbDeviceContext->ChildListHead);
        InitializeListEntry(&ucxUsbDeviceContext->ChildListEntry);
        InitializeListHead(&ucxUsbDeviceContext->USBDInterfaceListHead);
        InitializeListHead(&ucxUsbDeviceContext->EndpointListHeadForInvalidHandleTracking);
        KeInitializeSpinLock(&ucxUsbDeviceContext->USBDInterfaceListLock);
        KeInitializeSpinLock(&ucxUsbDeviceContext->InvalidHandleTrakingLock);

        ucxUsbDeviceContext->Disconnected = FALSE;
        ucxUsbDeviceContext->DeprogrammedDueToControllerReset = FALSE;
        ucxUsbDeviceContext->AllowChildrenToExitTreePurge = TRUE;
        ucxUsbDeviceContext->AllowEndpointsToExitTreePurge = TRUE;

        ucxRootHubContext = UcxRootHubGetContext(UcxRootHub);
        ucxRootHubContext->UcxUsbDeviceContextOfUcxRootHub = ucxUsbDeviceContext;

        LogInfo(FlagRootHub, "RootHub(0x%p)'s UcxUsbDeviceContext 0x%p", UcxRootHub, ucxUsbDeviceContext);

    } FINALLY {

    }
    
    return status;
}

_Must_inspect_result_
NTSTATUS
RootHub_AddEndpoint(
    __in 
        UCXCONTROLLER      UcxController,
    __in 
        UCXROOTHUB         UcxRootHub,
    __in
        UCXENDPOINT_TYPE   EndpointType
   )
/*++

Routine Description:

    This function adds a (virtual) interrupt or default endpoint for the
    UcxRootHub object.

Arguments:

    UcxController - Handle to the controller object

    UcxRootHub - Handle to the roothub object

    EndpointType - Type indicating if this function is being called for the
    roothub's interrupt or default endpoint.

Return Value:

    NT_SUCCESS status if the operation is successful.

--*/
{
    NTSTATUS                             status;
    WDF_OBJECT_ATTRIBUTES                objectAttributes;
    PUCXROOTHUB_PRIVCONTEXT              ucxRootHubContext;
    PUCXCONTROLLER_PRIVCONTEXT           ucxControllerContext;
    WDFQUEUE                             wdfQueue;
    WDF_IO_QUEUE_CONFIG                  queueConfig;
    WDF_IO_QUEUE_FORWARD_PROGRESS_POLICY queueForwardProgressPolicy;
    UCXENDPOINT_INIT                     ucxEndpointInit;
    PUCXENDPOINT_INIT                    pucxEndpointInit;
    UCXENDPOINT                          ucxEndpoint = NULL;
    PROOTHUB_CONTROL_QUEUE_CONTEXT       rootHubControlQueueContext;
    PROOTHUB_INTERRUPT_QUEUE_CONTEXT     rootHubInterruptQueueContext;

    TRY {

        ucxRootHubContext = UcxRootHubGetContext(UcxRootHub);
        ucxControllerContext = UcxControllerGetContext(UcxController);

        //
        // Lets Create a queue ourselves
        //
        
        if (EndpointType == UcxEndpointTypeDefault) {
            
            WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchParallel);
            queueConfig.Settings.Parallel.NumberOfPresentedRequests = 
                ucxRootHubContext->Config.NumberOfPresentedControlUrbCallbacks;
            
            queueConfig.EvtIoInternalDeviceControl = RootHub_EvtControlTxHandler;

            WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&objectAttributes,
                                                    ROOTHUB_CONTROL_QUEUE_CONTEXT);
        } else {
            
            WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchSequential);
            
            queueConfig.EvtIoInternalDeviceControl = RootHub_EvtInterruptTxHandler;
            
            WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&objectAttributes,
                                                    ROOTHUB_INTERRUPT_QUEUE_CONTEXT);
        }
        
        objectAttributes.ParentObject = UcxRootHub;
        
        status = WdfIoQueueCreate(ucxControllerContext->ParentWdfDevice,
                                  &queueConfig,
                                  &objectAttributes,
                                  &wdfQueue);
        
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfIoQueueCreate Failed %!STATUS!", status);
            LEAVE;
        }

        WDF_IO_QUEUE_FORWARD_PROGRESS_POLICY_DEFAULT_INIT(
            &queueForwardProgressPolicy, 
            ROOTHUB_ENDPOINT_NUMBER_OF_FORWARD_PROGRESS_REQUESTS);
        
        status = WdfIoQueueAssignForwardProgressPolicy(wdfQueue,
                                                       &queueForwardProgressPolicy);
        
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub,
                     "WdfIoQueueAssignForwardProgressPolicy for Roothub Endpoint Failed %!STATUS!", status);
            LEAVE;
        }

        if (EndpointType == UcxEndpointTypeDefault) {
            rootHubControlQueueContext = RootHubControlQueueGetContext(wdfQueue);
            rootHubControlQueueContext->Sig = SigRootHubControlQueueContext;
            rootHubControlQueueContext->UcxRootHub = UcxRootHub;
        } else {
            rootHubInterruptQueueContext = RootHubInterruptQueueGetContext(wdfQueue);
            rootHubInterruptQueueContext->Sig = SigRootHubInterruptQueueContext;
            rootHubInterruptQueueContext->Queue = wdfQueue;
            rootHubInterruptQueueContext->UcxRootHub = UcxRootHub;
            KeInitializeSpinLock(&rootHubInterruptQueueContext->PortChangeLock);
            rootHubInterruptQueueContext->PortChangeGenCountLastProcessed = 0;
            rootHubInterruptQueueContext->PortChangeGenCount = 1;
            rootHubInterruptQueueContext->IndicateWakeStatusEnabled = FALSE;
            rootHubInterruptQueueContext->WasLastTransferCancelled = FALSE;
            KeInitializeEvent(&rootHubInterruptQueueContext->NoPendingIndicateWakeStatus,
                              NotificationEvent,
                              TRUE);
            ucxRootHubContext->RootHubInterruptQueueContext = rootHubInterruptQueueContext;
        }
        
        //
        // Create a UCXENDPOINT
        //
        RtlZeroMemory(&ucxEndpointInit, sizeof(ucxEndpointInit));
        ucxEndpointInit.Sig = SigUcxEndpointInit;
        ucxEndpointInit.UcxUsbDevice = UCXROOTHUB_TO_UCXUSBDEVICE(UcxRootHub);
        ucxEndpointInit.EndpointType = EndpointType;

        //
        // Root hub endpoints have no state machine. Their lifetime is tied to the roothub.
        //
        ucxEndpointInit.CreateStateMachine = FALSE;

        if (EndpointType == UcxEndpointTypeDefault) {
            
            LogVerbose(FlagRootHub, 
                       "Adding Virtual default endpoint for Roothub 0x%p", UcxRootHub);
            ucxEndpointInit.EndpointTransferType = EndpointTransferTypeControl;
        
        } else {
            
            LogVerbose(FlagRootHub, 
                       "Adding Virtual interrupt endpoint for Roothub 0x%p", UcxRootHub);
            ucxEndpointInit.EndpointTransferType = EndpointTransferTypeInterrupt;
        
        }

        pucxEndpointInit = &ucxEndpointInit;

        status = EXPORT(UcxEndpointCreate)(NULL,
                                           UCXROOTHUB_TO_UCXUSBDEVICE(UcxRootHub),
                                           &pucxEndpointInit,
                                           WDF_NO_OBJECT_ATTRIBUTES,
                                           &ucxEndpoint);
        if (! NT_SUCCESS(status)) {
            LogError(FlagRootHub, "UcxEndpointCreate Failed %!STATUS!", status);
            LEAVE;
        }

        EXPORT(UcxEndpointSetWdfIoQueue)(NULL, ucxEndpoint, wdfQueue);

        if (EndpointType == UcxEndpointTypeDefault) {
            ucxRootHubContext->DefaultQueue = wdfQueue;
            LogInfo(FlagRootHub, "RootHub(0x%p)'s Virtual Default Ep 0x%p", 
                    UcxRootHub, ucxEndpoint);
        } else {
            ucxRootHubContext->UcxInterruptEndpoint = ucxEndpoint;
            LogInfo(FlagRootHub, "RootHub(0x%p)'s Virtual Interrupt Ep 0x%p", 
                    UcxRootHub, ucxEndpoint);
        }

    } FINALLY {

        if (!NT_SUCCESS(status)) {

            if (ucxEndpoint != NULL) {
                WdfObjectDelete(ucxEndpoint);
            }

        }

    }
    
    return status;
}

VOID
RootHub_EvtCleanupCallback(
    __in 
        WDFOBJECT UcxRootHub
   )
/*++

Routine Description:

    This is an Event Callback function called when the roothub object is deleted.

Arguments:

    UcxRootHub - Handle to the roothub object

--*/
{
    PUCXROOTHUB_PRIVCONTEXT     ucxRootHubContext;
    PUCXCONTROLLER_PRIVCONTEXT  ucxControllerContext;
    KIRQL                       irql;

    TRY {
        
        ucxRootHubContext = UcxRootHubGetContext(UcxRootHub);
        ucxControllerContext = ucxRootHubContext->UcxControllerContext;

        Etw_UcxRootHubDelete(NULL, ucxRootHubContext);

        KeAcquireSpinLock(&ucxControllerContext->TopologyLock, &irql);

        ucxControllerContext->UcxRootHubContext = NULL;
        ucxControllerContext->ChildListCount--;

        KeReleaseSpinLock(&ucxControllerContext->TopologyLock, irql);

    } FINALLY {

    }

}

_Must_inspect_result_
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
EXPORT(UcxRootHubCreate)(
    __in_opt 
        PUCX_GLOBALS               Globals,
    __in 
        UCXCONTROLLER              UcxController,
    __in 
        PUCX_ROOTHUB_CONFIG        Config,
    __in_opt 
        PWDF_OBJECT_ATTRIBUTES     Attributes,
    __out
        UCXROOTHUB                *RootHub
   )
/*++

Routine Description:

    This function creates the root hub object

Arguments:

    Globals - Driver globals of the controller driver. This information is not
        being used.

    UcxController - Handle to the controller object

    Config - Pointer to a config structure describing various configuration
        operations for the roothub creation.

    Attributes - Pointer to the object attributes structure

    RootHub - Output - Pointer to a locatin that will recieve a handle to the
        roothub object

Return Value:

    NT_SUCCESS status if a roothub object is  successfully created

--*/
{
    NTSTATUS                          status;
    UCXROOTHUB                        ucxRootHub = NULL;
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext = NULL;
    PUCXCONTROLLER_PRIVCONTEXT        ucxControllerContext;
    WDF_OBJECT_ATTRIBUTES             objectAttributes;
    PWDF_OBJECT_ATTRIBUTES            objectAttributes1;
    PWDF_OBJECT_ATTRIBUTES            objectAttributes2;
    KIRQL                             irql;

    UNREFERENCED_PARAMETER(Globals);

    TRY {
        //
        // Parameter Validation
        //

        //NT_ASSERT(!"Add the necessary Parameter Validation");

        //
        // Create ucxcontroller object, using the Attributes given to us by
        // the controller driver to us. If the controller driver didnt give us
        // the attributes, use your own attributes.
        //
        ucxControllerContext = UcxControllerGetContext(UcxController);

        *RootHub = NULL;

        WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&objectAttributes,
                                                UCXROOTHUB_PRIVCONTEXT);

        objectAttributes.EvtCleanupCallback = RootHub_EvtCleanupCallback;

        if (Attributes != WDF_NO_OBJECT_ATTRIBUTES) {

            //
            // UCXCONTROLLER is to be parented to the WDFDEVICE
            //
            Attributes->ParentObject = UcxController;

            objectAttributes1 = Attributes;
            objectAttributes2 = &objectAttributes;

        } else {

            //
            // Controller Driver did not provide an attributes structure,
            // use our own.
            //

            objectAttributes.ParentObject = UcxController;

            objectAttributes1 = &objectAttributes;
            objectAttributes2 = WDF_NO_OBJECT_ATTRIBUTES;

        }

        status = Ucx_WdfObjectCreateWith2Attributes(objectAttributes1,
                                                    objectAttributes2,
                                                    &ucxRootHub );

        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfObjectCreate Failed %!STATUS!", status);
            LEAVE;
        }

        ucxRootHubContext = UcxRootHubGetContext(ucxRootHub);

        //
        // Initialize and populate UcxRootHubContext
        //

        ucxRootHubContext->Sig = SigUcxRootHubContext;
        ucxRootHubContext->UcxRootHub = ucxRootHub;
        ucxRootHubContext->UcxControllerContext = ucxControllerContext;

        KeInitializeSpinLock(&ucxRootHubContext->RHPdoInfoLock);

        RtlCopyMemory(&ucxRootHubContext->Config,
                      Config,
                      Config->Size);

        //
        // The UCXROOTHUB object will also need an associated
        // UCXUSBDEVICE (hub driver will need that), thus add
        // plumbing for that
        //
        status = RootHub_AddUsbDeviceContext(UcxController, ucxRootHub);

        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "RootHub_AddUsbDeviceContext Failed %!STATUS!", status);
            LEAVE;
        }

        //
        // The RootHub driver also needs a UCXENDPOINT handle to the
        // (virtual) default endpoint of the RootHub, thus add plumbing for
        // that.
        //
        status = RootHub_AddEndpoint(UcxController,
                                     ucxRootHub,
                                     UcxEndpointTypeDefault);


        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "RootHub_AddEndpoint (Default) Failed %!STATUS!", status);
            LEAVE;
        }

        //
        // The RootHub driver would also needs a UCXENDPOINT handle to the
        // (virtual) interrupt endpoint of the RootHub, thus add plumbing for
        // that.
        //
        status = RootHub_AddEndpoint(UcxController,
                                     ucxRootHub,
                                     0);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "RootHub_AddEndpoint (Interrupt) Failed %!STATUS!", status);
            LEAVE;
        }

        status = RootHub_StaticPdoSetup(UcxController,
                                        ucxControllerContext->ParentWdfDevice,
                                        ucxRootHub);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "RootHub_StaticPdoSetup Failed %!STATUS!", status);
            LEAVE;
        }

        KeAcquireSpinLock(&ucxControllerContext->TopologyLock, &irql);

        ucxControllerContext->UcxRootHubContext = ucxRootHubContext;
        ucxControllerContext->ChildListCount = 1;

        KeReleaseSpinLock(&ucxControllerContext->TopologyLock, irql);

    } FINALLY {

        if (!NT_SUCCESS(status)) {

            if (ucxRootHub != NULL) {
                WdfObjectDelete(ucxRootHub);
            }

        } else {
            *RootHub = ucxRootHub;

            Etw_UcxRootHubCreate(NULL, ucxRootHubContext);
        }

        LogInfo(FlagRootHub,"UcxRootHubCreate 0x%x", status);

    }

    return status;
}


__drv_maxIRQL(DISPATCH_LEVEL)
VOID
RootHub_PortChanged(
    __in 
        PUCXROOTHUB_PRIVCONTEXT    UcxRootHubContext
   )
/*++

Routine Description:

    This export function notifies the UCX about a new port change event.

Arguments:

    UcxRootHubContext - Context of the roothub object

Return Value:

    VOID

Comments:

    This routine will increment the PortChangeGenerationCount.
    In addition it would record that future RH Intr Tx's should be sent to the
    HCD. 
    If there was a RH Intr Tx already pending, it will be sent to the HCD
    
--*/
{
    PROOTHUB_INTERRUPT_QUEUE_CONTEXT  rootHubInterruptQueueContext;
    KIRQL                             irql;
    WDFREQUEST                        rhIntrTx;
    NTSTATUS                          status;
    BOOLEAN                           indicateWakeStausEnabled;
    KIRQL                             currentIrql;

    TRY {

        LogVerbose(FlagRootHub, "Port Change for UcxRootHubContext 0x%p", UcxRootHubContext);

        indicateWakeStausEnabled = FALSE;

        rootHubInterruptQueueContext = UcxRootHubContext->RootHubInterruptQueueContext;

        //
        // This needs to be synchronized with Multiple calls to this function
        // itself and the EvtDeviceInternalIoControl of the RootHub's interrupt
        // endpoint
        //
        KeAcquireSpinLock(&rootHubInterruptQueueContext->PortChangeLock, &irql);

        rootHubInterruptQueueContext->PortChangeGenCount++;

        rhIntrTx = NULL;
        
        if (rootHubInterruptQueueContext->PendingRHIntrTx != NULL) {
            
            status = WdfRequestUnmarkCancelable(rootHubInterruptQueueContext->PendingRHIntrTx);

            if (NT_SUCCESS(status)) {
                rhIntrTx = rootHubInterruptQueueContext->PendingRHIntrTx;
                rootHubInterruptQueueContext->PendingRHIntrTx = NULL;
                rootHubInterruptQueueContext->PortChangeGenCountLastProcessed = 
                    rootHubInterruptQueueContext->PortChangeGenCount;
            } else {
                NT_ASSERT (status == STATUS_CANCELLED);
            }
        }

        indicateWakeStausEnabled = rootHubInterruptQueueContext->IndicateWakeStatusEnabled;
        if (indicateWakeStausEnabled) {
            KeClearEvent(&rootHubInterruptQueueContext->NoPendingIndicateWakeStatus);
        }

        KeReleaseSpinLock(&rootHubInterruptQueueContext->PortChangeLock, irql);

        if (indicateWakeStausEnabled) {

            Etw_UcxRootHubInitiatingWake(NULL, UcxRootHubContext);

            //
            // It is possible that calling WdfDeviceIndicateWakeStatus at
            // PASSIVE_LEVEL will cause a D0 IRP to be requested and a call to
            // RootHub_Pdo_EvtD0Entry on this same thread.  This will deadlock
            // while waiting on the NoPendingIndicateWakeStatus event.  To fix,
            // ensure the IRQL is at DISPATCH_LEVEL so the D0 entry routine is
            // called on a different thread (at PASSIVE_LEVEL).
            //
            currentIrql = KeRaiseIrqlToDpcLevel();

            (VOID) WdfDeviceIndicateWakeStatus(UcxRootHubContext->RootHubPdo, STATUS_SUCCESS);

            KeLowerIrql(currentIrql);

            KeSetEvent(&rootHubInterruptQueueContext->NoPendingIndicateWakeStatus,
                       IO_NO_INCREMENT,
                       FALSE);
        }

        if (NULL != rhIntrTx) {

            if (Controller_ResetReferenceAcquire(UcxRootHubContext->UcxControllerContext)){
                
                UcxRootHubContext->Config.EvtRootHubInterruptTx(UcxRootHubContext->UcxRootHub,
                                                                rhIntrTx);
                Controller_ResetReferenceRelease(UcxRootHubContext->UcxControllerContext);
            
            } else {
                
                LogInfo(FlagRootHub, "Controller_ResetReferenceAcquire failed");
                WdfRequestComplete(rhIntrTx, STATUS_NO_SUCH_DEVICE);
            
            }

        }

    } FINALLY {


    }

    return;
}

__drv_maxIRQL(DISPATCH_LEVEL)
VOID
EXPORT(UcxRootHubPortChanged)(
    __in_opt 
        PUCX_GLOBALS               Globals,
    __in 
        UCXROOTHUB                 UcxRootHub
   )
/*++

Routine Description:

    This export function notifies the UCX about a new port change event from HCD

Arguments:

    Globals - Driver globals of the controller driver. This information is not
        being used.

    UcxRootHub - Handle to the roothub object

Return Value:

    VOID

--*/
{
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext;    
    
    UNREFERENCED_PARAMETER(Globals);

    TRY {
        ucxRootHubContext = UcxRootHubGetContext(UcxRootHub);
        
        RootHub_PortChanged(ucxRootHubContext);
    
    } FINALLY {

    }

    return;
}

EVT_WDF_REQUEST_CANCEL RootHub_EvtInterruptTxCancel;

VOID RootHub_EvtInterruptTxCancel(
    WDFREQUEST Request
   )
/*++

Routine Description:

    This function is called when the hub driver wasnt to cancel a 
    roothub interrupt transfer that is pending in UCX

Arguments:

    WdfRequest - Handle to the WDFREQUEST for the RH interrupt transfer

Return Value:

    VOID

--*/
{
    KIRQL                             irql;
    PROOTHUB_INTERRUPT_QUEUE_CONTEXT  rootHubInterruptQueueContext;
    
    TRY {

        rootHubInterruptQueueContext = 
            RootHubInterruptQueueGetContext(WdfRequestGetIoQueue(Request));

        KeAcquireSpinLock(&rootHubInterruptQueueContext->PortChangeLock, &irql);

        NT_ASSERT(Request == rootHubInterruptQueueContext->PendingRHIntrTx);

        rootHubInterruptQueueContext->PendingRHIntrTx = NULL;

        rootHubInterruptQueueContext->WasLastTransferCancelled = TRUE;

        KeReleaseSpinLock(&rootHubInterruptQueueContext->PortChangeLock, irql);

        WdfRequestComplete(Request, STATUS_CANCELLED);

    } FINALLY {


    }

    return;

}

VOID
RootHub_EvtInterruptTxHandler(
    WDFQUEUE                WdfQueue,
    WDFREQUEST              WdfRequest,
    size_t                  OutputBufferLength,
    size_t                  InputBufferLingth,
    ULONG                   IoControlCode
   )
/*++

Routine Description:

    This event callback function handles the interrupt transfers on the
    roothub's interrupt pipe.

Arguments:

    WdfQueue - A handle to the Roothub's interrupt queue object.

    WdfRequest - A handle to the wdfrequest on this queue.

    OutputBufferLength - Not used

    InputBufferLength - Not used

    IoControlCode - Not used

Return Value:

    VOID

--*/
{
    PROOTHUB_INTERRUPT_QUEUE_CONTEXT  rootHubInterruptQueueContext;
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext;
    PURB                              urb;
    BOOLEAN                           sendTxToHCD = TRUE;
    BOOLEAN                           failTx = FALSE;
    KIRQL                             irql;
    NTSTATUS                          status = STATUS_SUCCESS;                

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLingth);
    UNREFERENCED_PARAMETER(IoControlCode);

    TRY {

        rootHubInterruptQueueContext = RootHubInterruptQueueGetContext(WdfQueue);
        ucxRootHubContext = UcxRootHubGetContext(rootHubInterruptQueueContext->UcxRootHub);

        //
        // Unpack the request to pull out the Urb
        //
        urb = (PURB) Ucx_WdfRequestGetArg1(WdfRequest); 

        NT_ASSERT(urb->UrbHeader.Function == URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER);

        KeAcquireSpinLock(&rootHubInterruptQueueContext->PortChangeLock, &irql);

        rootHubInterruptQueueContext->WasLastTransferCancelled = FALSE;

        NT_ASSERT(NULL == rootHubInterruptQueueContext->PendingRHIntrTx);

        if (TEST_FLAG(urb->UrbHeader.UsbdFlags, USBDFLAG_ROOTHUB_PEND_INTERRUPT_TX)) {
            //
            // Hub Recieved all 0's in the last interrupt tx. 
            // So it is requesting to pending the interrupt tx.
            //
            if (rootHubInterruptQueueContext->PortChangeGenCount 
                == rootHubInterruptQueueContext->PortChangeGenCountLastProcessed) {
                //
                // In addition, hcd has not notified us of any new port changes.
                // This implies we should hold on to this interrupt request.
                //
                sendTxToHCD = FALSE;
            } else {
                //
                // After we last sent a RH Interrupt Tx to the HCD, 
                // hcd has called UcxRootHubPortChanged. Thus we cannot
                // pend the interrupt transfer.
                //
            }

        }

        if (sendTxToHCD) {
            
            rootHubInterruptQueueContext->PortChangeGenCountLastProcessed = 
                rootHubInterruptQueueContext->PortChangeGenCount;
        
        } else {
            
            status = WdfRequestMarkCancelableEx(WdfRequest, RootHub_EvtInterruptTxCancel);

            if (NT_SUCCESS(status)) {
                rootHubInterruptQueueContext->PendingRHIntrTx = WdfRequest;
            } else {
                failTx = TRUE;
            }
        }

        KeReleaseSpinLock(&rootHubInterruptQueueContext->PortChangeLock, irql);

        NT_ASSERT(FALSE == (sendTxToHCD && failTx));

        if (failTx) {
            LEAVE;
        }

        if (sendTxToHCD) {

            if (FALSE == Controller_ResetReferenceAcquire(ucxRootHubContext->UcxControllerContext)){
                LogInfo(FlagRootHub, "Controller_ResetReferenceAcquire failed");
                failTx = TRUE;
                status = STATUS_NO_SUCH_DEVICE;
                LEAVE;
            }

            ucxRootHubContext->Config.EvtRootHubInterruptTx(ucxRootHubContext->UcxRootHub,
                                                            WdfRequest);

            Controller_ResetReferenceRelease(ucxRootHubContext->UcxControllerContext);
        }

    } FINALLY {

        if (failTx) {
            
            LogError(FlagRootHub, "Failing RH Intr Tx, 0x%p, 0x%!STATUS!", WdfRequest, status);
            
            WdfRequestComplete(WdfRequest, status);
        }
    
    }

}

VOID
RootHub_StartFailingRootHubIO(
    PUCXROOTHUB_PRIVCONTEXT           UcxRootHubContext
    )
/*++

Routine description:

    Start failing root hub IO due to condition such as controller reset or controller failed

Arguments:

    UcxRootHubContext - Root hub

Return value:

    None (always succeeds)

--*/
{
    WDFQUEUE                          rootHubInterruptQueue;
    PROOTHUB_INTERRUPT_QUEUE_CONTEXT  rootHubInterruptQueueContext;

    TRY {
        
        rootHubInterruptQueueContext = UcxRootHubContext->RootHubInterruptQueueContext;
        rootHubInterruptQueue = rootHubInterruptQueueContext->Queue;
        
        //
        // Call Purge On the Interrupt Queue Of the RootHub
        //
        WdfIoQueuePurge(rootHubInterruptQueue, NULL, NULL);

        //
        // Call Purge On the Control Queue Of the RootHub
        //
        WdfIoQueuePurge(UcxRootHubContext->DefaultQueue, NULL, NULL);

    } FINALLY {

    }
}

VOID
RootHub_ResumeRootHubIO(
    PUCXROOTHUB_PRIVCONTEXT           UcxRootHubContext
    )
/*++

Routine description:

    Restart root hub queues, if the controller hasn't failed

Arguments:

    UcxRootHubContext - Root hub

Return value:

    None

--*/
{
    WDFQUEUE                          rootHubInterruptQueue;
    PROOTHUB_INTERRUPT_QUEUE_CONTEXT  rootHubInterruptQueueContext;
    KIRQL                             irql;

    TRY {
        
        rootHubInterruptQueueContext = UcxRootHubContext->RootHubInterruptQueueContext;
        rootHubInterruptQueue = rootHubInterruptQueueContext->Queue;

        //
        // Increment the PortChangeGenCount, so that when we resume the RootHubIO, 
        // we do send the next transfer to HCD.
        // This has no effect if the controller fails before we start the queues.
        //
        KeAcquireSpinLock(&rootHubInterruptQueueContext->PortChangeLock, &irql);
        rootHubInterruptQueueContext->PortChangeGenCount++;
        KeReleaseSpinLock(&rootHubInterruptQueueContext->PortChangeLock, irql);

        //
        // Call Start On the Interrupt Queue Of the RootHub
        //
        WdfIoQueueStart(rootHubInterruptQueue);
        
        //
        // Call Start On the Control Queue Of the RootHub
        //
        WdfIoQueueStart(UcxRootHubContext->DefaultQueue);

    } FINALLY {

    }
}


VOID
RootHub_EvtControlTxHandler(
    WDFQUEUE                WdfQueue,
    WDFREQUEST              WdfRequest,
    size_t                  OutputBufferLength,
    size_t                  InputBufferLength,
    ULONG                   IoControlCode
   )
/*++

Routine Description:

    This event callback function handles the control transfers on the
    roothub's control pipe.

Arguments:

    WdfQueue - A handle to the Roothub's interrupt queue object.

    WdfRequest - A handle to the wdfrequest on this queue.

    OutputBufferLength - Not used

    InputBufferLength - Not used

    IoControlCode - Not used

Return Value:

    VOID

--*/
{

    BOOLEAN                           requestHandled = FALSE;
    BOOLEAN                           resetReferenceAcquired;
    PURB                              urb;
    PWDF_USB_CONTROL_SETUP_PACKET     setupPacket;
    UCXROOTHUB                        ucxRootHub;
    PROOTHUB_CONTROL_QUEUE_CONTEXT    rootHubControlQueueContext;
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext;
    PUCXCONTROLLER_PRIVCONTEXT        ucxControllerContext;
    NTSTATUS                          status = STATUS_NOT_SUPPORTED;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(IoControlCode);

    TRY {

        NT_ASSERT(IoControlCode == IOCTL_INTERNAL_USB_SUBMIT_URB);

        rootHubControlQueueContext = RootHubControlQueueGetContext(WdfQueue);
        ucxRootHub = rootHubControlQueueContext->UcxRootHub;
        ucxRootHubContext = UcxRootHubGetContext(ucxRootHub);
        ucxControllerContext = ucxRootHubContext->UcxControllerContext;

        resetReferenceAcquired = Controller_ResetReferenceAcquire(ucxControllerContext);

        if (resetReferenceAcquired == FALSE) {
            LogInfo(FlagRootHub,"Controller_ResetReferenceAcquire Failed %!STATUS!", status);
            status = STATUS_NO_SUCH_DEVICE;
            LEAVE;
        }

        urb = (PURB) Ucx_WdfRequestGetArg1(WdfRequest);

        setupPacket = (PWDF_USB_CONTROL_SETUP_PACKET) &urb->UrbControlTransferEx.SetupPacket[0];

        //
        // Check for Clear Hub Feature
        //
        if (setupPacket->Packet.bm.Request.Recipient == BMREQUEST_TO_DEVICE &&
            setupPacket->Packet.bm.Request.Type == BMREQUEST_CLASS &&
            setupPacket->Packet.bm.Request.Dir == BMREQUEST_HOST_TO_DEVICE &&
            setupPacket->Packet.bRequest == USB_REQUEST_CLEAR_FEATURE)
        {
            if (ucxRootHubContext->Config.EvtRootHubClearHubFeature != NULL) {

                LogVerbose(FlagRootHub, "Roothub Ctrl Tx: RootHubClearHubFeature, urb 0x%p", urb);
                ucxRootHubContext->Config.EvtRootHubClearHubFeature(ucxRootHub, WdfRequest);
                requestHandled = TRUE;
            }
            LEAVE;
        }

        //
        // Check for Clear Port Feature
        //
        if (setupPacket->Packet.bm.Request.Recipient == BMREQUEST_TO_OTHER &&
            setupPacket->Packet.bm.Request.Type == BMREQUEST_CLASS &&
            setupPacket->Packet.bm.Request.Dir == BMREQUEST_HOST_TO_DEVICE &&
            setupPacket->Packet.bRequest == USB_REQUEST_CLEAR_FEATURE)
        {
            if (ucxRootHubContext->Config.EvtRootHubClearPortFeature != NULL) {
                
                LogVerbose(FlagRootHub, "Roothub Ctrl Tx: RootHubClearPortFeature, urb 0x%p", urb);
                ucxRootHubContext->Config.EvtRootHubClearPortFeature(ucxRootHub, WdfRequest);
                requestHandled = TRUE;
            }
            LEAVE;
        }

        //
        // Check for Get Hub Status
        //
        if (setupPacket->Packet.bm.Request.Recipient == BMREQUEST_TO_DEVICE &&
            setupPacket->Packet.bm.Request.Type == BMREQUEST_CLASS &&
            setupPacket->Packet.bm.Request.Dir == BMREQUEST_DEVICE_TO_HOST &&
            setupPacket->Packet.bRequest == USB_REQUEST_GET_STATUS)
        {
            if (ucxRootHubContext->Config.EvtRootHubGetHubStatus != NULL) {
                
                LogVerbose(FlagRootHub, "Roothub Ctrl Tx: RootHubGetHubStatus, urb 0x%p", urb);
                ucxRootHubContext->Config.EvtRootHubGetHubStatus(ucxRootHub, WdfRequest);
                requestHandled = TRUE;
            }
            LEAVE;
        }

        //
        // Check for Get Port Status
        //
        if (setupPacket->Packet.bm.Request.Recipient == BMREQUEST_TO_OTHER &&
            setupPacket->Packet.bm.Request.Type == BMREQUEST_CLASS &&
            setupPacket->Packet.bm.Request.Dir == BMREQUEST_DEVICE_TO_HOST &&
            setupPacket->Packet.bRequest == USB_REQUEST_GET_STATUS)
        {
            if (ucxRootHubContext->Config.EvtRootHubGetPortStatus != NULL) {
                LogVerbose(FlagRootHub, "Roothub Ctrl Tx: RootHubGetPortStatus, urb 0x%p", urb);
                ucxRootHubContext->Config.EvtRootHubGetPortStatus(ucxRootHub, WdfRequest);
                requestHandled = TRUE;
            }
            LEAVE;
        }

        //
        // Check for Set Hub Feature
        //
        if (setupPacket->Packet.bm.Request.Recipient == BMREQUEST_TO_DEVICE &&
            setupPacket->Packet.bm.Request.Type == BMREQUEST_CLASS &&
            setupPacket->Packet.bm.Request.Dir == BMREQUEST_HOST_TO_DEVICE &&
            setupPacket->Packet.bRequest == USB_REQUEST_SET_FEATURE)
        {
            if (ucxRootHubContext->Config.EvtRootHubSetHubFeature != NULL) {

                LogVerbose(FlagRootHub, "Roothub Ctrl Tx: RootHubSetHubFeature, urb 0x%p", urb);
                ucxRootHubContext->Config.EvtRootHubSetHubFeature(ucxRootHub, WdfRequest);
                requestHandled = TRUE;
            }
            LEAVE;
        }

        //
        // Check for Set Port Feature
        //
        if (setupPacket->Packet.bm.Request.Recipient == BMREQUEST_TO_OTHER &&
            setupPacket->Packet.bm.Request.Type == BMREQUEST_CLASS &&
            setupPacket->Packet.bm.Request.Dir == BMREQUEST_HOST_TO_DEVICE &&
            setupPacket->Packet.bRequest == USB_REQUEST_SET_FEATURE)
        {
            if (ucxRootHubContext->Config.EvtRootHubSetPortFeature != NULL) {
                
                LogVerbose(FlagRootHub, "Roothub Ctrl Tx: RootHubSetPortFeature, urb 0x%p", urb);
                ucxRootHubContext->Config.EvtRootHubSetPortFeature(ucxRootHub, WdfRequest);
                requestHandled = TRUE;
            }
            LEAVE;
        }


        //
        // Check for Get Port Error Count
        //
        if (setupPacket->Packet.bm.Request.Recipient == BMREQUEST_TO_DEVICE &&
            setupPacket->Packet.bm.Request.Type == BMREQUEST_STANDARD &&
            setupPacket->Packet.bm.Request.Dir == BMREQUEST_DEVICE_TO_HOST &&
            setupPacket->Packet.bRequest == USB_REQUEST_GET_PORT_ERR_COUNT)
        {
            if (ucxRootHubContext->Config.EvtRootHubGetPortErrorCount != NULL) {
                
                LogVerbose(FlagRootHub, "Roothub Ctrl Tx: RootHubGetPortErrorCount, urb 0x%p", urb);
                ucxRootHubContext->Config.EvtRootHubGetPortErrorCount(ucxRootHub, WdfRequest);
                requestHandled = TRUE;
            }
            LEAVE;
        }

        if (ucxRootHubContext->Config.EvtRootHubControlUrb) {
            LogVerbose(FlagRootHub, "Roothub Ctrl Tx: RootHubControlUrb, urb 0x%p", urb);
            ucxRootHubContext->Config.EvtRootHubControlUrb(ucxRootHub, WdfRequest);
            requestHandled = TRUE;
        }

    } FINALLY {

        if (resetReferenceAcquired) {
            Controller_ResetReferenceRelease(ucxControllerContext);

            //
            // Unhandled request is not expected unless we are in controller reset
            //
            if (FALSE == requestHandled) {
                LogWarning(FlagRootHub, "Unexpected Request 0x%p", WdfRequest);
                NT_ASSERT(!"Unexpected Request");
            }
        }

        if (FALSE == requestHandled) {
            WdfRequestComplete(WdfRequest, status);
        }
    }

}

FORCEINLINE
WDFQUEUE
RootHub_GetTargetQueue(
    __in 
        PUCXCONTROLLER_PRIVCONTEXT  UcxControllerContext,
    __in 
        ULONG                       IoControlCode
   )
/*++

Routine Description:

    Looks at a IoControlCode and returns the correct target WdfQueue
    
Arguments: 
    
    IoControlCode - The Ioctl Code
    
Return Value:
    
    WDFQUEUE to which the Ioctl must be forwarded to.     

--*/
{
    WDFQUEUE wdfQueue;
    
    TRY {

        switch (IoControlCode) {

        case IOCTL_INTERNAL_USB_USBDEVICE_ENABLE:
        case IOCTL_INTERNAL_USB_USBDEVICE_RESET:
        case IOCTL_INTERNAL_USB_USBDEVICE_ADDRESS:
        case IOCTL_INTERNAL_USB_USBDEVICE_UPDATE:
        case IOCTL_INTERNAL_USB_USBDEVICE_HUB_INFO:
        case IOCTL_INTERNAL_USB_USBDEVICE_DISABLE:
        case IOCTL_INTERNAL_USB_ENDPOINTS_CONFIGURE:
        case IOCTL_INTERNAL_USB_DEFAULT_ENDPOINT_UPDATE:
        case IOCTL_INTERNAL_USB_ENDPOINT_RESET:
        case IOCTL_INTERNAL_USB_USBDEVICE_PURGEIO:
        case IOCTL_INTERNAL_USB_USBDEVICE_STARTIO:
        case IOCTL_INTERNAL_USB_USBDEVICE_ABORTIO:

            //
            // Forward the Request to the UsbDeviceMgmt queue
            //
            wdfQueue = UcxControllerContext->QueueUsbDeviceMgmt;
            break;

        case IOCTL_INTERNAL_USB_USBDEVICE_TREE_PURGEIO:

            //
            // Forward the Request to the TreePurge queue
            //
            wdfQueue = UcxControllerContext->QueueTreePurge;
            break;

        case IOCTL_INTERNAL_USB_ROOTHUB_GET_INFO:
        case IOCTL_INTERNAL_USB_ROOTHUB_GET_20PORT_INFO:
        case IOCTL_INTERNAL_USB_ROOTHUB_GET_30PORT_INFO:
        case IOCTL_INTERNAL_USB_REGISTER_COMPOSITE_DEVICE:
        case IOCTL_INTERNAL_USB_UNREGISTER_COMPOSITE_DEVICE:
        case IOCTL_INTERNAL_USB_SET_FUNCTION_HANDLE_DATA:
        case IOCTL_INTERNAL_USB_REQUEST_REMOTE_WAKE_NOTIFICATION:
        case IOCTL_INTERNAL_USB_QUERY_USB_CAPABILITY:
        case IOCTL_INTERNAL_USB_GET_DUMP_DATA:
        case IOCTL_INTERNAL_USB_FREE_DUMP_DATA:
        case IOCTL_INTERNAL_USB_NOTIFY_FORWARD_PROGRESS:
            //
            // Forward the Request to the UcxDefault queue
            //
            wdfQueue = UcxControllerContext->QueueUcxDefault;
            break;

        case IOCTL_INTERNAL_USB_ADDRESS0_OWNERSHIP_ACQUIRE:
            //
            // Forward the Request to the Address 0 Ownership Queue
            //
            wdfQueue = UcxControllerContext->QueueAddress0Ownership;
            break;

        default:
            wdfQueue = NULL;
            break;
        }
    } FINALLY {

    }

    return wdfQueue;
}

IO_COMPLETION_ROUTINE RootHub_Pdo_IrpCompletion;

NTSTATUS
RootHub_Pdo_IrpCompletion(
    PDEVICE_OBJECT  DeviceObject,    
    PIRP            Irp,
    PVOID           Context
   )
/*++

Routine Description:

    Completion routine (installed while pre-processing an Irp at the Roothub Pdo level) for 
    any ioctl except IOCTL_INTERNAL_USB_SUBMIT_URB

Arguments:

    DeviceObject - The Roothub Pdo

    Irp - The pointer to the irp 
    
    Context - Unreferenced.

Return Value:

    STATUS_MORE_PROCESSING_REQUIRED or STATUS_CONTINUE_COMPLETION depending on the IOCTL.

--*/
{
    PIO_STACK_LOCATION          irpStack;
    NTSTATUS                    status;
    WDFDEVICE                   device;
    PRHPDO_CONTEXT              rhPdoContext;
    PHUBUCX_ROOTHUB_INFO        hubUcxRootHubInfo;
    PUCXCONTROLLER_PRIVCONTEXT  ucxControllerContext;
    
    UNREFERENCED_PARAMETER(DeviceObject);

    TRY {

        device = (WDFDEVICE) Context;
        
        //
        // Propogate the pending flag
        //
        if (Irp->PendingReturned) {
            IoMarkIrpPending(Irp);
        }
        
        irpStack = IoGetCurrentIrpStackLocation(Irp);

        LogVerbose(FlagRootHub, "%!UCX_IOCTL! Irp %p Completing from Hcd", 
                   irpStack->Parameters.DeviceIoControl.IoControlCode, Irp);

        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
        
        case IOCTL_INTERNAL_USB_ROOTHUB_GET_INFO:
        
            if (!NT_SUCCESS(Irp->IoStatus.Status)) {
                status = STATUS_CONTINUE_COMPLETION;
                LEAVE;
            }

            rhPdoContext = RHPdoGetContext(device);
            ucxControllerContext = rhPdoContext->UcxControllerContext;
            hubUcxRootHubInfo = (PHUBUCX_ROOTHUB_INFO) irpStack->Parameters.Others.Argument1;

            //
            // Before the request was sent to HCD, Size was set to sizeof(ROOTHUB_INFO),
            // because only ROOTHUB_INFO is exposed to HCD. Fix it back to its original size.
            //
            hubUcxRootHubInfo->Size = sizeof(HUBUCX_ROOTHUB_INFO);

            rhPdoContext->UcxRootHubContext->NumberOf20Ports = hubUcxRootHubInfo->NumberOf20Ports;
            rhPdoContext->UcxRootHubContext->NumberOf30Ports = hubUcxRootHubInfo->NumberOf30Ports;

            status = STATUS_CONTINUE_COMPLETION;
            break;

        default: 
            status = STATUS_CONTINUE_COMPLETION;
            break;

        }

    } FINALLY {

    }
    
    return status;
}

IO_COMPLETION_ROUTINE RootHub_Pdo_DeviceMgmtIrpCompletion;

NTSTATUS
RootHub_Pdo_DeviceMgmtIrpCompletion(
    PDEVICE_OBJECT  DeviceObject,    
    PIRP            Irp,
    PVOID           Context
   )
/*++

Routine Description:

    Completion routine (installed while pre-processing an Irp at the Roothub Pdo level) for 
    any ioctl except IOCTL_INTERNAL_USB_SUBMIT_URB

Arguments:

    DeviceObject - The Roothub Pdo

    Irp - The pointer to the irp 
    
    Context - Unreferenced.

Return Value:

    STATUS_MORE_PROCESSING_REQUIRED or STATUS_CONTINUE_COMPLETION depending on the IOCTL.

--*/
{
    PIO_STACK_LOCATION                irpStack;
    NTSTATUS                          status;
    UCXUSBDEVICE                      ucxUsbDevice;
    PUCXUSBDEVICE_PRIVCONTEXT         ucxUsbDeviceContext;
    
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Context);

    TRY {

        //
        // Propogate the pending flag
        //
        if (Irp->PendingReturned) {
            IoMarkIrpPending(Irp);
        }
        
        irpStack = IoGetCurrentIrpStackLocation(Irp);
        
        LogVerbose(FlagRootHub, "%!UCX_IOCTL! Irp %p Completed from Hcd", 
                   irpStack->Parameters.DeviceIoControl.IoControlCode, Irp);

        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
        
        case IOCTL_INTERNAL_USB_ENDPOINTS_CONFIGURE: 

            UsbDevice_EndpointsConfigureCompleteFromHcd(
                Irp,
                (PENDPOINTS_CONFIGURE)irpStack->Parameters.Others.Argument1);

            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;

        case IOCTL_INTERNAL_USB_USBDEVICE_ENABLE: 

            UsbDevice_DefaultEndpointEnableCompleteFromHcd(
                Irp, 
                (PUSBDEVICE_ENABLE) irpStack->Parameters.Others.Argument1);

            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;

        case IOCTL_INTERNAL_USB_USBDEVICE_RESET: 

            ucxUsbDevice = ((PUSBDEVICE_RESET) irpStack->Parameters.Others.Argument1)->UsbDevice;
            ucxUsbDeviceContext = UcxUsbDeviceGetContext(ucxUsbDevice);

            if (ucxUsbDeviceContext->UsbDeviceResetFailedDueToControllerReset == 1) {
                //
                // We did not successfully complete the UsbDeviceReset operation due to a controller 
                // reset process.
                // Thus we do not process it on the completion path either. 
                //
                ucxUsbDeviceContext->UsbDeviceResetFailedDueToControllerReset = 0;
                status = STATUS_CONTINUE_COMPLETION;
            } else {
                UsbDevice_ResetCompleteFromHcd(
                    Irp, 
                    (PUSBDEVICE_RESET) irpStack->Parameters.Others.Argument1);
                status = STATUS_MORE_PROCESSING_REQUIRED;
            }

            break;

        case IOCTL_INTERNAL_USB_ENDPOINT_RESET:

            ucxUsbDevice = ((PENDPOINT_RESET) irpStack->Parameters.Others.Argument1)->UsbDevice;
            ucxUsbDeviceContext = UcxUsbDeviceGetContext(ucxUsbDevice);

            if (ucxUsbDeviceContext->EndpointResetFailedDueToControllerReset == 1) {
                //
                // We did not successfully complete the EndpointReset operation due to a controller 
                // reset process.
                // The EndpointReset request was not handled on the way down. 
                // Thus we do not process it on the completion path either. 
                //
                ucxUsbDeviceContext->EndpointResetFailedDueToControllerReset = 0;
                status = STATUS_CONTINUE_COMPLETION;
            } else {
                Endpoint_ResetCompleteFromHcd(
                    Irp,
                    (PENDPOINT_RESET) irpStack->Parameters.Others.Argument1);
                status = STATUS_MORE_PROCESSING_REQUIRED;
            }
            break;

        default: 
            status = STATUS_CONTINUE_COMPLETION;
            break;

        }

    } FINALLY {

    }
    return status;
}

NTSTATUS
RootHub_Pdo_EvtInternalDeviceControlIrpPreprocessCallback(
    WDFDEVICE   Device,
    PIRP        Irp
   )
/*++

Routine Description:

    This is a Irp Preprocess event callback of the controller extension. This
    is called when a new Irp reaches the Roothub PDO.

Arguments:

    Device - A handle representing the WDFDEVICE for the enumerated Roothub Pdo

    Irp - A pointer to the irp that has been sent to the Roothub pdo.

    DispatchContext - UNUSED

--*/
{
    PIO_STACK_LOCATION                irpStack;
    NTSTATUS                          status;
    WDFQUEUE                          wdfQueue;
    PDEVICE_OBJECT                    pdo;
    PUCXCONTROLLER_PRIVCONTEXT        ucxControllerContext;
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext;
    PRHPDO_CONTEXT                    rhPdoContext;

    TRY {        
        irpStack = IoGetCurrentIrpStackLocation(Irp);

        LogVerbose(FlagRootHub, "%!UCX_IOCTL! Irp %p From Hub", 
                   irpStack->Parameters.DeviceIoControl.IoControlCode, Irp);

        if (irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_INTERNAL_USB_SUBMIT_URB) {
            //
            //Special Handling for Urb Request due to performance & compat
            //
            status = Xrb_ProcessSubmitUrbIoctl(Device, Irp, irpStack);
            LEAVE;
        }

        pdo = WdfDeviceWdmGetDeviceObject(Device);
        rhPdoContext = RHPdoGetContext(Device);
        ucxControllerContext = rhPdoContext->UcxControllerContext;
        ucxRootHubContext = rhPdoContext->UcxRootHubContext;
        
        if (irpStack->Parameters.DeviceIoControl.IoControlCode 
            == IOCTL_INTERNAL_USB_RESET_PORT_ASYNC) {
            
            if (ucxControllerContext->Config.EvtControllerReset == NULL) {

                //
                // This controller does not support reset operation. Fail request from the 
                // roothub
                //
                status = STATUS_NOT_SUPPORTED;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                LogInfo(FlagRootHub, "Controller reset IOCTL %!STATUS! Controller %p doesn't support",
                        status, ucxControllerContext);

            } else if (ucxControllerContext->ControllerFailed != 0) {

                //
                // The controller does not want any more reset requests.
                // After the hub driver retries a few times, the hub stack will start tearing down.
                // If we are powering up on a missing controller, failing here prevents us from
                // processing the controller reset in Dx, which is invalid in the state machine.
                //
                status = STATUS_UNSUCCESSFUL;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                LogInfo(FlagRootHub, "Controller reset IOCTL %!STATUS! due to failed controller %p",
                        status, ucxControllerContext);

            } else {

                NT_ASSERT(ucxRootHubContext->AsyncResetIrpFromHub == NULL);
                IoMarkIrpPending(Irp);
                ucxRootHubContext->AsyncResetIrpFromHub = Irp;
                Controller_EventAdd(ucxControllerContext, ControllerResetEventHubNeedsReset);
                status = STATUS_PENDING;
            }

            LEAVE;
        }

        //
        // Get the Target Queue.
        // Only IOCTL_INTERNAL_USB_USBDEVICE_ENABLE needs that but for
        // code simplicity, fetch it anyway. This is not a perf path
        //
        wdfQueue = RootHub_GetTargetQueue(ucxControllerContext,
                                          irpStack->Parameters.DeviceIoControl.IoControlCode);
        
        if (NULL == wdfQueue) {
            //
            // This implies we do not understand this request.
            // Fail the request
            //
            //NT_ASSERTMSG("wdfQueue should not be NULL", FALSE);
            LogInfo(FlagRootHub, "Unknown Irp 0x%p, IoControlCode 0x%!UCX_IOCTL!", 
                     Irp, irpStack->Parameters.DeviceIoControl.IoControlCode);

            status = STATUS_NOT_SUPPORTED;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            LEAVE;
        }

        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
        
        case IOCTL_INTERNAL_USB_ENDPOINTS_CONFIGURE:     
        case IOCTL_INTERNAL_USB_USBDEVICE_ENABLE:
        case IOCTL_INTERNAL_USB_USBDEVICE_RESET:
        case IOCTL_INTERNAL_USB_ENDPOINT_RESET:
            
            //
            // These Ioctls will need a completion routine
            //
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutineSmart(pdo, 
                                        Irp, 
                                        RootHub_Pdo_DeviceMgmtIrpCompletion, 
                                        (PVOID)Device, 
                                        TRUE, 
                                        TRUE, 
                                        TRUE);
            break;

        case IOCTL_INTERNAL_USB_ROOTHUB_GET_INFO:
            
            //
            // These Ioctls will need a completion routine
            //
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutineSmart(pdo, 
                                        Irp, 
                                        RootHub_Pdo_IrpCompletion, 
                                        (PVOID)Device, 
                                        TRUE, 
                                        TRUE, 
                                        TRUE);
            break;
        
        default:
            
            IoSkipCurrentIrpStackLocation(Irp);
            break;
        }

        status = WdfDeviceWdmDispatchIrpToIoQueue(
            Device, 
            Irp, 
            wdfQueue,
            WDF_DISPATCH_IRP_TO_IO_QUEUE_PREPROCESSED_IRP);

    } FINALLY {


    }
    return status;
}

NTSTATUS 
RootHub_Pdo_EvtPrepareHardware(
    WDFDEVICE    Device,
    WDFCMRESLIST ResourcesRaw,
    WDFCMRESLIST ResourcesTranslated
   )
/*
Routine Description:

    This is a PrepareHardware Routine for the Roothub PDO.

    This routine just keeps track of whether RootHub PDO has been started or not.
    
Arguments:

    Device - A handle to a framework device object.

    ResourcesRaw - A handle to a framework resource-list object that identifies the raw hardware
        resources that the Plug and Play manager has assigned to the device.

    ResourcesTranslated - A handle to a framework resource-list object that identifies the 
        translated hardware resources that the Plug and Play manager has assigned to the device.

*/
{
    PRHPDO_CONTEXT                    rhPdoContext;
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext;
    PUCXCONTROLLER_PRIVCONTEXT        ucxControllerContext;
    KIRQL                             irql;
    NTSTATUS                          status;
    WDFSTRING                         deviceInterfaceString = NULL;
    WDF_OBJECT_ATTRIBUTES             objectAttributes;

    UNREFERENCED_PARAMETER(ResourcesRaw);
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    TRY {

        rhPdoContext = RHPdoGetContext(Device);        
        ucxRootHubContext = rhPdoContext->UcxRootHubContext;
        ucxControllerContext = ucxRootHubContext->UcxControllerContext;
        
        WDF_OBJECT_ATTRIBUTES_INIT(&objectAttributes);
        objectAttributes.ParentObject = Device;
        
        status = WdfStringCreate(NULL, &objectAttributes, &deviceInterfaceString);

        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, "WdfStringCreate Failed for RHPdo 0x%p, %!STATUS!", 
                     (PVOID)Device, status);
            LEAVE;
        }

        status = WdfDeviceRetrieveDeviceInterfaceString(Device,
                                                        &GUID_CLASS_USBHUB,
                                                        NULL,
                                                        deviceInterfaceString);
        if (!NT_SUCCESS(status)) {
            LogError(FlagRootHub, 
                     "WdfDeviceRetrieveDeviceInterfaceString for GUID_CLASS_USBHUB Failed. RhPdo 0x%p, %!STATUS!", 
                     Device, status);
            LEAVE;
        }
        
        //
        // The controller errata flags should be initialized by now.
        // Query the ClearTtBufferOnAsyncTransferCancel flag at this
        // time.
        //
        ucxControllerContext->ClearTtBufferOnAsyncTransferCancel =
            Controller_QueryClearTtBufferOnAsyncTransferCancel(ucxControllerContext);

        KeAcquireSpinLock(&ucxRootHubContext->RHPdoInfoLock, &irql);
        ucxRootHubContext->RHPdoStarted = TRUE;
        ucxRootHubContext->RHPdoSymbolicName = deviceInterfaceString;
        deviceInterfaceString = NULL;
        KeReleaseSpinLock(&ucxRootHubContext->RHPdoInfoLock, irql);
    
        Etw_UcxRootHubPrepare(NULL, ucxRootHubContext);

        if (g_WdfDriverUcxContext->SleepStudyEnabled) {
            status = UsbSleepStudy_RegisterPdo(WdfDeviceWdmGetPhysicalDevice(ucxRootHubContext->RootHubPdo),
                                               WdfDeviceWdmGetPhysicalDevice(ucxControllerContext->ParentWdfDevice),
                                               FALSE,
                                               &ucxRootHubContext->SleepStudyHandle);
            if (!NT_SUCCESS(status)) {
                LogError(FlagRootHub, "UsbSleepStudy_RegisterPdo Failed %!STATUS!", status);
                ucxRootHubContext->SleepStudyHandle = NULL;
                LEAVE;
            }
        }

        //
        // Mimic USBPORT behavior : Reutrn PowerSystemUnspecified until the roothub sees an Sx Irp
        //
        ucxRootHubContext->LastSystemSleepState = PowerSystemUnspecified;
    
    } FINALLY {

        if (!NT_SUCCESS(status)) {
            if (deviceInterfaceString != NULL) {
                WdfObjectDelete(deviceInterfaceString);
            }
        }

    }
    return status;
}

NTSTATUS 
RootHub_Pdo_EvtReleaseHardware(
    WDFDEVICE    Device,
    WDFCMRESLIST ResourcesTranslated
   )
/*
Routine Description:

    This is a ReleaseHardware Routine for the Roothub PDO.

    This routine just keeps track of whether RootHub PDO has been started or not.
    
Arguments:

    Device - A handle to a framework device object.

    ResourcesTranslated - A handle to a framework resource-list object that identifies the 
        translated hardware resources that the Plug and Play manager has assigned to the device.

*/
{
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext;
    PRHPDO_CONTEXT                    rhPdoContext;
    KIRQL                             irql;
    WDFSTRING                         deviceInterfaceString;
    
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    TRY {
        
        rhPdoContext = RHPdoGetContext(Device);

        ucxRootHubContext = rhPdoContext->UcxRootHubContext;
        
        Etw_UcxRootHubRelease(NULL, ucxRootHubContext);

        if (ucxRootHubContext->SleepStudyHandle)
        {
            UsbSleepStudy_UnregisterPdo(ucxRootHubContext->SleepStudyHandle);
            ucxRootHubContext->SleepStudyHandle = NULL;
        }

        KeAcquireSpinLock(&ucxRootHubContext->RHPdoInfoLock, &irql);
        ucxRootHubContext->RHPdoStarted = FALSE;
        deviceInterfaceString = ucxRootHubContext->RHPdoSymbolicName;
        ucxRootHubContext->RHPdoSymbolicName = NULL;
        KeReleaseSpinLock(&ucxRootHubContext->RHPdoInfoLock, irql);
    
    } FINALLY {

        if (deviceInterfaceString != NULL) {
            WdfObjectDelete(deviceInterfaceString);
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS
RootHub_Pdo_EvtD0Entry(
    WDFDEVICE              Device,
    WDF_POWER_DEVICE_STATE PreviousState
   )
/*
Routine Description:

    This is a D0Entry Routine for the Roothub PDO.

Arguments:

    Device - A handle to a framework device object.

    PreviousState - A WDF_POWER_DEVICE_STATE-typed enumerator that 
        identifies the previous device power state.


*/
{
    PRHPDO_CONTEXT                    rhPdoContext;
    NTSTATUS                          status;
    PUCXCONTROLLER_PRIVCONTEXT        ucxControllerContext;
    PUCXROOTHUB_PRIVCONTEXT           ucxRootHubContext;
    PROOTHUB_INTERRUPT_QUEUE_CONTEXT  rootHubInterruptQueueContext;
    KIRQL                             irql;

    UNREFERENCED_PARAMETER(PreviousState);

    TRY {

        rhPdoContext = RHPdoGetContext(Device);        
        ucxRootHubContext = rhPdoContext->UcxRootHubContext;
        rootHubInterruptQueueContext = ucxRootHubContext->RootHubInterruptQueueContext;
        ucxControllerContext = rhPdoContext->UcxControllerContext;

        //
        // Drain UCX's calls to WdfDeviceIndicateWakeStatus synchronously before powering up (if
        // powering up from S0 idle)
        //
        KeAcquireSpinLock(&rootHubInterruptQueueContext->PortChangeLock, &irql);
        rootHubInterruptQueueContext->IndicateWakeStatusEnabled = FALSE;
        KeReleaseSpinLock(&rootHubInterruptQueueContext->PortChangeLock, irql);

        KeWaitForSingleObject(&rootHubInterruptQueueContext->NoPendingIndicateWakeStatus,
                              Executive, 
                              KernelMode,
                              FALSE,
                              NULL);

        Controller_EventAdd(ucxControllerContext, ControllerResetEventRHPdoEnteredD0);

        ucxControllerContext->RHPdoInD0 = TRUE;
        status = STATUS_SUCCESS;
    
        if (ucxRootHubContext->SleepStudyHandle)
        {
            UsbSleepStudy_DeviceD0Entry(ucxRootHubContext->SleepStudyHandle);
        }

    } FINALLY {

    }

    return status;

}


NTSTATUS 
RootHub_Pdo_EvtD0Exit(
    WDFDEVICE              Device,
    WDF_POWER_DEVICE_STATE TargetState
   )
/*
Routine Description:

    This is a D0Exit Routine for the Roothub PDO.

Arguments:

    Device - A handle to a framework device object.

    TargetState - A WDF_POWER_DEVICE_STATE-typed enumerator that identifies the device power state 
        that the device is about to enter.

*/
{
    PRHPDO_CONTEXT                    rhPdoContext;
    PUCXCONTROLLER_PRIVCONTEXT        ucxControllerContext;
    PROOTHUB_INTERRUPT_QUEUE_CONTEXT  rootHubInterruptQueueContext;
    POWER_ACTION                      powerAction;
    KIRQL                             irql;
    BOOLEAN                           indicateWakeStatus;
    UNREFERENCED_PARAMETER(TargetState);

    TRY {

        rhPdoContext = RHPdoGetContext(Device);        
        rootHubInterruptQueueContext =
             rhPdoContext->UcxRootHubContext->RootHubInterruptQueueContext;

        ucxControllerContext = rhPdoContext->UcxControllerContext;
        
        Controller_EventAdd(ucxControllerContext, ControllerResetEventRHPdoWantToExitD0);

        //
        // Need to ensure that resert recovery is not in progress. 
        // If it is in progress, we have to block in this thread.
        //
        KeWaitForSingleObject(&ucxControllerContext->RHPdoMayExitD0,
                              Executive, 
                              KernelMode,
                              FALSE,
                              NULL);

        //
        // The system thread shouldn't be running at this time. 
        //
        // The System thread is really for the RootHub related work. 
        // This is just a sanity check that the system thread is not really doing any work at this 
        // time. If this assert hits, it may mean that we need to ensure system thread is 
        // being paused when XHCI FDO is exitting D0
        //
        NT_ASSERT(ucxControllerContext->SystemThreadPaused);

        ucxControllerContext->RHPdoInD0 = FALSE;

        powerAction = rhPdoContext->UcxRootHubContext->SystemPowerAction;

        KeAcquireSpinLock(&rootHubInterruptQueueContext->PortChangeLock, &irql);

        if (rootHubInterruptQueueContext->WasLastTransferCancelled == FALSE) {
            //
            // If the last Roothub interrupt transfer was not cancelled by Ucx, lets increment
            // our port change generation count by 1. This is because the hub driver may have 
            // ignored the last interrupt transfer if it was going to Dx state. Thus 
            // we may need to wake the roothub up.
            // 
            rootHubInterruptQueueContext->PortChangeGenCount++;
        }

        indicateWakeStatus = FALSE;
        
        if (powerAction == PowerActionNone) {

            //
            // This is a case of S0 Idle. Thus we need to wake the RootHub in case there 
            // is a port change event
            rootHubInterruptQueueContext->IndicateWakeStatusEnabled = TRUE;

            //
            // We would also need to wake the roothub, if a port change event has happened and 
            // the hub driver does not know about it yet.
            //
            if (rootHubInterruptQueueContext->PortChangeGenCount !=
                rootHubInterruptQueueContext->PortChangeGenCountLastProcessed) {
                indicateWakeStatus = TRUE;
            }
        } 

        KeReleaseSpinLock(&rootHubInterruptQueueContext->PortChangeLock, irql);

        if (indicateWakeStatus) {
            Etw_UcxRootHubInitiatingWake(NULL, rhPdoContext->UcxRootHubContext);
            (VOID) WdfDeviceIndicateWakeStatus(Device, STATUS_SUCCESS);
        }
        
        if (ucxControllerContext->UcxRootHubContext->SleepStudyHandle)
        {
            UsbSleepStudy_DeviceD0Exit(ucxControllerContext->UcxRootHubContext->SleepStudyHandle);
        }

    } FINALLY {

    }
    return STATUS_SUCCESS;
}

NTSTATUS 
RootHub_Pdo_EvtDeviceEnableWakeAtBus (
    WDFDEVICE           Device,
    SYSTEM_POWER_STATE  PowerState
    )
/*++

Routine Description:

    Called by WDF to arm the roothub pdo for wake

    If controller reset process is going on, we fail this request    
Returns:

--*/
{
    NTSTATUS                          status;
    PUCXCONTROLLER_PRIVCONTEXT        ucxControllerContext;
    PRHPDO_CONTEXT                    rhPdoContext;

    UNREFERENCED_PARAMETER(PowerState);

    TRY {

        rhPdoContext = RHPdoGetContext(Device);        
        ucxControllerContext = rhPdoContext->UcxControllerContext;

        if (ucxControllerContext->ResetInProgress) {
            LogError(FlagRootHub, "Controller 0x%p Reset is going on. Failing RootHub_Pdo_EvtDeviceEnableWakeAtBus", 
                     ucxControllerContext);
            status = STATUS_NO_SUCH_DEVICE;
            LEAVE;
        }

        LogInfo(FlagRootHub, "Controller 0x%p RootHub_Pdo_EvtDeviceEnableWakeAtBus", 
                ucxControllerContext);

        status = STATUS_SUCCESS;
    
    } FINALLY {
    }

    return status;
}


VOID 
RootHub_Pdo_EvtDeviceDisableWakeAtBus (
    WDFDEVICE       Device
    )
/*++

Routine Description:

    Called by WDF to disarm the device for wake

Returns:


--*/
{
    UNREFERENCED_PARAMETER(Device);
}

NTSTATUS
RootHub_EvtHubParentQueryInterfaceRequest(
    WDFDEVICE            RHPdo,    
    LPGUID               InterfaceType,    
    PINTERFACE           ExposedInterface,    
    PVOID                ExposedInterfaceSpecificData
    )
/*++

Routine Description:
    
    This is the EvtDeviceProcessQueryInterfaceRequest callback for the 
    GUID_HUB_PARENT_INTERFACE interface that the controller extension
    implments.
    
Arguments:

    RHPdo - A handle representing the WDFDEVICE for the enumerated Roothub Pdo
    
    InterfaceType - Pointer to the GUID_HUB_PARENT_INTERFACE
    
    ExposedInterface - A pointer to an INTERFACE structure that describes the 
        HUB_PARENT_INTERFACE structure.
        
    ExposedInterfaceSpecificData - NOT USED
    
Comments:
    

--*/
{
    NTSTATUS                                status;
    PHUB_PARENT_INTERFACE                   hubparentInterface; 
    UCXROOTHUB                              ucxRootHub;
    PRHPDO_CONTEXT                          rhPdoContext;
    PUCXCONTROLLER_PRIVCONTEXT              ucxControllerContext;

    UNREFERENCED_PARAMETER(InterfaceType);
    UNREFERENCED_PARAMETER(ExposedInterfaceSpecificData);

    TRY {

        rhPdoContext = RHPdoGetContext(RHPdo);
        ucxRootHub = rhPdoContext->UcxRootHubContext->UcxRootHub;
        ucxControllerContext = rhPdoContext->UcxControllerContext;

        if (ExposedInterface->Size != sizeof(HUB_PARENT_INTERFACE)) {
            LogError(FlagUsbdi, "Invalid size of Interface Expected %d, Actual %d",
                     sizeof(HUB_PARENT_INTERFACE), ExposedInterface->Size);
            status = STATUS_BUFFER_TOO_SMALL;
            LEAVE;
        }

        if (ExposedInterface->Version != HUB_PARENT_INTERFACE_VERSION_LATEST) {
            LogError(FlagUsbdi, "Unsupported version Expected %d, Actual %d",
                     HUB_PARENT_INTERFACE_VERSION_LATEST, ExposedInterface->Version);
            status = STATUS_NOT_SUPPORTED;
            LEAVE;
        }

        hubparentInterface = (PHUB_PARENT_INTERFACE) ExposedInterface;

        RtlZeroMemory(hubparentInterface, sizeof(*hubparentInterface));

        hubparentInterface->Size = sizeof(*hubparentInterface);
        hubparentInterface->Version = HUB_PARENT_INTERFACE_VERSION_LATEST;
        hubparentInterface->Context = (PVOID)ucxRootHub;
        hubparentInterface->InterfaceReference = WdfDeviceInterfaceReferenceNoOp;
        hubparentInterface->InterfaceDereference = WdfDeviceInterfaceDereferenceNoOp;
        hubparentInterface->HubDepth = 0;
        //
        // We will always report the root hub as wake
        // capable as there is no harm in Selective
        // Suspending the root hub. If the controller is 
        // not really wake capable, then the controller node 
        // will not go to selective suspend but UCX will
        // take care of waking the root hub when a port
        // change arrives
        //
        hubparentInterface->IsHubWakeCapable = TRUE;
        hubparentInterface->HubTopologyAddress.PciBusNumber
            = ucxControllerContext->Config.PciDeviceInfo.BusNumber;
        hubparentInterface->HubTopologyAddress.PciDeviceNumber
            = ucxControllerContext->Config.PciDeviceInfo.DeviceNumber;
        hubparentInterface->HubTopologyAddress.PciFunctionNumber
            = ucxControllerContext->Config.PciDeviceInfo.FunctionNumber;
        hubparentInterface->Hub = UCXROOTHUB_TO_UCXUSBDEVICE(ucxRootHub);
        hubparentInterface->WasHubResetOnResume = RootHub_WasResetOnResume;
        hubparentInterface->WasDeviceProgrammingLostOnResume 
            = RootHub_WasDeviceProgrammingLostOnResume;
        hubparentInterface->GetHubSymbolicLinkName = RootHub_GetHubSymbolicLinkName;

        status = STATUS_SUCCESS;

    } FINALLY {

    }
    
    return status;
}

NTSTATUS
RootHub_EvtHubControllerStackQueryInterfaceRequest(
    WDFDEVICE            RHPdo,    
    LPGUID               InterfaceType,    
    PINTERFACE           ExposedInterface,    
    PVOID                ExposedInterfaceSpecificData
    )
/*++

Routine Description:
    
    This is the EvtDeviceProcessQueryInterfaceRequest callback for the 
    GUID_HUB_CONTROLLERSTACK_INTERFACE interface that the controller extension
    implments.
    
Arguments:

    RHPdo - A handle representing the WDFDEVICE for the enumerated Roothub Pdo
    
    InterfaceType - Pointer to the GUID_HUB_CONTROLLERSTACK_INTERFACE
    
    ExposedInterface - A pointer to an INTERFACE structure that describes the 
        HUB_CONTROLLERSTACK_INTERFACE structure.
        
    ExposedInterfaceSpecificData - NOT USED
    
Comments:
    

--*/
{
    NTSTATUS                                status;
    PHUB_CONTROLLERSTACK_INTERFACE          hubinterface; 
    UCXUSBDEVICE                            hub;
    PUCXUSBDEVICE_PRIVCONTEXT               hubContext;

    UNREFERENCED_PARAMETER(InterfaceType);
    UNREFERENCED_PARAMETER(ExposedInterfaceSpecificData);

    TRY {

        hubinterface = (PHUB_CONTROLLERSTACK_INTERFACE) ExposedInterface;


        hubinterface->Size = sizeof(HUB_CONTROLLERSTACK_INTERFACE);
        hubinterface->Version = HUB_CONTROLLERSTACK_INTERFACE_VERSION_1_0; 
        hubinterface->Context = RHPdo;
        hubinterface->InterfaceReference = WdfDeviceInterfaceReferenceNoOp;
        hubinterface->InterfaceDereference = WdfDeviceInterfaceDereferenceNoOp;

        hubinterface->UsbDeviceCreate = UsbDevice_Create;
        hubinterface->UsbDeviceDelete = UsbDevice_Delete;
        hubinterface->UsbDeviceDisconnect = UsbDevice_Disconnect;
        hubinterface->UsbDeviceSetPdoInformation = UsbDevice_SetPdoInformation;
        hubinterface->IsDeviceDisconnected = UsbDevice_IsDeviceDisconnected;
        hubinterface->DefaultEndpointCreate = DefaultEndpoint_Create;
        hubinterface->EndpointCreate = Endpoint_Create;
        hubinterface->EndpointDelete = Endpoint_Delete;
        hubinterface->EndpointExposedToClient = Endpoint_ExposedToClient;
        hubinterface->EndpointGetUsbdPipeHandle = Endpoint_GetUsbdPipeHandle;
        hubinterface->EndpointGetMaximumTransferSize = Endpoint_GetMaximumTransferSize;
        hubinterface->Address0OwnershipRelease = Controller_Address0OwnershipRelease;
        hubinterface->ForwardProgressWorkItemAllocate = Controller_ForwardProgressWorkItemAllocateFromHub;
        hubinterface->ForwardProgressWorkItemDelete = Controller_ForwardProgressWorkItemDelete;
        hubinterface->ForwardProgressWorkItemEnqueue = Controller_ForwardProgressWorkItemEnqueue;
        hubinterface->ForwardProgressWorkItemFlush = Controller_ForwardProgressWorkItemFlush;
        hubinterface->ClearTTBufferComplete = Endpoint_ClearTTBufferComplete;
        hubinterface->AreSuperspeedStreamsSupported = Controller_AreSuperspeedStreamsSupported;
        hubinterface->ControllerStopIdleSynchronous = Controller_StopIdleSynchronous;
        hubinterface->ControllerResumeIdle = Controller_ResumeIdle;
        hubinterface->ControllerGetInfo = Controller_GetInfo;


        hub = hubinterface->Hub;

        hubContext = UcxUsbDeviceGetContext(hub);

        NT_ASSERT(IsTypeHub(hubContext->UsbDeviceType));

        hubContext->HubQIContext = hubinterface->HubContext;
        hubContext->HubQICallbackClearTTBuffer = hubinterface->ClearTTBuffer;
        hubContext->HubQICallbackNoPingResponse = hubinterface->NoPingResponse;

        status = STATUS_SUCCESS;

    } FINALLY {

    }
    
    return status;
}

NTSTATUS
RootHub_Pdo_GetLocationString(
    __in
        PVOID       Context,
    __deref_out
        PZZWSTR     *LocationStrings
    )
/*++

Routine Description:

    This routine allocates, fills in, and returns a Multi-Sz string containing the bus-relative
    location identifier string for the given device.
    
    The location string for the ROOTHUB is always "USBROOT(0)" 
    
    This interface is permitted to return a Multi-Sz containing multiple strings describing the
    same device, but in this implementation only the single strings listed above will be
    returned from the interface.  The string must still be in the format of a Multi-Sz however,
    meaning a double-NULL terminator is required.

Arguments:

    Context - Unused.

    LocationsStrings - Address of a pointer to a multi-string allocated and populated by this function.

Return Value:

    NTSTATUS code.

--*/
{
    PWCHAR              stringBuffer;
    NTSTATUS            status;
    ULONG               stringBufferLen;
    
    UNREFERENCED_PARAMETER(Context);

    TRY {
    
        *LocationStrings = NULL;

        //Additional WCHAR is for extra \0 in the end
        stringBufferLen = sizeof(ROOTHUB_LOCATION) + sizeof(WCHAR);

        stringBuffer = ExAllocatePoolWithTag(PagedPool,
                                             stringBufferLen, 
           