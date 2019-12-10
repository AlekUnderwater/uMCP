using System;
using System.Collections.Generic;
using System.IO.Ports;
using UCNLDrivers;
using UCNLNMEA;

namespace uMCP
{
    public enum uMCP_LERR_Enum
    {
        LERR_OK = 0,
        LERR_ARGUMENT_OUT_OF_RANGE = 1,
        LERR_CHECKSUM = 2,
        LERR_UNSUPPORTED = 3,
        LERR_UNKNOWN
    }

    public class PortACKEventArgs : EventArgs
    {
        public uMCP_LERR_Enum ErrorCode { get; private set; }
        public string SentenceID { get; private set; }

        public PortACKEventArgs(string sntID, uMCP_LERR_Enum errCode)
        {
            ErrorCode = errCode;
            SentenceID = sntID;
        }
    }

    public class uMCPNMEAPort
    {
        #region Properties

        NMEASerialPort port;
        PrecisionTimer timer;

        static bool nmeaSingleton = false;

        private delegate void parserDelegate(object[] parameters);
        private Dictionary<string, parserDelegate> parsers;

        delegate T NullChecker<T>(object parameter);
        NullChecker<int> intNullChecker = (x => x == null ? -1 : (int)x);
        NullChecker<double> doubleNullChecker = (x => x == null ? double.NaN : (double)x);
        NullChecker<string> stringNullChecker = (x => x == null ? string.Empty : (string)x);

        TSQueue<byte[]> dataToSend;

        bool isWaiting = true;
        public bool IsWaiting
        {
            get { return isWaiting; }
            private set
            {
                isWaiting = value;

                if (timer.IsRunning)
                    timer.Stop();

                if (!isWaiting)
                    OnPortFree();
                else
                    timer.Start();

                PortIsWatingChangedEventHandler.Rise(this, new EventArgs());
            }
        }

        #endregion

        #region Constructor

        public uMCPNMEAPort(SerialPortSettings portSettings)
        {
            #region Parsers

            parsers = new Dictionary<string, parserDelegate>()
            {
                { "0", new parserDelegate(LACK_Parse) },
                { "2", new parserDelegate(RACK_Parse) },
                { "3", new parserDelegate(RPKT_Parse) },
                { "5", new parserDelegate(STAT_Parse) }
            };

            #endregion

            #region port

            port = new NMEASerialPort(portSettings);

            port.PortError += (o, e) => { PortErrorEventHandler.Rise(this, e); };
            port.NewNMEAMessage += new EventHandler<NewNMEAMessageEventArgs>(port_MessageReceived);

            #endregion

            #region nmea

            if (!nmeaSingleton)
            {
                NMEAParser.AddManufacturerToProprietarySentencesBase(ManufacturerCodes.MCP);
                NMEAParser.AddProprietarySentenceDescription(ManufacturerCodes.MCP, "0", "c--c,x");
                NMEAParser.AddProprietarySentenceDescription(ManufacturerCodes.MCP, "1", "x,x,x,x,x");
                NMEAParser.AddProprietarySentenceDescription(ManufacturerCodes.MCP, "2", "h--h");
                NMEAParser.AddProprietarySentenceDescription(ManufacturerCodes.MCP, "3", "h--h");
                NMEAParser.AddProprietarySentenceDescription(ManufacturerCodes.MCP, "4", "h--h");
                NMEAParser.AddProprietarySentenceDescription(ManufacturerCodes.MCP, "5", "x,x");

                //#define IC_D2H_LACK             '0'        // $PMCP0,sentenceID,errCode  - local command ACK
                //#define IC_H2D_STRT             '1'        // $PMCP1,senderID,targetID,selectDefState,selIntMs,toutIntMs - restart protocol with specified params
                //#define IC_D2H_RACK             '2'        // $PMCP2,h--h // sent packet acknowledged
                //#define IC_D2H_RPKT             '3'        // $PMCP3,h--h // packet received
                //#define IC_H2D_SPKT             '4'        // $PMCP4,h--h // send packet
                //#define IC_D2H_STAT             '5'        // $PMCP5,state,select // protocol state changed
            }

            #endregion

            #region timer

            timer = new PrecisionTimer();
            timer.Period = 1000;
            timer.Mode = Mode.OneShot;
            timer.Tick += (o, e) => { PortTimeoutEventHandler.Rise(this, new EventArgs()); IsWaiting = false; };

            #endregion

            #region other

            dataToSend = new TSQueue<byte[]>(128, 128);
            dataToSend.ItemEnqueued += (o, e) => { if (!isWaiting) OnPortFree(); };

            #endregion
        }

