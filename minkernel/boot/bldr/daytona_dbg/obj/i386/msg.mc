;/*++
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;    msg.h
;
;Abstract:
;
;    This file contains the message definitions for osloader
;
;Author:
;
;    John Vert (jvert) 12-Nov-1993
;
;Revision History:
;
;Notes:
;
;    This file is generated from msg.mc
;
;--*/
;
;#ifndef _BLDR_MSG_
;#define _BLDR_MSG_
;
;

MessageID=9000 SymbolicName=BL_MSG_FIRST
Language=English
.

MessageID=9001 SymbolicName=LOAD_SW_INT_ERR_CLASS
Language=English
Windows could not start because of an error in the software.
Please report this problem as :
.

MessageID=9002 SymbolicName=LOAD_SW_MISRQD_FILE_CLASS
Language=English
Windows could not start because the following file was not found
and is required :
.

MessageID=9003 SymbolicName=LOAD_SW_BAD_FILE_CLASS
Language=English
Windows could not start because of a bad copy of the 
following file :
.

MessageID=9004 SymbolicName=LOAD_SW_MIS_FILE_CLASS
Language=English
Windows could not start because the following file is missing 
or corrupt:
.

MessageID=9005 SymbolicName=LOAD_HW_MEM_CLASS
Language=English
Windows could not start because of a hardware memory 
configuration problem.
.

MessageID=9006 SymbolicName=LOAD_HW_DISK_CLASS
Language=English
Windows could not start because of a computer disk hardware
configuration problem.
.

MessageID=9007 SymbolicName=LOAD_HW_GEN_ERR_CLASS
Language=English
Windows could not start because of a general computer hardware
configuration problem.
.

MessageID=9008 SymbolicName=LOAD_HW_FW_CFG_CLASS
Language=English
Windows could not start because of the following ARC firmware
boot configuration problem :
.

MessageID=9009 SymbolicName=DIAG_BL_MEMORY_INIT
Language=English
Check hardware memory configuration and amount of RAM.
.

MessageID=9010 SymbolicName=DIAG_BL_CONFIG_INIT
Language=English
Too many configuration entries.
.

MessageID=9011 SymbolicName=DIAG_BL_IO_INIT
Language=English
Could not access disk partition tables
.

MessageID=9012 SymbolicName=DIAG_BL_FW_GET_BOOT_DEVICE
Language=English
The 'osloadpartition' parameter setting is invalid.
.

MessageID=9013 SymbolicName=DIAG_BL_OPEN_BOOT_DEVICE
Language=English
Could not read from the selected boot disk.  Check boot path
and disk hardware.
.

MessageID=9014 SymbolicName=DIAG_BL_FW_GET_SYSTEM_DEVICE
Language=English
The 'systempartition' parameter setting is invalid.
.

MessageID=9015 SymbolicName=DIAG_BL_FW_OPEN_SYSTEM_DEVICE
Language=English
Could not read from the selected system boot disk.
Check 'systempartition' path.
.

MessageID=9016 SymbolicName=DIAG_BL_GET_SYSTEM_PATH
Language=English
The 'osloadfilename' parameter does not point to a valid file.
.

MessageID=9017 SymbolicName=DIAG_BL_LOAD_SYSTEM_IMAGE
Language=English
<Windows root>\system32\ntoskrnl.exe.
.

MessageID=9018 SymbolicName=DIAG_BL_FIND_HAL_IMAGE
Language=English
The 'osloader' parameter does not point to a valid file.
.

MessageID=9019 SymbolicName=DIAG_BL_LOAD_HAL_IMAGE_X86
Language=English
<Windows root>\system32\hal.dll.
.

MessageID=9020 SymbolicName=DIAG_BL_LOAD_HAL_IMAGE_ARC
Language=English
'osloader'\hal.dll
.
;#ifdef _X86_
;#define DIAG_BL_LOAD_HAL_IMAGE DIAG_BL_LOAD_HAL_IMAGE_X86
;#else
;#define DIAG_BL_LOAD_HAL_IMAGE DIAG_BL_LOAD_HAL_IMAGE_ARC
;#endif

MessageID=9021 SymbolicName=DIAG_BL_LOAD_SYSTEM_IMAGE_DATA
Language=English
Loader error 1.
.

MessageID=9022 SymbolicName=DIAG_BL_LOAD_HAL_IMAGE_DATA
Language=English
Loader error 2.
.

MessageID=9023 SymbolicName=DIAG_BL_LOAD_SYSTEM_DLLS
Language=English
load needed DLLs for kernel.
.

