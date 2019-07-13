'''
P7 library wrapper
'''
import sys;
import os;
import inspect;
from ctypes import *

g_hDll     = None;
os_charp   = c_wchar_p;
g_iVerbose = 1;
g_lClients = [];

#loading library into module
if os.name == "nt":
    if sizeof(c_voidp) == 4:
        g_hDll = cdll.LoadLibrary(os.environ['P7_BIN'] + "/P7x32.dll");
    else:
        g_hDll = cdll.LoadLibrary(os.environ['P7_BIN'] + "/P7x64.dll");
        
    ############################################################################
    #>>>UTF()
    def UTF(i_sUnicode):
        return i_sUnicode;
    #<<<UTF()
        
elif os.name == "posix":
    os_charp = c_char_p;
    
    g_hDll = cdll.LoadLibrary(os.environ['P7_BIN'] + "/libP7.so");

    ############################################################################
    #>>>UTF()
    def UTF(i_sUnicode):
        return i_sUnicode.encode('utf-8');
    #<<<UTF()
    
else:
    raise NameError('OS:', os.name, " isn't supported")

################################################################################
#                            P7 Client
################################################################################
#loading P7_Client_Create function
Client_Create           = g_hDll.P7_Client_Create
Client_Create.restype   = c_void_p
Client_Create.argtypes  = [os_charp]

#loading P7_Client_Get_Shared function
Client_Get_Shared          = g_hDll.P7_Client_Get_Shared
Client_Get_Shared.restype  = c_void_p
Client_Get_Shared.argtypes = [os_charp]

#loading P7_Client_Share function
Client_Share           = g_hDll.P7_Client_Share
Client_Share.restype   = c_uint
Client_Share.argtypes  = [c_void_p, os_charp]

#loading P7_Client_Add_Ref function
Client_Add_Ref          = g_hDll.P7_Client_Add_Ref
Client_Add_Ref.restype  = c_int
Client_Add_Ref.argtypes = [c_void_p]

#loading P7_Client_Release function
Client_Release          = g_hDll.P7_Client_Release
Client_Release.restype  = c_int
Client_Release.argtypes = [c_void_p]

#loading P7_Exceptional_Flush function
Exceptional_Flush       = g_hDll.P7_Exceptional_Flush
   
################################################################################
#                            P7 telemetry
################################################################################
#loading P7_Telemetry_Create function
Telemetry_Create           = g_hDll.P7_Telemetry_Create
Telemetry_Create.restype   = c_void_p
Telemetry_Create.argtypes  = [c_void_p, os_charp, c_void_p]

#loading P7_Telemetry_Share function
Telemetry_Share           = g_hDll.P7_Telemetry_Share
Telemetry_Share.restype   = c_uint
Telemetry_Share.argtypes  = [c_void_p, os_charp]

#loading P7_Telemetry_Get_Shared function
Telemetry_Get_Shared          = g_hDll.P7_Telemetry_Get_Shared
Telemetry_Get_Shared.restype  = c_void_p
Telemetry_Get_Shared.argtypes = [os_charp]

#loading P7_Telemetry_Create_Counter function
Telemetry_Create_Counter          = g_hDll.P7_Telemetry_Create_Counter
Telemetry_Create_Counter.restype  = c_uint
Telemetry_Create_Counter.argtypes = [c_void_p, os_charp, c_double, c_double, c_double, c_double, c_int, c_void_p]
    
#loading P7_Telemetry_Create_Counter function
Telemetry_Put_Value          = g_hDll.P7_Telemetry_Put_Value
Telemetry_Put_Value.restype  = c_uint
Telemetry_Put_Value.argtypes = [c_void_p, c_ushort, c_double]

#loading P7_Telemetry_Find_Counter function
Telemetry_Find_Counter          = g_hDll.P7_Telemetry_Find_Counter
Telemetry_Find_Counter.restype  = c_uint
Telemetry_Find_Counter.argtypes = [c_void_p, os_charp, c_void_p]

#loading P7_Telemetry_Add_Ref function
Telemetry_Add_Ref          = g_hDll.P7_Telemetry_Add_Ref
Telemetry_Add_Ref.restype  = c_int
Telemetry_Add_Ref.argtypes = [c_void_p]

