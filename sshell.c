#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h> 
#include <fcntl.h>
#define CMDLINE_MAX 512

typedef int Pipe[2];
struct Node { 
  		char **cmd; 
  		struct Node *next; 
}; 
int CountNumArgs(char *buffer){
	if (strtok(buffer, " \n") == NULL) { // base case
		return 0;
	}
	int num = 1; 
	while (strtok(NULL, " \n") != NULL) {
		num++;
	}
	return num;
}
char** parseEachPipe(char *buffer){
	char *ptr = strtok(buffer, " \n");
	char **cmd = NULL;
	int cnt = 0;
	int in = 0;
	cnt = CountNumArgs(buffer);
	cmd = malloc(sizeof(char *) * (cnt +1));
	for(int i=0; i<cnt; i++){
		cmd[i] = NULL;
	}
	while (ptr != NULL){
		cmd[in] = ptr;
		// fprintf(stdout, "%s\n", ptr);
		ptr = strtok(NULL, " \n");
		in++;
	}
	return cmd;
}

struct Node* parsePipe(char *buffer){ 
	//return the ptr to the head of the linked list and we will pass this to main
	char *token = strtok(buffer, "|");
	// int count = 0;
	struct Node *HEAD = NULL;
	HEAD = (struct Node*)malloc(sizeof(struct Node));
	struct Node *temp = HEAD;
	while (token != NULL) {
		temp->cmd = parseEachPipe(token);
		token = strtok(NULL, "|\n");
		// fprintf(stdout, "%s\n", token);
		if(token != NULL){
			temp->next = (struct Node*)malloc(sizeof(struct Node));
			temp = temp->next;
		}
	}
	return HEAD;
}
void pipeline(struct Node *plist){
	int fd[2];
	pipe(fd);
	while(plist->next != NULL){
			/* Create pipe */
		if (fork() != 0) {
			/* Parent */        
			close(fd[0]);                       
			dup2(fd[1], STDOUT_FILENO);          
			close(fd[1]);                         
			execvp(plist->cmd[0],plist->cmd);
		} 
		else { /* Child */        
			close(fd[1]);       
			close(STDIN_FILENO);
			/* Close stdin */        
			dup(fd[0]);     
			close(fd[0]);
			/* Close the empty file descriptor */        
			execvp(plist->next->cmd[0],plist->next->cmd);
		}
		plist = plist->next;
	}
}
int main(int argc, char **argv){
	pid_t pid;
	int status;
	char buffer[CMDLINE_MAX]; //original string, untouched
	char buffer1[CMDLINE_MAX];
	char buffer2[CMDLINE_MAX];
	char buffer3[CMDLINE_MAX];//copy 1 of our original string which will be use for initial parsing.
	bool run = true;
	int fd;
	while (run){
		char **cmd;
		fprintf(stdout, "sshell$ ");
		fgets(buffer, CMDLINE_MAX, stdin);
		strcpy (buffer1, buffer);
		strcpy (buffer2, buffer);
		strcpy (buffer3, buffer);
		char dir = 'a';
		// bool check = false;
		char *fileName = NULL;
		
		if (!isatty(STDIN_FILENO)) {
        printf("%s", buffer);
        fflush(stdout);
    	}

		for(int j=0; j<strlen(buffer1); j++) {
			if(buffer1[j] == '<'){
				dir = '<';
				strtok(buffer1, "'<'");
				fileName = strtok(NULL, " \n");
			}else if(buffer1[j] == '>'){
				dir = '>';
				strtok(buffer1, "'>'");
				fileName = strtok(NULL, " \n");
			}
		}
		int count;
		count = CountNumArgs(buffer);
		cmd = malloc(sizeof(char *) * (count+1));
		if(fileName == NULL && dir != 'a' && count !=1){
			if(dir == '>'){
				fprintf(stdout,"%s\n", "Error: no output file");
				goto emptyMemo;
			} else if(dir == '<'){
				fprintf(stdout,"%s\n", "Error: no input file");
				goto emptyMemo;
			}
		}
		for (int i=0; i<=count; i++){
			cmd[i] = NULL;
		}
		//Parsing Begins
		char *ptr = strtok(buffer1, " \n");
		int cnt = 0;
		// strtok(buffer2, "\n");
		
		while (ptr != NULL){
			// fprintf(stdout, "%s\n", ptr);
			cmd[cnt] = ptr;
			ptr = strtok(NULL, " \n");
			cnt = cnt + 1;
		}
		if (strcmp(cmd[0],"exit") == 0){
			fprintf(stderr,"%s\n","Bye...");
			run = false;
			goto emptyMemo;
		}
		if (strcmp(cmd[0],"pwd") == 0){
			getcwd (buffer1, CMDLINE_MAX);
			fprintf(stdout,"%s\n", buffer1);
			strtok(buffer3, "\n");
			fprintf(stdout, "+ completed '%s' [%d]\n", buffer3, WEXITSTATUS(status));
			goto emptyMemo;
		}
		if (strcmp(cmd[0],"cd") == 0){
			int ret = chdir(cmd[1]);
       		if (ret != 0) {
				fprintf(stderr,"%s\n","Error: no such directory");
			}else {
				getcwd (buffer1, CMDLINE_MAX);
				fprintf(stdout,"%s\n", buffer1);
				strtok(buffer3, "\n");
				fprintf(stdout, "+ completed '%s' [%d]\n", buffer3, WEXITSTATUS(status));
			}
			goto emptyMemo;
		}
		pid = fork();
		if (pid == 0){ /* Child */   
			if(dir == '>'){
				fd = open(fileName, O_WRONLY | O_CREAT, S_IRWXU);
				dup2(fd,STDOUT_FILENO);
				close(fd);
			} else if(dir =='<'){
				fd = open(fileName, O_RDONLY);
				dup2(fd,STDIN_FILENO);
				close(fd);
			}
			/* Phase 8: Sleep*/
			char *sleep = "sleep";
			if (!strcmp(cmd[0],sleep)){ 
				pid_t pid2 = fork();
				int reval;
				if (pid2==0){
					execvp(cmd[0], cmd);
					fprintf(stdout,"%s\n","Error: invalid command line");
				}else if (pid2 > 0){
					waitpid(pid2, &reval, WNOHANG);
					fprintf(stdout, "+ completed '%s' [%d]\n", *cmd, WEXITSTATUS(reval));
				}
			}
			execvp(cmd[0], cmd);
			fprintf(stdout,"%s\n","Error: invalid command line");
			exit(1);
		} else if (pid > 0) { /* Parent */
			waitpid(-1, &status, 0); // -1: parent willing to wait for child to finish compiling.
			// printf("Child exited with status: %d\n", WEXITSTATUS(status));
			// fprintf(stdout, "Return status value for '%s': %d\n", "date -u", status);
			strtok(buffer2, "\n");
			fprintf(stdout, "+ completed '%s' [%d]\n", buffer2, WEXITSTATUS(status));
		} else {
			perror("fork");
			exit(1);
		}
		emptyMemo:
			free(cmd);
			cmd = NULL;
	}

	return EXIT_SUCCESS;
}