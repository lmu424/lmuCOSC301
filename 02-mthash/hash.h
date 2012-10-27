#ifndef __HASH_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>

struct _hash_node {
    char * string;
    struct _hash_node *next;
};

typedef struct {
    struct _hash_node *head;
} list_t;

typedef struct hashtable_ {
	pthread_mutex_t mutex;
	int size;
	list_t ** table;	
} hashtable_t;

int hash(hashtable_t * hash, char * str);
int nexthighestPrime(int x);
int isPrime(int x);

// create a new hashtable; parameter is a size hint
hashtable_t *hashtable_new(int);

// free anything allocated by the hashtable library
void hashtable_free(hashtable_t *);

// add a new string to the hashtable
void hashtable_add(hashtable_t *, const char *);

// remove a string from the hashtable; if the string
// doesn't exist in the hashtable, do nothing
void hashtable_remove(hashtable_t *, const char *);

// print the contents of the hashtable
void hashtable_print(hashtable_t *);

#endif