MessageID=9024 SymbolicName=DIAG_BL_LOAD_HAL_DLLS
Language=English
load needed DLLs for HAL.
.

MessageID=9026 SymbolicName=DIAG_BL_FIND_SYSTEM_DRIVERS
Language=English
find system drivers.
.

MessageID=9027 SymbolicName=DIAG_BL_READ_SYSTEM_DRIVERS
Language=English
read system drivers.
.

MessageID=9028 SymbolicName=DIAG_BL_LOAD_DEVICE_DRIVER
Language=English
did not load system boot device driver.
.

MessageID=9029 SymbolicName=DIAG_BL_LOAD_SYSTEM_HIVE
Language=English
load system hardware configuration file.
.

MessageID=9030 SymbolicName=DIAG_BL_SYSTEM_PART_DEV_NAME
Language=English
find system partition name device name.
.

MessageID=9031 SymbolicName=DIAG_BL_BOOT_PART_DEV_NAME
Language=English
find boot partition name.
.

MessageID=9032 SymbolicName=DIAG_BL_ARC_BOOT_DEV_NAME
Language=English
did not properly generate ARC name for HAL and system paths.
.

MessageID=9034 SymbolicName=DIAG_BL_SETUP_FOR_NT
Language=English
Loader error 3.
.

MessageID=9035 SymbolicName=DIAG_BL_KERNEL_INIT_XFER
Language=English
<Windows root>\system32\ntoskrnl.exe
.

MessageID=9036 SymbolicName=LOAD_SW_INT_ERR_ACT
Language=English
Please contact your support person to report this problem.
.

MessageID=9037 SymbolicName=LOAD_SW_FILE_REST_ACT
Language=English
You can attempt to repair this file by starting Windows Setup
using the original Setup CD-ROM.
Select 'r' at the first screen to start repair.
.

MessageID=9038 SymbolicName=LOAD_SW_FILE_REINST_ACT
Language=English
Please re-install a copy of the above file.
.

MessageID=9039 SymbolicName=LOAD_HW_MEM_ACT
Language=English
Please check the Windows documentation about hardware memory
requirements and your hardware reference manuals for
additional information.
.

MessageID=9040 SymbolicName=LOAD_HW_DISK_ACT
Language=English
Please check the Windows documentation about hardware disk
configuration and your hardware reference manuals for
additional information.
.

MessageID=9041 SymbolicName=LOAD_HW_GEN_ACT
Language=English
Please check the Windows documentation about hardware
configuration and your hardware reference manuals for additional
information.
.

MessageID=9042 SymbolicName=LOAD_HW_FW_CFG_ACT
Language=English
Please check the Windows documentation about ARC configuration
options and your hardware reference manuals for additional
information.
.

MessageID=9043 SymbolicName=BL_LKG_MENU_HEADER
Language=English
     Hardware Profile/Configuration Recovery Menu

This menu allows you to select a hardware profile
to be used when Windows is started.

If your system is not starting correctly, then you may switch to a 
previous system configuration, which may overcome startup problems.
IMPORTANT: System configuration changes made since the last successful
startup will be discarded.
.

MessageID=9044 SymbolicName=BL_LKG_MENU_TRAILER
Language=English
Use the up and down arrow keys to move the highlight
to the selection you want. Then press ENTER.
.

MessageID=9045 SymbolicName=BL_LKG_MENU_TRAILER_NO_PROFILES
Language=English
No hardware profiles are currently defined. Hardware profiles
can be created from the System applet of the control panel.
.

MessageID=9046 SymbolicName=BL_SWITCH_LKG_TRAILER
Language=English
To switch to the Last Known Good configuration, press 'L'.
To Exit this menu and restart your computer, press F3.
.

MessageID=9047 SymbolicName=BL_SWITCH_DEFAULT_TRAILER
Language=English
To switch to the default configuration, press 'D'.
To Exit this menu and restart your computer, press F3.
.

;//
;// The following two messages are used to provide the mnemonic keypress
;// that toggles between the Last Known Good control set and the default
;// control set. (see BL_SWITCH_LKG_TRAILER and BL_SWITCH_DEFAULT_TRAILER)
;//
MessageID=9048 SymbolicName=BL_LKG_SELECT_MNEMONIC
Language=English
L
.

MessageID=9049 SymbolicName=BL_DEFAULT_SELECT_MNEMONIC
Language=English
D
.

MessageID=9050 SymbolicName=BL_LKG_TIMEOUT
Language=English
Seconds until highlighted choice will be started automatically: %d
.

MessageID=9051 SymbolicName=BL_LKG_MENU_PROMPT
Language=English

