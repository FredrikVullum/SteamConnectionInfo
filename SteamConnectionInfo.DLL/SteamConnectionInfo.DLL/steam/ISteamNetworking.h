#pragma once

#include "CSteamID.h"

enum ESteamIPType
{
	k_ESteamIPTypeIPv4 = 0,
	k_ESteamIPTypeIPv6 = 1,
};

enum class EP2PSend
{
	// Basic UDP send. Packets can't be bigger than 1200 bytes (your typical MTU size). Can be lost, or arrive out of order (rare).
		// The sending API does have some knowledge of the underlying connection, so if there is no NAT-traversal accomplished or
		// there is a recognized adjustment happening on the connection, the packet will be batched until the connection is open again.
	k_EP2PSendUnreliable = 0,

	// As above, but if the underlying p2p connection isn't yet established the packet will just be thrown away. Using this on the first
	// packet sent to a remote host almost guarantees the packet will be dropped.
	// This is only really useful for kinds of data that should never buffer up, i.e. voice payload packets
	k_EP2PSendUnreliableNoDelay = 1,

	// Reliable message send. Can send up to 1MB of data in a single message. 
	// Does fragmentation/re-assembly of messages under the hood, as well as a sliding window for efficient sends of large chunks of data. 
	k_EP2PSendReliable = 2,

	// As above, but applies the Nagle algorithm to the send - sends will accumulate 
	// until the current MTU size (typically ~1200 bytes, but can change) or ~200ms has passed (Nagle algorithm). 
	// Useful if you want to send a set of smaller messages but have the coalesced into a single packet
	// Since the reliable stream is all ordered, you can do several small message sends with k_EP2PSendReliableWithBuffering and then
	// do a normal k_EP2PSendReliable to force all the buffered data to be sent.
	k_EP2PSendReliableWithBuffering = 3

};


struct SteamIPAddress_t
{
	union {

		uint32			m_unIPv4;		// Host order
		uint8			m_rgubIPv6[16];		// Network order! Same as inaddr_in6.  (0011:2233:4455:6677:8899:aabb:ccdd:eeff)

		// Internal use only
		uint64			m_ipv6Qword[2];	// big endian
	};

	ESteamIPType m_eType;

	bool IsSet() const
	{
		if (k_ESteamIPTypeIPv4 == m_eType)
		{
			return m_unIPv4 != 0;
		}
		else
		{
			return m_ipv6Qword[0] != 0 || m_ipv6Qword[1] != 0;
		}
	}

	static SteamIPAddress_t IPv4Any()
	{
		SteamIPAddress_t ipOut;
		ipOut.m_eType = k_ESteamIPTypeIPv4;
		ipOut.m_unIPv4 = 0;

		return ipOut;
	}

	static SteamIPAddress_t IPv6Any()
	{
		SteamIPAddress_t ipOut;
		ipOut.m_eType = k_ESteamIPTypeIPv6;
		ipOut.m_ipv6Qword[0] = 0;
		ipOut.m_ipv6Qword[1] = 0;

		return ipOut;
	}

	static SteamIPAddress_t IPv4Loopback()
	{
		SteamIPAddress_t ipOut;
		ipOut.m_eType = k_ESteamIPTypeIPv4;
		ipOut.m_unIPv4 = 0x7f000001;

		return ipOut;
	}

	static SteamIPAddress_t IPv6Loopback()
	{
		SteamIPAddress_t ipOut;
		ipOut.m_eType = k_ESteamIPTypeIPv6;
		ipOut.m_ipv6Qword[0] = 0;
		ipOut.m_ipv6Qword[1] = 0;
		ipOut.m_rgubIPv6[15] = 1;

		return ipOut;
	}
};
struct P2PSessionState_t
{
    uint8 m_bConnectionActive;		// true if we've got an active open connection
    uint8 m_bConnecting;			// true if we're currently trying to establish a connection
    uint8 m_eP2PSessionError;		// last error recorded (see enum above)
    uint8 m_bUsingRelay;			// true if it's going through a relay server (TURN)
    int32 m_nBytesQueuedForSend;
    int32 m_nPacketsQueuedForSend;
    uint32 m_nRemoteIP;				// potential IP:Port of remote host. Could be TURN server. 
    uint16 m_nRemotePort;			// Only exists for compatibility with older authentication api's
};

class ISteamNetworking
{
public:
    virtual int SendP2PPacket(CSteamID, void const*, unsigned int, EP2PSend, int) = 0;
    virtual int IsP2PPacketAvailable(unsigned int*, int) = 0;
    virtual int ReadP2PPacket(void*, unsigned int, unsigned int*, CSteamID*, int) = 0;
    virtual int AcceptP2PSessionWithUser(CSteamID) = 0;
    virtual int CloseP2PSessionWithUser(CSteamID) = 0;
    virtual int CloseP2PChannelWithUser(CSteamID, int) = 0;
    virtual bool GetP2PSessionState(CSteamID, P2PSessionState_t*) = 0;
    virtual int AllowP2PPacketRelay(bool) = 0;
    virtual int CreateListenSocket(int, SteamIPAddress_t, unsigned short, bool) = 0;
    virtual int CreateP2PConnectionSocket(CSteamID, int, int, bool) = 0;
    virtual int CreateConnectionSocket(SteamIPAddress_t, unsigned short, int) = 0;
    virtual int DestroySocket(unsigned int, bool) = 0;
    virtual int DestroyListenSocket(unsigned int, bool) = 0;
    virtual int SendDataOnSocket(unsigned int, void*, unsigned int, bool) = 0;
    virtual int IsDataAvailableOnSocket(unsigned int, unsigned int*) = 0;
    virtual int RetrieveDataFromSocket(unsigned int, void*, unsigned int, unsigned int*) = 0;
    virtual int IsDataAvailable(unsigned int, unsigned int*, unsigned int*) = 0;
    virtual int RetrieveData(unsigned int, void*, unsigned int, unsigned int*, unsigned int*) = 0;
    virtual int GetSocketInfo(unsigned int, CSteamID*, int*, SteamIPAddress_t*, unsigned short*) = 0;
    virtual int GetListenSocketInfo(unsigned int, SteamIPAddress_t*, unsigned short*) = 0;
    virtual int GetSocketConnectionType(unsigned int) = 0;
    virtual int GetMaxPacketSize(unsigned int) = 0;
};