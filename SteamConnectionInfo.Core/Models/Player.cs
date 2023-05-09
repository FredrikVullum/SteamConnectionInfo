using Newtonsoft.Json;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace SteamConnectionInfoCore.Models
{
    public class Player : INotifyPropertyChanged
    {
        private uint   _steamIp = 0;
        private ushort _steamPort = 0;
        private string _steamName = "Unknown";
        private ulong  _steamId = 0;
        private string _country = "Unknown";
        private long   _ping = 0;

        [JsonProperty("steam_ip")]
        public uint SteamIp
        {
            get => _steamIp;
            set
            {
                _steamIp = value;
                OnPropertyChanged();
            }
        }

        [JsonProperty("steam_port")]
        public ushort SteamPort
        {
            get => _steamPort;
            set
            {
                _steamPort = value;
                OnPropertyChanged();
            }
        }

        [JsonProperty("steam_name")]
        public string SteamName
        {
            get => _steamName;
            set
            {
                _steamName = value;
                OnPropertyChanged();
            }
        }

        [JsonProperty("steam_id")]
        public ulong SteamId
        {
            get => _steamId;
            set
            {
                _steamId = value;
            }
        }

        [JsonProperty("country")]
        public string Country
        {
            get => _country;
            set
            {
                _country = value;
                OnPropertyChanged();
            }
        }

        [JsonProperty("ping")]
        public long Ping
        {
            get => _ping;
            set
            {
                _ping = value;
                OnPropertyChanged();
            }
        }

        public Player()
        {
            SteamIp = 0;
            SteamPort = 0;
            SteamName = "Unknown";
            SteamId = 0;
            Country = "Unknown";
            Ping = 0;
        }

        public override string ToString()
        {
            return $"Steam IP: {SteamIp}, Port: {SteamPort}, Name: {SteamName}, " +
                $"ID: {SteamId}, Country: {Country}, Ping: {Ping}";
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
