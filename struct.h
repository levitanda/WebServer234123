#include <stdbool.h>
#include <sys/time.h>

typedef struct Queue *Queue;
typedef struct Request *Request;
Queue new_queue(int size);
struct timeval time_head_received(Queue queue);
int queue_size(Queue queue);
void pull_in_queue(Queue in_progress_requests_queue, int value, struct timeval received);
int pull_out_queue(Queue queue);
Request new_request(int value, struct timeval received);
int pull_out_ind(Queue queue, int index);
int search_index(Queue queue, int value);
void change_max_queue_size(Queue queue,int new_size);
