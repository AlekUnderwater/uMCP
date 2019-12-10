using System;
using System.Collections.Generic;

namespace uMCP
{    
    /// <summary>    
    ///  Point-to-point uMCP node
    /// </summary>

    public class uMCPPPNode
    {
        #region Properties

        byte R = 0;
        byte N = 0;        
        byte A = 0;

        uMCPState state = uMCPState.HALTED;
        public uMCPState STATE
        {
            get { return state; }
            private set
            {
                if (value != state)
                {
                    state = value;
                    if (state == uMCPState.ISTART)
                    {
                        R = 0;
                        N = 0;                        
                        A = 0;
                    }

                    OnSTATEChangedEventHandler.Rise(this, new EventArgs());
                }
            }
        }

        DateTime timeoutTimerStartTime;
        bool isTimeoutTimerRunning = false;
        bool IsTimeoutTimerRunning
        {
            get { return isTimeoutTimerRunning; }
            set
            {
                if (value)
                    timeoutTimerStartTime = DateTime.Now;
                else
                {
                    // in case transmission passed faster than expected
                    if (isTxFinishedTimerRunning)
                        IsTxFinishedTimerRunning = false;
                }

                isTimeoutTimerRunning = value;                
            }
        }
        
        DateTime selectTimerStartTime;
        bool isSelectTimerRunning = false;
        bool IsSELECTTimerRunning
        {
            get { return isSelectTimerRunning; }
            set
            {
                if (value)
                    selectTimerStartTime = DateTime.Now;
                isSelectTimerRunning = value;                
            }
        }

        bool selectDefaultState = false;
        bool select = false;
        public bool SELECT
        {
            get { return select; }
            private set
            {
                if (selectDefaultState)
                    IsSELECTTimerRunning = (value != selectDefaultState);

                select = value;

                if (select)
                    PerformProtocol();

                OnSELECTChangedEventHandler.Rise(this, new EventArgs());
            }
        }

        bool isTimerStartPendingOnTxFinish = false;

        public byte ID { get; private set; }

        public byte TID { get; private set; }

        uMCPReceiver receiver;

        private delegate void packetProcessorDelegate(uMCPPacket packet);
        private Dictionary<uMCPPacketType, packetProcessorDelegate> onIncomingPacket;
        
        uint timeoutIntervalMs = 1000;
        uint selectIntervalMs = 2000;

        uMCPPacket sentPacket;

        TSQueue<byte[]> dataToSend;
        Dictionary<int, byte[]> sentDataBlocks;

        public byte PacketSize { get; private set; }
        
        double fixedTxDelayS = 0.05;
        double baudRateBps = 9600;
        public double BaudrateBps
        {
            get { return baudRateBps; }
            set
            {
                if ((value > 0) && (value <= 115200))
                    baudRateBps = value;
                else
                    throw new ArgumentOutOfRangeException();
            }
        }

        DateTime txFinishedTimerStartTime;
        uint txFinishedInterval = 1000;
        bool isTxFinishedTimerRunning = false;
        bool IsTxFinishedTimerRunning
        {
            get { return isTxFinishedTimerRunning; }
            set
            {
                if (value)
                    txFinishedTimerStartTime = DateTime.Now;

                isTxFinishedTimerRunning = value;
            }
        }
        
        bool SREP = false;
        bool SACK = false;

        public uint DTADTESent { get; private set; }
        public uint REPSent { get; private set; }
        public uint ACKSent { get; private set; }
        public uint Timeouts { get; private set; }
        public uint BytesTransmitted { get; private set; }        

        public uint DTADTEReceived { get; private set; }
        public uint REPReceived { get; private set; }
        public uint ACKReceived { get; private set; }
        public uint BytesReceived { get; private set; }

        int pipeliningLimit = 8;
        public int PipeliningLimit
        {
            get { return pipeliningLimit; }
            set
            {
                if ((value >= 1) && (value < 128))
                    pipeliningLimit = value;
                else
                    throw new ArgumentOutOfRangeException();
            }
        }

        #endregion

        #region Constructor

