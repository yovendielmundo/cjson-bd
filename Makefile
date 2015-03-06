BIN = client spr
OBJS =	$(BIN:=.o) lib/slr.o lib/socket.o lib/common.o lib/prompt.o dbs/dbs.o dbs/table/user_table.o

CFLAGS = -g -O2 -Wall
LDFLAGS = -pthread

all: $(BIN)

client: client.o lib/common.o lib/socket.o lib/slr.o lib/prompt.o

spr: spr.o lib/common.o lib/socket.o lib/slr.o dbs/dbs.o dbs/table/user_table.o

slr.o: lib/slr.c

common.o: lib/common.c

socket.o: lib/socket.c

prompt.o: lib/prompt.c

dbs.o: dbs/dbs.c 

user_table.o: dbs/table/user_table.c

.PHONY: clean
clean:
	-rm -rf $(OBJS) $(BIN) *.log core *~
