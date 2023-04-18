# 32 bit Matchsticks


This is a 32 bit port of the first Windows application that I ever wrote. For the 16 bit version see [here](https://github.com/rachbowyer/matchsticks). For the full story click [here](https://www.bowyer.info/post/the-first-ever-windows-app-that-i-wrote)


## How to play

Matchsticks is a version of [Nim](https://en.wikipedia.org/wiki/Nim). This is a port to Windows NT 3.5. It should work on other 32 bit versions of Windows. Then again it might not.


Use the mouse to select the matches you wish to remove and click the selected area to play your move. 



## How to build

Being strapped for cash, I wrote it using Turbo C++ for Windows 4.0 running on Windows 3.1 with Win32s. When I came to build it again 30 years later, I was unable to get this combination to work in an emulator. So I instead used Borland C++ 4.5 running on Windows NT 3.5. The dialog boxes originally had the Borland grey chiselled look controls but they didn't work out of the box on this platform. Rather than debug the issue, I replaced the controls in the resources file with the standard Windows controls. The code though is the same as it was 30 years ago when I originally wrote it.
 


## Licence

Copyright © Rachel Bowyer 1987, 1993. All rights reserved.