Press spacebar NOW to invoke Hardware Profile/Last Known Good menu
.

MessageID=9052 SymbolicName=BL_BOOT_DEFAULT_PROMPT
Language=English
Boot default hardware configuration
.

;//
;// The following messages are used during hibernation restoration
;//
MessageID=9053 SymbolicName=HIBER_BEING_RESTARTED
Language=English
The system is being restarted from its previous location.
Press the spacebar to interrupt.
.
MessageID=9054 SymbolicName=HIBER_ERROR
Language=English
The system could not be restarted from its previous location
.
MessageID=9055 SymbolicName=HIBER_ERROR_NO_MEMORY
Language=English
due to no memory.
.
MessageID=9056 SymbolicName=HIBER_ERROR_BAD_IMAGE
Language=English
because the restoration image is corrupt.
.
MessageID=9057 SymbolicName=HIBER_IMAGE_INCOMPATIBLE
Language=English
because the restoration image is not compatible with the current
configuration.
.
MessageID=9058 SymbolicName=HIBER_ERROR_OUT_OF_REMAP
Language=English
due to an internal error.
.
MessageID=9059 SymbolicName=HIBER_INTERNAL_ERROR
Language=English
due to an internal error.
.
MessageID=9060 SymbolicName=HIBER_READ_ERROR
Language=English
due to a read failure.
.
MessageID=9061 SymbolicName=HIBER_PAUSE
Language=English
System restart has been paused:
.
MessageID=9062 SymbolicName=HIBER_CANCEL
Language=English
Delete restoration data and proceed to system boot menu
.
MessageID=9063 SymbolicName=HIBER_CONTINUE
Language=English
Continue with system restart
.
MessageID=9064 SymbolicName=HIBER_RESTART_AGAIN
Language=English
The last attempt to restart the system from its previous location
failed.  Attempt to restart again?
.
MessageID=9065 SymbolicName=HIBER_DEBUG_BREAK_ON_WAKE
Language=English
Continue with debug breakpoint on system wake
.
MessageID=9066 SymbolicName=LOAD_SW_MISMATCHED_KERNEL
Language=English
Windows could not start because the specified kernel does 
not exist or is not compatible with this processor.
.
MessageID=9067 SymbolicName=BL_MSG_STARTING_WINDOWS
Language=English
Starting Windows...
.
MessageID=9068 SymbolicName=BL_MSG_RESUMING_WINDOWS
Language=English
Resuming Windows...
.

MessageID=9069 SymbolicName=BL_EFI_OPTION_FAILURE
Language=English
Windows could not start because there was an error reading
the boot settings from NVRAM.

Please check your firmware settings.  You may need to restore your
NVRAM settings from a backup.
.
MessageID=9070 SymbolicName=HIBER_MEMORY_INCOMPATIBLE
Language=English
because the memory configuration has changed.  If you proceed,
your hibernated context may be lost.  To properly resume, you
should turn your machine off now, restore the original memory
configuration, then reboot the system.

.

;
; //
; // Following messages are for the x86-specific
; // boot menu.
; //
;
MessageID=10001 SymbolicName=BL_ENABLED_KD_TITLE
Language=English
 [debugger enabled]
.

MessageID=10002 SymbolicName=BL_DEFAULT_TITLE
Language=English
Windows (default)
.

MessageID=10003 SymbolicName=BL_MISSING_BOOT_INI
Language=English
NTLDR: BOOT.INI file not found.
.

MessageID=10004 SymbolicName=BL_MISSING_OS_SECTION
Language=English
NTLDR: no [operating systems] section in boot.txt.
.

MessageID=10005 SymbolicName=BL_BOOTING_DEFAULT
Language=English
Booting default kernel from %s.
.

MessageID=10006 SymbolicName=BL_SELECT_OS
Language=English
Please select the operating system to start:
.

MessageID=10007 SymbolicName=BL_MOVE_HIGHLIGHT
Language=English


Use the up and down arrow keys to move the highlight to your choice.
Press ENTER to choose.
.

MessageID=10008 SymbolicName=BL_TIMEOUT_COUNTDOWN
Language=English
Seconds until highlighted choice will be started automatically:
.

MessageID=10009 SymbolicName=BL_INVALID_BOOT_INI
Language=English
Invalid BOOT.INI file
Booting from %s
.

MessageID=10010 SymbolicName=BL_REBOOT_IO_ERROR
Language=English
I/O Error accessing boot sector file
%s\BOOTSECT.DOS
.

MessageID=10011 SymbolicName=BL_DRIVE_ERROR
Language=English
NTLDR: Couldn't open drive %s
.

