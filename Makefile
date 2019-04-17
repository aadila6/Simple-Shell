sshell: sshell.o
	gcc -Wall -Werror sshell.o -o sshell

sshell.o: sshell.c
	gcc -Wall -Werror -c sshell.c -o sshell.o

clean:
	rm sshell *.o

