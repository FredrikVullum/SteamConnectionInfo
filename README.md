# SteamConnectionInfo.GUI
Steam Connection Info main application and overlay. Used for displaying extra data about players you are playing with in multiplayer Steam games that use P2P.

Inspired by [SteamP2PInfo](https://github.com/tremwil/SteamP2PInfo)

**HOW TO USE**

Download the Steam Connection Info ZIP file from [releases](https://github.com/FredrikVullum/SteamConnectionInfo.GUI/releases/), extract it somewhere and run SteamConnectionInfo.exe.

**IMPORTANT:**

It currently only supports games that are using the old SteamNetworking interface for P2P. Tested and works with Elden Ring, for example.

Steam Connection Info injects a DLL into steam.exe that hooks the function [ReadP2PPacket()](https://partner.steamgames.com/doc/api/ISteamNetworking#ReadP2PPacket) which runs in the steamclient module and uses this to establish a connection between Steam ID and IP address. The DLL communicates with Steam Connection Info through shared memory so that it can receive players to put in the list on the overlay. You need this DLL for Steam Connection Info to work properly. It needs to be in the same folder as SteamConnectionInfo.exe. If Steam is updated this might also make the DLL cause Steam to crash and thus a new version of the DLL is required.

Both steam.exe and the required DLL that is injected are x86. For the DLL injection to work properly Steam Connection Info must be compiled to x86.

**USAGE/HINTS**

Press Insert to lock the window, making it transparent for all input. Press insert again to unlock so you can move the window around and interact with it.

Press Home to hide the window. Press Home again to show it.

Control the window's opacity by right clicking on the tray icon -> Opacity -> Drag the slider

**TODO**

Add country/flag for player's IP

Add support for games using newer SteamNetworkingMessages interface for P2P

Add Latency column that shows a player's latency

Add functionality to hide/show specific columns

Fix the Opacity slider background in the tray menu

Add functionality for setting your own keybindings for lock/unlock and hide/show of Window

Create a a Git repository for the DLL that is injected