        #endregion

        #region Methods

        #region Public

        public void Open()
        {
            port.Open();
        }

        public void Close()
        {
            port.Close();
        }
        
        public void Start(byte SID, byte TID, bool selectDefaultState, int selIntMs, int toutIntMs)
        {
            if (!isWaiting)
            {
                var msg = NMEAParser.BuildProprietarySentence(ManufacturerCodes.MCP, "1",
                    new object[] { SID, TID, Convert.ToInt32(selectDefaultState), selIntMs, toutIntMs });
                port.SendData(msg);

                isWaiting = true;
            }
        }

        public void Send(byte[] data)
        {
            dataToSend.Enqueue(data);           
        }

        private void OnPortFree()
        {
            if (dataToSend.Count > 0)
            {
                var msg = NMEAParser.BuildProprietarySentence(ManufacturerCodes.MCP, "4", new object[] { dataToSend.Dequeue() });
                port.SendData(msg);
                IsWaiting = true;
            }
        }

        #endregion

        #region parsers

        private void LACK_Parse(object[] parameters)
        {
            string sntID = (string)parameters[0];
            uMCP_LERR_Enum errCode = (uMCP_LERR_Enum)Enum.ToObject(typeof(uMCP_LERR_Enum), (int)parameters[1]);
            PortACKReceivedEventHandler.Rise(this, new PortACKEventArgs(sntID, errCode));
            IsWaiting = false;
        }

        private void RACK_Parse(object[] parameters)
        {
            throw new NotImplementedException();
        }

        private void RPKT_Parse(object[] parameters)
        {            
            PacketReceivedEventHandler.Rise(this, new uMCPDataEventArgs((byte[])parameters[0]));
        }

        private void STAT_Parse(object[] parameters)
        {
            throw new NotImplementedException();
        }

        #endregion

        #endregion

        #region Handlers

        private void port_MessageReceived(object sender, NewNMEAMessageEventArgs e)
        {
            NMEASentence sentence = null;
            bool isParsed = false;

            try
            {
                sentence = NMEAParser.Parse(e.Message);
                isParsed = true;
            }
            catch (Exception ex)
            {
                /// TODO:
            }

            if (isParsed)
            {
                if (sentence is NMEAProprietarySentence)
                {
                    var pSentence = (sentence as NMEAProprietarySentence);
                    if (pSentence.Manufacturer == ManufacturerCodes.MCP)
                    {
                        if (parsers.ContainsKey(pSentence.SentenceIDString))
                            parsers[pSentence.SentenceIDString](pSentence.parameters);
                    }
                    else
                    {
                        /// TODO: skip unsupported manufacturer
                    }
                }
                else
                {
                    /// TODO: skip standard sentence
                }
            }
        }

        #endregion

        #region Events

        public EventHandler<SerialErrorReceivedEventArgs> PortErrorEventHandler;
        public EventHandler<uMCPDataEventArgs> PacketReceivedEventHandler;
        public EventHandler<PortACKEventArgs> PortACKReceivedEventHandler;
        public EventHandler PortIsWatingChangedEventHandler;
        public EventHandler PortTimeoutEventHandler;

        #endregion
    }
}
