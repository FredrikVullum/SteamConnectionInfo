# SteamConnectionInfo.GUI
Steam Connection Info main application and overlay

It currently only supports games that are using the old SteamNetworking interface for P2P.

**IMPORTANT:**

Steam Connection Info injects a DLL into steam.exe that hooks ReadP2PPacket() and uses this to establish a connection between Steam ID and IP address. The DLL communicates with Steam Connection Info so that it can receive players to put in the list on the overlay. You need this DLL for Steam Connection Info to work properly. It needs to be in the same folder as SteamConnectionInfo.exe.

Both steam.exe and the required DLL that is injected are x86. For the DLL injection to work properly Steam Connection Info must be compiled to x86.

**USAGE/HINTS:**

Press Insert to lock the window, making it transparent for all input. 

Press insert again to unlock so you can move the window around and interact with it.

Press Home to hide the window. Press Home again to show it.

Control the window's opacity by right clicking on the tray icon -> Opacity -> Drag the slider

**TODO:**

Add country/flag for player's IP

Add support for games using newer SteamNetworkingMessages interface for P2P

Add Latency column that shows a player's latency

Fix the Opacity slider background in the tray menu

Add functionality for setting your own keybindings for lock/unlock and hide/show of Window
 

