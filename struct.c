#include <stdio.h>
#include <stdlib.h>
#include "struct.h"

struct Queue{
    Request start;
    Request end;
    int size;
    int max_size;
};

struct Request{
    Request next;
    int info;
    struct timeval received;
};

Queue new_queue(int size){
    Queue queue = (Queue)malloc(sizeof(*queue));
    queue->start = NULL;
    queue->end = NULL;
    queue->size = 0;
    queue->max_size = size;
    return queue;
}

struct timeval time_head_received(Queue queue){
    struct timeval head_received_time;
    if (queue->size != 0) {
        head_received_time = queue->start->received;
    } else {
        head_received_time.tv_sec = 0;
        head_received_time.tv_usec = 0;
    }
    return head_received_time;
}

int queue_size(Queue queue){
    return queue->size;
}

int pull_out_queue(Queue queue) {
    if (queue->size == 0) {
        return -1;
    }
    int value = queue->start->info;
    Request next_request = queue->start->next;
    free(queue->start);
    queue->start = next_request;
    queue->size--;
    if (next_request == NULL) {
        queue->end = NULL;
    }
    return value;
}

void pull_in_queue(Queue in_progress_requests_queue, int value, struct timeval received){
    if (in_progress_requests_queue->size == in_progress_requests_queue->max_size) {
        return;
    }
    Request request = new_request(value, received);
    if (in_progress_requests_queue->size == 0) {
        in_progress_requests_queue->start = request;
        in_progress_requests_queue->end = request;
    } else {
        in_progress_requests_queue->end->next = request;
        in_progress_requests_queue->end = request;
    }
    in_progress_requests_queue->size++;
}

Request new_request(int value, struct timeval received){
    Request request = (Request)malloc(sizeof(*request));
    request->next = NULL;
    request->info = value;
    request->received = received;
    return request;
}

int pull_out_ind(Queue queue, int index) {
    if (queue->size == 0) {
        return -1;
    }
    if (index == 0) {
        return pull_out_queue(queue);
    }
    if (index < 0 || index >= queue->size) {
        return -1;
    }
    Request pull_out_req = queue->start;
    Request last_req = NULL;
    for (int i = 0; i < index; i++) {
        last_req = pull_out_req;
        pull_out_req = pull_out_req->next;
    }
    int value = pull_out_req->info;
    last_req->next = pull_out_req->next;
    free(pull_out_req);
    if (index == queue->size - 1) {
        queue->end = last_req;
    }
    queue->size--;
    return value;
}

int search_index(Queue queue, int value){
    if(queue->size != 0){
        Request current_request = queue->start;
        int ind = 0;
        while(current_request){
            if(value == current_request->info){
                return ind;
            }
            ind++;
            current_request = current_request->next;
        }
    }
    return -1;
}

void change_max_queue_size(Queue queue,int new_size){
    queue->max_size = new_size;
}



















