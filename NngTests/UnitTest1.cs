using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Nng;

namespace NngTests
{
    /// <summary>
    /// Trivial rep/req case
    /// </summary>
    [TestClass]
    public class UnitTest1
    {
        [TestMethod]
        public void TrivialRepReq()
        {
            Socket rep0;
            var errno = Protocols.Rep0(out rep0);
            Assert.IsTrue(errno == 0);
            Socket req0;
            errno = Protocols.Req0(out req0);
            Assert.IsTrue(errno == 0);
            Listener listener;
            errno = Listener.Listen(rep0, "ipc:///myFirstPipe1234", out listener, 0);
            Assert.IsTrue(errno == 0);
            Dialer dialer;
            errno = Dialer.Dial(req0, "ipc:///myFirstPipe1234", out dialer, 0);
            Assert.IsTrue(errno == 0);
            req0.Send(new byte[] { 1, 2, 3, 4 }, 0);
            byte[] data;
            byte[] data2;
            rep0.Receive(out data, 0);
            Assert.IsTrue(data.SequenceEqual(new byte[] { 1, 2, 3, 4 }));
            rep0.Send(data);
            req0.Receive(out data2, 0);
            Assert.IsTrue(data.SequenceEqual(new byte[] { 1, 2, 3, 4 }));
            Assert.IsTrue(req0.Close() == 0);
            Assert.IsTrue(rep0.Close() == 0);
        }

        [TestMethod]
        public void WrongOrderRepReq()
        {
            Socket rep0;
            var errno = Protocols.Rep0(out rep0);
            Assert.IsTrue(errno == 0);
            Socket req0;
            errno = Protocols.Req0(out req0);
            Assert.IsTrue(errno == 0);
            Dialer dialer;
            errno = Dialer.Dial(req0, "ipc:///myFirstPipe1234", out dialer, Flag.nonblock);
            // Assert.IsTrue(errno == 0);
            System.Threading.Thread.Sleep(100);
            Listener listener;
            errno = Listener.Listen(rep0, "ipc:///myFirstPipe1234", out listener, 0);
            Assert.IsTrue(errno == 0);

            errno = req0.Send(new byte[] { 1, 2, 3, 4 }, Flag.none);
            Assert.IsTrue(errno == 0);
            byte[] data;
            byte[] data2;
            rep0.Receive(out data, 0);
            Assert.IsTrue(data.SequenceEqual(new byte[] { 1, 2, 3, 4 }));
            rep0.Send(data);
            req0.Receive(out data2, 0);
            Assert.IsTrue(data.SequenceEqual(new byte[] { 1, 2, 3, 4 }));
            Assert.IsTrue(req0.Close() == 0);
            Assert.IsTrue(rep0.Close() == 0);
        }
        [TestMethod]
        public void ServerDeathRepReq()
        {
            Socket rep0;
            var errno = Protocols.Rep0(out rep0);
            Assert.IsTrue(errno == 0);
            Socket req0;
            errno = Protocols.Req0(out req0);
            Assert.IsTrue(errno == 0);
            Listener listener;
            errno = Listener.Listen(rep0, "ipc:///myFirstPipe1234", out listener, 0);
            Assert.IsTrue(errno == 0);
            Dialer dialer;
            errno = Dialer.Dial(req0, "ipc:///myFirstPipe1234", out dialer, 0);
            Assert.IsTrue(errno == 0);
            req0.Send(new byte[] { 1, 2, 3, 4 }, 0);
            byte[] data;
            byte[] data2;
            rep0.Receive(out data, 0);
            Assert.IsTrue(data.SequenceEqual(new byte[] { 1, 2, 3, 4 }));
            rep0.Close(); // that's unexpected.
            System.Threading.Thread.Sleep(500);

            // server restart
            errno = Protocols.Rep0(out rep0);
            Assert.IsTrue(errno == 0);
            errno = Listener.Listen(rep0, "ipc:///myFirstPipe1234", out listener, 0);
            Assert.IsTrue(errno == 0);
            errno = rep0.Receive(out data, 0);
            Assert.IsTrue(errno == 0);
            errno = rep0.Send(data);
            Assert.IsTrue(errno == 0);

            errno = req0.Receive(out data2, 0);
            Assert.IsTrue(errno == 0);
            Assert.IsTrue(data.SequenceEqual(new byte[] { 1, 2, 3, 4 }));
            Assert.IsTrue(req0.Close() == 0);
            Assert.IsTrue(rep0.Close() == 0);
        }
    }