        public uMCPPPNode(byte id, bool selDefState, uint toutIntervalMs, uint selIntervalMs, byte packetSize)
        {
            ID = id;

            selectDefaultState = selDefState;
            timeoutIntervalMs = toutIntervalMs;
            selectIntervalMs = selIntervalMs;

            PacketSize = packetSize;

            receiver = new uMCPReceiver(1024);

            onIncomingPacket = new Dictionary<uMCPPacketType, packetProcessorDelegate>();
            onIncomingPacket.Add(uMCPPacketType.STR, onSTR);
            onIncomingPacket.Add(uMCPPacketType.STA, onSTA);
            onIncomingPacket.Add(uMCPPacketType.REP, onREP);
            onIncomingPacket.Add(uMCPPacketType.ACK, onACK);
            onIncomingPacket.Add(uMCPPacketType.DTA, onDTADTE);
            onIncomingPacket.Add(uMCPPacketType.DTE, onDTADTE);

            dataToSend = new TSQueue<byte[]>();
            sentDataBlocks = new Dictionary<int, byte[]>();

            DTADTESent = 0;
            REPSent = 0;
            ACKSent = 0;
            Timeouts = 0;
            BytesTransmitted = 0;
            
            DTADTEReceived = 0;
            REPReceived = 0;
            ACKReceived = 0;
            BytesReceived = 0;
        }

        #endregion

        #region Methods

        #region Public

        public void Start(byte tID)
        {
            if (state == uMCPState.HALTED)
            {
                TID = tID;
                STATE = uMCPState.ISTART;
                SendPacket(new uMCPSTPacket(uMCPPacketType.STR, ID, TID), true);                
            }
            else
                throw new InvalidOperationException("Protocol should be in HALTED state to perform such operation");
        }

        public void Stop()
        {
            STATE = uMCPState.HALTED;            
        }

        public void OnIncomingData(byte[] data)
        {
            if (receiver.InputDataProcess(data) > 0)
            {
                var packets = receiver.ReadAllPackets();
                foreach (var packet in packets)
                    if ((packet.TID == ID) && (onIncomingPacket.ContainsKey(packet.PTYPE)))
                    {
                        OnActionInfoEventHandler.Rise(this, new uMCPActionInfoEventArgs(string.Format("Received: {0}", packet.ToString())));
                        onIncomingPacket[packet.PTYPE](packet);
                    }
            }
        }

        public void OnTransmissionFinished()
        {
            if (isTimerStartPendingOnTxFinish && !isTimeoutTimerRunning)
            {
                isTimerStartPendingOnTxFinish = false;
                IsTimeoutTimerRunning = true;
            }
            else
                PerformProtocol();
        }


        public bool IsCanSend(int dataSize)
        {
            return (sentDataBlocks.Count + dataSize / PacketSize < 255);
        }

        public bool Send(byte[] data)
        {
            if (IsCanSend(data.Length))
            {
                if (data.Length <= PacketSize)
                    dataToSend.Enqueue(data);
                else
                {
                    int rPos = 0;
                    int step = PacketSize;
                    while (rPos < data.Length)
                    {
                        if (data.Length - rPos >= PacketSize)
                            step = PacketSize;
                        else
                            step = data.Length - rPos;

                        byte[] block = new byte[step];
                        for (int i = 0; i < step; i++)
                            block[i] = data[rPos + i];

                        rPos += step;
                        dataToSend.Enqueue(block);
                    }
                }

                PerformProtocol();

                return true;
            }
            else
            {
                return false;
            }
        }

        public void OnTick()
        {
            DateTime now = DateTime.Now;

            if ((isSelectTimerRunning) &&
                (now.Subtract(selectTimerStartTime).TotalMilliseconds > selectIntervalMs))
            {
                SELECT = selectDefaultState;                
            }

            if ((isTimeoutTimerRunning) &&
                (now.Subtract(timeoutTimerStartTime).TotalMilliseconds > timeoutIntervalMs))
            {
                IsTimeoutTimerRunning = false;
                OnTimeout();

                Timeouts++;
            }

            if ((isTxFinishedTimerRunning) &&
                (now.Subtract(txFinishedTimerStartTime).TotalMilliseconds > txFinishedInterval))
            {
                IsTxFinishedTimerRunning = false;
                OnTransmissionFinished();
            }            
        }

