﻿namespace SteamConnectionInfoCore.Models
{
    public class ConfigurationModel
    {
        public bool WindowTransparentForInput { get; set; } = false;
        public bool WindowHidden { get; set; } = false;
        public double WindowPositionX { get; set; } = 960;
        public double WindowPositionY { get; set; } = 540;
        public int WindowOpacity { get; set; } = 80;
        public bool ColumnIpEnabled { get; set; } = true;
        public bool ColumnPortEnabled { get; set; } = true;
        public bool ColumnRelayEnabled { get; set; } = false;
        public bool ColumnCountryEnabled { get; set; } = true;
        public bool ColumnNameEnabled { get; set; } = true;
    }
}
