/* CPSC545 Spring2011 Project 2
* login: masuij(login used to submit)
* Linux
* date: 03/28/11
* name: Justin Masui,
* emails: veks11@gmail.com*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <queue>
#include <string>
#include <time.h>
#include <semaphore.h>

#define MOTHEREAGLE_SLEEP_TIME 1
#define BABYEAGLE_PLAY_TIME 2
#define BABYEAGLE_EAT_TIME 2
#define MAXLOOPS 3

using namespace std;


/************************/
/*	Globals		*/
/************************/
int numFeedingPots;
int numBabyEagles;
int numFeedings;
int fullPots;

sem_t s;
sem_t motherEagle;
pthread_mutex_t lk;

/*blocks the caller, a baby eagle, if all feeding pots are empty. One and only one baby eagle who finds out all feeding pots being empty should wake up the mother eagle. This function returns only if there is a feeding pot with food available to this baby eagle.*/
void ready_to_eat();

/*should be called when a baby eagle finishes his meal.*/
void finish_eating();

/*is only called by the mother eagle when she wants to take a nap.*/
void goto_sleep();

/*is called when the mother eagle has finished adding food in all m feeding pots.*/
void food_ready();

/*given function*/
int pthread_sleep (int seconds);

/*thread for the momma eagle*/
void *motherEagleThread_func(void *tmp){
	cout<<"Mother eagle started."<<endl;

    for(int i=0;i<MAXLOOPS;i++){
        pthread_sleep(MOTHEREAGLE_SLEEP_TIME);
        goto_sleep();
        pthread_sleep(MOTHEREAGLE_SLEEP_TIME);
        food_ready();
		cout<<"Mother eagle says \"Feeding ("<<i<<")\""<<endl;
    }
	return (void*)NULL;
    

}

/*thread for the baby eagle*/
void *babyEagleThread_func(void *tmp){
	cout<<"    Baby eagle started."<<endl;
    for(int i=0;i<MAXLOOPS;i++){
        pthread_sleep(BABYEAGLE_PLAY_TIME);
        ready_to_eat();
        pthread_sleep(BABYEAGLE_EAT_TIME);
        finish_eating();
    }
	return (void*) NULL;
}


int main(int argc, char *argv[]){
		
    if (argc != 4) {
        cerr << "Invalid command line - usage: <numFeedingPots> <numBabyEagles> <numFeedings>" << endl;
        exit(-1);
    }
	
	numFeedingPots=(int)atoi(argv[1]);
	numBabyEagles=(int)atoi(argv[2]);
	numFeedings=(int)atoi(argv[3]);
	fullPots=0;

    //int sem_init(sem_t *sem, int pshared, unsigned int value);
    sem_init(&s, 0, numFeedingPots);
    sem_init(&motherEagle, 0, 0);
	
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


    //do joins
    void *status;
    for(int t=0; t<numBabyEagles; t++) {
		int rc = pthread_join(babyEagleArr_tid[t], &status);
		if (rc) {
			printf("ERROR; return code from pthread_join() is %d\n ", rc);
			exit(-1);
		}
	}
  
	//void *status;
	pthread_join(motherEagle_tid, &status);
	cout<<"Mother eagle retires after serving "<<numFeedings+1<<".  Game ends!!!"<<endl;
	return 0;
}



/*blocks the caller, a baby eagle, if all feeding pots are empty. One and only one baby eagle who finds out all feeding pots being empty should wake up the mother eagle. This function returns only if there is a feeding pot with food available to this baby eagle.*/
void ready_to_eat(){
    cout<<"Baby eagle ready to eat"<<endl;
	pthread_mutex_lock(&lk);
	if(fullPots>0){
		fullPots--;
		sem_wait(&s);
		pthread_mutex_unlock(&lk);
	}else if(fullPots==0){
		cout<<"Mother eagle is awoke by baby eagle and starts preparing food."<<endl;
		pthread_mutex_unlock(&lk);
		sem_post(&motherEagle);

	}
}

/*should be called when a baby eagle finishes his meal.*/
void finish_eating(){
    cout<<"Baby eagle finished eating"<<endl;
}

/*is only called by the mother eagle when she wants to take a nap.*/
void goto_sleep(){
    cout<<"Mother eagle takes a nap"<<endl;
	sem_wait(&motherEagle);
}

/*is called when the mother eagle has finished adding food in all m feeding pots.*/
void food_ready(){
    cout<<"Mother Eagle finished adding food"<<endl;
	pthread_mutex_lock(&lk);
	fullPots=numFeedingPots;
	pthread_mutex_unlock(&lk);
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

