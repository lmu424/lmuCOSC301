/*  Laura Uhlig, Project 3

Used the consumer/producer method with 2 condition variables and 1 mutex through an array that I use as a queue- as the server gets a request, the socket is added to the queue.  The consumer (threadwork) gets each socket FIFO and will get the request and remove it from the queue.  

At one point my code was getting to poll, I would call webdriver.py in another terminal, and it would continue until a segfault with placing new_sock into my queue.  I had a lot of issues with what I should malloc, and how to avoid various segfaults (I mirrored the code used it mtlist.c from lab 5, when there was a linked list, and tried to make my queue a linked list, but this gave me numerous errors with malloc that I could not figure out, so I switched to the array). I think I solved the segfault issue here (it stopped complaining, at least), but by doing that caused another problem.  I'm not sure which would be easier.

*/



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
#include "network.h"




// global variable; can't be avoided because
// of asynchronous signal interaction
int still_running = TRUE;
void signal_handler(int sig) {
    still_running = FALSE;
}

typedef struct thread__args {
    int * thequeue;
    int first;
    int count;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t cond1;
    pthread_cond_t cond2;
} threadargs;

void * threadwork(void * v){
	while (still_running){
		threadargs * targs = (threadargs *)v;
		pthread_mutex_lock(&targs->mutex);
		//wait for there to be something on the queue
		while (targs->count == 0) {
			pthread_cond_wait(&targs->cond1, &targs->mutex); }

		
		char * buffer = malloc(sizeof(char *));
		char * newbuf = malloc(sizeof(char *));
		char * readbuf = malloc(sizeof(char *));
		char * temp = NULL;
		const time_t * tod;
		struct tm *result;
		//getcwd(newbuf, 1024);
		int fp;
		int new_sock = targs->thequeue[targs->first];
		
		
		/*right now, my code ignores the while loop and cond_wait and keeps going, calling getrequest() before poll gets a file, so this segfaults and fails. I have not been able to test the following code*/


		//Get the request
		getrequest(new_sock, buffer, 1024);
		//check if file exists with stat call
		struct stat check;
		// if it does not exist, call 404
		if (stat(buffer, &check) == 0) {
			sprintf(temp, HTTP_404);
			temp = "404";
		}
		else {
		//if it does exist, print HTTP
			sprintf(temp, HTTP_200,(int)check.st_size); 
			temp = "200";
			if (buffer[0] == '/'){
				buffer = &buffer[1]; }
			//open file for read
			fp = open(buffer, O_RDONLY);
			//read file, put in buffer
			read(fp, readbuf, 1024); 
			//send back to browser
			if (senddata(new_sock, readbuf, strlen(readbuf)) == -1){
				printf("Error"); }
			close(fp);
			free(readbuf);
		}

		//opened, read, and closed file- now, decrease count and signal cond just in case someone is waiting and count was full before
		targs->count--;
		targs->first++;
		pthread_cond_signal(&targs->cond2);
		pthread_mutex_unlock(&targs->mutex);

		//lock and unlock so a new thread can deal with a request
		pthread_mutex_lock(&targs->mutex);
		int fw;
		//open file to write to
		fw = open("weblog.txt", O_RDWR);
		char * anotherbuffer = malloc(sizeof(char *));
		//get time stamp
		char * thetime = asctime(localtime_r(tod, result));
		struct sockaddr_in client_address;
		//put info into a buffer
		sprintf(anotherbuffer, "%s:%d  %s  \"GET %s \"  %s  %d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), thetime, buffer, temp, (int)check.st_size);
		//write buffer to file
		write(fw, anotherbuffer, 1024);
		close(fw);
		pthread_mutex_unlock(&targs->mutex);
		//free malloc'd buffers
		free(newbuf);
		free(buffer);
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
	
	//declare struct variables
        threadargs * thread_args;// = (threadargs *) malloc(sizeof(threadargs));
	thread_args->first = 0;
	thread_args->count = 0;
	thread_args->size = numthreads;
	thread_args->thequeue = (int *)malloc(sizeof(int)*numthreads);
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
    
	//socket code
    int main_socket = prepare_server_socket(serverport);
    if (main_socket < 0) {
        exit(-1);
    }
    signal(SIGINT, signal_handler);

    struct sockaddr_in client_address;
    socklen_t addr_len;

    fprintf(stderr, "Server listening on port %d.  Going into request loop.\n", serverport);

	//begin while loop
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
	
	//if queue is full, wait until a consumer removes one
	while(thread_args->count == numthreads){
		pthread_cond_wait(&thread_args->cond2, &thread_args->mutex); }

	//find where to place new socket
	int index = (thread_args->first + thread_args->count)%numthreads;
	memcpy(&thread_args->thequeue[index], &new_sock, new_sock*sizeof(int));
	//something added to count- increment
	thread_args->count++;
	//signal consumer to get request
	pthread_cond_signal(&thread_args->cond1);
	pthread_mutex_unlock(&thread_args->mutex);
        }
    }
	//Make threads break while loop- since still_running was false, they should break out of while loop in threadwork (never tested, just assumed)
    thread_args->count == numthreads/2;
    pthread_cond_broadcast(&thread_args->cond1);
    pthread_cond_broadcast(&thread_args->cond2);
	//join all of the threads
    for (i = 0; i < numthreads; i++) {
        	pthread_join(threads[i], NULL); }
	//free the queue
    free(thread_args->thequeue);
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
