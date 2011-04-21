#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <queue>
#include <string>
#include <time.h>
#include <sstream>
#include <fstream>

using namespace std;

/************************/
/*	Globals		*/
/************************/
int numFeedingPots;
int numBabyEagles;
int numFeedings;

/*blocks the caller, a baby eagle, if all feeding pots are empty. One and only one baby eagle who finds out all feeding pots being empty should wake up the mother eagle. This function returns only if there is a feeding pot with food available to this baby eagle.*/
void ready_to_eat();

/*should be called when a baby eagle finishes his meal.*/
void finish_eating();

/*is only called by the mother eagle when she wants to take a nap.*/
void goto_sleep();

/*is called when the mother eagle has finished adding food in all m feeding pots.*/
void food_ready();


/*thread for the momma eagle*/
void *motherEagleThread_func(void *tmp){
	cout<<"mother thread"<<endl;
	return (void*)NULL;
}

/*thread for the baby eagle*/
void *babyEagleThread_func(void *tmp){
	cout<<"baby thread"<<endl;
	return (void*) NULL;
}

/*given function*/
int pthread_sleep (int seconds);

int main(int argc, char *argv[])
{
		
    if (argc != 4) {
        cerr << "Invalid command line - usage: <numFeedingPots> <numBabyEagles> <numFeedings>" << endl;
        exit(-1);
    }
	
	numFeedingPots=(int)atoi(argv[1]);
	numBabyEagles=(int)atoi(argv[2]);
	numFeedings=(int)atoi(argv[3]);
	
	//cout<<numFeedingPots<<numBabyEagles<<numFeedings<<endl;
	
	pthread_t motherEagle_tid;
	pthread_t babyEagleArr_tid[numBabyEagles];
	
	/*if ( -1 == pthread_create(&motherEagle_tid, NULL, &motherEagleThread_func, (void*) NULL)){
		perror("pthread_create");
		return -1;
	}*/
	long temp=1;
	int rc = pthread_create(&motherEagle_tid, NULL, motherEagleThread_func, (void*) temp);
	

	for(int i=0; i<numBabyEagles; i++){
		if ( -1 == pthread_create(&babyEagleArr_tid[i], NULL, &babyEagleThread_func, (void*) temp)){
			perror("pthread_create");
			return -1;
		}
	}

	return 0;
}





/******************************************************************************
pthread_sleep takes an integer number of seconds to pause the current thread We
provide this function because one does not exist in the standard pthreads library. We simply use a function that has a timeout.
*****************************************************************************/
int pthread_sleep (int seconds)
{
	pthread_mutex_t mutex;
	pthread_cond_t conditionvar;
	struct timespec timetoexpire;
	if(pthread_mutex_init(&mutex,NULL))
	{
		return -1;
	}
	if(pthread_cond_init(&conditionvar,NULL))
	{
		return -1;
	}
	//When to expire is an absolute time, so get the current time and add
	//it to our delay time
	timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;
	timetoexpire.tv_nsec = 100;
	return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
}

