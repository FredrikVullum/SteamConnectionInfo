using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Collections.Generic;
using System.Threading;
using Newtonsoft.Json;

using SteamConnectionInfoCore.Services;
using SteamConnectionInfoCore.Models;
using SteamConnectionInfoCore.Views;
using System.Windows.Controls.Primitives;

namespace SteamConnectionInfoWpf
{
    public partial class MainWindow : Window
    {
        public PlayerViewModel _playerViewModel;
        public MainWindow()
        {
            InitializeComponent();
            _playerViewModel = new PlayerViewModel();
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);

            bool mutexCreated;
            Mutex mutex = new Mutex(true, "SteamConnectionInfoWPFAppMutex", out mutexCreated);
            if (!mutexCreated)
            {
                MessageBox.Show("An instance of SteamConnectionInfo.WPF is already running." +
                 " Only one instance of SteamConnectionInfo.WPF is allowed to run.",
                 "Duplicate Instance Detected",
                 MessageBoxButton.OK,
                 MessageBoxImage.Exclamation);

                mutex.Close();
                Application.Current.Shutdown();
                return;
            }

            playersDataGrid.DataContext = _playerViewModel;

            ConfigurationService.Load();

            Left = ConfigurationService.Get(config => config.WindowPositionX);
            Top = ConfigurationService.Get(config => config.WindowPositionY);

            int opacityToSet =  ConfigurationService.Get(config => config.WindowOpacity);
            Opacity = opacityToSet / 100.0;
            OpacityLabel.Content = $"{opacityToSet}%";
            OpacitySlider.Value = opacityToSet;

            ThreadRunHelper.RunInjectionThread();
            ThreadRunHelper.RunSharedMemoryThread(this);

            WindowServiceHelper.ShowHideWindow(this);
            WindowServiceHelper.UnlockLockWindow(this);

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

        public static class WindowServiceHelper        
        {
            private static bool WindowHiddenLoaded              = false;
            private static bool WindowTransparentForInputLoaded = false;
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
                    mainWindow.playersDataGrid.UnselectAll();
                }
                else
                {
                    WindowService.SetWindowExNotTransparent(WindowHandle);
                    mainWindow.windowLockLabel.Content = "UNLOCKED";
                    mainWindow.windowLockLabel.Foreground = Brushes.Lime;
                    mainWindow.ResizeMode = ResizeMode.CanResizeWithGrip;
                }

                mainWindow.UpdateLayout();

                WindowTransparentForInputLoaded = true;
            }
        }

        public static class ThreadRunHelper
        {
            public static void RunInjectionThread()
            {
                Task.Run(() =>
                {
                    while (true)
                    {
                        DynamicLinkLibraryInjectionService.Inject();
                        Thread.Sleep(2000);
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

                                }
                            });
                        }

                        Thread.Sleep(1000);
                    }
                });
            }
        }
    }
}
