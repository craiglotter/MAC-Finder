MAC Finder 

MAC Finder is a simple script utility written for Commerce I.T. Its purpose is to control the copy of the correct The ISYS.ini files for the INET program found in the Commerce computer labs from the central file server to the local machine. 

There is a central folder that contains a number of folders based on computers MAC addresses stored on the main Commerce file server. Each PCs MAC address thus corresponds to one such folder. Each folder contains an ISYS.ini configuration file. What makes these files different is the unique IUser value found in each one. The program therefore ensures that the correct file is copied to the local machine by creating the source directory path from the local machine's IP address and then copying the required file.

Usage

Usage: MAC_Finder <source folder> <destination folder> <file to copy>
Note: Folders must not have a '\' character following them. (e.g. C:\Windows)

Example

Command: CommandMAC_Finder r:\isys c:\inet\config isys.ini
Result: 
r:\isys\00cf1e05.fd3\isys.ini copied to c:\inet\config\isys.ini if local machine's MAC address is 00-0C-F1-E0-5F-D3



Created by Craig Lotter, January 2003
