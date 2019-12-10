using System;
using System.Collections.Generic;

namespace uMCP
{
    /// <summary>
    ///
    /// </summary>

    public class uMCPSTPacket : uMCPPacket
    {
        public uMCPSTPacket(uMCPPacketType pType, byte sID, byte tID) 
            : base(pType, sID, tID)
        {
            if ((pType != uMCPPacketType.STA) &&
                (pType != uMCPPacketType.STR))
                throw new ArgumentOutOfRangeException("pType");
        }

        public override string ToString()
        {
            return PTYPE.ToString();
        }
    }   

    public class uMCPACKPacket : uMCPPacket
    {
        public byte RCNT
        {
            get { return base.rCNT; }
        }

        public byte TCNT
        {
            get { return base.tCNT; }
        }

        public uMCPACKPacket(byte sID, byte tID, byte rCnt, byte tCnt) : 
            base(uMCPPacketType.ACK, sID, tID)
        {
            base.rCNT = rCnt;
            base.tCNT = tCnt;
        }

        public override string ToString()
        {
            return string.Format("{0}(SID={1}, TID={2}, RCNT={3}, TCNT={4})", uMCPPacketType.ACK, SID, TID, RCNT, TCNT);
        }
    }

    public class uMCPREPPacket : uMCPPacket
    {
        public byte TCNT
        {
            get { return base.tCNT; }
        }

        public uMCPREPPacket(byte sID, byte tID, byte tCnt)
            : base(uMCPPacketType.REP, sID, tID)
        {
            base.tCNT = tCnt;
        }

        public override string ToString()
        {
            return string.Format("{0}(SID={1}, TID={2}, TCNT={3})", uMCPPacketType.REP, SID, TID, TCNT);
        }
    }

    public class uMCPDATAPacket : uMCPPacket
    {
        public byte RCNT { get { return base.rCNT; } }
        public byte TCNT { get { return base.tCNT; } }
        public byte[] DATA { get { return base.dATA; } }

        public uMCPDATAPacket(byte sID, byte tID, byte rCnt, byte tCnt, byte[] data, bool isSel)
            : base(isSel ? uMCPPacketType.DTE : uMCPPacketType.DTA, sID, tID)
        {
            base.rCNT = rCnt;
            base.tCNT = tCnt;
            base.dATA = data;            
        }

        public override string ToString()
        {
            return string.Format("{0}(SID={1}, TID={2}, RCNT={3}, TCNT={4})", PTYPE, SID, TID, RCNT, TCNT);
        }
    }

    public abstract class uMCPPacket
    {
        #region Properties

        public uMCPPacketType PTYPE { get; private set; }
        public byte SID { get; private set; }
        public byte TID { get; private set; }
        protected byte rCNT { get; set; }
        protected byte tCNT { get; set; }
        protected byte dCNT { get; set; }
        protected byte[] dATA { get; set; }

        public static readonly int MIN_SIZE = 5;
        public static readonly int SID_OFFSET = 2;
        public static readonly int TID_OFFSET = 3;
        public static readonly int RCNT_OFFSET = 4;
        public static readonly int TCNT_OFFSET = 5;

        #endregion

        #region Constructor

        public uMCPPacket(uMCPPacketType pType, byte sid, byte tid)
        {
            PTYPE = pType;
            SID = sid;
            TID = tid;
        }
       
        #endregion

        #region Methods
       
        public byte[] Serialize()
        {
            List<byte> result = new List<byte>();
            result.Add(uMCP.SIGN);
            result.Add((byte)PTYPE);
            result.Add(SID);
            result.Add(TID);
            
            if (PTYPE == uMCPPacketType.REP)
            {
                result.Add(tCNT);
            }
            else if ((PTYPE == uMCPPacketType.ACK) ||
                     (PTYPE == uMCPPacketType.DTA) ||
                     (PTYPE == uMCPPacketType.DTE))
            {
                result.Add(tCNT);
                result.Add(rCNT);
            }

            result.Add(CRC.CRC8_Get(result, 0, result.Count));

            if ((PTYPE == uMCPPacketType.DTA) ||
                (PTYPE == uMCPPacketType.DTE))
            {
                int dtaStartOffset = result.Count;
                result.Add(Convert.ToByte(dATA.Length));
                result.AddRange(dATA);
                result.Add(CRC.CRC8_Get(result, dtaStartOffset, dATA.Length + 1));
            }

            return result.ToArray();
        }

        #endregion
    }
}
