#ifndef __SHARED_QUEUE__
#define __SHARED_QUEUE__

#include <pthread.h>
#include <queue> 

using namespace std;

template <typename T> class SharedQueue{

	public:
		queue<T> queue;
		unsigned long int maxSize;
		pthread_mutex_t mutex;
		pthread_cond_t cond;


	public:

		SharedQueue(){
			pthread_mutex_init(&mutex, NULL);
			pthread_cond_init(&cond, NULL);
			maxSize = -1; //infinite capacity
		}

		SharedQueue(unsigned long int max){
			pthread_mutex_init(&mutex, NULL);
			pthread_cond_init(&cond, NULL);
			maxSize = max;
		}


		~SharedQueue() {
			pthread_mutex_destroy(&mutex);
			pthread_cond_destroy(&cond);
		}

		inline void push(T e){
			pthread_mutex_lock(&mutex);
			while (queue.size() == maxSize) {
				pthread_cond_wait(&cond, &mutex);
			}
			queue.push(e);
			pthread_mutex_unlock(&mutex);
			pthread_cond_signal(&cond); //by signaling here we reduce locking overhead
		}

		inline T pop(){
			pthread_mutex_lock(&mutex);
			while (queue.size() == 0) {
            	pthread_cond_wait(&cond, &mutex);
        	}
			T e = queue.front();
			queue.pop();
			pthread_mutex_unlock(&mutex);
			pthread_cond_signal(&cond);
			return e;
		}


		inline int size() {
        	pthread_mutex_lock(&mutex);
        	int size = queue.size();
        	pthread_mutex_unlock(&mutex);
        	return size;
    	}

};

#endif