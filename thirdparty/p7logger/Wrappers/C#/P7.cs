////////////////////////////////////////////////////////////////////////////////
//                                                                             /
// 2012-2019 (c) Baical                                                        /
//                                                                             /
// This library is free software; you can redistribute it and/or               /
// modify it under the terms of the GNU Lesser General Public                  /
// License as published by the Free Software Foundation; either                /
// version 3.0 of the License, or (at your option) any later version.          /
//                                                                             /
// This library is distributed in the hope that it will be useful,             /
// but WITHOUT ANY WARRANTY; without even the implied warranty of              /
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU           /
// Lesser General Public License for more details.                             /
//                                                                             /
// You should have received a copy of the GNU Lesser General Public            /
// License along with this library.                                            /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace P7
{
    /// <summary>
    /// shell for P7 library, depends on P7Client_64.dll or P7Client_32.dll
    /// 
    ///                       +-------------------+                            
    ///                       |       Sink        |                            
    ///                       | * Network (Baical)|                            
    ///                       | * FileTxt         |                          
    ///                       | * FileBin         |                          
    ///                       | * Console         |                          
    ///                       | * Syslog          |                          
    ///                       | * Auto            |                            
    ///                       | * Null            |                            
    ///                       +---------^---------+                            
    ///                                 |                                      
    ///                                 |                                      
    ///                       +---------+---------+           
    ///                       |     P7 Client     |           
    ///                       |    [ Channels ]   |           
    ///                       |  +-+ +-+    +--+  |           
    ///                       |  |0| |1| ...|31|  |           
    ///                       |  +^+ +^+    +-^+  |           
    ///                       +---|---|-------|---+           
    ///                           |   |       |               
    ///                     +-----+   |       +----------+    
    ///                     |         |                  |    
    ///                     |         |                  |    
    ///                 +---+---+ +---+--------+     +---+---+
    ///                 | Trace | | Telemetry  |     | Trace |
    ///                 |Channel| |  channel   | ... |Channel|
    ///                 +-------+ +------------+     +-------+
    ////////////////////////////////////////////////////////////////////////////
    ///                                                                       //
    ///       Documentation is located in <P7>/Documentation/P7.pdf           //
    ///                                                                       //
    ////////////////////////////////////////////////////////////////////////////
    /// 
    /// </summary>
    public static class Dll
    {
    #if DEBUG
        public const string DLL_NAME_32 = "P7x32d.dll";
        public const string DLL_NAME_64 = "P7x64d.dll";
    #else
        public const string DLL_NAME_32 = "P7x32.dll";
        public const string DLL_NAME_64 = "P7x64.dll";
    #endif

        #region internal class defines and DLL imports
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Exceptional_Flush", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void P7_Exceptional_Flush32();

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Exceptional_Flush", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void P7_Exceptional_Flush64();
        #endregion

        /// <summary>
        ///Application crash processor static function for P7                               
        ///See documentation for details.
        /// </summary>
        public static void Exceptional_Buffers_Flush()
        {
            if (8 == IntPtr.Size)
            {
                P7_Exceptional_Flush64();
            }
            else
            {
                P7_Exceptional_Flush32();
            }
        }                              
    }

    /// <summary>
    /// P7 client shell, used as transport engine for Telemetry & Traces
    /// See documentation for details.
    /// </summary>
    public class Client
    {
        #region internal class defines and DLL imports

        ////////////////////////////////////////////////////////////////////////
        //P7_Client_Create 
        private delegate System.IntPtr fnP7_Client_Create(String i_sArgs);

        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Client_Create", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Client_Create32([MarshalAs(UnmanagedType.LPWStr)]String i_sArgs);

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Client_Create", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Client_Create64([MarshalAs(UnmanagedType.LPWStr)]String i_sArgs);

        ////////////////////////////////////////////////////////////////////////
        //P7_Client_Get_Shared 
        private delegate System.IntPtr fnP7_Client_Get_Shared(String i_sName);

        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Client_Get_Shared", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Client_Get_Shared32([MarshalAs(UnmanagedType.LPWStr)]String i_sName);

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Client_Get_Shared", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Client_Get_Shared64([MarshalAs(UnmanagedType.LPWStr)]String i_sName);

        ////////////////////////////////////////////////////////////////////////
        //P7_Client_Share 
        private delegate System.UInt32 fnP7_Client_Share(System.IntPtr i_hClient,
                                                         String        i_sName
                                                        );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Client_Share", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Client_Share32(System.IntPtr i_hClient,
                                                              [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                             );
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Client_Share", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Client_Share64(System.IntPtr i_hClient,
                                                              [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                             );


        ////////////////////////////////////////////////////////////////////////
        //P7_Client_Add_Ref 
        private delegate System.Int32 fnP7_Client_Add_Ref(System.IntPtr i_hClient);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Client_Add_Ref", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Client_Add_Ref32(System.IntPtr i_hClient);

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Client_Add_Ref", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Client_Add_Ref64(System.IntPtr i_hClient);


        ////////////////////////////////////////////////////////////////////////
        //P7_Client_Release 
        private delegate System.Int32 fnP7_Client_Release(System.IntPtr i_hClient);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Client_Release", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Client_Release32(System.IntPtr i_hClient);

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Client_Release", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Client_Release64(System.IntPtr i_hClient);

        #endregion

        private System.IntPtr  m_hHandle            = IntPtr.Zero;
        fnP7_Client_Create     P7_Client_Create     = null;
        fnP7_Client_Get_Shared P7_Client_Get_Shared = null;
        fnP7_Client_Share      P7_Client_Share      = null;
        fnP7_Client_Add_Ref    P7_Client_Add_Ref    = null;
        fnP7_Client_Release    P7_Client_Release    = null;

        /// <summary>
        ///Creates new P7  client,  client  is  used  as  transport  engine  for 
        ///telemetry & trace channels, every client can handle up to 32 channels
        ///See documentation for details.
        /// </summary>
        public Client(String i_sArgs)
        {
            Initialize();

            m_hHandle = P7_Client_Create(i_sArgs);

            if (IntPtr.Zero == m_hHandle)
            {
                throw new System.ArgumentNullException("Can't create P7 client");
            }
        }

        /// <summary>
        /// Private constructor, called from Get_Shared() function
        ///See documentation for details.
        ///</summary>
        private Client(System.IntPtr i_hHandle)
        {
            Initialize();
            m_hHandle = i_hHandle;
        }

        /// <summary>
        /// P7 client destructor, automatically decrease reference counter
        /// See documentation for details.
        /// </summary>
        ~Client()
        {
            if (IntPtr.Zero != m_hHandle)
            {
                P7_Client_Release(m_hHandle);
                m_hHandle = IntPtr.Zero;
            }
        }

        /// <summary>
        /// Initialize delegates by proper functions 32/64 bits
        /// </summary>
        private void Initialize()
        {
            if (8 == IntPtr.Size)
            {
                P7_Client_Create     = P7_Client_Create64;
                P7_Client_Get_Shared = P7_Client_Get_Shared64;
                P7_Client_Share      = P7_Client_Share64;
                P7_Client_Add_Ref    = P7_Client_Add_Ref64;
                P7_Client_Release    = P7_Client_Release64;
            }
            else
            {
                P7_Client_Create     = P7_Client_Create32;
                P7_Client_Get_Shared = P7_Client_Get_Shared32;
                P7_Client_Share      = P7_Client_Share32;
                P7_Client_Add_Ref    = P7_Client_Add_Ref32;
                P7_Client_Release    = P7_Client_Release32;
            }
        }
        /// <summary>
        ///This functions allows you to get P7 client instance if it was created  
        ///by   someone   else   inside   current   process   and  shared  using 
        ///P7::Client::Share() function. If no instance  was  registered  inside 
        ///current process - function will return null. Do not forget to release 
        ///newly created P7 client instance at the end (destructor  will  do  it 
        ///for you)
        ///See documentation for details.
        /// </summary>
        public static Client Get_Shared(String i_sName)
        {
            System.IntPtr l_hHandle = System.IntPtr.Zero;

            if (8 == IntPtr.Size)
            {
                l_hHandle = P7_Client_Get_Shared64(i_sName);
            }
            else
            {
                l_hHandle = P7_Client_Get_Shared32(i_sName);
            }

            if (IntPtr.Zero != l_hHandle)
            {
                return new Client(l_hHandle);
            }

            return null;
        }

        /// <summary>
        /// Function to share current P7 object in address space of the current 
        /// process, to get shared instance  use  func. P7::Client::Get_Shared()
        /// See documentation for details.
        /// </summary>
        public bool Share(String i_sName)
        {
            System.UInt32 l_dwReturn = P7_Client_Share(m_hHandle, i_sName);

            return (0 != l_dwReturn) ? true : false;
        }

        /// <summary>
        /// Give access to P7 client handle
        /// </summary>
        public System.IntPtr Handle
        {
            get { return m_hHandle; }
        }

        /// <summary>
        /// Increase object reference counter, thread safe
        /// See documentation for details.
        /// </summary>
        public System.Int32 AddRef()
        {
            return P7_Client_Add_Ref(m_hHandle);
        }

        /// <summary>
        /// Decrease reference counter value, when it will be equal to 0- client
        /// will be destroyed.
        /// See documentation for details.
        /// </summary>
        public System.Int32 Release()
        {
            System.Int32 l_dwReturn = P7_Client_Release(m_hHandle);
            if (0 == l_dwReturn)
            {
                m_hHandle = IntPtr.Zero;
            }
            else if (0 == l_dwReturn)
            {
                Console.WriteLine("ERROR: P7 Client reference counter is damaged !");
                m_hHandle = IntPtr.Zero;
            }

            return l_dwReturn;
        }
    } //Client

    /// <summary>
    /// P7 Telemetry shell, allows you to create up to  256  different  counters
    /// Possible usage scenarios: CPU utilization statistics, memory utilization 
    /// statistics,   threads   counters,   handles  counters,  buffers  filling 
    /// statistics, threads cyclograms ... etc.
    /// CPU consuming: 
    ///  * ARM 926EJ (v5), 2 000 samples per second - 0,5% CPU, max ~50 000 per 
    ///    second 
    ///  * Intel E8400 (Core 2 duo), 25 000 samples per second - 0,5%  CPU,  max 
    ///    ~ 1 200 000 per second 
    ///  * Intel i7-870, 110 000 samples per second - 0,5% CPU, max ~3.5millions 
    ///    per second
    /// </summary>
    public class Telemetry
    {
        #region internal class defines and DLL imports
        ////////////////////////////////////////////////////////////////////////
        //P7_Telemetry_Create
        private delegate System.IntPtr fnP7_Telemetry_Create(System.IntPtr i_hClient,
                                                             String i_sName,
                                                             System.IntPtr i_pConf);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Telemetry_Create", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Telemetry_Create32(System.IntPtr i_hClient, 
                                                                  [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                  System.IntPtr i_pConf
                                                                 );
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Telemetry_Create", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Telemetry_Create64(System.IntPtr i_hClient,
                                                                  [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                  System.IntPtr i_pConf
                                                                 );

        ////////////////////////////////////////////////////////////////////////
        //P7_Telemetry_Create
        private delegate System.IntPtr fnP7_Telemetry_Get_Shared(String i_sName);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Telemetry_Get_Shared", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Telemetry_Get_Shared32([MarshalAs(UnmanagedType.LPWStr)]String i_sName);

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Telemetry_Get_Shared", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Telemetry_Get_Shared64([MarshalAs(UnmanagedType.LPWStr)]String i_sName);

        ////////////////////////////////////////////////////////////////////////
        //P7_Telemetry_Share
        private delegate System.UInt32 fnP7_Telemetry_Share(System.IntPtr i_hTelemetry,
                                                            String i_sName
                                                           );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Telemetry_Share", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Telemetry_Share32(System.IntPtr i_hTelemetry,
                                                                 [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                                );

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Telemetry_Share", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Telemetry_Share64(System.IntPtr i_hTelemetry,
                                                                 [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                                );


        ////////////////////////////////////////////////////////////////////////
        //P7_Telemetry_Create_Counter
        private delegate System.UInt32 fnP7_Telemetry_Create_Counter(System.IntPtr     i_hTelemetry,
                                                                     String            i_sName,
                                                                     System.Double     i_dbMin,
                                                                     System.Double     i_dbAlarmMin,
                                                                     System.Double     i_dbMax,
                                                                     System.Double     i_dbAlarmMax,
                                                                     System.Int32      i_bOn,
                                                                     ref System.UInt16 o_rCounter_ID
                                                                    );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Telemetry_Create_Counter", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Telemetry_Create_Counter32(System.IntPtr i_hTelemetry,
                                                                          [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                          System.Double     i_dbMin,
                                                                          System.Double     i_dbAlarmMin,
                                                                          System.Double     i_dbMax,
                                                                          System.Double     i_dbAlarmMax,
                                                                          System.Int32      i_bOn,
                                                                          ref System.UInt16 o_rCounter_ID
                                                                         );

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Telemetry_Create_Counter", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Telemetry_Create_Counter64(System.IntPtr i_hTelemetry,
                                                                          [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                          System.Double i_dbMin,
                                                                          System.Double i_dbAlarmMin,
                                                                          System.Double i_dbMax,
                                                                          System.Double i_dbAlarmMax,
                                                                          System.Int32 i_bOn,
                                                                          ref System.UInt16 o_rCounter_ID
                                                                         );

        ////////////////////////////////////////////////////////////////////////
        //P7_Telemetry_Put_Value
        private delegate System.UInt32 fnP7_Telemetry_Put_Value(System.IntPtr i_hTelemetry,
                                                                System.UInt16 i_wCounter_ID,
                                                                System.Double i_dbValue
                                                               );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Telemetry_Put_Value", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Telemetry_Put_Value32(System.IntPtr i_hTelemetry,
                                                                     System.UInt16 i_wCounter_ID,
                                                                     System.Double i_dbValue
                                                                    );

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Telemetry_Put_Value", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Telemetry_Put_Value64(System.IntPtr i_hTelemetry,
                                                                     System.UInt16 i_wCounter_ID,
                                                                     System.Double i_dbValue
                                                                    );

        ////////////////////////////////////////////////////////////////////////
        //P7_Telemetry_Find_Counter
        private delegate System.UInt32 fnP7_Telemetry_Find_Counter(System.IntPtr i_hTelemetry,
                                                                   [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                   ref System.UInt16 o_rCounter_ID
                                                                  );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Telemetry_Put_Value", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Telemetry_Find_Counter32(System.IntPtr i_hTelemetry,
                                                                        [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                        ref System.UInt16 o_rCounter_ID
                                                                       );

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Telemetry_Put_Value", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Telemetry_Find_Counter64(System.IntPtr i_hTelemetry,
                                                                        [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                        ref System.UInt16 o_rCounter_ID
                                                                       );


        ////////////////////////////////////////////////////////////////////////
        //P7_Telemetry_Add_Ref
        private delegate System.Int32 fnP7_Telemetry_Add_Ref(System.IntPtr i_hTelemetry);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Telemetry_Add_Ref", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Telemetry_Add_Ref32(System.IntPtr i_hTelemetry);
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Telemetry_Add_Ref", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Telemetry_Add_Ref64(System.IntPtr i_hTelemetry);


        ////////////////////////////////////////////////////////////////////////
        //P7_Telemetry_Release
        private delegate System.Int32 fnP7_Telemetry_Release(System.IntPtr i_hTelemetry);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Telemetry_Release", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Telemetry_Release32(System.IntPtr i_hTelemetry);
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Telemetry_Release", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Telemetry_Release64(System.IntPtr i_hTelemetry);
        #endregion

        /// <summary>
        /// P7 telemetry handle
        /// </summary>
        private System.IntPtr                 m_hHandle                   = IntPtr.Zero;
        private fnP7_Telemetry_Create         P7_Telemetry_Create         = null;
        private fnP7_Telemetry_Get_Shared     P7_Telemetry_Get_Shared     = null;
        private fnP7_Telemetry_Share          P7_Telemetry_Share          = null;
        private fnP7_Telemetry_Create_Counter P7_Telemetry_Create_Counter = null;
        private fnP7_Telemetry_Put_Value      P7_Telemetry_Put_Value      = null;
        private fnP7_Telemetry_Find_Counter   P7_Telemetry_Find_Counter   = null;
        private fnP7_Telemetry_Add_Ref        P7_Telemetry_Add_Ref        = null;
        private fnP7_Telemetry_Release        P7_Telemetry_Release        = null;

        /// <summary>
        /// creates new instance of P7 Telemetry object
        /// See documentation for details.
        /// </summary>
        /// <param name="i_pClient">P7 client previously created</param>
        /// <param name="i_sName">Name of the telemetry channel</param>
        public Telemetry(P7.Client i_pClient, String i_sName)
        {
            Initialize();

            if (    (null != i_pClient) 
                 || (IntPtr.Zero != i_pClient.Handle)
               )
            {
                m_hHandle = P7_Telemetry_Create(i_pClient.Handle, i_sName, IntPtr.Zero);
            }
            else
            {
                throw new System.ArgumentException("Can't create P7 telemetry, input parameter (i_pClient) is wrong");
            }

            if (IntPtr.Zero == m_hHandle)
            {
                throw new System.ArgumentNullException("Can't create P7 telemetry, more than 32 streams are used ?");
            }
        }

        /// <summary>
        /// Private constructor, called from Get_Shared() function
        /// </summary>
        /// <param name="i_hHandle">P7 client handle</param>
        private Telemetry(System.IntPtr i_hHandle)
        {
            Initialize();
            m_hHandle = i_hHandle;
        }

        /// <summary>
        /// P7 telemetry destructor, automatically decrease reference counter
        /// </summary>
        ~Telemetry()
        {
            if (IntPtr.Zero != m_hHandle)
            {
                P7_Telemetry_Release(m_hHandle);
                m_hHandle = IntPtr.Zero;
            }
        }


        /// <summary>
        /// Initialize delegates by proper functions 32/64 bits
        /// </summary>
        private void Initialize()
        {
            if (8 == IntPtr.Size)
            {
                P7_Telemetry_Create         = P7_Telemetry_Create64;
                P7_Telemetry_Get_Shared     = P7_Telemetry_Get_Shared64;
                P7_Telemetry_Share          = P7_Telemetry_Share64;
                P7_Telemetry_Create_Counter = P7_Telemetry_Create_Counter64;
                P7_Telemetry_Put_Value      = P7_Telemetry_Put_Value64;
                P7_Telemetry_Find_Counter   = P7_Telemetry_Find_Counter64;
                P7_Telemetry_Add_Ref        = P7_Telemetry_Add_Ref64;
                P7_Telemetry_Release        = P7_Telemetry_Release64;
            }
            else
            {
                P7_Telemetry_Create         = P7_Telemetry_Create32;
                P7_Telemetry_Get_Shared     = P7_Telemetry_Get_Shared32;
                P7_Telemetry_Share          = P7_Telemetry_Share32;
                P7_Telemetry_Create_Counter = P7_Telemetry_Create_Counter32;
                P7_Telemetry_Put_Value      = P7_Telemetry_Put_Value32;
                P7_Telemetry_Find_Counter   = P7_Telemetry_Find_Counter32;
                P7_Telemetry_Add_Ref        = P7_Telemetry_Add_Ref32;
                P7_Telemetry_Release        = P7_Telemetry_Release32;
            }
        }


        /// <summary>
        ///This functions allows you to get P7  telemetry  instance  if  it  was 
        ///created by someone else  inside  current  process  and  shared  using 
        ///P7::Telemetry::Share() function. If no instance was registered inside 
        ///current process - function will return null. Do not forget to release 
        ///newly created P7 telemetry instance at the end (destructor will do it 
        ///for you)
        ///See documentation for details.
        public static Telemetry Get_Shared(String i_sName)
        {
            System.IntPtr l_hHandle = System.IntPtr.Zero;

            if (8 == IntPtr.Size)
            {
                l_hHandle = P7_Telemetry_Get_Shared64(i_sName);
            }
            else
            {
                l_hHandle = P7_Telemetry_Get_Shared32(i_sName);
            }

            if (IntPtr.Zero != l_hHandle)
            {
                return new Telemetry(l_hHandle);
            }

            return null;
        }


        /// <summary>
        /// Function share current P7 telemetry object in address space of the 
        /// current process, to get shared instance  use  function:
        /// P7::Telemetry::Get_Shared()
        /// See documentation for details.
        /// </summary>
        public bool Share(String i_sName)
        {
            System.UInt32 l_dwReturn = P7_Telemetry_Share(m_hHandle, i_sName);

            return (0 != l_dwReturn) ? true : false;
        }

        /// <summary>
        /// creates new telemetry counter.
        /// See documentation for details.
        /// </summary>
        public bool Create(String            i_sName,
                           System.Double     i_dbMin,
                           System.Double     i_dbAlarmMin,
                           System.Double     i_dbMax,
                           System.Double     i_dbAlarmMax,
                           System.Int32      i_bOn,
                           ref System.UInt16 o_rCounter_ID
                          )
        {
            System.UInt32 l_dwReturn = P7_Telemetry_Create_Counter(m_hHandle,
                                                                   i_sName,
                                                                   i_dbMin,
                                                                   i_dbAlarmMin,
                                                                   i_dbMax,
                                                                   i_dbAlarmMax,
                                                                   i_bOn,
                                                                   ref o_rCounter_ID
                                                                  );
                
            return (0 != l_dwReturn) ? true : false;
        }

        /// <summary>
        /// add counter value
        /// See documentation for details.
        /// </summary>
        public bool Add(System.UInt16 i_wCounter_ID, System.Double i_dbValue)
        {
            System.UInt32 l_dwReturn = P7_Telemetry_Put_Value(m_hHandle,
                                                              i_wCounter_ID,
                                                              i_dbValue
                                                             );

            return (0 != l_dwReturn) ? true : false;
        }

        /// <summary>
        /// find counter ID by name
        /// See documentation for details.
        /// </summary>
        public bool Find_Counter(String i_sName, ref System.UInt16 o_rCounter_ID)
        {
            System.UInt32 l_dwReturn = P7_Telemetry_Find_Counter(m_hHandle,
                                                                 i_sName,
                                                                 ref o_rCounter_ID
                                                                );

            return (0 != l_dwReturn) ? true : false;
        }


        /// <summary>
        /// Increase object reference counter, thread safe
        /// See documentation for details.
        /// </summary>
        public System.Int32 AddRef()
        {
            return P7_Telemetry_Add_Ref(m_hHandle);
        }

        /// <summary>
        /// Decrease reference counter value, when it will be equal to 0 - channel
        /// will be destroyed.
        /// See documentation for details.
        /// </summary>
        public System.Int32 Release()
        {
            System.Int32 l_dwReturn = P7_Telemetry_Release(m_hHandle);
            if (0 == l_dwReturn)
            {
                m_hHandle = IntPtr.Zero;
            }
            else if (0 == l_dwReturn)
            {
                Console.WriteLine("ERROR: P7 telemetry reference counter is damaged !");
                m_hHandle = IntPtr.Zero;
            }

            return l_dwReturn;
        }
    }//Telemetry


    /// <summary>
    /// P7 Trace shell, allows you to sent trace/log messages to  Baical  server
    /// or local file
    /// </summary>
    public class Traces
    {
        #region internal class defines and DLL imports
        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Create
        private delegate System.IntPtr fnP7_Trace_Create(System.IntPtr i_hClient,
                                                         String        i_sName,
                                                         System.IntPtr i_pOpt
                                                        );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Create", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Trace_Create32(System.IntPtr i_hClient,
                                                              [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                              System.IntPtr i_pOpt
                                                             );
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Create", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Trace_Create64(System.IntPtr i_hClient,
                                                              [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                              System.IntPtr i_pOpt 
                                                             );

        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Get_Shared
        private delegate System.IntPtr fnP7_Trace_Get_Shared(String i_sName);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Get_Shared", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Trace_Get_Shared32([MarshalAs(UnmanagedType.LPWStr)]String i_sName);
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Get_Shared", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Trace_Get_Shared64([MarshalAs(UnmanagedType.LPWStr)]String i_sName);



        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Share
        private delegate System.UInt32 fnP7_Trace_Share(System.IntPtr i_hTrace,
                                                        String i_sName
                                                       );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Share", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Trace_Share32(System.IntPtr i_hTrace,
                                                             [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                            );
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Share", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Trace_Share64(System.IntPtr i_hTrace,
                                                             [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                            );

        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Set_Verbosity
        private delegate void fnP7_Trace_Set_Verbosity(System.IntPtr i_hTrace,
                                                       System.IntPtr i_hModule,
                                                       System.UInt32 i_dwVerbosity
                                                      );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Set_Verbosity", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void P7_Trace_Set_Verbosity32(System.IntPtr i_hTrace,
                                                            System.IntPtr i_hModule,
                                                            System.UInt32 i_dwVerbosity
                                                           );
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Set_Verbosity", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void P7_Trace_Set_Verbosity64(System.IntPtr i_hTrace,
                                                            System.IntPtr i_hModule,
                                                            System.UInt32 i_dwVerbosity
                                                           );


        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Register_Thread
        private delegate System.UInt32 fnP7_Trace_Register_Thread(System.IntPtr i_hTrace,
                                                                  [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                  System.UInt32 i_dwThreadId
                                                                 );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Register_Thread", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Trace_Register_Thread32(System.IntPtr i_hTrace,
                                                                       [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                       System.UInt32 i_dwThreadId
                                                                      );
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Register_Thread", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Trace_Register_Thread64(System.IntPtr i_hTrace,
                                                                       [MarshalAs(UnmanagedType.LPWStr)]String i_sName,
                                                                       System.UInt32 i_dwThreadId
                                                                      );


        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Unregister_Thread
        private delegate System.UInt32 fnP7_Trace_Unregister_Thread(System.IntPtr i_hTrace,
                                                                    System.UInt32 i_dwThreadId
                                                                   );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Unregister_Thread", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Trace_Unregister_Thread32(System.IntPtr i_hTrace,
                                                                         System.UInt32 i_dwThreadId
                                                                         );
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Unregister_Thread", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Trace_Unregister_Thread64(System.IntPtr i_hTrace,
                                                                         System.UInt32 i_dwThreadId
                                                                        );


        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Register_Module
        private delegate System.IntPtr fnP7_Trace_Register_Module(System.IntPtr i_hTrace,
                                                                  [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                                 );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Register_Module", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Trace_Register_Module32(System.IntPtr i_hTrace,
                                                                       [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                                      );
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Register_Module", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr P7_Trace_Register_Module64(System.IntPtr i_hTrace,
                                                                       [MarshalAs(UnmanagedType.LPWStr)]String i_sName
                                                                      );


        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Managed
        private delegate System.UInt32 fnP7_Trace_Managed(System.IntPtr i_hTrace,
                                                          System.UInt16 i_wTrace_ID,
                                                          System.UInt32 i_dwLevel,
                                                          System.IntPtr i_hModule,
                                                          System.UInt16 i_wLine,
                                                          String        i_sFile,
                                                          String        i_sFunction,
                                                          String        i_sMessage
                                                         );
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Managed", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Trace_Managed32(System.IntPtr i_hTrace,
                                                               System.UInt16 i_wTrace_ID,
                                                               System.UInt32 i_dwLevel,
                                                               System.IntPtr i_hModule,
                                                               System.UInt16 i_wLine,
                                                               [MarshalAs(UnmanagedType.LPWStr)]String i_sFile,
                                                               [MarshalAs(UnmanagedType.LPWStr)]String i_sFunction,
                                                               [MarshalAs(UnmanagedType.LPWStr)]String i_sMessage
                                                              );

        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Managed", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.UInt32 P7_Trace_Managed64(System.IntPtr i_hTrace,
                                                               System.UInt16 i_wTrace_ID,
                                                               System.UInt32 i_dwLevel,
                                                               System.IntPtr i_hModule,
                                                               System.UInt16 i_wLine,
                                                               [MarshalAs(UnmanagedType.LPWStr)]String i_sFile,
                                                               [MarshalAs(UnmanagedType.LPWStr)]String i_sFunction,
                                                               [MarshalAs(UnmanagedType.LPWStr)]String i_sMessage
                                                              );


        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Add_Ref
        private delegate System.Int32 fnP7_Trace_Add_Ref(System.IntPtr i_hTrace);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Add_Ref", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Trace_Add_Ref32(System.IntPtr i_hTrace);
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Add_Ref", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Trace_Add_Ref64(System.IntPtr i_hTrace);

        ////////////////////////////////////////////////////////////////////////
        //P7_Trace_Release
        private delegate System.Int32 fnP7_Trace_Release(System.IntPtr i_hTrace);
        [DllImport(Dll.DLL_NAME_32, EntryPoint = "P7_Trace_Release", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Trace_Release32(System.IntPtr i_hTrace);
        [DllImport(Dll.DLL_NAME_64, EntryPoint = "P7_Trace_Release", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 P7_Trace_Release64(System.IntPtr i_hTrace);
        #endregion

        /// <summary>
        /// Traces levels
        /// </summary>
        public enum Level
        {
            TRACE     = 0,
            DEBUG     = 1,
            INFO      = 2,
            WARNING   = 3,
            ERROR     = 4,
            CRITICAL  = 5,

            COUNT    
        }

        /// <summary>
        /// P7 trace handle
        /// </summary>
        private System.IntPtr                m_hHandle                  = IntPtr.Zero;
        private fnP7_Trace_Create            P7_Trace_Create            = null;       

        private fnP7_Trace_Get_Shared        P7_Trace_Get_Shared        = null;   
        private fnP7_Trace_Share             P7_Trace_Share             = null;        
        
        private fnP7_Trace_Set_Verbosity     P7_Trace_Set_Verbosity     = null;

        private fnP7_Trace_Register_Thread   P7_Trace_Register_Thread   = null;
        private fnP7_Trace_Unregister_Thread P7_Trace_Unregister_Thread = null;

        private fnP7_Trace_Register_Module   P7_Trace_Register_Module   = null;
        
        private fnP7_Trace_Managed           P7_Trace_Managed           = null;      

        private fnP7_Trace_Add_Ref           P7_Trace_Add_Ref           = null;
        private fnP7_Trace_Release           P7_Trace_Release           = null; 
        private const Int32                  m_iStackFrame              = 2;  


        /// <summary>
        /// creates new instance of P7 trace object
        /// See documentation for details.
        /// </summary>
        public Traces(P7.Client i_pClient, String i_sName)
        {
            Initialize();
            if (    (null != i_pClient)
                 || (IntPtr.Zero != i_pClient.Handle)
               )
            {
                m_hHandle = P7_Trace_Create(i_pClient.Handle, i_sName, IntPtr.Zero);
            }
            else
            {
                throw new System.ArgumentException("Can't create P7 trace, input parameter (i_pClient) is wrong");
            }

            if (IntPtr.Zero == m_hHandle)
            {
                throw new System.ArgumentNullException("Can't create P7 trace, more than 32 streams are used ?");
            }
        }

        /// <summary>
        /// Private constructor, called from Get_Shared() function
        /// </summary>
        /// <param name="i_hHandle">P7 client handle</param>
        private Traces(System.IntPtr i_hHandle)
        {
            Initialize();
            m_hHandle = i_hHandle;
        }

        /// <summary>
        /// P7 telemetry destructor, automatically decrease reference counter
        /// </summary>
        ~Traces()
        {
            if (IntPtr.Zero != m_hHandle)
            {
                P7_Trace_Release(m_hHandle);
                m_hHandle = IntPtr.Zero;
            }
        }

        /// <summary>
        /// Initialize delegates by proper functions 32/64 bits
        /// </summary>
        private void Initialize()
        {
            if (8 == IntPtr.Size)
            {
                P7_Trace_Create            = P7_Trace_Create64;
                P7_Trace_Get_Shared        = P7_Trace_Get_Shared64;
                P7_Trace_Share             = P7_Trace_Share64;
                P7_Trace_Set_Verbosity     = P7_Trace_Set_Verbosity64;
                P7_Trace_Register_Thread   = P7_Trace_Register_Thread64;
                P7_Trace_Unregister_Thread = P7_Trace_Unregister_Thread64;
                P7_Trace_Register_Module   = P7_Trace_Register_Module64;
                P7_Trace_Managed           = P7_Trace_Managed64;
                P7_Trace_Add_Ref           = P7_Trace_Add_Ref64;
                P7_Trace_Release           = P7_Trace_Release64;
            }
            else
            {
                P7_Trace_Create            = P7_Trace_Create32;
                P7_Trace_Get_Shared        = P7_Trace_Get_Shared32;
                P7_Trace_Share             = P7_Trace_Share32;
                P7_Trace_Set_Verbosity     = P7_Trace_Set_Verbosity32;
                P7_Trace_Register_Thread   = P7_Trace_Register_Thread32;
                P7_Trace_Unregister_Thread = P7_Trace_Unregister_Thread32;
                P7_Trace_Register_Module   = P7_Trace_Register_Module32;
                P7_Trace_Managed           = P7_Trace_Managed32;
                P7_Trace_Add_Ref           = P7_Trace_Add_Ref32;
                P7_Trace_Release           = P7_Trace_Release32;
            }
        }

        /// <summary>
        ///This functions allows you to get P7 trace instance if it was  created
        ///by someone else inside current process and shared using Traces::Share
        ///function. If no instance was  registered  inside  current  process - 
        ///function will return null. Do not forget to release newly created  P7
        ///trace instance at the end (destructor will do it for you)
        ///See documentation for details.
        /// </summary>
        public static Traces Get_Shared(String i_sName)
        {
            System.IntPtr l_hHandle = System.IntPtr.Zero;

            if (8 == IntPtr.Size)
            {
                l_hHandle = P7_Trace_Get_Shared64(i_sName);
            }
            else
            {
                l_hHandle = P7_Trace_Get_Shared32(i_sName);
            }

            if (IntPtr.Zero != l_hHandle)
            {
                return new Traces(l_hHandle);
            }

            return null;
        }


        /// <summary>
        /// Function share current P7 trace  object  in  address  space  of  the 
        /// current process, to get shared instance  use  function:
        /// P7::Traces::Get_Shared()
        /// See documentation for details.
        /// </summary>
        public bool Share(String i_sName)
        {
            System.UInt32 l_dwReturn = P7_Trace_Share(m_hHandle, i_sName);

            return (0 != l_dwReturn) ? true : false;
        }

        /// <summary>
        /// Set minimal trace verbosity, all traces with less priority will be 
        /// rejected by trace instance. You may set verbosity online from Baical
        /// server
        /// See documentation for details.
        /// </summary>
        public void Set_Verbosity(System.IntPtr i_hModule, Traces.Level i_eLevel)
        {
            P7_Trace_Set_Verbosity(m_hHandle, i_hModule, (UInt32)i_eLevel);
        }

        /// <summary>
        /// function used to specify name for current/special thread.
        /// See documentation for details.
        /// </summary>
        public bool Register_Thread(String i_sName, UInt32 i_dwThreadID = 0)
        {
            System.UInt32 l_dwReturn = P7_Trace_Register_Thread(m_hHandle, i_sName, i_dwThreadID);

            return (0 != l_dwReturn) ? true : false;
        }

        /// <summary>
        /// function used to unregister current/special thread.
        /// See documentation for details.
        /// </summary>
        public bool Unregister_Thread(UInt32 i_dwThreadID = 0)
        {
            System.UInt32 l_dwReturn = P7_Trace_Unregister_Thread(m_hHandle, i_dwThreadID);
            return (0 != l_dwReturn) ? true : false;
        }

        /// <summary>
        /// function used to register new/retrieve existing module handle
        /// See documentation for details.
        /// </summary>
        public System.IntPtr Register_Module(String i_sName)
        {
            return P7_Trace_Register_Module(m_hHandle, i_sName);
        }


        /// <summary>
        /// Send trace message to Baical server or file
        /// See documentation for details.
        /// N.B.: In release mode stack frame information is unavailable, to keep it you need to disable code optimization or disable 
        ///       code inlining: [MethodImpl(MethodImplOptions.NoInlining)], but both methods reduces code performance !
        /// </summary>
        public bool Add(UInt16        i_wTraceId, 
                        Traces.Level  i_eLevel,
                        System.IntPtr i_hModule,
                        Int32         i_dwStackFrame,
                        String        i_sMessage
                       )
        {
            StackTrace    l_cSt          = new StackTrace(true);
            StackFrame    l_cSf          = l_cSt.GetFrame(i_dwStackFrame);
            System.UInt32 l_dwReturn     = 0;
            int           l_iLineNumber  = 0;
            string        l_sMethodName  = null;
            string        l_sFileName    = null;

            if (null != l_cSf)
            {
                System.Reflection.MethodBase l_cMethod = l_cSf.GetMethod();

                if (null != l_cMethod)
                {
                    l_sMethodName = l_cMethod.Name;
                }

                l_sFileName   = l_cSf.GetFileName();
                l_iLineNumber = l_cSf.GetFileLineNumber();
            }

            l_dwReturn = P7_Trace_Managed(m_hHandle,
                                          i_wTraceId,
                                          (UInt32)i_eLevel,
                                          i_hModule,
                                          (UInt16)l_iLineNumber,
                                          (null != l_sFileName) ? l_sFileName : "<optimized>",
                                          (null != l_sMethodName) ? l_sMethodName : "<optimized>",
                                          i_sMessage
                                         );

            l_cSf = null;
            l_cSt = null;

            return (0 != l_dwReturn) ? true : false;
        }

        /// <summary>
        /// Send trace message
        /// See documentation for details.
        /// </summary>
        public bool Trace(System.IntPtr i_hModule, String i_sMessage)
        {
            return Add(0, Level.TRACE, i_hModule, m_iStackFrame, i_sMessage);
        }

        /// <summary>
        /// Send debug message
        /// See documentation for details.
        /// </summary>
        public bool Debug(System.IntPtr i_hModule, String i_sMessage)
        {
            return Add(0, Level.DEBUG, i_hModule, m_iStackFrame, i_sMessage);
        }

        /// <summary>
        /// Send info message
        /// See documentation for details.
        /// </summary>
        public bool Info(System.IntPtr i_hModule, String i_sMessage)
        {
            return Add(0, Level.INFO, i_hModule, m_iStackFrame, i_sMessage);
        }

        /// <summary>
        /// Send warning message
        /// See documentation for details.
        /// </summary>
        public bool Warning(System.IntPtr i_hModule, String i_sMessage)
        {
            return Add(0, Level.WARNING, i_hModule, m_iStackFrame, i_sMessage);
        }

        /// <summary>
        /// Send error message
        /// See documentation for details.
        /// </summary>
        public bool Error(System.IntPtr i_hModule, String i_sMessage)
        {
            return Add(0, Level.ERROR, i_hModule, m_iStackFrame, i_sMessage);
        }

        /// <summary>
        /// Send critical message
        /// See documentation for details.
        /// </summary>
        public bool Critical(System.IntPtr i_hModule, String i_sMessage)
        {
            return Add(0, Level.CRITICAL, i_hModule, m_iStackFrame, i_sMessage);
        }

        /// <summary>
        /// Increase object reference counter, thread safe
        /// See documentation for details.
        /// </summary>
        public System.Int32 AddRef()
        {
            return P7_Trace_Add_Ref(m_hHandle);
        }

        /// <summary>
        /// Decrease reference counter value, when it will be equal to 0 - channel
        /// will be destroyed.
        /// See documentation for details.
        /// </summary>
        public System.Int32 Release()
        {
            System.Int32 l_dwReturn = P7_Trace_Release(m_hHandle);
            if (0 == l_dwReturn)
            {
                m_hHandle = IntPtr.Zero;
            }
            else if (0 == l_dwReturn)
            {
                Console.WriteLine("ERROR: P7 trace reference counter is damaged !");
                m_hHandle = IntPtr.Zero;
            }

            return l_dwReturn;
        }
    }//Traces
}//P7
