# SteamConnectionInfo

Inspired by [SteamP2PInfo](https://github.com/tremwil/SteamP2PInfo)

![Example Image](29.04.2023-2.png)

**IMPORTANT**

Steam Connection Info only works with games that utilize the older [SteamNetworking](https://partner.steamgames.com/doc/api/ISteamNetworking) API.

I plan on adding support for the newer [SteamNetworkingMessages](https://partner.steamgames.com/doc/api/ISteamNetworkingMessages) API in the future.

**HOW TO DOWNLOAD**

You have two options:

1. Clone the repository and build it yourself. Keep in mind that only x86 Release mode is supported. Publish it and copy the files at \bin\x86\Release\net6.0-windows\publish\win-x86 to a folder of your choosing. Run SteamConnectionInfo.WPF.exe

2. Download the SCI.zip from [Releases](https://github.com/FredrikVullum/SteamConnectionInfo/releases) and extract the contents to a folder of your choosing. Run SteamConnectionInfo.WPF.exe

For SteamConnectionInfo to work SteamConnectionInfo.dll must be in the same folder as SteamConnectionInfo.WPF.exe.

**HINTS**

Press Insert to lock the window, making it transparent for all input. Press insert again to unlock so you can move the window around and interact with it.

Press Home to hide the window. Press Home again to show it.

Right click on the tray icon to access other functionality like hiding columns, changing the window opacity, etc.

**HOW DOES IT WORK?**

SteamConnectionInfo.WPF injects the DLL from SteamConnectionInfo.DLL into steam.exe and this DLL hooks the function [ReadP2PPacket()](https://partner.steamgames.com/doc/api/ISteamNetworking#ReadP2PPacket) which is found in the steamclient module and uses this to establish a connection between Steam ID and IP address. DLL from SteamConnectionInfo.DLL communicates with SteamConnectionInfo.WPF through shared memory so that SteamConnectionInfo.WPF can receive data about the players and show it to you in the overlay.

**SOLUTION STRUCTURE**

SteamConnectionInfo.DLL is a C++ project that builds the DLL.

SteamConnectionInfo.Core is a C# class library that contains functions necessary for SteamConnectionInfo.WPF

SteamConnectionInfo.WPF is a C# project that contains the main executable which shows the overlay, interacts with the DLL, etc.

**TODO**

Add support for games using newer SteamNetworkingMessages interface for P2P

Fix the Opacity slider background in the tray menu

Add functionality to set your own keybindings for locking/unlocking and hiding/showing the overlay

Add functionality to change the overlay style and font

Add functionality to close P2P session based on player latency and/or country?
