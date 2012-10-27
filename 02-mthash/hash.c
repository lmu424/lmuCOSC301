#include "hash.h"


/*My hash function- adds the ASCII values of each letter, mod by the size of the hash table (which is a prime) */
int hashit(hashtable_t * hashtable, const char * str){
	pthread_mutex_lock(&hashtable->mutex);	
	unsigned int hashvalue = 0;
	int x = strlen(str); 
	int i = 0;
	for (; i<x; i++){
		hashvalue = str[i] + (hashvalue<<5) - hashvalue;
	}
	hashvalue = hashvalue%hashtable->size;
	
	pthread_mutex_unlock(&hashtable->mutex);
	return hashvalue;
}

/* functions to find the next highest prime when given any int */
int nexthighestPrime(hashtable_t * hashtable, int x){
	pthread_mutex_lock(&hashtable->mutex);	
	if (isPrime(hashtable, x) == 1){
		return x; }
	if(x%2 == 0){
		
		x+=1; 
		//pthread_mutex_unlock(&hashtable->mutex);
		}
	while(isPrime(hashtable, x) == 0){
		//pthread_mutex_lock(&hashtable->mutex);
		x+=2; 
		//pthread_mutex_unlock(&hashtable->mutex);
		}
	pthread_mutex_unlock(&hashtable->mutex);
	return x;
} 

int isPrime(hashtable_t * hashtable, int x){
	pthread_mutex_lock(&hashtable->mutex);
	int i = 3;
	if (x == 2) return 1;
	else if (x%2 == 0 || x <= 1) return 0;
	else {
		for (; i<= sqrt(x); i+=2){
			if(x%i == 0) return 0; }
	}
	pthread_mutex_unlock(&hashtable->mutex);
	return 1;	
}

// create a new hashtable; parameter is a size hint
hashtable_t *hashtable_new(int sizehint) {
	if (sizehint < 1) return NULL;
	//create hash table
	hashtable_t * hash_table;
	hash_table = malloc(sizeof(hashtable_t));
	//make prime
	//int size = nexthighestPrime(hash_table, sizehint);
	int size = sizehint;
	//create first node-> unnecessary??
	hash_table->table = malloc(sizeof(list_t *) * size);
	pthread_mutex_init (&hash_table->mutex, NULL);
	int i = 0;
	for (; i<size; i++) {
		hash_table->table[i] = malloc(sizeof(struct _hash_node));
		hash_table->table[i]->head = NULL; }
	hash_table->size = size;	
	return hash_table;
}

// free anything allocated by the hashtable library
void hashtable_free(hashtable_t *hashtable) {
	pthread_mutex_lock(&hashtable->mutex);
	struct _hash_node * node;
	struct _hash_node * tmp;
	int i = 0;
	for (; i<hashtable->size; i++){
		node = hashtable->table[i]->head;
		while (node != NULL){
			tmp = node;
			node = node->next;
			free(tmp->string);
			free(tmp); }
	}
	free(hashtable ->table);
	free(hashtable);
	pthread_mutex_unlock(&hashtable->mutex);
	return;	
}

// add a new string to the hashtable
void hashtable_add(hashtable_t *hashtable, const char *s) {
	pthread_mutex_lock(&hashtable->mutex);
	fprintf(stderr, "%s\n", s);
	struct _hash_node * tmp;
	struct _hash_node *new_node = malloc(sizeof(struct _hash_node));
	if (!new_node) {
		fprintf(stderr, "No memory while attempting to create a new list node!\n");
		abort();
	}
	pthread_mutex_unlock(&hashtable->mutex);
	int val = hashit(hashtable, s);
	pthread_mutex_lock(&hashtable->mutex);
	tmp = hashtable->table[val]->head;
	fprintf(stderr, "%d\n", val);
	pthread_mutex_unlock(&hashtable->mutex);
	//if no other nodes on this yet!
	if (!tmp) {
		pthread_mutex_lock(&hashtable->mutex);
		new_node->string = strdup(s);
		hashtable->table[val]->head = new_node;
		new_node->next = NULL;
		pthread_mutex_unlock(&hashtable->mutex);
		return;
	}
	//otherwise, put the new string at the beginning of the linked list for this spot in the table.
	pthread_mutex_lock(&hashtable->mutex);
	new_node->string = strdup(s);
	new_node->next = hashtable->table[val]->head;
	hashtable->table[val]->head = new_node;
	pthread_mutex_unlock(&hashtable->mutex);
	return;
}

// remove a string from the hashtable; if the string
// doesn't exist in the hashtable, do nothing
void hashtable_remove(hashtable_t *hashtable, const char *s) {
	int val = hashit(hashtable, s);
	printf("%s should be removed now.\n", s);
	pthread_mutex_lock(&hashtable->mutex);
	struct _hash_node * tmp = hashtable->table[val]->head;
	struct _hash_node * prev = NULL;
	
	for (; !tmp && (strcmp(s, tmp->string) != 0); prev = tmp, tmp = tmp->next);
	pthread_mutex_unlock(&hashtable->mutex);
	if (!tmp) return;
	pthread_mutex_lock(&hashtable->mutex);
	if (prev == NULL) {
		hashtable->table[val]->head = tmp->next;
		printf("%s removed\n", s); }
	else{
		prev->next = tmp->next; 
		printf("%s removed\n", s);}
	free(tmp->string);
	free(tmp);
	pthread_mutex_unlock(&hashtable->mutex);
	return;

}
	/*
	// if it is the very first string in the linked list
	if (strcmp(s, tmp->string) == 0) { 
		pthread_mutex_lock(&hashtable->mutex);
		hashtable->table[val]->head = tmp->next;
		free(tmp->string);
		free(tmp);
		printf("%d removed\n", val);
		pthread_mutex_unlock(&hashtable->mutex);
		return; }
	pthread_mutex_lock(&hashtable->mutex);
	printf("here\n");
	tmp = hashtable->table[val]->head;
	prev = hashtable->table[val]->head;
	printf("chocolate\n");	
	for (; tm ; tmp = tmp->next) {
		printf("%d here we are!\n", val);
		if (strcmp(s, tmp->string) == 0) {
			printf("%d good.", val);
			prev->next = tmp->next;
			free(prev->string);
			free(prev); 
			printf("%d removed\n", val);
			pthread_mutex_unlock(&hashtable->mutex);
			return;}
		prev = prev->next;
	}
	printf("%d wasn't in there...\n", val);
	pthread_mutex_unlock(&hashtable->mutex);
	return;
	
}*/

// print the contents of the hashtable
void hashtable_print(hashtable_t *hashtable) {
	pthread_mutex_lock(&hashtable->mutex);
	printf("Hashtable contents:\n");
	struct _hash_node *tmp;
	int i = 0;
	printf("size: %d\n", hashtable->size);
	for(; i < hashtable->size; i++){
		tmp = hashtable->table[i]->head;
		while(tmp){
			printf("	%d: %s\n", i, tmp->string);
			tmp = tmp->next; }
		}
	pthread_mutex_unlock(&hashtable->mutex);
	return;
}
	


