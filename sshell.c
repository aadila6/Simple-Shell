
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
	if (strtok(buffer, " \n") == NULL) { //Base case
		return 0;
	}
	int num = 1; 
	while (strtok(NULL, " \n") != NULL) {
		num++;
	}
	return num;
}
char** parseEachPipe(char *buffer) {
	char *ptr = strtok(buffer, " \n");
	char **cmd = NULL;
	int cnt = 0;
	int in = 0;
	cnt = CountNumArgs(buffer);
	cmd = malloc(sizeof(char *) * (cnt + 1));
	// fprintf(stdout, "%s\n", ptr);
	// for(int i=0; i<cnt; i++){
	// 	cmd[i] = NULL;
	// }
	while (ptr != NULL){
		cmd[in] = ptr;
		// fprintf(stdout, "%s\n", ptr);
		in++;
		ptr = strtok(NULL, " \n");
	}
	return cmd;
}

struct Node* parsePipe(char *buffer){ 
	//return the ptr to the head of the linked list and we will pass this to main
	char *rest = NULL;
	char *token = strtok_r(buffer, "|", &rest);
	// int count = 0;
	struct Node *HEAD = NULL;
	HEAD = (struct Node*)malloc(sizeof(struct Node));
	struct Node *temp = HEAD;
	while (token != NULL) {
		// fprintf(stdout, "%s\n", token);
		temp->cmd = parseEachPipe(token);
		token = strtok_r(NULL, "|", &rest);
		temp->next = (struct Node*)malloc(sizeof(struct Node));
		temp = temp->next;
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
	char buffer1[CMDLINE_MAX];//copy 1 of our original string which will be use for initial parsing.
	bool run = true;
	int fd;
	//While loop keeps the entire execution where it will continue restart the sshell after one execution. 
	//Until we hit exit
	while (run){
		char **cmd = NULL;
		fprintf(stdout, "sshell$ ");
		fgets(buffer, CMDLINE_MAX, stdin);
		strcpy (buffer1, buffer);
		//Checks for PHASE 5&6 where < and >
		char dir = 'a';
		char *fileName = NULL;
		// int pipefd[2];
		//--------------- Detect the pipe initialization -------------
		// pipeline(test);
		bool isPipe = false;
		for(int k=0; k<strlen(buffer1); k++){
			if(buffer1[k] == '|'){
				isPipe = true;
			}
		}
		if(isPipe){
			struct Node *test = NULL;
			test = parsePipe(buffer1);
			pipeline(test);
		}

		for(int j=0; j<strlen(buffer1); j++) {
			if(buffer1[j] == '<'){
				dir = '<';
				strtok(buffer1, "'<'");
				fileName = strtok(NULL, " \n");
			} else if(buffer1[j] == '>'){
				dir = '>';
				strtok(buffer1, "'>'"); 
				fileName = strtok(NULL, " \n");
			}
		}
		if(dir != 'a' && fileName == NULL){
			fprintf(stdout,"%s\n", "Error: no output file");
		}
		
		int count;
		count = CountNumArgs(buffer);
		cmd = malloc(sizeof(char *) * (count+1));
		for (int i=0; i<=count; i++){
			cmd[i] = NULL;
		}
		
		char *ptr = strtok(buffer1, " \n");
		int cnt = 0;
		while (ptr != NULL){
			// fprintf(stdout, "%s\n", ptr);
			cmd[cnt] = ptr;
			ptr = strtok(NULL, " \n");
			cnt = cnt + 1;
		}
		if (strcmp(cmd[0],"exit") == 0){
			fprintf(stderr,"%s\n","Bye..");
			run = false;
			goto emptyMemo;
		}
		if (strcmp(cmd[0],"pwd") == 0){
			getcwd (buffer1, CMDLINE_MAX);
			fprintf(stdout,"%s\n", buffer1);
			goto emptyMemo;
		}
		if (strcmp(cmd[0],"cd") == 0){
			 int ret = chdir(cmd[1]);
       		 if (ret != 0) { // success
            	fprintf(stderr,"%s\n","Error: no such directory");}
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
			// char *sleep = "sleep";
			// if (!strcmp(cmd[0],sleep)){ 
			// 	pid_t pid2 = fork();
			// 	int reval;
			// 	if (pid2==0){
			// 		execvp(cmd[0], cmd);
			// 		fprintf(stdout,"%s\n","Error: invalid command line");
				
			// 	}else if (pid2 > 0){
			// 		waitpid(pid2, &reval, WNOHANG);
			// 		fprintf(stdout, "+ completed '%s' [%d]\n", *cmd, WEXITSTATUS(reval));

			// 	}
			// }
			
			execvp(cmd[0], cmd);
			fprintf(stdout,"%s\n","Error: invalid command line");
			exit(1);
		} else if (pid > 0) { /* Parent */
			waitpid(-1, &status, 0); // -1: parent willing to wait for child to finish compiling.
			// printf("Child exited with status: %d\n", WEXITSTATUS(status));
			fprintf(stdout, "+ completed '%s' [%d]\n", buffer, WEXITSTATUS(status));
			exit(0);
		
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


