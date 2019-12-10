using System;

namespace uMCP
{
    [Flags]
    public enum uMCPPacketFlags
    {
        None,
        ACK = 1,
        REP = 2,
        STA = 4,
        STR = 8,
        DTA = 16,
        SEL = 32        
    }

    public enum uMCPPacketType : byte
    {
        ACK = (1 | 32),
        REP = (2 | 32),
        STA = (4 | 32),
        STR = (8 | 32),
        DTA = (1 | 16),
        DTE = (1 | 16 | 32),
        INVALID
    }

    public enum uMCPState
    {
        HALTED,
        ISTART,
        ASTART,
        RUNNING,
        INVALID
    }

    public class uMCPDataEventArgs : EventArgs
    {
        public byte[] Data { get; private set; }

        public uMCPDataEventArgs(byte[] data)
        {
            Data = data;
        }
    }

    public class uMCPActionInfoEventArgs : EventArgs
    {
        public string Info { get; private set; }

        public uMCPActionInfoEventArgs(string info)
        {
            Info = info;
        }
    }
    
    public static class uMCP
    {
        public static readonly int STASTR_HSIZE = 5;
        public static readonly int REP_HSIZE    = 6;
        public static readonly int ACKDTA_HSIZE = 7;
        public static readonly byte SIGN        = 0xAD;

        public static void Rise(this EventHandler handler, object sender, EventArgs e)
        {
            if (handler != null)
                handler(sender, e);
        }

        public static void Rise<TEventArgs>(this EventHandler<TEventArgs> handler,
            object sender, TEventArgs e) where TEventArgs : EventArgs
        {
            if (handler != null)
                handler(sender, e);
        }      


        
    }
}
