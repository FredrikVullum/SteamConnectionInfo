﻿using SteamConnectionInfoCore.Models;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace SteamConnectionInfoCore.Views
{
    public class PlayerViewModel : INotifyPropertyChanged
    {
        private ObservableCollection<Player>? _players;

        public ObservableCollection<Player> Players
        {
            get => _players!;
            set
            {
                _players = value;
                OnPropertyChanged();
            }
        }

        public PlayerViewModel()
        {
            Players = new ObservableCollection<Player>();
        }

        public void AddPlayer(Player player)
        {
            Players.Add(player);
        }

        public void RemovePlayer(Player player)
        {
            Players.Remove(player);
        }

        public void UpdatePlayer(Player player)
        {
            var existingPlayer = Players.FirstOrDefault(p => p.SteamId == player.SteamId);
            if (existingPlayer != null)
            {
                if(existingPlayer.SteamIp != player.SteamIp)
                    existingPlayer.SteamIp = player.SteamIp;

                if (existingPlayer.SteamPort != player.SteamPort)
                    existingPlayer.SteamPort = player.SteamPort;

                if (existingPlayer.Ping != player.Ping)
                    existingPlayer.Ping = player.Ping;

                if (existingPlayer.SteamName != player.SteamName)
                    existingPlayer.SteamName = player.SteamName;

                if (existingPlayer.Country != player.Country)
                    existingPlayer.Country = player.Country;
            }
            else
            {
                Players.Add(player);
            }
        }

        public void DeletePlayer(Player player)
        {
            Players.Remove(player);
        }

        public void MergePlayers(IEnumerable<Player> players)
        {
            foreach (var player in players.Where(p => !Players.Any(p2 => p2.SteamId == p.SteamId)))
            {
                AddPlayer(player);
            }

            foreach (var player in Players.Where(p => !players.Any(p2 => p2.SteamId == p.SteamId)).ToList())
            {
                RemovePlayer(player);
            }

            foreach (var player in players.Where(p => Players.Any(p2 => p2.SteamId == p.SteamId)))
            {
                UpdatePlayer(player);
            }
        }

        public void ClearPlayers()
        {
            if (Players.Count > 0)
            {
                Players.Clear();
            }
        }
        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
