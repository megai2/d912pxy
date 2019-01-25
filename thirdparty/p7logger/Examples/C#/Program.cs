using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

//importing P7 namespace, file is located in <P7 dir>/Wrappers/C#/P7.cs
using P7;


namespace CSharp_Example
{
    class Program
    {
        private const System.UInt16 m_wModuleMain = 0;

        static void Main(string[] args)
        {            
            ////////////////////////////////////////////////////////////////////
            //initialize P7 client
            P7.Client l_pClient = new P7.Client("/P7.Sink=Baical /P7.Addr=localhost /P7.Pool=16000");

            
            ////////////////////////////////////////////////////////////////////
            //initialize P7 trace channel, using prev. created client
            P7.Traces     l_pTrace  = new P7.Traces(l_pClient, "Trace Channel");
            System.IntPtr l_hModule = l_pTrace.Register_Module("Example");

            l_pTrace.Register_Thread("Application");

            //delivering messages
            for (Int64 l_iI = 0; l_iI < 1000; l_iI++)
            {
                l_pTrace.Trace(l_hModule, "Trace message");
                l_pTrace.Warning(l_hModule, "Warning Message");
                l_pTrace.Error(l_hModule, "Error message");
            }

            ////////////////////////////////////////////////////////////////////
            //initialize P7 telemetry channel, using prev. created client
            P7.Telemetry l_pTel = new P7.Telemetry(l_pClient, "Telemetry channel");
            System.Byte  l_bId1 = 0;
            System.Byte  l_bId2 = 0;

            l_pTel.Create("Counter #1", 0, 1000, 900, 1, ref l_bId1);
            l_pTel.Create("Counter #2", 0, 1000, 900, 1, ref l_bId2);

            for (Int64 l_iI = 0; l_iI < 100000; l_iI++)
            {
                l_pTel.Add(l_bId1, l_iI % 1000);
                l_pTel.Add(l_bId2, (l_iI + 500) % 1000);
            }

            ////////////////////////////////////////////////////////////////////
            //release telemetry, trace & client
            l_pTrace.Unregister_Thread();

            l_pTel    = null;
            l_pTrace  = null;
            l_pClient = null;
        }
    }
}
