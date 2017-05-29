COMPFLAGS = gcc -g -c -o
LINKFLAGS = gcc -g -o
EXTRAFLAGS = -lpthread

#FAZER make PARA COMPILAR e LINKAR##############################
All: server gateway client client_add
################################################################################
gateway: gateway.o serverlist.o
	$(LINKFLAGS) $@ $^ $(EXTRAFLAGS)
################################################################################
server: server.o photolist.o phototransfer.o keywordlist.o idlist.o
	$(LINKFLAGS) $@ $^ $(EXTRAFLAGS)

server.o: server.c messages.h clientAPI.h phototransfer.h
	$(COMPFLAGS) $@ $<
################################################################################
client: client.o clientAPI.o phototransfer.o
	$(LINKFLAGS) $@ $^ $(EXTRAFLAGS)

client.o: client.c messages.h clientAPI.h phototransfer.h
	$(COMPFLAGS) $@ $<

client_add: client_add.o clientAPI.o phototransfer.o
	$(LINKFLAGS) $@ $^ $(EXTRAFLAGS)

client_add.o: client_add.c messages.h clientAPI.h phototransfer.h
	$(COMPFLAGS) $@ $<
################################################################################
clientAPI.o: clientAPI.c clientAPI.h
	$(COMPFLAGS) $@ $<
photolist.o: photolist.c photolist.h
	$(COMPFLAGS) $@ $<
serverlist.o: serverlist.c serverlist.h
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
	rm -f gateway server client *.o
