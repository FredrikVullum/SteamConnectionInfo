using Newtonsoft.Json;
using System.Linq.Expressions;
using System.Reflection;
using SteamConnectionInfoCore.Models;

namespace SteamConnectionInfoCore.Services
{
    public static class LogService
    {
        private static readonly string _filePath = "playerlog.json";

        private static bool Loaded = false;
        private static void Create()
        {
            try
            {
                File.Create(_filePath).Close();
                Loaded = true;
            }
            catch
            {

            }
            Loaded = false;
        }

        public static void Load()
        {
            if (!File.Exists(_filePath)){
                Create();
            }
            Loaded = true;
        }
        public static void Insert(Player player)
        {
            if (!Loaded)
                return;

            if (player.SteamId == 0)
                return;
            if (player.SteamIp == 0)
                return;
            if (player.SteamPort == 0)
                return;
            if (string.IsNullOrEmpty(player.SteamName))
                return;
            if (string.IsNullOrEmpty(player.Country))
                return;
        
            try
            {
                string[] lines = File.ReadAllLines(_filePath);

                LogPlayer? playerFromLog = null;

                for(int i = lines.Length - 1; i >= 0; i--)
                {
                    var entry = JsonConvert.DeserializeObject<LogPlayer>(lines[i]);

                    if (entry == null)
                        continue;

                    if(entry.SteamId == player.SteamId)
                    {
                        playerFromLog = entry;
                        break;
                    }
                }

                if (playerFromLog == null || playerFromLog.Port != player.SteamPort)
                {
                    var output = new LogPlayer {
                        TimestampUtc = DateTime.UtcNow,
                        Ip = player.SteamIp,
                        Port = player.SteamPort,
                        Country = player.Country,
                        SteamName = player.SteamName,
                        SteamId = player.SteamId
                    };

                    using (StreamWriter file = new StreamWriter(_filePath, true))
                    {
                        file.WriteLine(JsonConvert.SerializeObject(output));
                    }
                }
            }
            catch
            {

            }
        }
    }
}