    /// <summary>
    /// Complicated, concurrent RPC case
    /// </summary>
    [TestClass]
    public class UnitTest2
    {
        Socket rep0;
        Socket req0;
        Aio clientAio;
        Aio serverAio;
        Listener listener;
        Dialer dialer;

        Int32 cookieCounter = 700;
        System.Collections.Generic.SynchronizedCollection<Cookie> cookieCollection = new System.Collections.Generic.SynchronizedCollection<Cookie>();
        System.Collections.Generic.SynchronizedCollection<Timing> timings = new System.Collections.Generic.SynchronizedCollection<Timing>();

        class Cookie
        {
            public Int32 cookie = 0;
            public object thisLock = new object();
            public Msg msg;
        }
        class Timing
        {
            public int index;
            public DateTime time;
            public TimeSpan timeSpan;
            public Timing(int index, DateTime time)
            {
                this.index = index;
                this.time = time;
            }
        }

        private Int32 RPCNextCookie(out byte[] data)
        {
            Int32 i = System.Threading.Interlocked.Increment(ref cookieCounter) | -0x80000000;
            data = BitConverter.GetBytes(i);
            Array.Reverse(data);
            return i;
        }

        void RPCClientCallback(object o)
        {
            Errno result = clientAio.Result();
            if (result == Errno.ok)
            {
                Msg msg = clientAio.GetMsg();
                req0.Receive(clientAio); // get ready to receive again

                byte[] data = msg.Header();
                Array.Reverse(data);
                int c = BitConverter.ToInt32(data, 0);
                var cookie = cookieCollection.FirstOrDefault(x => x.cookie == c);
                if (cookie != null)
                {
                    cookie.msg = msg;
                    lock (cookie.thisLock)
                    {
                        System.Threading.Monitor.Pulse(cookie.thisLock);
                    }
                    cookieCollection.Remove(cookie);
                }
                else
                {
                    Assert.Fail();
                }
            }
        }

        void RPCClient1(uint ms)
        {
            Errno nngResult;
            Msg msg = new Msg(0);
            nngResult = msg.AppendU32(ms);
            Assert.IsTrue(nngResult == Errno.ok);
            Cookie cookie = new Cookie();
            byte[] cookieData;
            cookie.cookie = RPCNextCookie(out cookieData);
            cookieCollection.Add(cookie);
            nngResult = msg.HeaderAppend(cookieData);
            Assert.IsTrue(nngResult == Errno.ok);
            nngResult = req0.Send(msg, 0);
            Assert.IsTrue(nngResult == Errno.ok);

            for (msg = null; msg == null;)
            {
                lock (cookie.thisLock)
                {
                    System.Threading.Monitor.Wait(cookie.thisLock);
                    msg = cookie.msg;
                }
            }
            msg.Free();

        }
        void RPCClientStart()
        {
            Errno errno;
            errno = Protocols.Req0(out req0);
            Assert.IsTrue(errno == Errno.ok);
            errno = req0.SetOptInt("raw", 1);
            Assert.IsTrue(errno == Errno.ok);
            errno = Dialer.Dial(req0, "ipc:///myfirstpipe12345", out dialer, 0);
            Assert.IsTrue(errno == Errno.ok);
            errno = Aio.Alloc(out clientAio, RPCClientCallback, null);
            Assert.IsTrue(errno == Errno.ok);
            req0.Receive(clientAio);
        }

        void RPCServerCallback(object o)
        {
            Errno result = serverAio.Result();
            if (result == Errno.ok)
            {
                Msg msg = serverAio.GetMsg();
                rep0.Receive(serverAio); // get ready to receive again
                uint ms;
                result = msg.TrimU32(out ms);
                // the actual work. Note that this doesn't need a running thread
                System.Threading.Thread.Sleep((int)ms);
                result = rep0.Send(msg, 0);
            }
        }


