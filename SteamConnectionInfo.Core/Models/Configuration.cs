namespace SteamConnectionInfoCore.Models
{
    public class ConfigurationModel
    {
        public bool WindowTransparentForInput { get; set; } = false;
        public bool WindowHidden { get; set; } = false;
        public double WindowPositionX { get; set; } = 960;
        public double WindowPositionY { get; set; } = 540;
        public int WindowOpacity { get; set; } = 80;
    }
}
