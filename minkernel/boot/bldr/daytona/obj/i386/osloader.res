        ��  ��                  p      �� ��     0 	        p4   V S _ V E R S I O N _ I N F O     ���       �    �?   	                    �   S t r i n g F i l e I n f o   �   0 4 0 9 0 4 B 0   L   C o m p a n y N a m e     M i c r o s o f t   C o r p o r a t i o n   @   F i l e D e s c r i p t i o n     B o o t   L o a d e r   `    F i l e V e r s i o n     5 . 2 . 3 7 9 0 . 0   ( s r c k i t . 2 5 0 4 1 4 - 1 8 4 5 )   :   I n t e r n a l N a m e   o s l o a d e r . e x e     � .  L e g a l C o p y r i g h t   �   M i c r o s o f t   C o r p o r a t i o n .   A l l   r i g h t s   r e s e r v e d .   B   O r i g i n a l F i l e n a m e   o s l o a d e r . e x e     j %  P r o d u c t N a m e     M i c r o s o f t �   W i n d o w s �   O p e r a t i n g   S y s t e m     :   P r o d u c t V e r s i o n   5 . 2 . 3 7 9 0 . 0     D    V a r F i l e I n f o     $    T r a n s l a t i o n     	��       �� ��     0 	           (#  @#  d   B#  H#  |  J#  n#  �  '  "'  �  �*  +  �  +  +  t  �,  �,  �  �:  �:  �     BL_MSG_FIRST
  d   Windows could not start because of an error in the software.
Please report this problem as :
 \   Windows could not start because the following file was not found
and is required :
   P   Windows could not start because of a bad copy of the 
following file :
   T   Windows could not start because the following file is missing 
or corrupt:
   T   Windows could not start because of a hardware memory 
configuration problem.
 \   Windows could not start because of a computer disk hardware
configuration problem.
   `   Windows could not start because of a general computer hardware
configuration problem.
    d   Windows could not start because of the following ARC firmware
