# SteamConnectionInfo

Inspired by [SteamP2PInfo](https://github.com/tremwil/SteamP2PInfo)

![Example Image](29.04.2023-2.png)

**IMPORTANT**

SteamConnectionInfo only works with games that utilize the older [SteamNetworking](https://partner.steamgames.com/doc/api/ISteamNetworking) API.

**HOW TO DOWNLOAD**

You have two options:

1. Download SteamConnectionInfo.zip from [Releases](https://github.com/FredrikVullum/SteamConnectionInfo/releases) and extract the contents to a folder of your choosing.

2. Clone the repository and build it yourself. Keep in mind that only x86 Release mode is supported. Publish it and copy the files at \bin\x86\Release\net6.0-windows\publish\win-x86 to a folder of your choosing.

**HOW TO RUN**

1. Set your game to windowed mode. SteamConnectionInfo will not work if the game is in fullscreen mode.
2. Make sure SteamConnectionInfo.WPF.exe is in the same folder as SteamConnectionInfo.dll
3. Run SteamConnectionInfo.WPF.exe

If you can't start SteamConnectionInfo.WPF.exe at all, the problem is most likely that you're missing the .NET runtime.

If you can start SteamConnectionInfo.WPF.exe but the overlay says "UNLOADED" and causes Steam to crash every time you launch it, the problem is most likely that the byte patterns that SteamConnectionInfo.dll uses to find important addresses in steamclient.dll are outdated. To fix this you either have to wait for me to update the DLL or you have to find the pattern yourself with a tool like IDA Pro and compile SteamConnectionInfo.dll yourself.

If you can start SteamConnectionInfo.WPF.exe but the overlay says "UNLOADED" while steam is still running, the problem is most likely that the DLL injection is failing. To fix this you could try adding an AV exclusion to the folder where SteamConnectionInfo.WPF.exe and SteamConnectionInfo.dll are located.

**HOW TO USE**

Right click the tray icon on your taskbar to access functionality like hiding columns, changing the window opacity, etc.

Press Insert to lock the window, making it transparent for all input. Press insert again to unlock so you can move the window around and interact with it.

Press Home to hide the window. Press Home again to show it.

Right click the tray icon on your taskbar and click on EXIT to exit SteamConnectionInfo.

**HOW DOES IT WORK?**

SteamConnectionInfo.WPF.exe injects SteamConnectionInfo.dll into steam.exe, this DLL hooks the function [ReadP2PPacket()](https://partner.steamgames.com/doc/api/ISteamNetworking#ReadP2PPacket) which is found in steamclient.dll. It uses this hook to find important information about the players you are playing with like their Steam ID and their IP address. SteamConnectionInfo.dll communicates with SteamConnectionInfo.WPF.exe through shared memory so that the data gathered from SteamConnectionInfo.dll can be used and displayed by SteamConnectionInfo.WPF.exe

SteamConnectionInfo.dll uses npcap to monitor packets to measure pings, curl for accessing ip-api.com for resolving IP addresses to country, and nlohmann for serializing players to JSON in C++.

**SOLUTION STRUCTURE**

SteamConnectionInfo.DLL is a C++ project that builds SteamConnectionInfo.dll

SteamConnectionInfo.Core is a C# class library that contains core functions for SteamConnectionInfo.WPF

SteamConnectionInfo.WPF is the C# project that builds SteamConnectionInfo.WPF.exe

**TODO**

Add support for games using newer SteamNetworkingMessages interface for P2P

Fix the Opacity slider background in the tray menu

Add functionality to set your own keybindings for locking/unlocking and hiding/showing the overlay

Add functionality to change the overlay style and font

Add functionality to close P2P session based on player latency and/or country?
