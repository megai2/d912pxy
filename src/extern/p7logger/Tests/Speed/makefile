#///////////////////////////////////////////////////////////////////////////////
#                          P7 speed  test                                     //
#///////////////////////////////////////////////////////////////////////////////
#To compile successfully you should respect next folder location:
#  -P7
#    -Headers
#    -Sources
#    -Tests
#  -Shared 

# Environment
TEMP_DIR=P7_Speed_Test_Temp
BIN_DIR=./../../Binaries
H1_DIR=./../../Shared
H2_DIR=$(H1_DIR)/Platforms/Linux_x86
H3_DIR=./../../Headers
OBJ_DIR=${BIN_DIR}/${TEMP_DIR}

$(BIN_DIR)/P7_Speed_Test:$(OBJ_DIR)/Speed.o
	g++ -o $(BIN_DIR)/P7_Speed_Test $(OBJ_DIR)/Speed.o $(BIN_DIR)/libP7.a -lpthread -ldl -lrt

$(OBJ_DIR)/Speed.o:Speed.cpp
	rm -f -r $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)
	g++ -Wall -c -O2 -I$(H1_DIR) -I$(H2_DIR) -I$(H3_DIR) -MMD -MP -MF $(OBJ_DIR)/Speed.o.d -o $(OBJ_DIR)/Speed.o Speed.cpp

clean:
	rm -f -r $(OBJ_DIR)
	
