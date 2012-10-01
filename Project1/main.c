/*
 * project 1 (shell) main.c template 
 *
 * project partner names and other information up here, please
 *
 */

/* you probably won't need any other header files for this project */
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


/*
char * * tokenify(char * str, char * temp){
	char * * array = malloc(sizeof(char *));
	int i = 0;
	char * x;
	if ((x = strtok(str, temp)) != NULL){
		char * * newarray = malloc(sizeof(char *) * (i+1));
		memcpy(newarray, array, sizeof(char *)*(i+1));
		free(array);
		array = newarray;
		array[i] = x;
		i++;
	}
	while ((x = strtok(NULL, temp)) != NULL) {
		char * * newarray = malloc(sizeof(char *) * (i+1));
		memcpy(newarray, array, sizeof(char *)*(i+1));
		free(array);
		array = newarray;
		array[i] = x;
		i++;
	}
	char * * newarray = malloc(sizeof(char *) * (i+1));
	memcpy(newarray, array, sizeof(char *)*(i+1));
	free(array);
	array = newarray;
	array[i] = NULL;
	return array;
} 
*/
char ** tokenify(char *s, char *t)
{
    //const char *sep=" \t\n";
    char *word = NULL;

    // find out exactly how many tokens we have
    int words = 0;
    char * temp = strdup(s);
    //int a = 0;
    for (word = strtok(temp, t); word; word = strtok(NULL, t)) 
	words++ ;
    free (temp);

   // printf("words: %d\n", words);
   // printf("s: %s\n", s);

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
{
	if (str[1] == NULL) {
		if (x==1) printf("Current mode: parallel\n");
		if (x==0) printf("Current mode: sequential\n");	
		return x;
	}
	else if (strcasecmp(str[1], "parallel") == 0 || strcmp(str[1], "p") == 0) {
		x = 1;
		printf("Current mode: parallel\n");
		return x;
	}
	else if (strcasecmp(str[1],"sequential") == 0 || strcmp(str[1], "s") == 0) {
		x = 0;
		printf("Current mode: sequential\n");
		return x;
	}
	else {
		printf("Error on input. Try again.\n");
		return x;
	}
}

int main(int argc, char **argv) {
    char *prompt = "hitme> ";
    printf("%s", prompt);
    fflush(stdout);
    int curMode = 0;
    char buffer[1024];
    while (fgets(buffer, 1024, stdin) != NULL) {
        /* process current command line in buffer */
        
        int i = 0;
        //int j = 0;
	for (; i<1024; i++) {
		if (buffer[i] == '#' || buffer[i] == '\n'){
			buffer[i] = '\0';
			break;
	}
	}
	char ***cmd = NULL;
	cmd = tokenify(buffer, ";");
	//cmd = test; 
	i = 0;
	char temp[4] = {' ', '\n', '\t', '\0'};
 	while (cmd[i] != NULL){
		cmd[i] = tokenify(cmd[i], temp);
		i++;
	}
	i = 0;
	//if (curMode == 0){
	while (cmd[i] != NULL){
        pid_t p = fork();
        if (p == 0) {
            /* in child */
            if (execv(cmd[i][0], cmd[i]) < 0) {
		if (strcasecmp(cmd[i][0], "mode") == 0){
			curMode = ourMode(cmd[0], curMode);}
		else if (strcasecmp(cmd[i][0], "exit") == 0){
			printf ("This is exit.\n");
		}
		else {
                fprintf(stderr, "execv failed: %s\n", strerror(errno));
	}
	if (curMode == 0){
	int rstatus = 0;
        p = wait(&rstatus);
            } }

        } else if (p > 0) {
            /* in parent */
            int rstatus = 0;
            pid_t childp = wait(&rstatus);

            /* for this simple starter code, the only child process we should
               "wait" for is the one we just spun off, so check that we got the
               same process id */ 
            assert(p == childp);

            //printf("Parent got carcass of child process %d, return val %d\n", childp, rstatus);
        } else {
            /* fork had an error; bail out */
            fprintf(stderr, "fork failed: %s\n", strerror(errno));
        }
	//int rstatus = 0;
	//p = wait(&rstatus);
        printf("%s", prompt);
        fflush(stdout);
	
	i++; } 
	//}
    }
	
    return 0;
}
