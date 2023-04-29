using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using Newtonsoft.Json;
using SteamConnectionInfoCore.Services;
using SteamConnectionInfoCore.Models;
using SteamConnectionInfoCore.Views;
using System.Windows.Controls.Primitives;
using System.Linq;

namespace SteamConnectionInfoWpf
{
    public partial class MainWindow : Window
    {
        public         PlayerViewModel _playerViewModel;
        private static Mutex?          _startUpCheckMutex = null;
        public MainWindow()
        {
            _playerViewModel = new PlayerViewModel();

            bool mutexCreated = false;
            _startUpCheckMutex = new Mutex(true, "SteamConnectionInfo.WPF", out mutexCreated);

            if (!mutexCreated || !_startUpCheckMutex.WaitOne(TimeSpan.Zero, true))
            {
                MessageBox.Show("An instance of SteamConnectionInfo.WPF is already running." +
                 " Only one instance of SteamConnectionInfo.WPF is allowed to run.",
                 "Duplicate Instance Detected",
                 MessageBoxButton.OK,
                 MessageBoxImage.Exclamation);

                Application.Current.Shutdown();
                return;
            }

            ConfigurationService.Load();
            LogService.Load();

            KeyService.RegisterGlobalKeyPress((key) =>
            {
                if (key == KeyService.VK_INSERT)
                {
                    WindowServiceHelper.UnlockLockWindow(this);
                    return;
                }
                if (key == KeyService.VK_HOME)
                {
                    WindowServiceHelper.ShowHideWindow(this);
                    return;
                }
            });

            Width = ConfigurationService.Get(config => config.WindowWidth);
            Height = ConfigurationService.Get(config => config.WindowHeight);

            InitializeComponent();
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
           
            Left = ConfigurationService.Get(config => config.WindowPositionX);
            Top = ConfigurationService.Get(config => config.WindowPositionY);

            int opacityToSet = ConfigurationService.Get(config => config.WindowOpacity);
            Opacity = opacityToSet / 100.0;
            OpacityLabel.Content = $"{opacityToSet}%";
            OpacitySlider.Value = opacityToSet;

            ThreadRunHelper.RunInjectionThread(this);
            ThreadRunHelper.RunSharedMemoryThread(this);

            WindowServiceHelper.ShowHideWindow(this);
            WindowServiceHelper.UnlockLockWindow(this);
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            ConfigurationService.Upsert(config => config.WindowWidth, Width);
            ConfigurationService.Upsert(config => config.WindowHeight, Height);
        }
        private void PlayersDataGrid_Loaded(object sender, RoutedEventArgs e)
        {
            PlayersDataGrid.DataContext = _playerViewModel;

            checkBoxIp.IsChecked = ConfigurationService.Get(config => config.ColumnIpEnabled);
            checkBoxPing.IsChecked = ConfigurationService.Get(config => config.ColumnPingEnabled);
            checkBoxRelay.IsChecked = ConfigurationService.Get(config => config.ColumnRelayEnabled);
            checkBoxCountry.IsChecked = ConfigurationService.Get(config => config.ColumnCountryEnabled);
            checkBoxName.IsChecked = ConfigurationService.Get(config => config.ColumnNameEnabled);

            WindowServiceHelper.ShowOrHideColumn(PlayersDataGrid, checkBoxIp, (bool)checkBoxIp.IsChecked);
            WindowServiceHelper.ShowOrHideColumn(PlayersDataGrid, checkBoxPing, (bool)checkBoxPing.IsChecked);
            WindowServiceHelper.ShowOrHideColumn(PlayersDataGrid, checkBoxRelay, (bool)checkBoxRelay.IsChecked);
            WindowServiceHelper.ShowOrHideColumn(PlayersDataGrid, checkBoxCountry, (bool)checkBoxCountry.IsChecked);
            WindowServiceHelper.ShowOrHideColumn(PlayersDataGrid, checkBoxName, (bool)checkBoxName.IsChecked);

            checkBoxLogging.IsChecked = ConfigurationService.Get(config => config.LoggingEnabled);
            checkBoxFilterCountry.IsChecked = ConfigurationService.Get(config => config.CountryFilterEnabled);
            checkBoxFilterPing.IsChecked = ConfigurationService.Get(config => config.PingFilterEnabled);
        }


