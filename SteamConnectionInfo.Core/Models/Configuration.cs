namespace SteamConnectionInfoCore.Models
{
    public class ConfigurationModel
    {
        public bool WindowTransparentForInput { get; set; } = false;
        public bool WindowHidden { get; set; } = false;
        public double WindowWidth { get; set; } = 475;
        public double WindowHeight { get; set; } = 200;
        public double WindowPositionX { get; set; } = 960;
        public double WindowPositionY { get; set; } = 540;
        public int WindowOpacity { get; set; } = 80;
        public bool ColumnIpEnabled { get; set; } = false;
        public bool ColumnPingEnabled { get; set; } = true;
        public bool ColumnCountryEnabled { get; set; } = true;
        public bool ColumnNameEnabled { get; set; } = true;
        public bool LoggingEnabled { get; set; } = false;
        public bool PingFilterEnabled { get; set; } = false;
        public bool CountryFilterEnabled { get; set; } = false;

    }
}
