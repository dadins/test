gcc -g -O0 -o server util.c conf.c connection.c worker.c output.c server.c -I./include -lpthread -ldl
gcc -shared -fpic -O0 -o libjaws.so output_jaws.c -I./include