        private void OpacitySlider_DragCompleted(object sender, DragCompletedEventArgs e)
        {
            if (OpacitySlider != null)
            {
                int newOpacity = (int)OpacitySlider.Value;
                ConfigurationService.Upsert(config => config.WindowOpacity, newOpacity);
            }
        }
        private void OpacitySlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if(OpacityLabel != null)
            {
                int newOpacity = (int)(e.NewValue);
                Opacity = newOpacity / 100.0;
                OpacityLabel.Content = $"{newOpacity}%";
            }
        }
        private void Border_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                if (ResizeMode != System.Windows.ResizeMode.NoResize)
                {
                    ResizeMode = System.Windows.ResizeMode.NoResize;
                    UpdateLayout();
                }

                DragMove();

                ConfigurationService.Upsert(config => config.WindowPositionX, Left);
                ConfigurationService.Upsert(config => config.WindowPositionY, Top);
            }

            if (ResizeMode == System.Windows.ResizeMode.NoResize)
            {
                ResizeMode = System.Windows.ResizeMode.CanResizeWithGrip;
                UpdateLayout();
            }


            Keyboard.ClearFocus();
        }
        private void MenuItemExit_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }
        private void MenuItemLock_Click(object sender, RoutedEventArgs e)
        {
            WindowServiceHelper.UnlockLockWindow(this);
        }
        private void MenuItemShow_Click(object sender, RoutedEventArgs e)
        {
            WindowServiceHelper.ShowHideWindow(this);
        }
        private void MenuItemOpacity_Click(object sender, RoutedEventArgs e)
        {
            MenuItem? clickedItem = sender as MenuItem;

            if(clickedItem == null)
                return;

            var tag = clickedItem.Tag;
            if (tag == null) 
                return;

            var tagStr = tag.ToString();

            if (string.IsNullOrWhiteSpace(tagStr))
                return;

            double opacity = 0.8;

            try
            {
                double.TryParse(tagStr, out opacity);
            }
            catch
            {
                return;
            }

            Opacity = opacity;

            ConfigurationService.Upsert(config => config.WindowOpacity, opacity);
        }
        private void CheckBoxColumn_Checked(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as CheckBox;

            if(checkbox == null) 
                return;

            WindowServiceHelper.ShowOrHideColumn(PlayersDataGrid, checkbox, true);
        }
        private void CheckBoxColumn_Unchecked(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as CheckBox;

            if (checkbox == null)
                return;

            WindowServiceHelper.ShowOrHideColumn(PlayersDataGrid, checkbox, false);
        }
        private void CheckBoxLog_Checked(object sender, RoutedEventArgs e)
        {

            ConfigurationService.Upsert(config => config.LoggingEnabled, true);
        }
        private void CheckBoxLog_Unchecked(object sender, RoutedEventArgs e)
        {

            ConfigurationService.Upsert(config => config.LoggingEnabled, false);
        }
        private void CheckBoxFilterPing_Checked(object sender, RoutedEventArgs e)
        {

            ConfigurationService.Upsert(config => config.PingFilterEnabled, true);
        }
        private void CheckBoxFilterPing_Unchecked(object sender, RoutedEventArgs e)
        {

            ConfigurationService.Upsert(config => config.PingFilterEnabled, false);
        }
        private void CheckBoxFilterCountry_Checked(object sender, RoutedEventArgs e)
        {

            ConfigurationService.Upsert(config => config.CountryFilterEnabled, true);
        }
        private void CheckBoxFilterCountry_Unchecked(object sender, RoutedEventArgs e)
        {

            ConfigurationService.Upsert(config => config.CountryFilterEnabled, false);
        }

        public static class WindowServiceHelper        
        {
            private static bool WindowHiddenLoaded              = false;
            private static bool WindowTransparentForInputLoaded = false;

            public static bool WindowLocked = false;
            public static void ShowHideWindow(MainWindow mainWindow)
            {
                if (WindowHiddenLoaded)
                    ConfigurationService.Upsert(config => config.WindowHidden, !ConfigurationService.Get(config => config.WindowHidden));

                if (ConfigurationService.Get(config => config.WindowHidden))
                {
                    mainWindow.Visibility = Visibility.Collapsed;
                }
                else
                {
                    mainWindow.Visibility = Visibility.Visible;
                }

                WindowHiddenLoaded = true;
            }

            public static void UnlockLockWindow(MainWindow mainWindow)
            {
                if (WindowTransparentForInputLoaded)
                    ConfigurationService.Upsert(config => config.WindowTransparentForInput,
                        !ConfigurationService.Get(config => config.WindowTransparentForInput));

                IntPtr WindowHandle = new WindowInteropHelper(mainWindow).Handle;

                if (WindowHandle == IntPtr.Zero)
                    return;

                if (ConfigurationService.Get(config => config.WindowTransparentForInput))
                {
                    WindowService.SetWindowExTransparent(WindowHandle);
                    mainWindow.windowLockLabel.Content = "LOCKED";
                    mainWindow.windowLockLabel.Foreground = Brushes.LightGray;
                    mainWindow.ResizeMode = ResizeMode.NoResize;
                    mainWindow.PlayersDataGrid.UnselectAll();
                    WindowLocked = true;
                }
                else
                {
                    WindowService.SetWindowExNotTransparent(WindowHandle);
                    mainWindow.windowLockLabel.Content = "UNLOCKED";
                    mainWindow.windowLockLabel.Foreground = Brushes.SkyBlue;
                    mainWindow.ResizeMode = ResizeMode.CanResizeWithGrip;
                    WindowLocked = false;
                }

                mainWindow.UpdateLayout();

                WindowTransparentForInputLoaded = true;

            }

            public static void ShowOrHideColumn(DataGrid playersDataGrid, CheckBox checkbox, bool show)
            {
                if(playersDataGrid == null) 
                    return; 

                if(!playersDataGrid.IsLoaded) 
                    return;

                string header = checkbox.Content?.ToString() ?? "";
                DataGridColumn? column = playersDataGrid.Columns.FirstOrDefault(c => (c.Header ?? "").ToString() == header);

                if (column != null)
                {
                    column.Visibility = show ? Visibility.Visible : Visibility.Collapsed;

                    switch (header)
                    {
                        case "IP":
                            ConfigurationService.Upsert(config => config.ColumnIpEnabled, show);
                            break;
                        case "PING":
                            ConfigurationService.Upsert(config => config.ColumnPingEnabled, show);
                            break;
                        case "RELAY":
                            ConfigurationService.Upsert(config => config.ColumnRelayEnabled, show);
                            break;
                        case "COUNTRY":
                            ConfigurationService.Upsert(config => config.ColumnCountryEnabled, show);
                            break;
                        case "NAME":
                            ConfigurationService.Upsert(config => config.ColumnNameEnabled, show);
                            break;
                    }
                }
            }
        }

        public static class ThreadRunHelper
        {
            public static void RunInjectionThread(MainWindow mainWindow)
            {
                Task.Run(() =>
                {
                    while (true)
                    {
                        DynamicLinkLibraryInjectionService.Inject();

                        mainWindow.Dispatcher.Invoke(() =>
                        {
                            if (DynamicLinkLibraryInjectionService.IsDllInjected)
                            {
                                mainWindow.dllStatusLabel.Content = "LOADED";
                                mainWindow.dllStatusLabel.Foreground = Brushes.Lime;
                            }
                            else
                            {
                                mainWindow.dllStatusLabel.Content = "UNLOADED";
                                mainWindow.dllStatusLabel.Foreground = Brushes.Red;
                            }
                        });

                        Thread.Sleep(1000);
                    }
                });
            }
            public static void RunSharedMemoryThread(MainWindow mainWindow)
            {
                Task.Run(() =>
                {
                    while (true)
                    {
                        string data = SharedMemoryService.Read();

                        if (string.IsNullOrEmpty(data))
                        {
                            mainWindow.Dispatcher.Invoke(() =>
                            {
                                mainWindow._playerViewModel.ClearPlayers();
                                mainWindow.numPlayersLabel.Content = "0";
                            });
                        }
                        else
                        {
                            var players = JsonConvert.DeserializeObject<List<Player>>(data);
                           
                            mainWindow.Dispatcher.Invoke(() =>
                            {
                                if (players == null || players.Count < 1)
                                {
                                    mainWindow._playerViewModel.ClearPlayers();
                                    mainWindow.numPlayersLabel.Content = "0";
                                }
                                else
                                {
                                    mainWindow._playerViewModel.MergePlayers(players);
                                    mainWindow.numPlayersLabel.Content = mainWindow._playerViewModel.Players.Count.ToString();

                                    if(mainWindow.checkBoxLogging.IsChecked == true)
                                    {
                                        foreach (var player in mainWindow._playerViewModel.Players)
                                        {
                                            LogService.Insert(player);
                                        }
                                    }
                                }
                            });
                        }

                        Thread.Sleep(500);
                    }
                });
            }
        }
    }
}
