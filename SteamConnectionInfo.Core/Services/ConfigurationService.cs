using Newtonsoft.Json;
using System.Linq.Expressions;
using System.Reflection;
using SteamConnectionInfoCore.Models;

namespace SteamConnectionInfoCore.Services
{
  
    public static class ConfigurationService
    {
        private static ConfigurationModel?             _configurationModel;
        private static readonly string                 _filePath           = "appsettings.json";
        private static readonly JsonSerializerSettings _serializerSettings = new JsonSerializerSettings
        {
            Formatting = Formatting.Indented,
            NullValueHandling = NullValueHandling.Ignore
        };

        private static void Create()
        {
            _configurationModel = new ConfigurationModel();

            try
            {
                var fileContent = JsonConvert.SerializeObject(_configurationModel, _serializerSettings);
                File.WriteAllText(_filePath, fileContent);
            }
            catch { };
        }

        public static void Load()
        {
            if (!File.Exists(_filePath))
            {
                Create();
                return;
            }

            try
            {
                var fileContent = File.ReadAllText(_filePath);
                _configurationModel = JsonConvert.DeserializeObject<ConfigurationModel>(fileContent, _serializerSettings) ?? new ConfigurationModel();
            }
            catch{  Create();  }
        }

        public static T? Get<T>(Expression<Func<ConfigurationModel, T>> propertyExpression)
        {
            var memberExpression = propertyExpression.Body as MemberExpression;
            if (memberExpression == null)
            {
                return default;
            }

            var propertyInfo = memberExpression.Member as PropertyInfo;
            if (propertyInfo == null)
            {
                return default;
            }

            return (T?)propertyInfo.GetValue(_configurationModel);
        }

        public static void Upsert<T>(Expression<Func<ConfigurationModel, T>> propertyExpression, T value)
        {
            var memberExpression = propertyExpression.Body as MemberExpression;
            if (memberExpression == null)
            {
                return;
            }

            var propertyInfo = memberExpression.Member as PropertyInfo;
            if (propertyInfo == null)
            {
                return;
            }

            propertyInfo.SetValue(_configurationModel, value);

            try
            {
                var fileContent = JsonConvert.SerializeObject(_configurationModel, _serializerSettings);

                File.WriteAllText(_filePath, fileContent);
            }
            catch { };
        }
    }
}
