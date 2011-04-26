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
#include <sstream>

#define MOTHEREAGLE_SLEEP_TIME 1
#define BABYEAGLE_PLAY_TIME 2
#define BABYEAGLE_EAT_TIME 2

using namespace std;
/************************/
/*	Data Structure	*/
/************************/
struct motherEagleData{
	int numFeedings;
};

struct babyEagleData{
	int eagleId;
	int numFeedings;
};

/************************/
/*	Globals		*/
/************************/
int numFeedingPots;
int numBabyEagles;
int numFeedings;
int fullPots;

int numWaitingBabyEagles=0;
bool alreadyWoken=false;
bool motherRetired=false;  //TODO:  Needs to be protected

sem_t semFeedingPots; //value is equivalent to the number of feedingPots available
sem_t motherEagle; //0 means motherEagle is sleeping, 1 means motherEagle is active
sem_t semWaiting;
pthread_mutex_t mutFeedingPots=PTHREAD_MUTEX_INITIALIZER;//locking the feedingPots
pthread_mutex_t tty_lk=PTHREAD_MUTEX_INITIALIZER;//locking the tty

/*is only called by the mother eagle when she wants to take a nap.*/
void goto_sleep();

/*is called when the mother eagle has finished adding food in all m feeding pots.*/
void food_ready(int nthServing);

/*blocks the caller, a baby eagle, if all feeding pots are empty. One and only one baby eagle who finds out all feeding pots being empty should wake up the mother eagle. This function returns only if there is a feeding pot with food available to this baby eagle.*/
void ready_to_eat(int eagleId);

/*should be called when a baby eagle finishes his meal.*/
void finish_eating(int eagleId);

/*given function*/
int pthread_sleep (int seconds);

/*locks and unlock tty*/
void safePrint(string output);

/*converts int to string*/
string itos(int i);

/*returns a string with num spaces specified in input*/
string strSpaces(int numSpaces);

/*thread for the momma eagle*/
void *motherEagleThread_func(void *motherEagleArgs){
	struct motherEagleData *args;
	args=(struct motherEagleData *) motherEagleArgs;
	
	safePrint("Mother eagle started.");
	
    for(int i=0;i<numFeedings;i++){
        pthread_sleep(MOTHEREAGLE_SLEEP_TIME);
        goto_sleep();
        pthread_sleep(MOTHEREAGLE_SLEEP_TIME);
        food_ready(i);
    }
	
	motherRetired=true;
	
	//If motherEagle is retiring then signal all children to wake up and fly away
	for(int i=0;i<numBabyEagles;i++){
		sem_post(&semWaiting);
	}
	
	
	return (void*)NULL;
}

