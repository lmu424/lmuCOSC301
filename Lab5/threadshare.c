#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mymutex;
/**
 * worker thread entry point
 */
void *worker(void *varg)
{
    long *counter = (long *)varg;

    printf("thread 0x%0lx starting\n", (long)pthread_self());
    int i = 0;
    for (; i < 100000; i += 1) {
	pthread_mutex_lock(&mymutex);
        *counter += 1;
	pthread_mutex_unlock(&mymutex); 
    }
    printf("thread 0x%0lx ending with counter %ld\n", (long)pthread_self(), *counter);

    return NULL;
}


int
main(int argc, char **argv)
{
    pthread_t thread_a, thread_b;
    pthread_mutex_init(&mymutex, NULL);

    // the shared variable - allocated on the heap
    long *counter = malloc(sizeof(long));
    *counter = 0UL;
    
    pthread_create(&thread_a, NULL, worker, counter);
    pthread_create(&thread_b, NULL, worker, counter);

    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);

    printf("end counter value (main thread): %ld\n", *counter);

    free(counter);
    pthread_mutex_destroy(&mymutex);
    return 0;
}