        public override string ToString()
        {
            return string.Format("R={0}, N={1}, A={2}, SACK={3}, SREP={4}, SELECT={5}, STATE={6}",
                R, N, A, SACK, SREP, SELECT, STATE);
        }

        #endregion

        #region Private

        private void SendPacket(uMCPPacket packet, bool isStartTimer)
        {
            OnActionInfoEventHandler.Rise(this, new uMCPActionInfoEventArgs(string.Format("Sending: {0}", packet.ToString())));

            var serializedPacket = packet.Serialize();
            OnOutcomingEventHandler.Rise(this, new uMCPDataEventArgs(serializedPacket));
            sentPacket = packet;
            isTimerStartPendingOnTxFinish = isStartTimer;
            txFinishedInterval = Convert.ToUInt32(1000 * (fixedTxDelayS + serializedPacket.Length * 8 / baudRateBps));
            IsTxFinishedTimerRunning = true;

            SELECT = (packet.PTYPE == uMCPPacketType.DTA);
            //  if (packet.PTYPE != uMCPPacketType.DTA) // sending SELECT flag
            //     SELECT = false;

            if (packet.PTYPE == uMCPPacketType.ACK)
                ACKSent++;
            else if ((packet.PTYPE == uMCPPacketType.DTA) || (packet.PTYPE == uMCPPacketType.DTE))
                DTADTESent++;
            else if (packet.PTYPE == uMCPPacketType.REP)
                REPSent++;
        }

        private void OnTimeout()
        {
            OnActionInfoEventHandler.Rise(this, new uMCPActionInfoEventArgs("TIMEOUT"));

            if (state == uMCPState.ISTART)
                SendPacket(new uMCPSTPacket(uMCPPacketType.STR, ID, TID), true);
            else if (state == uMCPState.ASTART)
                SendPacket(new uMCPSTPacket(uMCPPacketType.STA, ID, TID), true);
            else if (state == uMCPState.RUNNING)
            {
                SREP = true;
                PerformProtocol();
            }
        }

        private void PerformProtocol()
        {
            if (state == uMCPState.RUNNING)
            {
                if ((!isTxFinishedTimerRunning) &&
                    (!isTimeoutTimerRunning) &&
                    (select))
                {
                    if (dataToSend.Count == 0)
                    {
                        if (SREP)
                        {
                            SendPacket(new uMCPREPPacket(ID, TID, N), true);
                            SREP = false;
                        }
                        else if (sentDataBlocks.Count > 0)
                        {
                            byte x = Convert.ToByte((A + 1) % 256);
                            SendPacket(new uMCPDATAPacket(ID, TID, R, x, sentDataBlocks[x], true), true);
                        }
                        else if ((!selectDefaultState) || (SACK))
                        {
                            SendPacket(new uMCPACKPacket(ID, TID, R, N), false);
                            SACK = false;
                        }                                         
                    }
                    else
                    {
                        var blockToSend = dataToSend.Dequeue();
                        N++;
                        sentDataBlocks.Add(N, blockToSend);
                        bool isDTE = (dataToSend.Count == 0) || (sentDataBlocks.Count >= pipeliningLimit);
                        SendPacket(new uMCPDATAPacket(ID, TID, R, N, blockToSend, isDTE), isDTE);
                    }
                }
            }             
        }

        private void AcknowledgeSentItems(int to)
        {
            for (byte a = A; a != to; a++)
            {
                A++;
                var acknowledgedBlock = sentDataBlocks[A];
                BytesTransmitted += Convert.ToUInt32(acknowledgedBlock.Length);
                sentDataBlocks.Remove(A);
                OnDataBlockAcknowledgedEventHandler.Rise(this, new uMCPDataEventArgs(acknowledgedBlock));
            }

            if (sentDataBlocks.Count == 0)
                OnTransmitterEmptyEventHandler.Rise(this, new EventArgs()); 
        }

        private bool IsByteInRangeExclusive(byte st, byte nd, byte val)
        {
            bool result = false;
            byte idx = st;            
            byte _nd = nd;
            _nd--;
            while ((idx != _nd) && (!result))
            {
                idx++;
                if (idx == val)
                    result = true;                
            }

            return result;
        }

