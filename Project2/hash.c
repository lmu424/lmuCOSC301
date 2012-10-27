#include "hash.h"


/*My hash function- using the Java hashCode() function. I tried a few different ones, all of which performed differently or without being apparently better than this, so here it is. 
*/
int hashit(hashtable_t * hashtable, const char * str){
	pthread_mutex_lock(&hashtable->mutex);	
	//declare variables
	unsigned int hashvalue = 0;
	int x = strlen(str); 
	int i = 0;
	//perform on each character in the string
	for (; i<x; i++){
		hashvalue += str[i]*31^(x-(i+1)); 
	}
	//mod by the size of the table
	hashvalue = hashvalue%hashtable->size;
	
	pthread_mutex_unlock(&hashtable->mutex);
	return hashvalue;
}

/* functions to find the next highest prime when given any int. Neither functions are thread safe because they're only called from hashtable_new, which is not required to be thread safe*/
int nexthighestPrime(int x){
	if (isPrime(x) == 1){
		return x; }
	if(x%2 == 0){
		x+=1; 
		}
	while(isPrime(x) == 0){
		x+=2; 
		}
	return x;
} 

int isPrime(int x){
	int i = 3;
	if (x == 2) return 1;
	else if (x%2 == 0 || x <= 1) return 0;
	else {
		for (; i<= sqrt(x); i+=2){
			if(x%i == 0) return 0; }
	}
	return 1;	
}


// create a new hashtable; parameter is a size hint
hashtable_t *hashtable_new(int sizehint) {

	if (sizehint < 1) return NULL;

	//create hash table
	hashtable_t * hash_table;
	hash_table = malloc(sizeof(hashtable_t));
	int size = nexthighestPrime(sizehint);
	printf("size = %d\n", size);

	//malloc size of table
	hash_table->table = malloc(sizeof(list_t *) * size);
	pthread_mutex_init (&hash_table->mutex, NULL);

	if (!hash_table || !hash_table->table) {
		fprintf(stderr, "No memory while attempting to create a new hashtable!\n");
		abort();
	}

	int i = 0;
	//make all buckets void
	for (; i<size; i++) {
		hash_table->table[i] = malloc(sizeof(struct _hash_node));
		hash_table->table[i]->head = NULL; }

	hash_table->size = size;
	
	return hash_table;
}

// free anything allocated by the hashtable library
void hashtable_free(hashtable_t *hashtable) {
	pthread_mutex_lock(&hashtable->mutex);
	//declare variables
	struct _hash_node * node;
	struct _hash_node * tmp;
	int i = 0;
	//go through every linked list from the table- if there is a string, free it. 
	for (; i<hashtable->size; i++){
		node = hashtable->table[i]->head;
		while (node){
			tmp = node;
			node = node->next;
			free(tmp->string);
			free(tmp); }
	}
	//free the table and the hashtable
	free(hashtable ->table);
	free(hashtable);
	pthread_mutex_unlock(&hashtable->mutex);
	return;	
}



// add a new string to the hashtable
void hashtable_add(hashtable_t *hashtable, const char *s) {
	pthread_mutex_lock(&hashtable->mutex);
	struct _hash_node * tmp;
	//create new node
	struct _hash_node *new_node = malloc(sizeof(struct _hash_node));
	if (!new_node) {
		fprintf(stderr, "No memory while attempting to create a new list node!\n");
		abort();
	}

	pthread_mutex_unlock(&hashtable->mutex);
	//hash the string
	int val = hashit(hashtable, s);
	pthread_mutex_lock(&hashtable->mutex);
	tmp = hashtable->table[val]->head;
	pthread_mutex_unlock(&hashtable->mutex);
	//if this is the first string added to this hash value
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
	//hash the string to find where it was placed
	int val = hashit(hashtable, s);

	pthread_mutex_lock(&hashtable->mutex);
	struct _hash_node * tmp = hashtable->table[val]->head;
	struct _hash_node * prev = NULL;
	/*find where it is in the given linked list, keeping track of the previous item and the current item with prev and tmp, respectively. */
	for (; !tmp && (strcmp(s, tmp->string) != 0); prev = tmp, tmp = tmp->next);

	pthread_mutex_unlock(&hashtable->mutex);
	//if it isn't on the hash, return.
	if (!tmp) return;
	pthread_mutex_lock(&hashtable->mutex);
	//if it was the last string hashed, prev == NULL
	if (prev == NULL) {
		hashtable->table[val]->head = tmp->next; }
	//otherwise, change prev->next
	else{
		prev->next = tmp->next; }
	//free the string and the node the string was stored in
	free(tmp->string);
	free(tmp);
	pthread_mutex_unlock(&hashtable->mutex);
	return;

}

// print the contents of the hashtable
void hashtable_print(hashtable_t *hashtable) {
	pthread_mutex_lock(&hashtable->mutex);
	printf("Hashtable contents:\n");
	struct _hash_node *tmp;
	int i = 0;
	printf("size: %d\n", hashtable->size);
	//go through entire hashtable to print all of the items
	for(; i < hashtable->size; i++){
		tmp = hashtable->table[i]->head;
		while(tmp){
			printf("	%d: %s\n", i, tmp->string);
			tmp = tmp->next; }
		}
	pthread_mutex_unlock(&hashtable->mutex);
	return;
}
	


