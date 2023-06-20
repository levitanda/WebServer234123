#include "segel.h"
#include "request.h"
#include "struct.h"
#include <pthread.h>

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
Queue waiting_request_queue = NULL;
Queue in_progress_requests_queue = NULL;
pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_cond_t bcond;
int* sthread;
int* dthread;
int* sumup_thread;


void getargs(int *port, int argc, char *argv[], int *thread, int *size, int *max_size, char* distribution)
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    } else if (argc == 6) *max_size = strtol(argv[5], NULL, 10);
    *port = strtol(argv[1], NULL, 10);
    *thread = strtol(argv[2], NULL, 10);
    *size = strtol(argv[3], NULL, 10);
    strncpy(distribution, argv[4], sizeof(distribution) - 1);
    distribution[sizeof(distribution) - 1] = '\0';
}


void* handle_thread(void* args)
{
    int ind = ((int *)args)[0];
    while (1) {
        pthread_mutex_lock(&mutex);
        while (queue_size(waiting_request_queue)==0) {
            pthread_cond_wait(&cond, &mutex);
        }
        struct timeval received = time_head_received(waiting_request_queue);
        int fd = pull_out_queue(waiting_request_queue); 
        pull_in_queue(in_progress_requests_queue, fd, received);
        pthread_mutex_unlock(&mutex);
        struct timeval handle_time;
        gettimeofday(&handle_time, NULL);
        requestHandle(fd, sthread, dthread, sumup_thread, ind, received, handle_time);
        Close(fd);
        pthread_mutex_lock(&mutex);
        int index = search_index(in_progress_requests_queue, fd);
        pull_out_ind(in_progress_requests_queue, index);
        pthread_cond_signal(&bcond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    int listenfd, con, port, clientlen, threads, queues_size, max_size;
    char distribution[7];
    struct sockaddr_in clientaddr;
    getargs(&port, argc, argv, &threads, &queues_size, &max_size, distribution);
    waiting_request_queue = new_queue(queues_size);
    in_progress_requests_queue = new_queue(threads);
    
    pthread_t* usual_threads = malloc(sizeof(*usual_threads)*threads);
    for(int i = 0; i < threads; i++){
        int* thread_args = malloc(sizeof(int));
        *thread_args = i;
        pthread_create(&usual_threads[i], NULL, handle_thread, (void *)thread_args);
    }
    dthread = malloc(sizeof(int)*threads);
    sthread = malloc(sizeof(int)*threads);
    sumup_thread = malloc(sizeof(int)*threads);
    for(int i =0; i < threads; i++){
        dthread[i] = 0;
        sthread[i] = 0;
    }
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&bcond, NULL);
    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        con = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        pthread_mutex_lock(&mutex);
        
        if (queue_size(waiting_request_queue) + queue_size(in_progress_requests_queue) == queues_size) {
            if (strcmp(distribution, "block") != 0) {
                if (strcmp(distribution, "dh") == 0) {
                    if (queue_size(waiting_request_queue)!=0) {
                        int fd = pull_out_queue(waiting_request_queue);
                        Close(fd);
                    } else {
                        Close(con);
                        pthread_mutex_unlock(&mutex);
                        continue;
                    }
                } else if (strcmp(distribution, "random") == 0) {
                    if (queue_size(waiting_request_queue)!=0) {
                        int dropping = (int)((queue_size(waiting_request_queue) + 1) / 2);
                        for (int i = 0; i < dropping; i++) {
                            if (queue_size(waiting_request_queue)==0)
                                break;
                            int random_number = rand();
                            int index = random_number % queue_size(waiting_request_queue);
                            int fd = pull_out_ind(waiting_request_queue, index);
                            Close(fd);
                        }
                    } else {
                        Close(con);
                        pthread_mutex_unlock(&mutex);
                        continue;
                    }
                } else if (strcmp(distribution, "dt") == 0) {
                    Close(con);
                    pthread_mutex_unlock(&mutex);
                    continue;
                } else if (strcmp(distribution, "bf") == 0){
                    while (queue_size(in_progress_requests_queue) + queue_size(waiting_request_queue) != 0){
                        pthread_cond_wait(&bcond, &mutex);
                    }
                } else if (strcmp(distribution, "dynamic") == 0){
                    if (queues_size < max_size){
                        queues_size++;
                        change_max_queue_size(waiting_request_queue, queues_size);
                        Close(con);
                        pthread_mutex_unlock(&mutex);
                        continue;
                    } else {
                        Close(con);
                        pthread_mutex_unlock(&mutex);
                        continue;
                    }
                }
            } else {
                while (queue_size(in_progress_requests_queue) + queue_size(waiting_request_queue) == queues_size) {
                    pthread_cond_wait(&bcond, &mutex);
                }
            }
        }
        struct timeval received;
        gettimeofday(&received, NULL);
        pull_in_queue(waiting_request_queue, con, received);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex); 
    }
      
}


    


 
