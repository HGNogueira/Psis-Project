COMPFLAGS = gcc -Wall -c -o
LINKFLAGS = gcc -Wall -o

#FAZER make PARA COMPILAR e LINKAR##############################
life3d-mpi: life3d-mpi.o $(LL_LIB_PATH)/linked_list.o $(HT_LIB_PATH)/hashtable.o
	$(LINKFLAGS_MPI) $@ $^ $(EXTRA_FLAGS)

life3d-mpi.o: new_ver.c $(LL_LIB_PATH)/linked_list.h $(HT_LIB_PATH)/hashtable.h
	$(COMPFLAGS_MPI) $@ $<
################################################################################
#FAZER make serial PARA COMPILAR E LINKAR VERSAO SERIAL#########################
serial: life3d

life3d: life3d.o $(LL_LIB_PATH)/linked_list.o $(HT_LIB_PATH)/hashtable.o
	$(LINKFLAGS) $@ $^

life3d.o: life3d.c $(LL_LIB_PATH)/linked_list.h $(HT_LIB_PATH)/hashtable.h
	$(COMPFLAGS) $@ $<
################################################################################
#FAZER make clean PARA LIMPAR APENAS OS FICHEIROS DO PROGRAMA###################
clean:
	rm -f life3d-mpi *.o
#FAZER make clean_all PARA LIMPAR APP FILES E A BIBLIOTECAS#####################
clean_all:
	rm -f life3d-mpi life3d *.o  $(LL_LIB_PATH)/*.o $(HT_LIB_PATH)/*.o
################################################################################
