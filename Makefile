CC=g++

TARGETS= snmpGet main

NET_SNMP_CONFIG=net-snmp-config
CFLAGS=`$(NET_SNMP_CONFIG) --cflags` -Wall -Wextra -Werror \
	-Wno-unused-parameter
BUILDLIBS=`$(NET_SNMP_CONFIG) --libs`
BUILDAGENTLIBS=`$(NET_SNMP_CONFIG) --agent-libs`

DLFLAGS=-fPIC -shared

all: $(TARGETS)

snmpGet: inc/snmpGet.o
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o $@ inc/snmpGet.c

main: src/main.o
	$(CC) -o $@ src/$@.o inc/snmpGet.o $(BUILDLIBS) -lpqxx -lpq

clean:
	rm -f -- *.o $(TARGETS)