boot configuration problem :
   @   Check hardware memory configuration and amount of RAM.
    (   Too many configuration entries.
   0   Could not access disk partition tables
    <   The 'osloadpartition' parameter setting is invalid.
   X   Could not read from the selected boot disk.  Check boot path
and disk hardware.
  <   The 'systempartition' parameter setting is invalid.
   X   Could not read from the selected system boot disk.
Check 'systempartition' path.
 H   The 'osloadfilename' parameter does not point to a valid file.
    ,   <Windows root>\system32\ntoskrnl.exe.
 @   The 'osloader' parameter does not point to a valid file.
  (   <Windows root>\system32\hal.dll.
     'osloader'\hal.dll
       Loader error 1.
      Loader error 2.
   $   load needed DLLs for kernel.
      load needed DLLs for HAL.
    find system drivers.
     read system drivers.
  0   did not load system boot device driver.
   0   load system hardware configuration file.
  0   find system partition name device name.
       find boot partition name.
 D   did not properly generate ARC name for HAL and system paths.
     Loader error 3.
   ,   <Windows root>\system32\ntoskrnl.exe
  D   Please contact your support person to report this problem.
    �   You can attempt to repair this file by starting Windows Setup
using the original Setup CD-ROM.
Select 'r' at the first screen to start repair.
  4   Please re-install a copy of the above file.
   �   Please check the Windows documentation about hardware memory
requirements and your hardware reference manuals for
additional information.
   �   Please check the Windows documentation about hardware disk
configuration and your hardware reference manuals for
additional information.
    �   Please check the Windows documentation about hardware
configuration and your hardware reference manuals for additional
information.
 �   Please check the Windows documentation about ARC configuration
options and your hardware reference manuals for additional
information.
  �       Hardware Profile/Configuration Recovery Menu

This menu allows you to select a hardware profile
to be used when Windows is started.

If your system is not starting correctly, then you may switch to a 
previous system configuration, which may overcome startup problems.
IMPORTANT: System configuration changes made since the last successful
startup will be discarded.
   l   Use the up and down arrow keys to move the highlight
to the selection you want. Then press ENTER.
    �   No hardware profiles are currently defined. Hardware profiles
can be created from the System applet of the control panel.
    |   To switch to the Last Known Good configuration, press 'L'.
To Exit this menu and restart your computer, press F3.
    t   To switch to the default configuration, press 'D'.
To Exit this menu and restart your computer, press F3.
       L
    D
 L   Seconds until highlighted choice will be started automatically: %d
    L   
Press spacebar NOW to invoke Hardware Profile/Last Known Good menu
  ,   Boot default hardware configuration
   d   The system is being restarted from its previous location.
Press the spacebar to interrupt.
   D   The system could not be restarted from its previous location
     due to no memory.
 0   because the restoration image is corrupt.
 X   because the restoration image is not compatible with the current
configuration.
      due to an internal error.
     due to an internal error.
     due to a read failure.
    (   System restart has been paused:
   @   Delete restoration data and proceed to system boot menu
   $   Continue with system restart
  l   The last attempt to restart the system from its previous location
failed.  Attempt to restart again?
 4   Continue with debug breakpoint on system wake
 x   Windows could not start because the specified kernel does 
not exist or is not compatible with this processor.
      Starting Windows...
      Resuming Windows...
   �   Windows could not start because there was an error reading
the boot settings from NVRAM.

Please check your firmware settings.  You may need to restore your
NVRAM settings from a backup.
    �   because the memory configuration has changed.  If you proceed,
your hibernated context may be lost.  To properly resume, you
should turn your machine off now, restore the original memory
configuration, then reboot the system.

        [debugger enabled]
      Windows (default)
 (   NTLDR: BOOT.INI file not found.
   <   NTLDR: no [operating systems] section in boot.txt.
    (   Booting default kernel from %s.
   4   Please select the operating system to start:
  h   

Use the up and down arrow keys to move the highlight to your choice.
Press ENTER to choose.
  H   Seconds until highlighted choice will be started automatically:
   0   Invalid BOOT.INI file
Booting from %s
    <   I/O Error accessing boot sector file
%s\BOOTSECT.DOS
 $   NTLDR: Couldn't open drive %s
 0   NTLDR: Fatal Error %d reading BOOT.INI
    0   
NTDETECT V5.2 Checking Hardware ...

      NTDETECT failed
   H   Current Selection:
  Title..: %s
  Path...: %s
  Options: %s
       Enter new load options:
       [EMS enabled]
       Invalid BOOT.INI file
 @   Windows Advanced Options Menu
Please select an option:
      Safe Mode
     Safe Mode with Networking
 (   Step-by-Step Confirmation Mode
    $   Safe Mode with Command Prompt
    VGA Mode
  H   Directory Services Restore Mode (Windows domain controllers only)
 L   Last Known Good Configuration (your most recent settings that worked)
    Debugging Mode
       Enable Boot Logging
   P   For troubleshooting and advanced startup options for Windows, press F8.
      Enable VGA Mode
   <   
Press ESCAPE to disable safeboot and boot normally.
     Start Windows Normally
        Return to OS Choices Menu
    Reboot
    <  We apologize for the inconvenience, but Windows did not start successfully.  A 
recent hardware or software change might have caused this.

If your computer stopped responding, restarted unexpectedly, or was 
automatically shut down to protect your files and folders, choose Last Known 
Good Configuration to revert to the most recent settings that worked.

If a previous startup attempt was interrupted due to a power failure or because 
the Power or Reset button was pressed, or if you aren't sure what caused the 
problem, choose Start Windows Normally.
 �   Windows was not shutdown successfully.  If this was due to the system not 
responding, or if the system shutdown to protect data, you might be able to 
recover by choosing one of the Safe Mode configurations from the menu below:
    $   Seconds until Windows starts:
    �
 纵      �
 絩   L   Windows could not start due to an error while booting from a RAMDISK.
 @   The RAMDISK options specified in boot.ini are invalid.
    8   Windows failed to build a bootable RAMDISK image.
 0   Windows failed to open the RAMDISK image.
 (   Searching for a build server...
   8   Requesting a bootable image from build server...
  @   Build Server IP = %d.%d.%d.%d. Last request timed out.  
  @   Build Server IP = %d.%d.%d.%d. Last request is pending.  
 @   Build Server IP = %d.%d.%d.%d. Last request failed.     
  (   Build Server IP = %d.%d.%d.%d.
        Loading RAMDISK image...
  (   TFTP download from %d.%d.%d.%d
    4   Multicast TFTP download from %d.%d.%d.%d:%d.
  