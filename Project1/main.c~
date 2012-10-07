/*
 *
 * Laura Uhlig and Sebastian Sigmon
 * We both discussed our conceptual plans for the project and outline how we should go about it.  Laura did the coding for the initial separating the command line, fixing tokenify and creating the mode method, and implementing parallel and sequential modes. Sebastian then fixed memory leaks, printed the usage, and ensured all weird imputs would be dealt with without crashing, along with the coding for part 2. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>

//The basic linked list structure for ongoing jobs.
struct node{
	pid_t pid;
	char command[1024];
	int state;
	struct node *next;
};

//Insert a node at the head of a list.
void listInsert(pid_t pid, char *command, int state, struct node **head){
	struct node *new = malloc(sizeof(struct node));
	strncpy(new->command, command, 1024);
	new->pid = pid;
	new->state = state;
	new->next = *head;	
	*head = new;
}

//Clear a list.
void listClear(struct node *list){
	while(list != NULL){
		struct node *tmp = list;
		list = list->next;
		free(tmp);
	}
}

//Delete a specific node from a list.
void listDelete(pid_t pid, struct node **list){
	if((*list)->pid == pid){
		struct node *tmp = *list;
		*list = (*list)->next;
		free(tmp);
		return;
	}
	struct node *tmp = *list;
	while(tmp->next != NULL){
		struct node *tmpnext = tmp->next;
		if(tmpnext->pid == pid) {
			tmp->next = tmpnext->next;
			free(tmpnext);
			return;
		}
		tmp = tmp->next;
	}
}

//Set the state of a given node.
void setState(pid_t pid, int state, struct node *list){
	while(list != NULL){
		if(list->pid == pid){
			list->state = state;
		}
		list = list->next;
	}
}

char ** tokenify(char *s, char *t)
//Used the tokenify function from our lab that we had to fix
{
    
    char *word = NULL;

    // find out exactly how many tokens we have
    int words = 0;
    char * temp = strdup(s);
    for (word = strtok(temp, t); word; word = strtok(NULL, t)){
	words++;
    }
    free (temp);


    // allocate the array of char *'s, with one additional
    char **array = malloc(sizeof(char*)*(words+1));
    int i = 0;
    for (word = strtok(s, t); word; word = strtok(NULL, t)) 
{
        //printf("adding word %s to array pos %d\n", word, i);
        array[i] = strdup(word);
        i++;
    }
    array[i] = NULL;
    return array;
}


int ourMode(char ** str, int x)
//Whenever a 'mode' command is sent
{
	if (str[1] == NULL) {
	//prints what mode we're currently in
		if (x==1) printf("Current mode is parallel. \n");
		if (x==0) printf("Current mode is sequential. \n");	
		return x;
	}
	else if (strcasecmp(str[1], "parallel") == 0 || strcmp(str[1], "p") == 0) {
	//changes to parallel by changing our mode variable
		x = 1;
		return x;
	}
	else if (strcasecmp(str[1],"sequential") == 0 || strcmp(str[1], "s") == 0) {
	//changes to sequential
		x = 0;
		return x;
	}
	else {
	//someone typed something funny after mode and we can't do anything with it
		printf("Error on input. Try again.\n");
		return x;
	}
}

//Free outer and inner blocks of memory.
void freeMem(char ***array){
	int i = 0;
	while(array[i] != NULL){
		int j = 0;
		while(array[i][j] != NULL){
			free(array[i][j]);
			j++;
		}
		free(array[i]);
		i++;
	}
}

//Free inner blocks of memory.
void freeMem1(char **array){
	int i = 0;
	while(array[i] != NULL){
		free(array[i]);
		i++;
	}	
}


//Strips whitespace from a character in place.
void stripWhite(char* s){
	if (s == NULL){
		return;
	}
	char buffer[strlen(s)+1];
	int i = 0;
	int j = 0;
	for(;i<strlen(s);i++){
		if(!isspace(s[i])){
			buffer[j] = s[i];
			j++;
		}	
	}
	buffer[j] = '\0';
	strcpy(s,buffer);
}

//Pulls the paths out of a file; in this case, shell-config.
char **readPaths(char *filename){
	FILE *f;
	char c;
	char buffer[1024];
	int numLines = 0;
	int i = 0;
	int skip = 0;
	f = fopen(filename, "r");

	if (f==NULL){
		return NULL;
	}
	while ((c=fgetc(f)) != EOF){
		if(c=='\n'){
			numLines++;
		}
	}
	fclose(f);
		
	char **pathList = malloc(sizeof(char*)*(numLines+1));
	f = fopen(filename, "r");
	if (f != NULL){
		while (fgets(buffer,1024,f)!=NULL) {
			stripWhite(buffer);
			if(!(buffer[0]=='\0' && strlen(buffer)<1)){
				pathList[i] = strdup(buffer);
				i++;
			} else {
				skip++;
			}			
		}
		pathList[numLines-skip] = NULL;
		fclose(f);
	}
	return pathList;
}



int main(int argc, char **argv) {
	char *prompt = "hitme> ";
	printf("%s", prompt);
	fflush(stdout);
	//Set up variables we'll need.
	int curMode = 0;
	int curMode2 = 0;
	int curModeIndex = 0;
	int exitTrue = 0;
	char buffer[1024];
	int pathTrue = 0;
	int printPrompt = 0;

	struct node *head = NULL;

	//If we have the option of pathing to a command, get the pathways we'll check.
	char **pathList = readPaths("shell-config");
	if(pathList != NULL){
		pathTrue = 1;
	}

	while (1) {
		//Poll for I/O on the keyboard.
		struct pollfd pfd = {0,POLLIN};

		//Only reprint the prompt if we need to.
		if(printPrompt){
			printf("%s",prompt);
			fflush(stdout);
			printPrompt = 0;
		}
			
		int rv = poll(&pfd, 1, 1000);
		if (rv == 0){
			//No I/O activity; use the time to check on ongoing jobs.
			struct node *anode = head;
			while (anode != NULL){
				int pstatus = 0;
				int pstate = waitpid(anode->pid,&pstatus,WNOHANG);
				if(pstate>0){
					printf("Executed command %s. \n",anode->command);
					anode = anode->next;
					listDelete(pstate,&head);
					printPrompt = 1;
				} else if (pstate < 0) {
					printf("Error retrieving process status.\n");
					break;
				} else {
					anode = anode->next;
				}
			}
		} else if (rv < 0){
			//Some sort of error occured.
			printf("Error while polling.");
			break;
		} else {
			//I/O; if it's a complete command, parse and execute.
			if (fgets(buffer,1024,stdin) != NULL){
	        		int i = 0;
				//Process command line; newline characters or comments are replaced by the null character to terminate the string.
				for (; i<1024; i++) {
					if (buffer[i] == '#' || buffer[i] == '\n'){
						buffer[i] = '\0';
						break;
					}	
				}
			//Split the buffer by semicolons into a set of commands. Further split each of those commands around whitespace characters.
			char ** cmd1 = tokenify(buffer, ";");
			int w = 0;

			while (cmd1[w] != NULL){
				w++;
			}

			char ** cmd[w];
			cmd[0] = NULL;
			i = 0;
			char temp[4] = {' ', '\n', '\t', '\0'};
	 	
			while (i != w){
				cmd[i] = tokenify(cmd1[i], temp);
				cmd[w] = NULL;
				i++;
			}

			pid_t p;
			i = 0;
			while (cmd[i] != NULL){//Process each command.
				if (cmd[i][0] == NULL){
					i++;
					continue; //Skip over empty commands.
				}
				if (strcasecmp(cmd[i][0], "mode") == 0 || strcasecmp(cmd[i][0], "exit") == 0){//In the event of mode or exit commands.
					if (strcasecmp(cmd[i][0], "mode") == 0){
						curMode2 = ourMode(cmd[i], curMode2);//Handle this with the special function.
						if(cmd[i][1] != NULL){
							curModeIndex = i;//For keeping track of what will happen later.
						}
					}
					else {//Exit command.
						int quitTrue = 1;
						struct node *tmp = head;
						while(tmp != NULL){
							if(tmp->state != 1){
								quitTrue = 0;//If any jobs are running, we can't quit.
							}
							tmp = tmp->next;
						}
						if(quitTrue){
							exitTrue = 1;//Exits at the end of the while loop.
						} else{
							printf("Error: Please wait until running jobs are finished to exit.\n");
						}
					}
				} else if (!strcasecmp(cmd[i][0],"pause")){//Pause command.
					setState(atoi(cmd[i][1]),1,head);
					if(kill(atoi(cmd[i][1]),SIGSTOP)!= -1){
						printf("Paused process %i.\n",atoi(cmd[i][1]));
					} else {
						printf("Error while pausing.\n");
					}
				} else if(!strcasecmp(cmd[i][0], "jobs")){//Jobs command.
					struct node *tmp = head;
					printf("\nPID CMD Paused\n");
					while (tmp != NULL){
						printf("%i %s %i\n",tmp->pid,tmp->command,tmp->state);//Prints PID, command, and state of a job.
						tmp = tmp->next;
					}
				} else if(!strcasecmp(cmd[i][0], "resume")){//Resume command.
					setState(atoi(cmd[i][1]),0,head);
					if(kill(atoi(cmd[i][1]),SIGCONT)!= -1){
						printf("Resumed process %i.\n",atoi(cmd[i][1]));
					}
				} else {//Not a built-in command, so we're going to have to execute it.
					p = fork();
		        		if (p == 0) {//In the child process.
						if (pathTrue){//We are checking paths.
							int q = 0;
							while(pathList[q] != NULL){//Check each path.
								struct stat statResult;	
								char pathBuffer[1024];
								strcpy(pathBuffer, pathList[q]);
								strcat(pathBuffer, "/");
								strcat(pathBuffer, cmd[i][0]);//Concat the resulting path together.
								int retValue = stat(pathBuffer, &statResult);//See if it's valid.
								if(retValue<0){
									q++;
								} else {
									cmd[i][0] = pathBuffer;//If it is, replace the existing command with the path.
									if(execv(cmd[i][0], cmd[i])<0){
										fprintf(stderr, "Path exists but execv failed: %s\n", strerror(errno));
										exit(0);
									}		
								}
							}
						}
	            				if (execv(cmd[i][0], cmd[i]) < 0) {//No path existed or we aren't using paths, so execv normally.
							fprintf(stderr, "Execv failed: %s\n", strerror(errno));
				                	exit(0);
						}
					} else if (p > 0) {//In parent process.
						if (curMode == 0){//Sequential mode.
	       						int rstatus = 0;
							wait(&rstatus);//All we need to do is wait for the job to terminate.
						} else {//Parallel mode.
							listInsert(p,cmd[i][0],0,&head);//We add this to our set of ongoing jobs.
						}
		       			} else {
						//Something went wrong with fork.
           					fprintf(stderr, "Fork error: %s\n", strerror(errno));
        				}
				}
			i++; 
		} 
	
		if (curMode != curMode2){//Check for and handle mode changes.
			curMode = ourMode(cmd[curModeIndex],curMode);
			if(curMode == 1){
				printf("Current mode is parallel. \n");
			} else {
				printf("Current mode is sequential. \n");
			}
			curModeIndex = 0;
		}
	
		
		//Clear out the old memory.
		freeMem(cmd);
		freeMem1(cmd1);
		free(cmd1);
	
		//Break if you're exiting.
		if(exitTrue){
			break;
		}
		
		//Otherwise, reprint the prompt. 
		printf("%s", prompt);
        	fflush(stdout);
		}

		if(feof(stdin)){//An EOF character (Ctrl+D or reading from a file) can dump you here without doing anything. Terminate the shell.
			break;
		}
		}
	}

	//Free paths.
	if (pathList != NULL){
		freeMem1(pathList);
		free(pathList);
	}
	//Check and print time spent in user/kernel mode.
	int whoParent = RUSAGE_SELF;
	struct rusage usageParent;
	int retParent;
	int whoChildren = RUSAGE_CHILDREN;
	struct rusage usageChildren;
	int retChildren;
	retParent = getrusage(whoParent, &usageParent);	
	retChildren = getrusage(whoChildren, &usageChildren);
	if(!retParent){
		printf("Shell time in user mode: %ld.%06ld seconds. \n", usageParent.ru_utime.tv_sec, usageParent.ru_utime.tv_usec);
		printf("Shell time in kernel mode: %ld.%06ld seconds. \n", usageParent.ru_stime.tv_sec, usageParent.ru_stime.tv_usec);
	}
	if(!retChildren){
		printf("Time spent in user mode for commands: %ld.%06ld seconds. \n", usageChildren.ru_utime.tv_sec, usageChildren.ru_utime.tv_usec);
		printf("Time spent in kernel mode for commands: %ld.%06ld seconds. \n", usageChildren.ru_stime.tv_sec, usageChildren.ru_stime.tv_usec);
	}
	//Exit.
	exit(0);
    return 0;
}