#loading P7_Telemetry_Release function
Telemetry_Release          = g_hDll.P7_Telemetry_Release
Telemetry_Release.restype  = c_int
Telemetry_Release.argtypes = [c_void_p]




################################################################################
#                            P7 trace
################################################################################
#loading P7_Trace_Create function
Trace_Create           = g_hDll.P7_Trace_Create
Trace_Create.restype   = c_void_p
Trace_Create.argtypes  = [c_void_p, os_charp, c_void_p]

Trace_Register_Module          = g_hDll.P7_Trace_Register_Module
Trace_Register_Module.restype  = c_void_p
Trace_Register_Module.argtypes = [c_void_p, os_charp]


#loading P7_Trace_Get_Shared function
Trace_Get_Shared           = g_hDll.P7_Trace_Get_Shared
Trace_Get_Shared.restype   = c_void_p
Trace_Get_Shared.argtypes  = [os_charp]

#loading P7_Trace_Share function
Trace_Share          = g_hDll.P7_Trace_Share
Trace_Share.restype  = c_uint
Trace_Share.argtypes = [c_void_p, os_charp]

#loading P7_Trace_Set_Verbosity function
Trace_Set_Verbosity          = g_hDll.P7_Trace_Set_Verbosity
Trace_Set_Verbosity.argtypes = [c_void_p, c_void_p, c_uint]

#loading P7_Telemetry_Share function
Trace_Add           = g_hDll.P7_Trace_Managed
Trace_Add.restype   = c_uint
Trace_Add.argtypes  = [c_void_p, c_ushort, c_uint, c_void_p, c_ushort, os_charp, os_charp, os_charp]

#loading P7_Telemetry_Add_Ref function
Trace_Add_Ref          = g_hDll.P7_Trace_Add_Ref
Trace_Add_Ref.restype  = c_int
Trace_Add_Ref.argtypes = [c_void_p]

#loading P7_Client_Release function
Trace_Release          = g_hDll.P7_Trace_Release
Trace_Release.restype  = c_int
Trace_Release.argtypes = [c_void_p]


################################################################################
#                           Client class
################################################################################
class Client:
    m_hHandle     = None
    m_sName       = None
    #keep this references to prevent dll and function unloads before garbage
    #collector start working
    m_hDll        = None
    m_fRelease    = None
    def __init__(self, i_sName, i_hHandle, i_hDll, i_fRelease):
        self.m_hHandle  = i_hHandle;
        self.m_hDll     = i_hDll;
        self.m_fRelease = i_fRelease;
        self.m_sName    = i_sName;
        
    def __del__(self):
        l_iRC = 0;
        
        if g_iVerbose != 0:
            print("P7 Client::__del__(", self.m_sName, "), entering .... ");
        
        if self.m_hHandle != None and self.m_fRelease != None:
            l_iRC = self.m_fRelease(self.m_hHandle);
        else:
            print("P7 Client::__del__(", self.m_sName, ") Error, not correct data");
     
        if g_iVerbose != 0:
            print("P7 Client::__del__(", self.m_sName, "), reference counter = ", l_iRC);
            
        self.m_fRelease = None;
        self.m_hDll     = None;    
#<<< class Client



