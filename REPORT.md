# **ECS 150**
---

## **Project 1:** Simple Shell
---

### Implementation
We first created the Makefile that, when run, creates an executable for our
shell program using the command "gcc". Then, using the *fork + exec + wait*
method provided by the lecture slides, we were able to execute the command.
Next, we created four buffers (buffer, buffer1, buffer2, buffer3) containing
a maximum of 512 characters. Then, we created a while loop in our main 
function that executes the entire process until we input the *exit* command.
Using *fgets()* to receive commands from the user and making three copies 
of it, we parse the commands using strtok() in our CountNumArgs() function, 
which splits the string into tokens any time there is a space or "\n". The 
copies are stored in different buffers: buffer1, buffer2, and buffer3; we 
do this because once strtok(buffer1, " \n") passes, we can no longer use 
buffer1 for storage, as it already contains the parsed tokens. Now, once 
we have the number of tokens, we begin to dynamically allocate the memory 
using malloc(), which is stored in "char **cmd". Then, we initialized each 
command to NULL because we don't know if the commands automatically point 
to NULL. 

Then, we parsed the string using strtok() stored into char *ptr. The while 
loop walks through the pointer and replaces \0 with empty spaces, and \n 
at the end of the line, iterates through each token with \0 and stores 
cmd[count] in each iteration. Now, we have parsed the commands and stored 
them appropriately.

In Phase 4, we implemented the builtin commands: exit, pwd, cd, using 
strcmp(). For these commands, we created emptyMemo to set each command to 
NULL. For the exit command, we simply set the boolean value to false, which 
causes the shell exit. For the pwd command, we used getcwd(buffer, 512); 
this stores the current directory back into our buffer and this buffer is 
printed. For input redirection, we looped through the commands to detect 
"<" and ">"; the tokens after "<" or ">" are stored in fileName. If found, 
we tokenize the signs by replacing their locations with \0. We continue to 
parse other commands and store them into cmd[count], which will be executed. 
Additionally, we added an if statement: if(fileName == NULL && 
dir != 'a' && count !=1) that prints out errors accordingly.

To implement piping for phase 7, we parsed each command as we did in previous 
phases by detecting "|", and we created struct Node* parsepipe() to store each 
command between the pipe. Furthermore, we created another function that parsed 
each pipe where we returned the double pointer command. 

In our main function, we detected if there exists a "|" in a for loop. If such 
pipe exists, we create a struct of a linked list that connects each command by 
throwing the input string to the parsedPipe function; this breaks each command 
between pipes and store them in nodes, connecting them in this linked list. 
Then, we have a pipeLine function that creates a pipe using code from the 
lecture slides.

But, our piping still has bugs that need to be fixed for the piping process. 
However, we have successfully connected the adjacent commands with in and out 
order. The head of the linked list is the first command. In the struct *Node, 
it has a character **pointer pointing to it. The size of the list is found by 
calling CountNumArgs(). In order to parse each argument between pipes, we 
created another function parseEachPipe() that takes in a buffer containing 
characters that breaks and returns the commands. In our parsePipe() we parse 
each struct *Node while storing them into temp->cmd, and iterating through the 
entire linked list. Additionally, our pipeline() function takes in arguments at 
the head of the pointer pointing to the head nodes of the node and parses each 
function whenever it walks through the linked list and executes execvp() each 
pair of commands while pList is not NULL.

### Resources
+ Information/code provided on lecture slides
+ StackOverflow
+ GeeksForGeeks
+ cplusplus.com for strtok()