        #region Custom packet processor

        private void onSTR(uMCPPacket packet)
        {
            SELECT = true;
            if ((state == uMCPState.HALTED) || (state == uMCPState.ISTART))
            {
                #region state == HALTED || ISTART

                STATE = uMCPState.ASTART;
                IsTimeoutTimerRunning = false;
                SendPacket(new uMCPSTPacket(uMCPPacketType.STA, ID, TID), true);

                #endregion
            }
            else if (state == uMCPState.ASTART)
            {
                #region state == ASTART

                STATE = uMCPState.RUNNING;
                IsTimeoutTimerRunning = false;
                SendPacket(new uMCPACKPacket(ID, TID, 0, 0), false);

                #endregion
            }
            else
            {
                STATE = uMCPState.HALTED;
            }            
        }

        private void onSTA(uMCPPacket packet)
        {
            SELECT = true;

            if ((state == uMCPState.ISTART) || (state == uMCPState.ASTART) || (state == uMCPState.RUNNING))
            {
                STATE = uMCPState.RUNNING;
                IsTimeoutTimerRunning = false;
                SendPacket(new uMCPACKPacket(ID, TID, 0, 0), false);                
            }
        }

        private void onREP(uMCPPacket packet)
        {
            REPReceived++;

            if (state == uMCPState.RUNNING)
            {
                IsTimeoutTimerRunning = false;
                SACK = true;
                SREP = false;
                SELECT = true;                
            }
        }

        private void onACK(uMCPPacket packet)
        {
            ACKReceived++;

            uMCPACKPacket aPacket = (packet as uMCPACKPacket);
            if (state == uMCPState.ASTART)
            {
                #region state == ASTART

                if (aPacket.RCNT == 0)
                {
                    IsTimeoutTimerRunning = false;
                    STATE = uMCPState.RUNNING;
                }
                
                #endregion
            }
            else if (state == uMCPState.RUNNING)
            {
                #region state == RUNNING

                SREP = false;
                IsTimeoutTimerRunning = false;
                if ((aPacket.RCNT == N) || (IsByteInRangeExclusive(A, N, aPacket.RCNT)))
                    AcknowledgeSentItems(aPacket.RCNT);

                #endregion
            }

            SELECT = true;
        }

        private void onDTADTE(uMCPPacket packet)
        {
            DTADTEReceived++;

            if (state == uMCPState.RUNNING)
            {
                uMCPDATAPacket dPacket = (packet as uMCPDATAPacket);
                                
                if (dPacket.TCNT <= R + 1)
                {
                    if (dPacket.TCNT == Convert.ToByte((R + 1) % 256))
                    {
                        R++;
                        OnDataBlockReceivedEventHandler.Rise(this, new uMCPDataEventArgs(dPacket.DATA));
                        BytesReceived += Convert.ToUInt32(dPacket.DATA.Length);
                    }

                    SACK = true;
                }

                SREP = false;
                IsTimeoutTimerRunning = false;
                            
                if ((dPacket.RCNT == N) || (IsByteInRangeExclusive(A, N, dPacket.RCNT)))
                    AcknowledgeSentItems(dPacket.RCNT);

                if (dPacket.PTYPE == uMCPPacketType.DTA)
                    IsTimeoutTimerRunning = true;

                SELECT = (dPacket.PTYPE == uMCPPacketType.DTE);
            }
        }

        #endregion

        #endregion

        #endregion

        #region Events

        public EventHandler OnSTATEChangedEventHandler;
        public EventHandler OnSELECTChangedEventHandler;

        public EventHandler<uMCPDataEventArgs> OnDataBlockReceivedEventHandler;
        public EventHandler<uMCPDataEventArgs> OnOutcomingEventHandler;
        public EventHandler<uMCPDataEventArgs> OnDataBlockAcknowledgedEventHandler;

        public EventHandler OnTransmitterEmptyEventHandler;

        public EventHandler<uMCPActionInfoEventArgs> OnActionInfoEventHandler;

        #endregion
    }
}
