
ModemCheck - The modem analyzator ! :)
======================================

This tool analyzes your modem and creates a file called
RAM:ModemInfo. It contains all the collected information
about your modem. To help to improove Genesis, please
send this file to dolphin@zool.unizh.ch with the
subject 'Modem Check'.

The program will ask you for the exact name/type of your
modem. Please enter the manufacturer and the type
(something like "ZyXEL U-1496E plus").

Usage:

modemcheck [device unit]

Without any argument it will open the standard serial.device
on unit 0. To use a different device simply use this syntax:

modemcheck myser.device 1


Michael

