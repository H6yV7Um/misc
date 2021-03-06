/*
 desc: 	
 author: interma (interma@outlook.com)
 created: 06/07/16 11:03:02 HKT
*/

//This program is demo for using pthreads with libev.
//Try using Timeout values as large as 1.0 and as small as 0.000001
//and notice the difference in the output

//(c) 2009 debuguo
//(c) 2013 enthusiasticgeek for stack overflow
//Free to distribute and improve the code. Leave credits intact

#include <ev.h>
#include <stdio.h> // for puts
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock;
double timeout = 0.00001;
ev_timer timeout_watcher;
int timeout_count = 0;

ev_async async_watcher;
int async_count = 0;

struct ev_loop* loop2;

void* loop2thread(void* args)
{
    printf("Inside loop 2"); // Here one could initiate another timeout watcher
    ev_loop(loop2, 0);       // similar to the main loop - call it say timeout_cb1
    return NULL;
}

static void async_cb (EV_P_ ev_async *w, int revents)
{
    //puts ("async ready");
    pthread_mutex_lock(&lock);     //Don't forget locking
    ++async_count;
    printf("async = %d, timeout = %d \n", async_count, timeout_count);
    pthread_mutex_unlock(&lock);   //Don't forget unlocking
}

static void timeout_cb (EV_P_ ev_timer *w, int revents) // Timer callback function
{
    //puts ("timeout");
    if (ev_async_pending(&async_watcher)==false) { //the event has not yet been processed (or even noted) by the event loop? (i.e. Is it serviced? If yes then proceed to)
        ev_async_send(loop2, &async_watcher); //Sends/signals/activates the given ev_async watcher, that is, feeds an EV_ASYNC event on the watcher into the event loop.
    }

    pthread_mutex_lock(&lock);     //Don't forget locking
    ++timeout_count;
    pthread_mutex_unlock(&lock);   //Don't forget unlocking
    w->repeat = timeout;
    ev_timer_again(loop, &timeout_watcher); //Start the timer again.
}

int main (int argc, char** argv)
{
    if (argc < 2) {
        puts("Timeout value missing.\n./demo <timeout>");
        return -1;
    }
    timeout = atof(argv[1]);

    struct ev_loop *loop = EV_DEFAULT;  //or ev_default_loop (0);

    //Initialize pthread
    pthread_mutex_init(&lock, NULL);
    pthread_t thread;

    // This loop sits in the pthread
    loop2 = ev_loop_new(0);

    //This block is specifically used pre-empting thread (i.e. temporary interruption and suspension of a task, without asking for its cooperation, with the intention to resume that task later.)
    //This takes into account thread safety
    ev_async_init(&async_watcher, async_cb);
    ev_async_start(loop2, &async_watcher);
    pthread_create(&thread, NULL, loop2thread, NULL);

    ev_timer_init (&timeout_watcher, timeout_cb, timeout, 0.); // Non repeating timer. The timer starts repeating in the timeout callback function
    ev_timer_start (loop, &timeout_watcher);

    // now wait for events to arrive
    ev_loop(loop, 0);
    //Wait on threads for execution
    pthread_join(thread, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}
