using System.IO.MemoryMappedFiles;
using System.Text;

namespace SteamConnectionInfoCore.Services
{
    public static class SharedMemoryService
    {
        private const int                BufferSize      = 4096;
        private static readonly Encoding EncodingUTF8    = Encoding.UTF8;
        public static string Read()
        {
            string result = "";

            EventWaitHandle?          _producerReadyEvent;
            EventWaitHandle?          _consumerReadyEvent;
            MemoryMappedFile?         _memoryMappedFile;
            MemoryMappedViewAccessor? _memoryMappedViewAccessor;

            if (!EventWaitHandle.TryOpenExisting("ProducerReady", out _producerReadyEvent))
                return result;

            if (_producerReadyEvent.SafeWaitHandle.IsInvalid || _producerReadyEvent.SafeWaitHandle.IsClosed)
                return result;

            if (!EventWaitHandle.TryOpenExisting("ConsumerReady", out _consumerReadyEvent))
                return result;

            if (_consumerReadyEvent.SafeWaitHandle.IsInvalid || _consumerReadyEvent.SafeWaitHandle.IsClosed)
                return result;

            try
            {
                _memoryMappedFile = MemoryMappedFile.OpenExisting("SteamConnectionInfoSharedMemoryRegion", MemoryMappedFileRights.Read);
            }
            catch { return result; }

            if (_memoryMappedFile.SafeMemoryMappedFileHandle.IsInvalid || _memoryMappedFile.SafeMemoryMappedFileHandle.IsClosed)
                return result;

            try
            {
                _memoryMappedViewAccessor = _memoryMappedFile.CreateViewAccessor(0, BufferSize, MemoryMappedFileAccess.Read);
            }
            catch { return result; }
        
            if (_memoryMappedViewAccessor.SafeMemoryMappedViewHandle.IsInvalid || _memoryMappedViewAccessor.SafeMemoryMappedViewHandle.IsClosed)
                return result;

            try
            { 
                _consumerReadyEvent.Set();
            }
            catch { return result; }


            try {
                if (!_producerReadyEvent.WaitOne(TimeSpan.FromSeconds(1)))
                {
                    return result;
                }
            }
            catch { return result; }


            byte[] buffer = new byte[BufferSize];

            try
            {
                _memoryMappedViewAccessor.ReadArray(0, buffer, 0, BufferSize);
            }
            catch{ return result; }

            if (buffer.Length <= 0)
                return result;

            try
            {
                result = EncodingUTF8.GetString(buffer).TrimEnd('\0');
            }
            catch { return ""; }

            try
            {
                _producerReadyEvent.Reset();
            } catch { return result; }
            

            return result;
        }
    }
}
