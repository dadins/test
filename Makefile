#gcc -g -O0 -o server util.c conf.c connection.c worker.c output.c server.c -I./include -lpthread -ldl
#gcc -shared -fpic -O0 -o libjaws.so output_jaws.c -I./include
CC = gcc
CFLAGS = -O0
CLIBS  = -lpthread -ldl
INCDIR = -I./include

all: server client 

CLIENT_OBJS = client.o
SERVER_OBJS = util.o conf.o connection.o worker.o output.o server.o

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(INCDIR) $(SERVER_OBJS) -o $@ $(CLIBS) 

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(INCDIR) $(CLIENT_OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@

###############################################lib.so############################################
so: libjaws.so libps.so libnetstat.so libls.so

SHARE_CFLAGS = $(CFLAGS) -shared -fpic

JAWS_OBJS 		= output_jaws.o 
PS_OBJS 		= output_ps.o
NETSTAT_OBJS 	= output_netstat.o
LS_OBJS 		= output_ls.o

libjaws.so: $(JAWS_OBJS)
	$(CC) $(SHARE_CFLAGS) $(INCDIR) $(JAWS_OBJS) -o $@

libps.so: $(PS_OBJS)
	$(CC) $(SHARE_CFLAGS) $(INCDIR) $(PS_OBJS) -o $@

libls.so: $(LS_OBJS)
	$(CC) $(SHARE_CFLAGS) $(INCDIR) $(LS_OBJS) -o $@

libnetstat.so: $(NETSTAT_OBJS)
	$(CC) $(SHARE_CFLAGS) $(INCDIR) $(NETSTAT_OBJS) -o $@

output_%.o: output_%.c
	$(CC) $(SHARE_CFLAGS) $(INCDIR) -c $< -o $@

clean:
	@rm -f server client *.so *.o
