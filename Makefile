all:udpserver udpsend udpmtusend udpmtuserver
udpsend:udpsend.c
	gcc -g -Wall -o udpsend udpsend.c
udpserver:udpserver.c
	gcc -g -Wall -o udpserver udpserver.c
udpmtusend:udpmtusend.c util.c
	gcc -g -Wall -o udpmtusend udpmtusend.c
udpmtuserver:udpmtuserver.c util.c
	gcc -g -Wall -o udpmtuserver udpmtuserver.c
indent: udpsend.c udpserver.c udpmtusend.c udpmtuserver.c
	indent udpsend.c  udpserver.c -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4  \
-cli0 -d0 -di1 -nfc1 -i8 -ip0 -l160 -lp -npcs -nprs -npsl -sai \
-saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1
