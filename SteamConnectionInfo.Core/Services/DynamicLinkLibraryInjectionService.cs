using System.Diagnostics;
using System.Runtime.InteropServices;

namespace SteamConnectionInfoCore.Services
{
    public static class DynamicLinkLibraryInjectionService
    {
        private const uint PROCESS_CREATE_THREAD     = 0x0002;
        private const uint PROCESS_QUERY_INFORMATION = 0x0400;
        private const uint PROCESS_VM_OPERATION      = 0x0008;
        private const uint PROCESS_VM_WRITE          = 0x0020;
        private const uint PROCESS_VM_READ           = 0x0010;
        private const uint MEM_COMMIT                = 0x00001000;
        private const uint MEM_RESERVE               = 0x00002000;
        private const uint PAGE_READWRITE            = 0x04;

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetModuleHandle(string? lpModuleName);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, int dwProcessId);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out IntPtr lpNumberOfBytesWritten);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttribute, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

        public static void Inject()
        {
            Process? steamProcess = Process.GetProcessesByName("steam").FirstOrDefault();

            if (steamProcess == null)
            {
                return;
            }

            string dllPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "SteamConnectionInfo.dll");

            if (!File.Exists(dllPath))
            {
                return;
            }

            foreach (ProcessModule module in steamProcess.Modules)
            {
                if (string.Equals(module.FileName, dllPath, StringComparison.OrdinalIgnoreCase))
                {
                    return;
                }
            }

            IntPtr processHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, steamProcess.Id);

            if (processHandle == IntPtr.Zero)
            {

                return;
            }

            IntPtr loadLibraryAddress = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

            if (loadLibraryAddress == IntPtr.Zero)
            {
                return;
            }

            byte[] dllPathBytes = System.Text.Encoding.ASCII.GetBytes(dllPath);

            IntPtr dllPathAddress = VirtualAllocEx(processHandle, IntPtr.Zero, (uint)dllPathBytes.Length, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

            if (dllPathAddress == IntPtr.Zero)
            {
                return;
            }

            IntPtr bytesWritten;
            if (!WriteProcessMemory(processHandle, dllPathAddress, dllPathBytes, (uint)dllPathBytes.Length, out bytesWritten))
            {
                return;
            }

            if (bytesWritten.ToInt32() != dllPathBytes.Length)
            {
                return;
            }

            if (CreateRemoteThread(processHandle, IntPtr.Zero, 0, loadLibraryAddress, dllPathAddress, 0, IntPtr.Zero) == IntPtr.Zero)
            {
                return;
            }
        }
    }
}
