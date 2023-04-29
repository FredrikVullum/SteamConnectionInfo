
using System;
using System.Globalization;
using System.Net;
using System.Windows.Data;

namespace SteamConnectionInfoWpf.Converters
{
    public class UIntToIPAddressConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is uint uintValue)
            {
                byte[] bytes = BitConverter.GetBytes(uintValue);
                Array.Reverse(bytes);
                return new IPAddress(bytes).ToString();
            }
            return value;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