        void RPCServerStart()
        {
            Errno errno;
            errno = Protocols.Rep0(out rep0);
            Assert.IsTrue(errno == Errno.ok);
            errno = rep0.SetOptInt("raw", 1);
            Assert.IsTrue(errno == Errno.ok);
            errno = Listener.Listen(rep0, "ipc:///myfirstpipe12345", out listener, 0);
            Assert.IsTrue(errno == Errno.ok);
            errno = Aio.Alloc(out serverAio, RPCServerCallback, null);
            rep0.Receive(serverAio);
        }
        /// <summary>
        /// Run 20 Tasks, each taking a different time (50ms, 100ms, 150ms up to 1000ms),
        /// at once, through RPC. Theoretical time with an infinite number of threads
        /// would be 1000ms, with 8 threads:
        /// thread #1 1000ms+ 250ms + 200ms
        /// thread #2 950ms + 300ms + 150ms
        /// ...
        /// Thread #8 650ms + 600ms
        /// I count 1450ms.
        /// </summary>
        [TestMethod]
        public void LuxuriousRepReq()
        {
            RPCServerStart();
            RPCClientStart();
            DateTime start = DateTime.Now;
            System.Threading.Tasks.Parallel.For(0, 20,
                index =>
                {
                    RPCClient1((uint)(1000 - index * 50));
                    timings.Add(new Timing(index, DateTime.Now));
                }
                );
            DateTime end = DateTime.Now;
            TimeSpan runTime = end - start;
            TimeSpan total = new TimeSpan();
            double theoreticalTime = 0;
            for (int i = 0; i < 20; i++)
            {
                timings[i].timeSpan = timings[i].time - start;
                total += (timings[i].timeSpan);
                theoreticalTime += (1.0 - i * 0.050);
            }
            double parallel = theoreticalTime / runTime.TotalSeconds;

            req0.Close();
            rep0.Close();
        }
    }
    /// <summary>
    /// Dijkstra on a BUS
    /// </summary>
    [TestClass]
    public class UnitTest3
    {
        Socket[] sockets;
        Listener[] listeners;
        int[] potentials;
        Dialer[,] dialers;
        Aio[] receivers;
        Aio[] senders;

        Random random;
        const int n = 500;

        int countReceived = 0;
        [TestMethod]
        public void DijkstraBus()
        {
            sockets = new Socket[n];
            potentials = new int[n];
            dialers = new Dialer[n, 2];
            receivers = new Aio[n];
            senders = new Aio[n];
            listeners = new Listener[n];

            random = new Random(1);

            for (int i = 0; i < n; i++)
            {
                Protocols.Bus0(out sockets[i]);
                listeners[i] = new Listener(sockets[i], "ipc:///Dijkstra" + i.ToString());
                potentials[i] = int.MaxValue;
            }

            // every node connects to two other random nodes
            for (int i = 0; i < n; i++)
            {
                int target = random.Next(n);
                dialers[i, 0] = new Dialer(sockets[i], "ipc:///Dijkstra" + target.ToString());
                target = random.Next(n);
                dialers[i, 1] = new Dialer(sockets[i], "ipc:///Dijkstra" + target.ToString());
            }

            // arm the aios
            for (int i = 0; i < n; i++)
            {
                senders[i] = new Aio(Whatever, i);
                receivers[i] = new Aio(Received, i);
                sockets[i].Receive(receivers[i]);
            }
            Msg msg = new Msg(0);
            msg.AppendU32(0);
            potentials[0] = 0;
            sockets[0].Send(msg);
            //msg.Dispose();
            int oldCount = 0;
            for (;;)
            {
                System.Threading.Thread.Sleep(100);
                if (countReceived > oldCount )
                {
                    oldCount = countReceived;
                } else
                {
                    break;
                }
            }
            for (int i = 0; i < n; i++)
            {
                sockets[i].Close();
            }
            return;
        }
        void Received(object o)
        {
            System.Threading.Interlocked.Increment(ref countReceived);
            int j = (int)o;
            Aio aio = receivers[j];
            Errno errno = aio.Result();
            if (errno != 0)
            {
                return;
            }
            Msg msg = aio.GetMsg();

            sockets[j].Receive(receivers[j]);

            uint receivedValue2;
            msg.ChopU32(out receivedValue2);
            int receivedValue = (int)receivedValue2;
            bool valueHasChanged = false;
            for (;;) {
                int oldValue = potentials[j];
                if (receivedValue + 1 < oldValue)
                {
                    int beforeExchange = System.Threading.Interlocked.CompareExchange(ref potentials[j], receivedValue + 1, oldValue);
                    if (beforeExchange == oldValue)
                    {
                        valueHasChanged = true;
                        break;
                    }
                } else
                {
                    break;
                }
            }
            if (valueHasChanged)
            {
                msg.Clear();
                msg.AppendU32((uint)potentials[j]);
                //sockets[j].Send(msg);
                lock (senders[j])
                {
                    senders[j].Wait();
                    senders[j].SetMsg(msg);
                    sockets[j].Send(senders[j]);
                }
            } else
            {
                msg.Dispose();
            }

        }
        void Whatever(object o)
        {
            int j = (int)o;
            Aio aio = senders[j];
            Errno errno = aio.Result();
            if (errno != Errno.ok)
            {
                aio.GetMsg().Free();
            }
        }
    }
}
