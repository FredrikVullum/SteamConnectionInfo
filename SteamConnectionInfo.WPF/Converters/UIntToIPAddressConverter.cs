
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
                return new IPAddress(uintValue).ToString();
            }
            return value;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
