'''
P7 library usage example
'''
import sys;
import os;

#-------------------------------------------------------------------------------
#Loading P7 module by relative path and update PATH system env. variable to be
#able to load P7Client_XX.dll/SO
l_sPath = os.path.dirname(os.path.realpath(__file__)) + "/../../Wrappers/Py/";
if l_sPath not in sys.path:
    sys.path.insert(0, l_sPath)

l_sPath = os.path.dirname(os.path.realpath(__file__)) + "/../../Binaries/";
if l_sPath not in os.environ['PATH']:
    os.environ['PATH'] += os.pathsep + l_sPath;
os.environ['P7_BIN'] = l_sPath;

import P7;

#-------------------------------------------------------------------------------
#registering P7 client - transport engine. Every client can handle up to 32
#individual channels (telemetry, trace).
#You may do it once for many scripts running as single program, or create
#many clients with unique names and reuse them by trace or telemetry channels.
#Creating it one you can reuse it from different modules.
#N.B.: information about argument string parameters is located in documentation
P7.Register_Client(P7.UTF(u"MyClient"),
                   P7.UTF(u"/P7.Addr=localhost /P7.Sink=Auto /P7.Pool=8192 /P7.Name=PytonExample"));

#-------------------------------------------------------------------------------
#creating telemetry channel or getting already created one (by name)
#Client with name "MyClient" have to be created before
l_cTel = P7.Get_Telemetry_Channel(P7.UTF(u"MyTelemetry"), P7.UTF(u"MyClient"));

#create 2 telemetry counters
bId1 = l_cTel.Create(P7.UTF(u"MyCounter1"), 0, 1000, 950, 1);
bId2 = l_cTel.Create(P7.UTF(u"MyCounter2"), 0, 1000, 950, 1);

#deliver telemetry samples
for num in range(0, 10000):
    l_cTel.Add(bId1, num % 1000)
    l_cTel.Add(bId2, (num + 500) % 1000)


#-------------------------------------------------------------------------------
#creating trace channel or getting already created one (by name)
#Client with name "MyClient" have to be created before
l_cTrace = P7.Get_Trace_Channel(P7.UTF(u"MyTrace"), P7.UTF(u"MyClient"));

#register module
l_hModule = l_cTrace.Register_Module(P7.UTF(u"MyModule"));

#set verbosity level for the module
l_cTrace.Set_Verbosity(l_hModule, l_cTrace.m_iTrace);

#deliver trace messages
for num in range(0, 1000):
    l_cTrace.Trace(l_hModule, P7.UTF(u"test trace message"));
    l_cTrace.Info(l_hModule, P7.UTF(u"test info message"));
    l_cTrace.Error(l_hModule, P7.UTF(u"test critical message"));


#Extracting information about stack, file line, name & function takes
#a lot of time, about additional 500 microseconds on modern PC (2014)
#if you disable stack information - trace functions will be accelerated
#about 50-150 times depending on python version 
l_cTrace.Enable_Stack_Info(False);

#deliver trace messages again without stack information (file name, line, func)
#if prev. delivering of 3000 messages takes more than 2.5 seconds on Intel Core2
#duo - wihtout trace info it takes about 40 ms, this is price for accessing to
#python inspect.stack() function
for num in range(0, 1000):
    l_cTrace.Debug(l_hModule, P7.UTF(u"test trace message"));
    l_cTrace.Warning(l_hModule, P7.UTF(u"test info message"));
    l_cTrace.Critical(l_hModule, P7.UTF(u"test critical message"));
    
#or you can enable stack information for channel and disable for specific traces    
l_cTrace.Enable_Stack_Info(True);
l_cTrace.Trace(l_hModule, P7.UTF(u"test trace message without stack information"), False);

    
#objects will be destroyed automatically by garbage collector and rest of the
#data collected in internal buffers of P7 will be delivered, but if your app. is
#crushed and you want to deliver buffered data you may call function
#P7.Exceptional_Flush_Buffers()
