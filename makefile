COMPFLAGS = gcc -g -c -o
LINKFLAGS = gcc -g -o
EXTRAFLAGS = -lpthread

#FAZER make PARA COMPILAR e LINKAR##############################
All: peer gateway client
################################################################################
gateway: gateway.o peerlist.o
	$(LINKFLAGS) $@ $^ $(EXTRAFLAGS)
################################################################################
peer: peer.o photolist.o phototransfer.o keywordlist.o idlist.o
	$(LINKFLAGS) $@ $^ $(EXTRAFLAGS)

peer.o: peer.c messages.h API.h phototransfer.h
	$(COMPFLAGS) $@ $<
################################################################################
client: client.o library.o phototransfer.o
	$(LINKFLAGS) $@ $^ $(EXTRAFLAGS)

client.o: client.c messages.h API.h phototransfer.h
	$(COMPFLAGS) $@ $<
################################################################################
library.o: library.c API.h
	$(COMPFLAGS) $@ $<
photolist.o: photolist.c photolist.h
	$(COMPFLAGS) $@ $<
peerlist.o: peerlist.c peerlist.h
	$(COMPFLAGS) $@ $<
phototransfer.o: phototransfer.c phototransfer.h
	$(COMPFLAGS) $@ $<
keywordlist.o: keywordlist.c keywordlist.h
	$(COMPFLAGS) $@ $<
idlist.o: idlist.c idlist.h
	$(COMPFLAGS) $@ $<

test: keywordlist.o idlist.o
	$(LINKFLAGS) $@ $^ $(EXTRAFLAGS)
################################################################################
clean:
	rm -f gateway peer client *.o