################################################################################
#                           Telemetry class
################################################################################
class Telemetry:
    m_hHandle     = None
    m_sName       = None
    #keep this references to prevent dll and function unloads before garbage
    #collector start working
    m_hDll        = None
    m_fRelease    = None
    ##--------------------------------------------------------------------------
    def __init__(self, i_sName, i_hHandle, i_hDll, i_fRelease):
        self.m_hHandle  = i_hHandle;
        self.m_hDll     = i_hDll;
        self.m_fRelease = i_fRelease;
        self.m_sName    = i_sName;
    #<<< __init__    
        
    ##--------------------------------------------------------------------------
    def __del__(self):
        l_iRC = 0;
        
        if g_iVerbose != 0:
            print("P7 Telemetry::__del__(", self.m_sName, "), entering .... ");
        
        if self.m_hHandle != None and self.m_fRelease != None:
            l_iRC = self.m_fRelease(self.m_hHandle);
        else:
            print("P7 Telemetry::__del__(", self.m_sName, ") Error, not correct data");
     
        if g_iVerbose != 0:
            print("P7 Telemetry::__del__(", self.m_sName, "), reference counter = ", l_iRC);
            
        self.m_fRelease = None;
        self.m_hDll     = None;
    #<<< __del__    
        
    ##--------------------------------------------------------------------------
    #return counter ID, or -1 in case of error
    def Create(self, i_sName, i_llMin, i_llAlarmMin, i_llMax, i_llAlarmMax, i_bOn):
        l_bId  = c_ushort(65535);
        l_iRes = Telemetry_Create_Counter(self.m_hHandle,
                                          i_sName,
                                          i_llMin,
										  i_llAlarmMin,
                                          i_llMax,
                                          i_llAlarmMax,
                                          i_bOn,
                                          byref(l_bId)
                                         );
        if l_iRes == 0:
            l_bId = -1;
            print("P7 Telemetry::Create_Counter(", i_sName, "), failed to create");
        return l_bId    
    #<<< Create_Counter    

    ##--------------------------------------------------------------------------
    #return True/False
    def Add(self, i_bId, i_llValue):
        l_iRes = Telemetry_Put_Value(self.m_hHandle, i_bId, i_llValue);
        
        if l_iRes == 0:
            return False;
        
        return True;   
    #<<< Add    

    ##--------------------------------------------------------------------------
    #return counter ID, or -1 in case of error
    def Find_Counter(self, i_sName):
        l_bId  = c_ushort(65535);
        l_iRes = Telemetry_Find_Counter(self.m_hHandle,
                                        i_sName,
                                        byref(l_bId)
                                       );
        if l_iRes == 0:
            l_bId = -1;
            print("P7 Telemetry::Find_Counter(", i_sName, "), failed to find by name");
        return l_bId    
    #<<< Create_Counter    
#<<< class Telemetry


