  -------------------------------------------------------------------
                           Crossmatch

                         U.are.U SDK for Linux
                            Version 3.0.0
                        
                            Readme File
                           
                           May 23, 2017
                                
  --------------------------------------------------------------------
           (c) 2017 Crossmatch, Inc. All Rights Reserved. 


This document provides late-breaking or other information that supplements
the Crossmatch U.are.U SDK for Linux documentation.


-------------------------
How to Use This Document
-------------------------

This document can be viewed with a text editor such as"vi" running under Linux. 


---------
CONTENTS
---------

1.   INSTALLATION

2.   COMPATIBILITY

3.   SYSTEM REQUIREMENTS

4.   RELEASE NOTES

5.   KNOWN ISSUES
     
6.   SUPPORT AND FEEDBACK


----------------
1. INSTALLATION
----------------

Note: Installation on ARM systems should be done through extracting the ARM tarballs from the /opt/Crossmatch/redist directory as described in the "Installing on the Target Hardware" section below.

You must have root privileges to install this product.


There are two steps to the installation:

1. Installing on the development system
2. Installing on the device (the target hardware)

These steps are described below.


Installing on the Development System
------------------------------------

1- Download (or copy from a CD) the file "CM-UareU-SDK-3.0.0-1.20170520_1651.tar.gz" and place in within your /home/user directory.
2- Extract or uncompress the file contents into a temporary directory.
3- Run install as root (sudo ./install) from the created temporary directory.
4- The product will install into the system /opt directory.

Installing on the Target Hardware
---------------------------------

1- Copy the appropriate tarballs from /opt/Crossmatch/urusdk-linux/redist to the target x86, x64, ARM, or ARMHF device and unpack them.
2- Copy the /opt/Crossmatch/urusdk-linux/Linux/Samples contents to the target device. 
3- Export the LD_LIBRARY_PATH to the /opt/Crossmatch/urusdk-linux/Linux/lib directory.
4- Plug in a supported reader. 
5- Execute the samples to view exmples of the SDK features. 


-----------------
2. COMPATIBILITY
-----------------

The Crossmatch U.are.U SDK for Linux contains the 6.0.1 and 7.0.0 recognition engines, and is compatible with the Crossmatch U.are. U 4500/5100/5200/5300 series fingerprint readers and the Eikon 510/710 and Eikon2 readers. 


-----------------------
3. SYSTEM REQUIREMENTS
-----------------------

Intel based systems

- Pentium-class processor
- 128 MB RAM or the minimum amount required to run your Linux operating system
- GNU Compiler Collection (GCC), G++, GNU make and kernel-devel.
- OpenJDK and/or Oracle JDK 6/7 or higher.
- A USB 2.0 or 3.0 controller is required when using a U.are.U 5xxx fingerprint reader.

ARM based systems

ARMv4T EABI with CPU from 400MHz and at least 16Mb of available RAM.
ARMv6+VFP2 EABI with CPU from 400MHz and at least 16Mb of available RAM.

Notes
 
- The GCC version used to build the kernel on the target machine and the GCC
  version used to build the kernel module must match.

- On some distributions the kernel-devel package is insufficient since the kernel-source 
  package is required as well.



-----------------
4. RELEASE NOTES
-----------------

- New Features:
	-Added Eikon readers support to the main installation of the SDK. The Touchchip Add-on is no longer required.
	-Added support for ARMHF platform.
	
- In-lace upgrades are not supported. You should uninstall any earlier versions of the Linux SDK before installing this version.. 

- RTE installation is included in this release.


----------------
5. KNOWN ISSUES
----------------

- Aware WSQ1000 SDK on SELinux

On systems where SELinux policy is enforced, libwsq1000.so from Aware WSQ1000 SDK cannot be loaded, because it requires the executable stack. A work-around is to disable SELinux or to remove the executable stack requirement from the libwsq1000.so.

- An error may occur during Capture when the USB bus re-enumerates devices. Simply reselect the reader and continue the Capture.

- Sleep and Hibernate functionality is not stable on Linux. Sometimes readers are not responsive after the computer wakes up. In that case the reader must be closed, re-enumerated and reopened again.

- If the reader is plugged in before the installation of the SDK, udev service has to be restarted, or the reader must be disconnected and reconnected again in order to correctly set up permissions on the reader.


------------------------
6. SUPPORT AND FEEDBACK
------------------------

We provide fee-based support packages and support for registered users at our web site http://www.Crossmatch.com. 

Free technical support is available through the Crossmatch Developer Connection forums at www.Crossmatch.com/webforums.

If you have suggestions for future product releases or require technical support for the SDK, send email to techsupport@Crossmatch.com. In the subject line, type "U.are.U SDK for Linux 3.0.0 support."

You can also purchase a Developer Support package at our web store: https://store2.esellerate.net/store/checkout/CustomLayout.aspx?s=STR1045285899&pc=&page=OnePageCatalog.htm
