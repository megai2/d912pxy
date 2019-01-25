#///////////////////////////////////////////////////////////////////////////////
#                          P7.Trace library                                   //
#///////////////////////////////////////////////////////////////////////////////
#To compile successfully you should respect next folder location:
#  -P7
#    -Headers
#    -Sources
#    -Tests
#  -Shared 

#mkdir -p ./../Binaries
#mkdir -p ./../Binaries/Temp

# Environment
TEMP_DIR=P7_Client_Temp
BIN_DIR=./../Binaries
H1_DIR=./../Shared
H2_DIR=$(H1_DIR)/Platforms/Linux_x86
H3_DIR=./../Headers
OBJ_DIR=${BIN_DIR}/${TEMP_DIR}

$(BIN_DIR)/libP7.so:$(BIN_DIR)/libP7.a $(OBJ_DIR)/Proxy.o $(OBJ_DIR)/Trace.o $(OBJ_DIR)/CRC32.o $(OBJ_DIR)/Client.o $(OBJ_DIR)/Telemetry.o $(OBJ_DIR)/ClBaical.o $(OBJ_DIR)/ClNull.o $(OBJ_DIR)/ClFile.o $(OBJ_DIR)/ClText.o
	g++ -shared -fPIC -Wl,-soname,libP7.so.2 -o $(BIN_DIR)/libP7.so $(OBJ_DIR)/Proxy.o $(OBJ_DIR)/Trace.o $(OBJ_DIR)/CRC32.o $(OBJ_DIR)/Client.o $(OBJ_DIR)/Telemetry.o $(OBJ_DIR)/ClBaical.o $(OBJ_DIR)/ClNull.o $(OBJ_DIR)/ClFile.o $(OBJ_DIR)/ClText.o -lpthread -ldl -lrt

$(BIN_DIR)/libP7.a:$(OBJ_DIR)/Proxy.o $(OBJ_DIR)/Trace.o $(OBJ_DIR)/CRC32.o $(OBJ_DIR)/Client.o $(OBJ_DIR)/Telemetry.o $(OBJ_DIR)/ClBaical.o $(OBJ_DIR)/ClNull.o $(OBJ_DIR)/ClFile.o $(OBJ_DIR)/ClText.o
	ar -rv $(BIN_DIR)/libP7.a $(OBJ_DIR)/Proxy.o $(OBJ_DIR)/Trace.o $(OBJ_DIR)/CRC32.o $(OBJ_DIR)/Client.o $(OBJ_DIR)/Telemetry.o $(OBJ_DIR)/ClBaical.o $(OBJ_DIR)/ClNull.o $(OBJ_DIR)/ClFile.o $(OBJ_DIR)/ClText.o

$(OBJ_DIR)/Proxy.o:Proxy.cpp $(OBJ_DIR)/Trace.o
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/Proxy.o.d -o $(OBJ_DIR)/Proxy.o Proxy.cpp

$(OBJ_DIR)/Trace.o:Trace.cpp $(OBJ_DIR)/Telemetry.o
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/Trace.o.d -o $(OBJ_DIR)/Trace.o Trace.cpp

$(OBJ_DIR)/Telemetry.o:Telemetry.cpp $(OBJ_DIR)/ClBaical.o
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/Telemetry.o.d -o $(OBJ_DIR)/Telemetry.o Telemetry.cpp

$(OBJ_DIR)/ClBaical.o:ClBaical.cpp $(OBJ_DIR)/ClNull.o
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/ClBaical.o.d -o $(OBJ_DIR)/ClBaical.o ClBaical.cpp

$(OBJ_DIR)/ClNull.o:ClNull.cpp $(OBJ_DIR)/ClFile.o
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/ClNull.o.d -o $(OBJ_DIR)/ClNull.o ClNull.cpp

$(OBJ_DIR)/ClFile.o:ClFile.cpp $(OBJ_DIR)/Client.o
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/ClFile.o.d -o $(OBJ_DIR)/ClFile.o ClFile.cpp

$(OBJ_DIR)/Client.o:Client.cpp $(OBJ_DIR)/ClText.o
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/Client.o.d -o $(OBJ_DIR)/Client.o Client.cpp

$(OBJ_DIR)/ClText.o:ClText.cpp $(OBJ_DIR)/CRC32.o
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/ClText.o.d -o $(OBJ_DIR)/ClText.o ClText.cpp

$(OBJ_DIR)/CRC32.o:CRC32.cpp
	rm -f -r $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)
	g++ -Wall -fPIC -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/CRC32.o.d -o $(OBJ_DIR)/CRC32.o CRC32.cpp

clean:
	rm -f -r $(OBJ_DIR)
	