MessageID=10012 SymbolicName=BL_READ_ERROR
Language=English
NTLDR: Fatal Error %d reading BOOT.INI
.

MessageID=10013 SymbolicName=BL_NTDETECT_MSG
Language=English

NTDETECT V5.2 Checking Hardware ...

.

MessageID=10014 SymbolicName=BL_NTDETECT_FAILURE
Language=English
NTDETECT failed
.

MessageID=10015 SymbolicName=BL_DEBUG_SELECT_OS
Language=English
Current Selection:
  Title..: %s
  Path...: %s
  Options: %s
.

MessageID=10016 SymbolicName=BL_DEBUG_NEW_OPTIONS
Language=English
Enter new load options:
.

MessageID=10017 SymbolicName=BL_HEADLESS_REDIRECT_TITLE
Language=English
 [EMS enabled]
.

MessageID=10018 SymbolicName=BL_INVALID_BOOT_INI_FATAL
Language=English
Invalid BOOT.INI file
.


;
; //
; // Following messages are for the advanced boot menu
; //
;

MessageID=11001 SymbolicName=BL_ADVANCEDBOOT_TITLE
Language=English
Windows Advanced Options Menu
Please select an option:
.

MessageID=11002 SymbolicName=BL_SAFEBOOT_OPTION1
Language=English
Safe Mode
.

MessageID=11003 SymbolicName=BL_SAFEBOOT_OPTION2
Language=English
Safe Mode with Networking
.

MessageID=11004 SymbolicName=BL_SAFEBOOT_OPTION3
Language=English
Step-by-Step Confirmation Mode
.

MessageID=11005 SymbolicName=BL_SAFEBOOT_OPTION4
Language=English
Safe Mode with Command Prompt
.

MessageID=11006 SymbolicName=BL_SAFEBOOT_OPTION5
Language=English
VGA Mode
.

MessageID=11007 SymbolicName=BL_SAFEBOOT_OPTION6
Language=English
Directory Services Restore Mode (Windows domain controllers only)
.

MessageID=11008 SymbolicName=BL_LASTKNOWNGOOD_OPTION
Language=English
Last Known Good Configuration (your most recent settings that worked)
.

MessageID=11009 SymbolicName=BL_DEBUG_OPTION
Language=English
Debugging Mode
.

;#if defined(REMOTE_BOOT)
;MessageID=11010 SymbolicName=BL_REMOTEBOOT_OPTION1
;Language=English
;Remote Boot Maintenance and Troubleshooting
;.
;
;MessageID=11011 SymbolicName=BL_REMOTEBOOT_OPTION2
;Language=English
;Clear Offline Cache
;.
;
;MessageID=11012 SymbolicName=BL_REMOTEBOOT_OPTION3
;Language=English
;Load using files from server only
;.
;#endif // defined(REMOTE_BOOT)

MessageID=11013 SymbolicName=BL_BOOTLOG
Language=English
Enable Boot Logging
.

MessageID=11014 SymbolicName=BL_ADVANCED_BOOT_MESSAGE
Language=English
For troubleshooting and advanced startup options for Windows, press F8.
.

MessageID=11015 SymbolicName=BL_BASEVIDEO
Language=English
Enable VGA Mode
.

MessageID=11016 SymbolicName=BL_DISABLE_SAFEBOOT
Language=English

Press ESCAPE to disable safeboot and boot normally.
.

MessageID=11017 SymbolicName=BL_MSG_BOOT_NORMALLY
Language=English
Start Windows Normally
.
MessageID=11018 SymbolicName=BL_MSG_OSCHOICES_MENU
Language=English
Return to OS Choices Menu
.

MessageID=11019 SymbolicName=BL_MSG_REBOOT
Language=English
Reboot
.

MessageID=11020 SymbolicName=BL_ADVANCEDBOOT_AUTOLKG_TITLE
Language=English
We apologize for the inconvenience, but Windows did not start successfully.  A 
recent hardware or software change might have caused this.

If your computer stopped responding, restarted unexpectedly, or was 
automatically shut down to protect your files and folders, choose Last Known 
Good Configuration to revert to the most recent settings that worked.

If a previous startup attempt was interrupted due to a power failure or because 
the Power or Reset button was pressed, or if you aren't sure what caused the 
problem, choose Start Windows Normally.
.

MessageID=11021 SymbolicName=BL_ADVANCEDBOOT_AUTOSAFE_TITLE
Language=English
Windows was not shutdown successfully.  If this was due to the system not 
responding, or if the system shutdown to protect data, you might be able to 
recover by choosing one of the Safe Mode configurations from the menu below:
.

