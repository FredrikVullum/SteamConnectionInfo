# Steam Connection Info
Steam Connection Info is used for displaying extra data about players you are playing with in multiplayer Steam games that use P2P.

Inspired by [SteamP2PInfo](https://github.com/tremwil/SteamP2PInfo)

**IMPORTANT:**

It currently only supports games that are using the old SteamNetworking interface for P2P. Tested and works with Elden Ring, for example.

SteamConnectionInfo.WPF injects the SteamConnectionInfo DLL into steam.exe and this DLL hooks the function [ReadP2PPacket()](https://partner.steamgames.com/doc/api/ISteamNetworking#ReadP2PPacket) which runs in the steamclient module and uses this to establish a connection between Steam ID and IP address. The SteamConnectionInfo DLL communicates with SteamConnectionInfo.WPF through shared memory so that it can receive players to put in the list on the overlay.

For the DLL injection to work properly SteamConnectionInfo.WPF and SteamConnectionInfo DLL must be compiled to x86. The SteamConnectionInfo DLL also needs to be in the same folder as SteamConnectionInfo.WPF.exe

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

Add functionality to change overlay style and font