/*thread for the baby eagle*/
void *babyEagleThread_func(void *babyEagleArgs){
	struct babyEagleData *args;
	args=(struct babyEagleData *) babyEagleArgs;
	
	safePrint(strSpaces(args->eagleId)+"Baby eagle "+itos(args->eagleId)+" started.");
	
    while(1){

        ready_to_eat(args->eagleId);
        pthread_sleep(BABYEAGLE_EAT_TIME);

        finish_eating(args->eagleId);
        pthread_sleep(BABYEAGLE_PLAY_TIME);
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

    sem_init(&semFeedingPots, 0, numFeedingPots);
    sem_init(&motherEagle, 0, 0);
	sem_init(&semWaiting, 0, 0);
	
	pthread_t motherEagle_tid;
	pthread_t babyEagleArr_tid[numBabyEagles];
	
	struct motherEagleData motherEagleArgs={ numFeedings };
	struct babyEagleData babyEagleArgs[numBabyEagles];
	
	int rc;//return code

	rc = pthread_create(&motherEagle_tid, NULL, motherEagleThread_func, (void*) &motherEagleArgs);
	if ( rc == -1){
		perror("pthread_create");
		return -1;
	}

	for(int i=0; i<numBabyEagles; i++){
		babyEagleArgs[i].eagleId=i;
		babyEagleArgs[i].numFeedings=numFeedings;
		rc=pthread_create(&babyEagleArr_tid[i], NULL, &babyEagleThread_func, (void*) &babyEagleArgs[i]);
		if ( rc == -1){
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

	cout<<"Mother eagle retires after serving "<<numFeedings<<".  Game ends!!!"<<endl;

    //clean up
    sem_destroy(&semFeedingPots); 
    sem_destroy(&motherEagle);
    sem_destroy(&semWaiting);
    pthread_mutex_destroy(&mutFeedingPots);
    pthread_mutex_destroy(&tty_lk);
	return 0;
}

/*is only called by the mother eagle when she wants to take a nap.*/
void goto_sleep(){
    safePrint("Mother eagle takes a nap");
	sem_wait(&motherEagle);
}

/*is called when the mother eagle has finished adding food in all m feeding pots.*/
void food_ready(int nthServing){

    pthread_mutex_lock(&mutFeedingPots);
	

	
	alreadyWoken=false;
	fullPots=numFeedingPots;
	cout<<"Mother eagle says \"Feeding ("<<nthServing<<")\""<<endl;
	pthread_mutex_unlock(&mutFeedingPots);
	
	//wake up waiting baby eagles
	int semVal;
	sem_getvalue(&semFeedingPots, &semVal);
	cout<<"----------------"<<endl;
	cout<<"before semVal="<<semVal<<endl;
	cout<<"----------------"<<endl;
	//int numSignals=numWaitingBabyEagles;
	cout<<"DEBUG:  numWaitingBabyEagles: "<<numWaitingBabyEagles<<endl;
	cout<<"DEBUG:  numFeedingPots: "<<numFeedingPots<<endl;
	for(int i=0;i<numFeedingPots;i++){
		numWaitingBabyEagles--;
		//sem_post(&semFeedingPots);
		sem_post(&semWaiting);
	}
	sem_getvalue(&semFeedingPots, &semVal);
	cout<<"----------------"<<endl;
	cout<<"after semVal="<<semVal<<endl;
	cout<<"----------------"<<endl;
	

}


/*blocks the caller, a baby eagle, if all feeding pots are empty. One and only one baby eagle who finds out all feeding pots being empty should wake up the mother eagle. This function returns only if there is a feeding pot with food available to this baby eagle.*/
void ready_to_eat(int eagleId){
	safePrint(strSpaces(eagleId)+"Baby eagle "+itos(eagleId)+" ready to eat, fullPots= "+itos(fullPots));
	
	bool fed=false;
	while(fed==false){

		pthread_mutex_lock(&mutFeedingPots);
		if(motherRetired==false && fullPots>0){
			sem_wait(&semFeedingPots);
			cout<<strSpaces(eagleId)<<"Baby eagle "<<eagleId<<" is eating using feeding pot "<< fullPots <<endl;
			cout.flush();
			fullPots--;	
			fed=true;
			cout.flush();
			pthread_mutex_unlock(&mutFeedingPots);
			sem_post(&semFeedingPots);
		}else if(motherRetired==false && fullPots ==0){
			if( alreadyWoken==false ){
				sem_post(&motherEagle);
				alreadyWoken=true;
				pthread_mutex_lock(&tty_lk);
				cout<<"Mother eagle is awoke by baby eagle "<<eagleId<<" and starts preparing food."<<endl;
				pthread_mutex_unlock(&tty_lk);
			}
			numWaitingBabyEagles++;
			pthread_mutex_unlock(&mutFeedingPots);

			//safePrint("DEBUG:  Baby eagle "+itos(eagleId)+" hungry, Waiting");
			sem_wait(&semWaiting);
			//safePrint("DEBUG:  Baby eagle "+itos(eagleId)+" hungry, NOT Waiting");
			
		}else if(motherRetired==true && fullPots >0){
			sem_wait(&semFeedingPots);		
			cout<<strSpaces(eagleId)<<"Baby eagle "<<eagleId<<" is eating using feeding pot "<< fullPots <<endl;
			fullPots--;	
			fed=true;
			cout.flush();
			pthread_mutex_unlock(&mutFeedingPots);
			sem_post(&semFeedingPots);
		}else if(motherRetired==true && fullPots ==0){
			//cout<<"DEBUG:  motherEagle retired, Baby eagle "<<eagleId<<" Exiting"<<endl; 
			pthread_mutex_unlock(&mutFeedingPots);
			pthread_exit(NULL);
		}
	
	}

}

/*should be called when a baby eagle finishes his meal.*/
void finish_eating(int eagleId){
	safePrint(strSpaces(eagleId)+"Baby eagle "+itos(eagleId)+" finished eating");
	if(motherRetired==true && fullPots==0){
		pthread_exit(NULL);
	}
	
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

void safePrint(string output){
	pthread_mutex_lock(&tty_lk);
	cout<<output<<endl;
	cout.flush();
	pthread_mutex_unlock(&tty_lk);
}

string itos(int i){
	std::string s;
	std::stringstream out;
	out << i;
	return out.str();
}

/*returns a string with num spaces specified in input*/
string strSpaces(int numSpaces){
	string rs; //return string
	for(int i=0; i<numSpaces; i++){
		rs=rs+" ";
	}
	return rs;
}
