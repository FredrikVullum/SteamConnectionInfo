#include "SteamTypes.h"

class CGameID
{
public:

	CGameID()
	{
		m_gameID.m_nType = k_EGameIDTypeApp;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nModID = 0;
	}

	explicit CGameID(uint64 ulGameID)
	{
		m_ulGameID = ulGameID;
	}

	explicit CGameID(int32 nAppID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
	}

	explicit CGameID(uint32 nAppID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
	}

	CGameID(uint32 nAppID, uint32 nModID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
		m_gameID.m_nModID = nModID;
		m_gameID.m_nType = k_EGameIDTypeGameMod;
	}

	// Hidden functions used only by Steam
	explicit CGameID(const char* pchGameID);
	const char* Render() const;					// render this Game ID to string
	static const char* Render(uint64 ulGameID);		// static method to render a uint64 representation of a Game ID to a string

	// must include checksum_crc.h first to get this functionality
#if defined( CHECKSUM_CRC_H )
	CGameID(uint32 nAppID, const char* pchModPath)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
		m_gameID.m_nType = k_EGameIDTypeGameMod;

		char rgchModDir[MAX_PATH];
		Q_FileBase(pchModPath, rgchModDir, sizeof(rgchModDir));
		CRC32_t crc32;
		CRC32_Init(&crc32);
		CRC32_ProcessBuffer(&crc32, rgchModDir, Q_strlen(rgchModDir));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

	CGameID(const char* pchExePath, const char* pchAppName)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nType = k_EGameIDTypeShortcut;

		CRC32_t crc32;
		CRC32_Init(&crc32);
		CRC32_ProcessBuffer(&crc32, pchExePath, Q_strlen(pchExePath));
		CRC32_ProcessBuffer(&crc32, pchAppName, Q_strlen(pchAppName));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

#if defined( VSTFILEID_H )

	CGameID(VstFileID vstFileID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nType = k_EGameIDTypeP2P;

		CRC32_t crc32;
		CRC32_Init(&crc32);
		const char* pchFileId = vstFileID.Render();
		CRC32_ProcessBuffer(&crc32, pchFileId, Q_strlen(pchFileId));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

#endif /* VSTFILEID_H */

#endif /* CHECKSUM_CRC_H */


	uint64 ToUint64() const
	{
		return m_ulGameID;
	}

	uint64* GetUint64Ptr()
	{
		return &m_ulGameID;
	}

	bool IsMod() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeGameMod);
	}

	bool IsShortcut() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeShortcut);
	}

	bool IsP2PFile() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeP2P);
	}

	bool IsSteamApp() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeApp);
	}

	uint32 ModID() const
	{
		return m_gameID.m_nModID;
	}

	uint32 AppID() const
	{
		return m_gameID.m_nAppID;
	}

	bool operator == (const CGameID& rhs) const
	{
		return m_ulGameID == rhs.m_ulGameID;
	}

	bool operator != (const CGameID& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator < (const CGameID& rhs) const
	{
		return (m_ulGameID < rhs.m_ulGameID);
	}

	bool IsValid() const
	{
		// each type has it's own invalid fixed point:
		switch (m_gameID.m_nType)
		{
		case k_EGameIDTypeApp:
			return m_gameID.m_nAppID != k_uAppIdInvalid;
			break;
		case k_EGameIDTypeGameMod:
			return m_gameID.m_nAppID != k_uAppIdInvalid && m_gameID.m_nModID & 0x80000000;
			break;
		case k_EGameIDTypeShortcut:
			return (m_gameID.m_nModID & 0x80000000) != 0;
			break;
		case k_EGameIDTypeP2P:
			return m_gameID.m_nAppID == k_uAppIdInvalid && m_gameID.m_nModID & 0x80000000;
			break;
		default:
#if defined(Assert)
			Assert(false);
#endif
			return false;
		}

	}

	void Reset()
	{
		m_ulGameID = 0;
	}



private:

	enum EGameIDType
	{
		k_EGameIDTypeApp = 0,
		k_EGameIDTypeGameMod = 1,
		k_EGameIDTypeShortcut = 2,
		k_EGameIDTypeP2P = 3,
	};

	struct GameID_t
	{
#ifdef VALVE_BIG_ENDIAN
		unsigned int m_nModID : 32;
		unsigned int m_nType : 8;
		unsigned int m_nAppID : 24;
#else
		unsigned int m_nAppID : 24;
		unsigned int m_nType : 8;
		unsigned int m_nModID : 32;
#endif
	};

	union
	{
		uint64 m_ulGameID;
		GameID_t m_gameID;
	};
};

