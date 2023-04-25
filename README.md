# Steam Connection Info
Steam Connection Info is solution for Windows made for displaying extra data about players you are playing with in multiplayer Steam games that use P2P.

Inspired by [SteamP2PInfo](https://github.com/tremwil/SteamP2PInfo)

**IMPORTANT:**

Steam Connection Info currently only supports games that are using the old SteamNetworking interface for P2P. Tested and works with Elden Ring, for example.

DLL from SteamConnectionInfo.DLL is automatically injected into steam.exe. For the DLL injection to work properly SteamConnectionInfo.WPF and DLL from SteamConnectionInfo.DLL must be compiled to x86. DLL from SteamConnectionInfo.DLL also needs to be in the same folder as SteamConnectionInfo.WPF.exe

**USAGE/HINTS**

Press Insert to lock the window, making it transparent for all input. Press insert again to unlock so you can move the window around and interact with it.

Press Home to hide the window. Press Home again to show it.

Control the window's opacity by right clicking on the tray icon -> Opacity -> Drag the slider

**HOW DOES IT WORK?**

SteamConnectionInfo.WPF injects the DLL from SteamConnectionInfo.DLL into steam.exe and this DLL hooks the function [ReadP2PPacket()](https://partner.steamgames.com/doc/api/ISteamNetworking#ReadP2PPacket) which is found in the steamclient module and uses this to establish a connection between Steam ID and IP address. DLL from SteamConnectionInfo.DLL communicates with SteamConnectionInfo.WPF through shared memory so that SteamConnectionInfo.WPF can receive data about the players and show it to you in the overlay.

**STRUCTURE**

SteamConnectionInfo.DLL is a C++ project that builds the DLL.

SteamConnectionInfo.Core is a C# class library that contains functions necessary for SteamConnectionInfo.WPF

SteamConnectionInfo.WPF is a C# project that contains the main executable which shows the overlay, interacts with the dll, etc.

**TODO**

Add Latency column that shows a player's latency in the overlay

Add support for games using newer SteamNetworkingMessages interface for P2P

Fix the Opacity slider background in the tray menu

Add functionality to set your own keybindings for locking/unlocking and hiding/showing the overlay

Add functionality to change the overlay style and font

Add functionality to close P2P session based on player latency and/or country?

Add functionality to save all players you have played with in a file