################################################################################
#                           Trace class
################################################################################
class Traces:
    #levels
    m_iTrace     = 0;
    m_iDebug     = 1;
    m_iInfo      = 2;
    m_iWarning   = 3;
    m_iError     = 4;
    m_iCritical  = 5;
    
    m_hHandle    = None;
    m_sName      = None;
    #keep this references to prevent dll and function unloads before garbage
    #collector start working
    m_hDll       = None;
    m_fRelease   = None;
    m_iStack     = -1;
    
    
    ##--------------------------------------------------------------------------
    def __init__(self, i_sName, i_hHandle, i_hDll, i_fRelease):
        self.m_hHandle  = i_hHandle;
        self.m_hDll     = i_hDll;
        self.m_fRelease = i_fRelease;
        self.m_sName    = i_sName;
    #<<< __init__
    
        
    ##--------------------------------------------------------------------------
    def __del__(self):
        l_iRC = 0;
        
        if g_iVerbose != 0:
            print("P7 Traces::__del__(", self.m_sName, "), entering .... ");
        
        if self.m_hHandle != None and self.m_fRelease != None:
            l_iRC = self.m_fRelease(self.m_hHandle);
        else:
            print("P7 Traces::__del__(", self.m_sName, ") Error, not correct data");
     
        if g_iVerbose != 0:
            print("P7 Traces::__del__(", self.m_sName, "), reference counter = ", l_iRC);
            
        self.m_fRelease = None;
        self.m_hDll     = None;
    #<<< __del__


    ##--------------------------------------------------------------------------
    #Extracting information about stack, file line, name & function takes
    #a lot of time, about additional 500 microseconds on modern PC (2014)
    #if you disable stack information - trace functions will be accelerated
    #about 50-150 times depending on python version 
    def Enable_Stack_Info(self, i_bEnabled):
        if i_bEnabled:
            self.m_iStack = 1;
        else:
            self.m_iStack = 0;
    #<<< Enable_Stack_Info


    ##--------------------------------------------------------------------------
    def Register_Module(self, i_sName):
        return Trace_Register_Module(self.m_hHandle, i_sName);
    #<<< Set_Verbosity
    

    ##--------------------------------------------------------------------------
    def Set_Verbosity(self, i_hModule, i_iLevel):
        Trace_Set_Verbosity(self.m_hHandle, i_hModule, i_iLevel);
    #<<< Set_Verbosity    

    
    ##--------------------------------------------------------------------------
    def Trace_Embedded(self, i_hModule, i_sMessage, i_iLevel, i_iStackFrame = 1):
        l_iId = 0;
        
        #cheching that stack inspect working as predicted, workaround for python
        #bug when instead file name & module name it returns (<string>, <string>)
        #and the operation takes a lot of time
        #This check will take place only for frist function call
        if self.m_iStack < 0:
            l_sStackFileName1 = os.path.basename(inspect.stack()[1][1]);
            l_sStackFileName2 = os.path.basename(inspect.stack()[i_iStackFrame][1]);
            l_sRealFileName   = os.path.basename(__file__);
            if l_sStackFileName1.lower() != l_sRealFileName.lower() or l_sStackFileName2.lower() == "<string>":
                self.m_iStack = 0;
            else:
                self.m_iStack = 1;
        
        if i_iStackFrame > 0 and self.m_iStack != 0:
            l_cFrame = inspect.stack()[i_iStackFrame];
        else:
            l_cFrame = [0, u"-", 0, u"-"];
            l_iId    = 1;
            
        l_iRes = Trace_Add(self.m_hHandle,
                           l_iId,
                           i_iLevel,
                           i_hModule,
                           l_cFrame[2], #file line number
                           UTF(l_cFrame[1]), #file name
                           UTF(l_cFrame[3]), #function name
                           i_sMessage
                          );
        if l_iRes == 0:
            return False;
        
        return True;   
    #<<< Trace_Embedded    


    ##--------------------------------------------------------------------------
    def Trace(self, i_hModule, i_sMessage, i_bUseStackInfo = True):
        return self.Trace_Embedded(i_hModule,
                                   i_sMessage,
                                   self.m_iTrace,
                                   2 if i_bUseStackInfo else 0
                                   );
    #<<< Trace    


    ##--------------------------------------------------------------------------
    def Debug(self, i_hModule, i_sMessage, i_bUseStackInfo = True):
        return self.Trace_Embedded(i_hModule,
                                   i_sMessage,
                                   self.m_iDebug,
                                   2 if i_bUseStackInfo else 0
                                   );
    #<<< Debug    


    ##--------------------------------------------------------------------------
    def Info(self, i_hModule, i_sMessage, i_bUseStackInfo = True):
        return self.Trace_Embedded(i_hModule,
                                   i_sMessage,
                                   self.m_iInfo,
                                   2 if i_bUseStackInfo else 0
                                   );
    #<<< Info    


    ##--------------------------------------------------------------------------
    def Warning(self, i_hModule, i_sMessage, i_bUseStackInfo = True):
        return self.Trace_Embedded(i_hModule,
                                   i_sMessage,
                                   self.m_iWarning,
                                   2 if i_bUseStackInfo else 0
                                   );
    #<<< Warning    


    ##--------------------------------------------------------------------------
    def Error(self, i_hModule, i_sMessage, i_bUseStackInfo = True):
        return self.Trace_Embedded(i_hModule,
                                   i_sMessage,
                                   self.m_iError,
                                   2 if i_bUseStackInfo else 0
                                   );
    #<<< Error    


    ##--------------------------------------------------------------------------
    def Critical(self, i_hModule, i_sMessage, i_bUseStackInfo = True):
        return self.Trace_Embedded(i_hModule,
                                   i_sMessage,
                                   self.m_iCritical,
                                   2 if i_bUseStackInfo else 0
                                   );
    #<<< Critical    
#<<< class Traces


################################################################################
#>>>Register_Client()
#Function register new client instance
#return True/False
def Register_Client(i_sName, i_sArgs = None):
    l_bReturn = True;
    l_hClient = Client_Get_Shared(i_sName);

    if l_hClient != None:
        Client_Release(l_hClient);
        l_hClient = None;
        l_bReturn = False;
        print("P7::Register_Client(", i_sName, "), object with the same name is already registerd");
    else:
        l_hClient = Client_Create(i_sArgs);
        if l_hClient != None:
            Client_Share(l_hClient, i_sName);
            g_lClients.append(Client(i_sName, l_hClient, g_hDll, Client_Release));
        else:
            l_bReturn = False;
            print("P7::Register_Client(", i_sName, "), failed to create new client object");
            
    return l_bReturn;
