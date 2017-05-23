/* 
 * File:   main.c
 * Author: Matthew
 *
 * Created on April 29, 2016, 2:49 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

/************************************************************************
 * preprocessor directives
 ************************************************************************/
#define INITIAL_NUMBER_COKES 5
#define MAX_NUMBER_COKES 10

#define MAX_RUNS 10

#define NUMBER_PRODUCERS 2
#define NUMBER_CONSUMERS 20


/************************************************************************
 * for convenience, global variables - to be read by threads
 ************************************************************************/
pthread_mutex_t lock;
pthread_cond_t not_empty, not_full;
int cokes = INITIAL_NUMBER_COKES;


/************************************************************************
 * function prototype declarations
 ************************************************************************/
void* thread_function(void* function);

void refill_coke(void);
void consume_coke(void);


/************************************************************************
 * MAIN
 ************************************************************************/
int main(int argc, const char * argv[]) {
    
    int i;
    pthread_t producers[NUMBER_PRODUCERS];
    pthread_t consumers[NUMBER_CONSUMERS];

    // init mutex and condition variables
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);
    
    // create consumers
    for (i=0; i<NUMBER_CONSUMERS; i++) {
        if (pthread_create(&consumers[i], NULL, thread_function, *consume_coke)) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    
    // create producers
    for (i=0; i<NUMBER_PRODUCERS; i++) {
        if (pthread_create(&producers[i], NULL, thread_function, *refill_coke)) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // just sleep and then bail out
    sleep(5);
    
    return EXIT_SUCCESS;
}


/************************************************************************
 * generic thread function
 ************************************************************************/
void* thread_function(void* function) {
    int i;
    
    for (i=0; i<MAX_RUNS; i++) {
        // call function, either of refill_coke() or consume_coke()
        ((void (*)(void))function)();
    }

    pthread_exit(NULL);
}


/************************************************************************
 * refilling and consuming coke functions
 ************************************************************************/
void refill_coke(void) {
    //lock mutex
    pthread_mutex_lock(&lock);
    //if the coke machine is full, wait on conditional variable
    while(cokes == MAX_NUMBER_COKES) {
        printf("Full. Time to sleep\n");
        pthread_cond_wait(&not_full, &lock);
    }
    //reset number of cokes
    cokes = MAX_NUMBER_COKES;
    
    printf("Cokes refilled\n");
    
    //when coke machine is refilled, broadcast conditional variable to all sleeping consumers
    pthread_cond_broadcast(&not_empty);
    //unlock mutex
    pthread_mutex_unlock(&lock);
}


void consume_coke(void) {
    //lock mutex
    pthread_mutex_lock(&lock);
    //if coke machine is empty, wait on conditional variable
    while(cokes == 0) {
        printf("Empty. Time to sleep\n");
        pthread_cond_wait(&not_empty, &lock);
    }
    
    //print message and remove one coke
    printf("Coke taken from %d\n", cokes--);
    
    //broadcast conditional variable to wake up all sleeping producers when a coke is consumed
    pthread_cond_broadcast(&not_full);
    //unlock mutex
    pthread_mutex_unlock(&lock);
}
