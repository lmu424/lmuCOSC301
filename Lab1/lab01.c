#include <string.h>
#include <stdlib.h>

void removewhitespace(char *s){	
/* This function iterates through the string once. If it encounters any type of white space, it moves every character including the '\0' back one space. j is used so i does not change. When a white space is encountered, j becomes i (the index of the white space). Every item j+1 and on is moved backwards into the place of j. It includes the '\0' because strlen does not include it, but we are going one further (j+1).
*/
	int i = 0;
	int j;
	int l = strlen(s);
	for(; i<l; i++){
		if (s[i] == '\n' || s[i] == '\t' || s[i] == ' '){
			j=i;
			for (;j < l;j++){
				s[j] = s[j+1];
			}
			l = strlen(s);
		}
	}
}


char * c2pascal(char *s){
/* Test if s is longer than 255, because the first index cannot hold a number greater than this and thus the string cannot be formatted in Pascal.  Create a new string news and make the first index the length of string s.  For all characters in the string, the integer value of this character is placed into the new string one index above the original.  news is the same size as s because the number of characters at the front of a pascal string pushes each character back, which overwrites the '\0' not present in the pascal string.
*/
	if (strlen(s) > 255) {
		return NULL; }	
	int i = 0;
	char * news = malloc(sizeof(char)*strlen(s));
	news[0] = (char)strlen(s);
	for (;i<strlen(s);i++) {
		news[i+1] = s[i];
	}
	return news;
}

char * pascal2c(char *s){
/* Now, a new pointer is created and each index of news is replaced with the character of each index of s. The final index is replaced with '\0'. 
*/ 
	int i = 0;
	int x = s[0];
	char * news = malloc(sizeof(char)*x);
	for (;i<x-1;i++){
		news[i] = s[i+1];
	}
	news[x] = '\0';
	return news;
}

char * * tokenify(char * str){
	char * * array = malloc(sizeof(char *));
	int i = 0;
	char temp[4] = {' ', '\n', '\t', '\0'};
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