#<<<Register_Client()


################################################################################
#>>>Get_Telemetry_Channel()
def Get_Telemetry_Channel(i_sTelemetryName, i_sClientName = None):
    #trying to get shared telemetry instance
    l_hTel = Telemetry_Get_Shared(i_sTelemetryName);
    
    if l_hTel != None:
        return Telemetry(i_sTelemetryName, l_hTel, g_hDll, Telemetry_Release);
    
    #if shared instance of telemetry isn't exists, trying to get existing
    #client instance
    l_hClient = Client_Get_Shared(i_sClientName);
    
    if l_hClient == None:
        if i_sClientName == None:
            i_sClientName = "None";
        print("P7::Get_Telemetry_Channel(",
              i_sTelemetryName,
              "), failed to retrieve client with name:",
              i_sClientName);
        return None;

    #creating new telemetry channel instance
    l_cReturn = None;
    l_hTel    = Telemetry_Create(l_hClient, i_sTelemetryName, 0)
    
    if l_hTel != None:
        Telemetry_Share(l_hTel, i_sTelemetryName);
        l_cReturn = Telemetry(i_sTelemetryName, l_hTel, g_hDll, Telemetry_Release);
    else:
        print("P7::Get_Telemetry_Channel(",
              i_sTelemetryName,
              "), failed to create");
        
    Client_Release(l_hClient);
    l_hClient = None;
         
    return l_cReturn;        
#<<<Get_Telemetry_Channel()


################################################################################
#>>>Get_Trace_Channel()
def Get_Trace_Channel(i_sTraceName, i_sClientName = None):
    #trying to get shared telemetry instance
    l_hTel = Trace_Get_Shared(i_sTraceName);
    
    if l_hTel != None:
        return Traces(i_sTraceName, l_hTel, g_hDll, Trace_Release);
    
    #if shared instance of telemetry isn't exists, trying to get existing
    #client instance
    l_hClient = Client_Get_Shared(i_sClientName);
    
    if l_hClient == None:
        if i_sClientName == None:
            i_sClientName = "None";
        print("P7::Get_Trace_Channel(",
              i_sTraceName,
              "), failed to retrieve client with name:",
              i_sClientName);
        return None;

    #creating new telemetry channel instance
    l_cReturn = None;
    l_hTel    = Trace_Create(l_hClient, i_sTraceName, 0)
    
    if l_hTel != None:
        Trace_Share(l_hTel, i_sTraceName);
        l_cReturn = Traces(i_sTraceName, l_hTel, g_hDll, Trace_Release);
    else:
        print("P7::Get_Trace_Channel(",
              i_sTraceName,
              "), failed to create");
        
    Client_Release(l_hClient);
    l_hClient = None;
         
    return l_cReturn;        
#<<<Get_Trace_Channel()


################################################################################
#>>>Exceptional_Flush_Buffers()
def Exceptional_Flush_Buffers():
    Exceptional_Flush();
#<<<Exceptional_Flush_Buffers()

################################################################################
#Register_Client("MyClient","/P7.Addr=127.0.0.1");
#
#l_cTel   = Get_Telemetry_Channel("MyTelemetry", "MyClient");
#l_cTrace = Get_Trace_Channel("MyTrace", "MyClient");
#
#bId1 = l_cTel.Create_Counter("MyCounter1", 0, 1000, 950, 1);
#bId2 = l_cTel.Create_Counter("MyCounter2", 0, 1000, 950, 1);
#
#for num in range(0, 10000):
#    l_cTel.Add(bId1, num % 1000)
#    l_cTel.Add(bId2, (num + 500) % 1000)
#
#for num in range(0, 1000):
#    l_cTrace.Trace(0, "test trace message");
#    l_cTrace.Info(0, "test info message");
#    l_cTrace.Critical(0, "test critical message");


