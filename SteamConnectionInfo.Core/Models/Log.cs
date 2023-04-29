namespace SteamConnectionInfoCore.Models
{
    public class LogPlayer
    {
        public DateTime TimestampUtc { get; set; }
        public uint Ip { get; set; }
        public ushort Port { get; set; }
        public string Country { get; set; } = "Unknown";
        public byte UsingSteamRelay { get; set; }
        public string SteamName { get; set; } = "Unknown";
        public ulong SteamId { get; set; }
    }
}

