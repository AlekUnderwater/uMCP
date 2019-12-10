using System;
using System.IO.Ports;
using System.Threading;
using UCNLDrivers;

namespace uMCP
{
    public class uMCPPort : IDisposable
    {
        #region Properties

        SerialPort port;
        uMCPPPNode node;
        PrecisionTimer timer;

        public bool IsOpen { get { return port.IsOpen; } }
        public uMCPState State { get { return node.STATE; } }
        public bool Select { get { return node.SELECT; } }

        int portLock = 0;

        public byte ID { get { return node.ID; } }

        bool disposing = false;

        #endregion

        #region Constructor

        public uMCPPort(string portName, BaudRate baudrate, byte id, bool selectDefaultState, uint selectIntMs, uint timeoutIntMs, byte packetSize)
        {
            timer = new PrecisionTimer();
            timer.Period = 100;
            timer.Mode = Mode.Periodic;
            timer.Tick += (o, e) => { node.OnTick(); };

            node = new uMCPPPNode(id, selectDefaultState, timeoutIntMs, selectIntMs, packetSize);
            node.OnSTATEChangedEventHandler += (o, e) => { OnStateChangedEventHandler.Rise(this, e); };
            node.OnSELECTChangedEventHandler += (o, e) => { OnSelectChangedEventHandler.Rise(this, e); };
            node.OnDataBlockAcknowledgedEventHandler += (o, e) => { OnDataBlockAcknowledgedEventHandler.Rise(this, e); };
            node.OnDataBlockReceivedEventHandler += (o, e) => { OnDataBlockReceivedEventHandler.Rise(this, e); };
            node.OnActionInfoEventHandler += (o, e) => { OnActionInfoEventHandler.Rise(this, e); };
            node.OnTransmitterEmptyEventHandler += (o, e) => { OnTransmitterEmptyEventHandler.Rise(this, e); };
            node.OnOutcomingEventHandler += new EventHandler<uMCPDataEventArgs>(node_OnOutcomingEventHandler);

            port = new SerialPort(portName, (int)baudrate);
            port.DataReceived += (o, e) =>
                {
                    var data = new byte[port.BytesToRead];
                    port.Read(data, 0, data.Length);
                    node.OnIncomingData(data);
                };

            port.ErrorReceived += (o, e) => { PortErrorEventHandler.Rise(this, e); };                        
        }

        #endregion

        #region Methods

        public void Start()
        {
            if (port.IsOpen)
                node.Start(node.ID);
            else
                throw new InvalidOperationException("Port should be opened first");
        }

        public void Stop()
        {
            node.Stop();
        }

        public void Open()
        {
            while (Interlocked.CompareExchange(ref portLock, 1, 0) != 0)
                Thread.SpinWait(1);

            port.Open();
            timer.Start();

            Interlocked.Decrement(ref portLock);
        }

        public void Close()
        {
            while (Interlocked.CompareExchange(ref portLock, 1, 0) != 0)
                Thread.SpinWait(1);

            node.Stop();

            if (timer.IsRunning)
                timer.Stop();

            port.Close();

            Interlocked.Decrement(ref portLock);
        }

        public void Send(byte[] data)
        {
            node.Send(data);
        }

        #endregion

        #region Handlers

        private void node_OnOutcomingEventHandler(object sender, uMCPDataEventArgs e)
        {
            while (Interlocked.CompareExchange(ref portLock, 1, 0) != 0)
                Thread.SpinWait(1);
            port.Write(e.Data, 0, e.Data.Length);
            Interlocked.Decrement(ref portLock);
        }

        #endregion

        #region Events

        public EventHandler OnStateChangedEventHandler;
        public EventHandler OnSelectChangedEventHandler;
        public EventHandler<uMCPDataEventArgs> OnDataBlockAcknowledgedEventHandler;
        public EventHandler<uMCPDataEventArgs> OnDataBlockReceivedEventHandler;
        public EventHandler<uMCPActionInfoEventArgs> OnActionInfoEventHandler;
        public EventHandler OnTransmitterEmptyEventHandler;

        public EventHandler<SerialErrorReceivedEventArgs> PortErrorEventHandler;

        #endregion

        #region IDisposable

        public void Dispose()
        {
            if (!disposing)
            {
                disposing = true;

                if (port != null)
                {
                    if (port.IsOpen)
                    {
                        try
                        {
                            port.Close();
                        }
                        catch { }
                    }
                    port.Dispose();                    
                }

                if (timer != null)
                {
                    if (timer.IsRunning)
                        timer.Stop();
                    timer.Dispose();
                }

                if (node != null)
                {
                    node.Stop();                    
                }
            }
        }

        #endregion
    }
}