struct FriendGameInfo_t {
	CGameID m_gameID;
	uint32  m_unGameIP;
	uint16 m_usGamePort;
	uint16 m_usQueryPort;
	CSteamID m_steamIDLobby;
};

class ISteamFriends
{
public:
    virtual unknown_ret GetPersonaName() = 0;
    virtual unknown_ret SetPersonaName(char const*) = 0;
    virtual unknown_ret IsPersonaNameSet() = 0;
    virtual unknown_ret GetPersonaState() = 0;
    virtual unknown_ret SetPersonaState(int) = 0;
    virtual unknown_ret NotifyUIOfMenuChange(bool, bool, bool, bool) = 0;
    virtual unknown_ret GetFriendCount(int) = 0;
    virtual unknown_ret GetFriendArray(CSteamID*, signed char*, int, int) = 0;
    virtual unknown_ret GetFriendArrayInGame(CSteamID*, unsigned long long*, int) = 0;
    virtual unknown_ret GetFriendByIndex(int, int) = 0;
    virtual unknown_ret GetOnlineFriendCount() = 0;
    virtual unknown_ret GetFriendRelationship(CSteamID) = 0;
    virtual unknown_ret GetFriendPersonaState(CSteamID) = 0;
    virtual const char* GetFriendPersonaName(CSteamID) = 0;
    virtual unknown_ret GetSmallFriendAvatar(CSteamID) = 0;
    virtual unknown_ret GetMediumFriendAvatar(CSteamID) = 0;
    virtual unknown_ret GetLargeFriendAvatar(CSteamID) = 0;
    virtual unknown_ret BGetFriendAvatarURL(char*, unsigned int, CSteamID, int) = 0;
    virtual unknown_ret GetFriendAvatarHash(char*, unsigned int, CSteamID) = 0;
    virtual unknown_ret SetFriendRegValue(CSteamID, char const*, char const*) = 0;
    virtual unknown_ret GetFriendRegValue(CSteamID, char const*) = 0;
    virtual unknown_ret DeleteFriendRegValue(CSteamID, char const*) = 0;
    virtual unknown_ret GetFriendGamePlayed(CSteamID, FriendGameInfo_t*) = 0;
    virtual unknown_ret GetFriendGamePlayedExtraInfo(CSteamID) = 0;
    virtual unknown_ret GetFriendGameServer(CSteamID) = 0;
    virtual unknown_ret GetFriendPersonaStateFlags(CSteamID) = 0;
    virtual unknown_ret GetFriendSessionStateInfo(CSteamID) = 0;
    virtual unknown_ret GetFriendRestrictions(CSteamID) = 0;
    virtual unknown_ret GetFriendBroadcastID(CSteamID) = 0;
    virtual unknown_ret GetFriendPersonaNameHistory(CSteamID, int) = 0;
    virtual unknown_ret RequestPersonaNameHistory(CSteamID) = 0;
    virtual unknown_ret GetFriendPersonaNameHistoryAndDate(CSteamID, int, unsigned int*) = 0;
    virtual unknown_ret AddFriend(CSteamID) = 0;
    virtual unknown_ret RemoveFriend(CSteamID) = 0;
    virtual unknown_ret HasFriend(CSteamID, int) = 0;
    virtual unknown_ret RequestUserInformation(CSteamID, bool) = 0;
    virtual unknown_ret SetIgnoreFriend(CSteamID, bool) = 0;
    virtual unknown_ret ReportChatDeclined(CSteamID) = 0;
    virtual unknown_ret CreateFriendsGroup(char const*) = 0;
    virtual unknown_ret DeleteFriendsGroup(short) = 0;
    virtual unknown_ret RenameFriendsGroup(char const*, short) = 0;
    virtual unknown_ret AddFriendToGroup(CSteamID, short) = 0;
    virtual unknown_ret RemoveFriendFromGroup(CSteamID, short) = 0;
    virtual unknown_ret IsFriendMemberOfFriendsGroup(CSteamID, short) = 0;
    virtual unknown_ret GetFriendsGroupCount() = 0;
    virtual unknown_ret GetFriendsGroupIDByIndex(short) = 0;
    virtual unknown_ret GetFriendsGroupName(short) = 0;
    virtual unknown_ret GetFriendsGroupMembershipCount(short) = 0;
    virtual unknown_ret GetFirstFriendsGroupMember(short) = 0;
    virtual unknown_ret GetNextFriendsGroupMember(short) = 0;
    virtual unknown_ret GetGroupFriendsMembershipCount(CSteamID) = 0;
    virtual unknown_ret GetFirstGroupFriendsMember(CSteamID) = 0;
    virtual unknown_ret GetNextGroupFriendsMember(CSteamID) = 0;
    virtual unknown_ret GetPlayerNickname(CSteamID) = 0;
    virtual unknown_ret SetPlayerNickname(CSteamID, char const*) = 0;
    virtual unknown_ret GetFriendSteamLevel(CSteamID) = 0;
    virtual unknown_ret GetChatMessagesCount(CSteamID) = 0;
    virtual unknown_ret GetChatMessage(CSteamID, int, void*, int, int*, unsigned long long*, unsigned int*) = 0;
    virtual unknown_ret SendMsgToFriend(CSteamID, int, char const*) = 0;
    virtual unknown_ret ClearChatHistory(CSteamID) = 0;
    virtual unknown_ret GetKnownClanCount() = 0;
    virtual unknown_ret GetKnownClanByIndex(int) = 0;
    virtual unknown_ret GetClanCount() = 0;
    virtual unknown_ret GetClanByIndex(int) = 0;
    virtual unknown_ret GetClanName(CSteamID) = 0;
    virtual unknown_ret GetClanTag(CSteamID) = 0;
    virtual unknown_ret GetFriendActivityCounts(int*, int*, bool) = 0;
    virtual unknown_ret GetClanActivityCounts(CSteamID, int*, int*, int*) = 0;
    virtual unknown_ret DownloadClanActivityCounts(CSteamID*, int) = 0;
    virtual unknown_ret GetFriendsGroupActivityCounts(short, int*, int*) = 0;
    virtual unknown_ret IsClanPublic(CSteamID) = 0;
    virtual unknown_ret IsClanOfficialGameGroup(CSteamID) = 0;
    virtual unknown_ret JoinClanChatRoom(CSteamID) = 0;
    virtual unknown_ret LeaveClanChatRoom(CSteamID) = 0;
    virtual unknown_ret GetClanChatMemberCount(CSteamID) = 0;
    virtual unknown_ret GetChatMemberByIndex(CSteamID, int) = 0;
    virtual unknown_ret SendClanChatMessage(CSteamID, char const*) = 0;
    virtual unknown_ret GetClanChatMessage(CSteamID, int, void*, int, int*, CSteamID*) = 0;
    virtual unknown_ret IsClanChatAdmin(CSteamID, CSteamID) = 0;
    virtual unknown_ret IsClanChatWindowOpenInSteam(CSteamID) = 0;
    virtual unknown_ret OpenClanChatWindowInSteam(CSteamID) = 0;
    virtual unknown_ret CloseClanChatWindowInSteam(CSteamID) = 0;
    virtual unknown_ret SetListenForFriendsMessages(bool) = 0;
    virtual unknown_ret ReplyToFriendMessage(CSteamID, char const*) = 0;
    virtual unknown_ret GetFriendMessage(CSteamID, int, void*, int, int*) = 0;
    virtual unknown_ret InviteFriendToClan(CSteamID, CSteamID) = 0;
    virtual unknown_ret AcknowledgeInviteToClan(CSteamID, bool) = 0;
    virtual unknown_ret GetFriendCountFromSource(CSteamID) = 0;
    virtual unknown_ret GetFriendFromSourceByIndex(CSteamID, int) = 0;
    virtual unknown_ret IsUserInSource(CSteamID, CSteamID) = 0;
    virtual unknown_ret GetCoplayFriendCount() = 0;
    virtual unknown_ret GetCoplayFriend(int) = 0;
    virtual unknown_ret GetFriendCoplayTime(CSteamID) = 0;
    virtual unknown_ret GetFriendCoplayGame(CSteamID) = 0;
    virtual unknown_ret SetRichPresence(unsigned int, char const*, char const*) = 0;
    virtual unknown_ret ClearRichPresence(unsigned int) = 0;
    virtual unknown_ret GetFriendRichPresence(unsigned int, CSteamID, char const*) = 0;
    virtual unknown_ret GetFriendRichPresenceKeyCount(unsigned int, CSteamID) = 0;
    virtual unknown_ret GetFriendRichPresenceKeyByIndex(unsigned int, CSteamID, int) = 0;
    virtual unknown_ret RequestFriendRichPresence(unsigned int, CSteamID) = 0;
    virtual unknown_ret JoinChatRoom(CSteamID) = 0;
    virtual unknown_ret LeaveChatRoom(CSteamID) = 0;
    virtual unknown_ret InviteUserToChatRoom(CSteamID, CSteamID) = 0;
    virtual unknown_ret SendChatMsg(CSteamID, int, char const*) = 0;
    virtual unknown_ret GetChatRoomMessagesCount(CSteamID) = 0;
    virtual unknown_ret GetChatRoomEntry(CSteamID, int, CSteamID*, void*, int, int*) = 0;
    virtual unknown_ret ClearChatRoomHistory(CSteamID) = 0;
    virtual unknown_ret SerializeChatRoomDlg(CSteamID, void const*, int) = 0;
    virtual unknown_ret GetSizeOfSerializedChatRoomDlg(CSteamID) = 0;
    virtual unknown_ret GetSerializedChatRoomDlg(CSteamID, void*, int, int*, int*) = 0;
    virtual unknown_ret ClearSerializedChatRoomDlg(CSteamID) = 0;
    virtual unknown_ret KickChatMember(CSteamID, CSteamID) = 0;
    virtual unknown_ret BanChatMember(CSteamID, CSteamID) = 0;
    virtual unknown_ret UnBanChatMember(CSteamID, CSteamID) = 0;
    virtual unknown_ret SetChatRoomType(CSteamID, ELobbyType) = 0;
    virtual unknown_ret GetChatRoomLockState(CSteamID, bool*) = 0;
    virtual unknown_ret GetChatRoomPermissions(CSteamID, unsigned int*) = 0;
    virtual unknown_ret SetChatRoomModerated(CSteamID, bool) = 0;
    virtual unknown_ret BChatRoomModerated(CSteamID) = 0;
    virtual unknown_ret NotifyChatRoomDlgsOfUIChange(CSteamID, bool, bool, bool, bool) = 0;
    virtual unknown_ret TerminateChatRoom(CSteamID) = 0;
    virtual unknown_ret GetChatRoomCount() = 0;
    virtual unknown_ret GetChatRoomByIndex(int) = 0;
    virtual unknown_ret GetChatRoomName(CSteamID) = 0;
    virtual unknown_ret BGetChatRoomMemberDetails(CSteamID, CSteamID, unsigned int*, unsigned int*) = 0;
    virtual unknown_ret CreateChatRoom(int, unsigned long long, char const*, ELobbyType, CSteamID, CSteamID, CSteamID, unsigned int, unsigned int, unsigned int) = 0;
    virtual unknown_ret JoinChatRoomGroup(unsigned long long, unsigned long long) = 0;
    virtual unknown_ret ShowChatRoomGroupInvite(char const*) = 0;
    virtual unknown_ret VoiceCallNew(CSteamID, CSteamID) = 0;
    virtual unknown_ret VoiceCall(CSteamID, CSteamID) = 0;
    virtual unknown_ret VoiceHangUp(CSteamID, int) = 0;
    virtual unknown_ret SetVoiceSpeakerVolume(float) = 0;
    virtual unknown_ret SetVoiceMicrophoneVolume(float) = 0;
    virtual unknown_ret SetAutoAnswer(bool) = 0;
    virtual unknown_ret VoiceAnswer(int) = 0;
    virtual unknown_ret AcceptVoiceCall(CSteamID, CSteamID) = 0;
    virtual unknown_ret VoicePutOnHold(int, bool) = 0;
    virtual unknown_ret BVoiceIsLocalOnHold(int) = 0;
    virtual unknown_ret BVoiceIsRemoteOnHold(int) = 0;
    virtual unknown_ret SetDoNotDisturb(bool) = 0;
    virtual unknown_ret EnableVoiceNotificationSounds(bool) = 0;
    virtual unknown_ret SetPushToTalkEnabled(bool, bool) = 0;
    virtual unknown_ret IsPushToTalkEnabled() = 0;
    virtual unknown_ret IsPushToMuteEnabled() = 0;
    virtual unknown_ret SetPushToTalkKey(int) = 0;
    virtual unknown_ret GetPushToTalkKey() = 0;
    virtual unknown_ret IsPushToTalkKeyDown() = 0;
    virtual unknown_ret EnableVoiceCalibration(bool) = 0;
    virtual unknown_ret IsVoiceCalibrating() = 0;
    virtual unknown_ret GetVoiceCalibrationSamplePeak() = 0;
    virtual unknown_ret SetMicBoost(bool) = 0;
    virtual unknown_ret GetMicBoost() = 0;
    virtual unknown_ret HasHardwareMicBoost() = 0;
    virtual unknown_ret GetMicDeviceName() = 0;
    virtual unknown_ret StartTalking(int) = 0;
    virtual unknown_ret EndTalking(int) = 0;
    virtual unknown_ret VoiceIsValid(int) = 0;
    virtual unknown_ret SetAutoReflectVoice(bool) = 0;
    virtual unknown_ret GetCallState(int) = 0;
    virtual unknown_ret GetVoiceMicrophoneVolume() = 0;
    virtual unknown_ret GetVoiceSpeakerVolume() = 0;
    virtual unknown_ret TimeSinceLastVoiceDataReceived(int) = 0;
    virtual unknown_ret TimeSinceLastVoiceDataSend(int) = 0;
    virtual unknown_ret BCanSend(int) = 0;
    virtual unknown_ret BCanReceive(int) = 0;
    virtual unknown_ret GetEstimatedBitsPerSecond(int, bool) = 0;
    virtual unknown_ret GetPeakSample(int, bool) = 0;
    virtual unknown_ret SendResumeRequest(int) = 0;
    virtual unknown_ret OpenFriendsDialog(bool, bool) = 0;
    virtual unknown_ret OpenChatDialog(CSteamID) = 0;
    virtual unknown_ret OpenInviteToTradeDialog(CSteamID) = 0;
    virtual unknown_ret StartChatRoomVoiceSpeaking(CSteamID, CSteamID) = 0;
    virtual unknown_ret EndChatRoomVoiceSpeaking(CSteamID, CSteamID) = 0;
    virtual unknown_ret GetFriendLastLogonTime(CSteamID) = 0;
    virtual unknown_ret GetFriendLastLogoffTime(CSteamID) = 0;
    virtual unknown_ret GetChatRoomVoiceTotalSlotCount(CSteamID) = 0;
    virtual unknown_ret GetChatRoomVoiceUsedSlotCount(CSteamID) = 0;
    virtual unknown_ret GetChatRoomVoiceUsedSlot(CSteamID, int) = 0;
    virtual unknown_ret GetChatRoomVoiceStatus(CSteamID, CSteamID) = 0;
    virtual unknown_ret BChatRoomHasAvailableVoiceSlots(CSteamID) = 0;
    virtual unknown_ret BIsChatRoomVoiceSpeaking(CSteamID, CSteamID) = 0;
    virtual unknown_ret GetChatRoomPeakSample(CSteamID, CSteamID, bool) = 0;
    virtual unknown_ret ChatRoomVoiceRetryConnections(CSteamID) = 0;
    virtual unknown_ret SetPortTypes(unsigned int) = 0;
    virtual unknown_ret ReinitAudio() = 0;
    virtual unknown_ret SetInGameVoiceSpeaking(CSteamID, bool) = 0;
    virtual unknown_ret IsInGameVoiceSpeaking() = 0;
    virtual unknown_ret ActivateGameOverlay(char const*) = 0;
    virtual unknown_ret ActivateGameOverlayToUser(char const*, CSteamID) = 0;
    virtual unknown_ret ActivateGameOverlayToWebPage(char const*, int) = 0;
    virtual unknown_ret ActivateGameOverlayToStore(unsigned int, int) = 0;
    virtual unknown_ret ActivateGameOverlayInviteDialog(CSteamID) = 0;
    virtual unknown_ret ActivateGameOverlayRemotePlayTogetherInviteDialog(CSteamID) = 0;
    virtual unknown_ret ActivateGameOverlayInviteDialogConnectString(char const*) = 0;
    virtual unknown_ret ProcessActivateGameOverlayInMainUI(char const*, CSteamID, unsigned int, bool, int) = 0;
    virtual unknown_ret NotifyGameOverlayStateChanged(bool) = 0;
    virtual unknown_ret NotifyGameServerChangeRequested(char const*, char const*) = 0;
    virtual unknown_ret NotifyLobbyJoinRequested(unsigned int, CSteamID, CSteamID) = 0;
    virtual unknown_ret NotifyRichPresenceJoinRequested(unsigned int, CSteamID, char const*) = 0;
    virtual unknown_ret GetClanRelationship(CSteamID) = 0;
    virtual unknown_ret GetClanInviteCount() = 0;
    virtual unknown_ret GetFriendClanRank(CSteamID, CSteamID) = 0;
    virtual unknown_ret VoiceIsAvailable() = 0;
    virtual unknown_ret TestVoiceDisconnect(int) = 0;
    virtual unknown_ret TestChatRoomPeerDisconnect(CSteamID, CSteamID) = 0;
    virtual unknown_ret TestVoicePacketLoss(float) = 0;
    virtual unknown_ret FindFriendVoiceChatHandle(CSteamID) = 0;
    virtual unknown_ret RequestFriendsWhoPlayGame(CGameID) = 0;
    virtual unknown_ret GetCountFriendsWhoPlayGame(CGameID) = 0;
    virtual unknown_ret GetFriendWhoPlaysGame(unsigned int, CGameID) = 0;
    virtual unknown_ret GetCountFriendsInGame(CGameID) = 0;
    virtual unknown_ret SetPlayedWith(CSteamID) = 0;
    virtual unknown_ret RequestClanOfficerList(CSteamID) = 0;
    virtual unknown_ret GetClanOwner(CSteamID) = 0;
    virtual unknown_ret GetClanOfficerCount(CSteamID) = 0;
    virtual unknown_ret GetClanOfficerByIndex(CSteamID, int) = 0;
    virtual unknown_ret GetUserRestrictions() = 0;
    virtual unknown_ret RequestFriendProfileInfo(CSteamID) = 0;
    virtual unknown_ret GetFriendProfileInfo(CSteamID, char const*) = 0;
    virtual unknown_ret InviteUserToGame(unsigned int, CSteamID, char const*) = 0;
    virtual unknown_ret GetOnlineConsoleFriendCount() = 0;
    virtual unknown_ret RequestTrade(CSteamID) = 0;
    virtual unknown_ret TradeResponse(unsigned int, bool) = 0;
    virtual unknown_ret CancelTradeRequest(CSteamID) = 0;
    virtual unknown_ret HideFriend(CSteamID, bool) = 0;
    virtual unknown_ret GetFollowerCount(CSteamID) = 0;
    virtual unknown_ret IsFollowing(CSteamID) = 0;
    virtual unknown_ret EnumerateFollowingList(unsigned int) = 0;
    virtual unknown_ret RequestFriendMessageHistory(CSteamID) = 0;
    virtual unknown_ret RequestFriendMessageHistoryForOfflineMessages() = 0;
    virtual unknown_ret GetCountFriendsWithOfflineMessages() = 0;
    virtual unknown_ret GetFriendWithOfflineMessage(int) = 0;
    virtual unknown_ret ClearFriendHasOfflineMessage(unsigned int) = 0;
    virtual unknown_ret RequestEmoticonList() = 0;
    virtual unknown_ret GetEmoticonCount() = 0;
    virtual unknown_ret GetEmoticonName(int) = 0;
    virtual unknown_ret ClientLinkFilterInit() = 0;
    virtual unknown_ret LinkDisposition(char const*) = 0;
    virtual unknown_ret GetFriendPersonaName_Public(CSteamID) = 0;
    virtual unknown_ret GetPlayerNickname_Public(CSteamID) = 0;
    virtual unknown_ret SetFriendsUIActiveClanChatList(unsigned int*, int) = 0;
    virtual unknown_ret GetNumChatsWithUnreadPriorityMessages() = 0;
    virtual unknown_ret SetNumChatsWithUnreadPriorityMessages(int) = 0;
    virtual unknown_ret RegisterProtocolInOverlayBrowser(char const*) = 0;
    virtual unknown_ret HandleProtocolForOverlayBrowser(unsigned int, char const*) = 0;
};