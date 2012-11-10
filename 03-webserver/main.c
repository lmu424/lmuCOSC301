#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "list.h"


#include "network.h"


// global variable; can't be avoided because
// of asynchronous signal interaction
int still_running = TRUE;
void signal_handler(int sig) {
    still_running = FALSE;
}

struct threadargs {
    int * thequeue;
    int first;
    int count;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t cond1;
    pthread_cond_t cond2;
} ;

void * threadwork(struct threadargs * v){
	while (still_running){
		struct threadargs * targs = (struct threadargs *)v;
		pthread_mutex_lock(&targs->mutex);
		//wait for there to be something on the queue
		while (targs->size == targs->count) {
			pthread_cond_wait(&targs->cond1, &targs->mutex); }

		char * buffer = malloc(sizeof(char *));
		char * newbuf = malloc(sizeof(char *));
		char * readbuf = malloc(sizeof(char*));
		char * temp = NULL;
		//getcwd(newbuf, 1024);
		FILE * fp;
		int new_sock = targs->thequeue[targs->first];
		targs->first++;
		targs->count--;
		
		//Get the request
		getrequest(new_sock, buffer, 1024);
		struct stat check;
		if (stat(buffer, &check) == 0) {
			sprintf(temp, HTTP_404);
			temp = "404";
		}
		else {
			sprintf(temp, HTTP_200, check.st_size); 
			temp = "200";
			if (buffer[0] == '/'){
				*buffer = buffer[1]; }
			fp = open(buffer, NULL);
			read(fp, readbuf, 1024); 
			if (senddata(new_sock, readbuf, strlen(readbuf)) == -1){
				printf("Error"); }
			close(readbuf);
			free(readbuf);
		}

		pthread_cond_signal(&targs->cond2);
		pthread_mutex_unlock(&targs->mutex);
		pthread_mutex_lock(&targs->mutex);
		int fw;
		fw = open("weblog.txt", NULL);
		char * anotherbuffer = malloc(sizeof(char *));
		const time_t tod;
		struct tm *result;
		result = asctime(localtime_r(tod, result));
		struct sockaddr_in client_address;
		sprintf(anotherbuffer, "%s:%d  %s  \"GET %s \"  %s  %d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), result, buffer, temp, check.st_size);
		
		write(fw, anotherbuffer, 1024);
		close(fw);
		pthread_mutex_unlock(&targs->mutex);
		free(newbuf);
		free(buffer);
		free(readbuf);
		free(anotherbuffer);
	}
	return;
}

void usage(const char *progname) {
    fprintf(stderr, "usage: %s [-p port] [-t numthreads]\n", progname);
    fprintf(stderr, "\tport number defaults to 3000 if not specified.\n");
    fprintf(stderr, "\tnumber of threads is 1 by default.\n");
    exit(0);
}

void runserver(int numthreads, unsigned short serverport) {
    //////////////////////////////////////////////////

    // create your pool of threads here

    //////////////////////////////////////////////////
	
	//create the queue, malloc
   // list_t * queue = (list_t*)malloc(sizeof(list_t)) ;
   // list_init(queue);
	
	//declare struct variables
        struct threadargs * thread_args;
	thread_args->first = 0;
	thread_args->count = 0;
	thread_args->size = numthreads;
	thread_args->thequeue[numthreads];
	pthread_mutex_init(&thread_args->mutex, NULL);
	pthread_cond_init(&thread_args->cond1, NULL);
	pthread_cond_init(&thread_args->cond2, NULL);
   
	//create the threads
   pthread_t threads[numthreads];
    int i = 0;

    for (i = 0; i < numthreads; i++) {
	if (0 > pthread_create(&threads[i], NULL, threadwork, (void *)&thread_args)) {
	fprintf(stderr, "Error creating thread: %s\n", strerror(errno)); }
    }
    
    int main_socket = prepare_server_socket(serverport);
    if (main_socket < 0) {
        exit(-1);
    }
    signal(SIGINT, signal_handler);

    struct sockaddr_in client_address;
    socklen_t addr_len;

    fprintf(stderr, "Server listening on port %d.  Going into request loop.\n", serverport);
    while (still_running) {
	pthread_mutex_lock(&thread_args->mutex);
        struct pollfd pfd = {main_socket, POLLIN};
        int prv = poll(&pfd, 1, 10000);

        if (prv == 0) {
            continue;
        } else if (prv < 0) {
            PRINT_ERROR("poll");
            still_running = FALSE;
            continue;
        }
        
        addr_len = sizeof(client_address);
        memset(&client_address, 0, addr_len);

        int new_sock = accept(main_socket, (struct sockaddr *)&client_address, &addr_len);
        if (new_sock > 0) {
            
            fprintf(stderr, "Got connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

           ////////////////////////////////////////////////////////
           /* You got a new connection.  Hand the connection off
            * to one of the threads in the pool to process the
            * request.
            *
            * Don't forget to close the socket (in the worker thread)
            * when you're done.
            */
           ////////////////////////////////////////////////////////
	//Malloc the new node
	/*struct __list_node *new_node = (struct __list_node *)malloc (sizeof(struct __list_node));
	if (!new_node) {
		fprintf(stderr, "No memory while attempting to create a new list node!\n");
		abort();
    }
	new_node->data = new_sock;
	new_node->next = NULL;

	//put new_sock into linked list at the end
	if (queue->head == NULL) {
		queue->head = new_node;
		new_node->next = queue->last;
		// status will be 1 as long as there is something in the queue, and 0 if there is nothing there to work on
		thread_args->status = 1;
	}
	else if (queue->head != NULL && queue->last == NULL){
		queue->last = new_node;
	}
	else {
		queue->last->next = new_node;
		queue->last = queue->last->next;
	}
	*/

	while(thread_args->count == numthreads){
		pthread_cond_wait(&thread_args->cond2, &thread_args->mutex); }

	int index = (thread_args->first + thread_args->count)%numthreads;
	thread_args->thequeue[index] = new_sock;
	thread_args->count++;

	pthread_cond_signal(&thread_args->cond1);
	pthread_mutex_unlock(&thread_args->mutex);
        }
    }
    pthread_cond_broadcast(&thread_args->cond1);
    pthread_cond_broadcast(&thread_args->cond2);
    for (i = 0; i < numthreads; i++) {
        	pthread_join(threads[i], NULL); }
	 
    fprintf(stderr, "Server shutting down.\n");
    close(main_socket);
}


int main(int argc, char **argv) {
    unsigned short port = 3000;
    int num_threads = 1;

    int c;
    while (-1 != (c = getopt(argc, argv, "hp:"))) {
        switch(c) {
            case 'p':
                port = atoi(optarg);
                if (port < 1024) {
                    usage(argv[0]);
                }
                break;

            case 't':
                num_threads = atoi(optarg);
                if (num_threads < 1) {
                    usage(argv[0]);
                }
                break;
            case 'h':
            default:
                usage(argv[0]);
                break;
        }
    }

    runserver(num_threads, port);
    
    fprintf(stderr, "Server done.\n");
    exit(0);
}
