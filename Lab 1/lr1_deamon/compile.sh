gcc -c deamonize.c -I ../apue/include/ -L ../apue/lib/ -lapue
gcc -c already_running.c
gcc -c main.c
gcc -c lockfile.c
gcc -o prog lockfile.c deamonize.o already_running.o main.o -I apue/include/ -L apue/lib/ -lapue
