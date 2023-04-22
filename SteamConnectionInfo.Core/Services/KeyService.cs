using System.Diagnostics;
using System.Runtime.InteropServices;

namespace SteamConnectionInfoCore.Services
{
    public class KeyService
    {
        public const int                    VK_INSERT = 0x2D;
        public const int                    VK_HOME   = 0x24;
      
        private static LowLevelKeyboardProc _proc     = HookCallback;
        private static IntPtr               _hookId   = IntPtr.Zero;
        private static Action<int>?         _callback;

        private delegate IntPtr LowLevelKeyboardProc(int nCode, IntPtr wParam, IntPtr lParam);

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetModuleHandle(string? lpModuleName);
        [DllImport("user32.dll")]
        private static extern IntPtr SetWindowsHookEx(int idHook, LowLevelKeyboardProc lpfn, IntPtr hMod, uint dwThreadId);
        [DllImport("user32.dll")]
        private static extern bool UnhookWindowsHookEx(IntPtr hhk);
        [DllImport("user32.dll")]
        private static extern IntPtr CallNextHookEx(IntPtr hhk, int nCode, IntPtr wParam, IntPtr lParam);
        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern short GetAsyncKeyState(int vKey);

        public static void RegisterGlobalKeyPress(Action<int> callback)
        {
            _callback = callback;
            _hookId = SetHook(_proc);
        }
        private static IntPtr SetHook(LowLevelKeyboardProc proc)
        {
            using (Process curProcess = Process.GetCurrentProcess())
            {
                ProcessModule? curModule = curProcess.MainModule;
                if (curModule != null)
                {
                    return SetWindowsHookEx(13, proc, GetModuleHandle(curModule.ModuleName), 0);
                }
                else
                {
                    throw new Exception("Failed to get the main module for the current process");
                }
            }
        }
        private static IntPtr HookCallback(int nCode, IntPtr wParam, IntPtr lParam)
        {
            if (nCode >= 0 && wParam == (IntPtr)0x100) // Key down
            {
                int vkCode = Marshal.ReadInt32(lParam);
                if (vkCode == VK_INSERT)
                    _callback?.Invoke(vkCode);
                if (vkCode == VK_HOME)
                    _callback?.Invoke(vkCode);
            }
            return CallNextHookEx(_hookId, nCode, wParam, lParam);
        }
    }
}