MessageID=11022 SymbolicName=BL_ADVANCEDBOOT_TIMEOUT
Language=English
Seconds until Windows starts:
.

;
;//
;// Following messages are the symbols used
;// in the Hibernation Restore percent completed UI. 
;//
;// These symbols are here because they employ high-ASCII
;// line drawing characters, which need to be localized
;// for fonts that may not have such characters (or have
;// them in a different ASCII location). 
;//  
;// This primarily affects FE character sets. 
;//
;// Note that only the FIRST character of any of these
;// messages is ever looked at by the code.  
;//
;
; // Just a base message, contains nothing.
;
;

; //
; // NOTE : donot change the Message ID values for HIBER_UI_BAR_ELEMENT
; // and BLDR_UI_BAR_BACKGROUND from 11513 & 11514
;

;
; // The character used to draw the percent-complete bar
;
;
MessageID=11513 SymbolicName=HIBER_UI_BAR_ELEMENT
Language=English
�
.

;
; // The character used to draw the percent-complete bar
;
;
MessageID=11514 SymbolicName=BLDR_UI_BAR_BACKGROUND
Language=English
�
.

;#if defined(REMOTE_BOOT)
;;
;; //
;; // Following messages are for warning the user about
;; // an impending autoformat of the hard disk.
;; //
;;
;
;MessageID=12002 SymbolicName=BL_WARNFORMAT_TRAILER
;Language=English
;Press ENTER to continue booting Windows, otherwise turn off
;your remote boot machine.
;.
;
;MessageID=12003 SymbolicName=BL_WARNFORMAT_CONTINUE
;Language=English
;Continue
;.
;MessageID=12004 SymbolicName=BL_FORCELOGON_HEADER
;Language=English
;          Auto-Format
;
;Windows has detected a new hard disk in your remote boot
;machine.  Before Windows can use this disk, you must logon and
;validate that you have access to this disk.
;
;WARNING:  Windows will automatically repartition and format
;this disk to accept the new operating system.  All data on the
;hard disk will be lost if you choose to continue.  If you do
;not want to continue, power off the computer now and contact
;your administrator.
;.
;#endif // defined(REMOTE_BOOT)

;
; //
; // Ramdisk related messages. DO NOT CHANGE the message numbers
; // as they are kept in sync with \base\boot\inc\ramdisk.h.
; //
;

MessageID=15000 SymbolicName=BL_RAMDISK_GENERAL_FAILURE
Language=English
Windows could not start due to an error while booting from a RAMDISK.
.

MessageID=15001 SymbolicName=BL_RAMDISK_INVALID_OPTIONS
Language=English
The RAMDISK options specified in boot.ini are invalid.
.

MessageID=15002 SymbolicName=BL_RAMDISK_BUILD_FAILURE
Language=English
Windows failed to build a bootable RAMDISK image.
.

MessageID=15003 SymbolicName=BL_RAMDISK_BOOT_FAILURE
Language=English
Windows failed to open the RAMDISK image.
.

MessageID=15004 SymbolicName=BL_RAMDISK_BUILD_DISCOVER
Language=English
Searching for a build server...
.

MessageID=15005 SymbolicName=BL_RAMDISK_BUILD_REQUEST
Language=English
Requesting a bootable image from build server...
.

MessageID=15006 SymbolicName=BL_RAMDISK_BUILD_PROGRESS_TIMEOUT
Language=English
Build Server IP = %d.%d.%d.%d. Last request timed out.  
.

MessageID=15007 SymbolicName=BL_RAMDISK_BUILD_PROGRESS_PENDING
Language=English
Build Server IP = %d.%d.%d.%d. Last request is pending.  
.

MessageID=15008 SymbolicName=BL_RAMDISK_BUILD_PROGRESS_ERROR
Language=English
Build Server IP = %d.%d.%d.%d. Last request failed.     
.

MessageID=15009 SymbolicName=BL_RAMDISK_BUILD_PROGRESS
Language=English
Build Server IP = %d.%d.%d.%d.
.

MessageID=15010 SymbolicName=BL_RAMDISK_DOWNLOAD
Language=English
Loading RAMDISK image...
.

MessageID=15011 SymbolicName=BL_RAMDISK_DOWNLOAD_NETWORK
Language=English
TFTP download from %d.%d.%d.%d
.

MessageID=15012 SymbolicName=BL_RAMDISK_DOWNLOAD_NETWORK_MCAST
Language=English
Multicast TFTP download from %d.%d.%d.%d:%d.
.

;#endif // _BLDR_MSG_

