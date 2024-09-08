# Makefile

all: dbclient dbserver

dbclient:
	gcc dbclient.c -o dbclient -Wall -Werror -std=gnu99

dbserver:
	gcc dbserver.c -o dbserver -Wall -Werror -std=gnu99 -pthread

clean:
	rm -f dbclient dbserver
