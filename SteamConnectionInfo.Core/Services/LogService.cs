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

            try
            {
                string[] lines = File.ReadAllLines(_filePath);

                Log? latestPlayerEntry = null;

                foreach (string line in lines)
                {
                    var entry = JsonConvert.DeserializeObject<Log>(line);

                    if (entry == null || entry.Player == null)
                        continue;

                    if (entry.Player.SteamId == player.SteamId)
                    {
                        latestPlayerEntry = entry;

                        // If the existing player has a different SteamPort, add the new player to the log file
                        if (entry.Player.SteamPort != player.SteamPort)
                        {
                            var output = new Log { TimestampUtc = DateTime.UtcNow, Player = player };

                            using (StreamWriter file = new StreamWriter(_filePath, true))
                            {
                                file.WriteLine(JsonConvert.SerializeObject(output));
                            }
                        }
                    }
                }

                if (latestPlayerEntry == null || latestPlayerEntry?.Player?.SteamPort != player.SteamPort)
                {
                    var output = new Log { TimestampUtc = DateTime.UtcNow, Player = player };

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
