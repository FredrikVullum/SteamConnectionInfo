using Newtonsoft.Json;

namespace SteamConnectionInfoCore.Models
{
    public class Player
    {
        [JsonProperty("steam_ip")]
        public uint SteamIp { get; set; }

        [JsonProperty("steam_port")]
        public ushort SteamPort { get; set; }

        [JsonProperty("steam_name")]
        public string SteamName { get; set; }

        [JsonProperty("steam_id")]
        public ulong SteamId { get; set; }

        [JsonProperty("steam_relay")]
        public byte SteamRelay { get; set; }

        public Player()
        {
            SteamIp = 43634546;
            SteamPort = 656;
            SteamName = "aewrewar";
            SteamId = 54645645654;
            SteamRelay = 1;
        }

        public override string ToString()
        {
            return $"Steam IP: {SteamIp}, Port: {SteamPort}, Name: {SteamName}, ID: {SteamId}, Relay: {SteamRelay}";
        }
    }
}
