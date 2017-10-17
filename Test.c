#include "my_pthread_t.h"
#include <stdio.h>

//static volatile int counter=0;
typedef struct _myarg_t{
	int a;
	int b;
}myarg_t;

void *mythread(void *arg){
	myarg_t *m = (myarg_t *) arg;
	printf("%d %d\n",m->a,m->b);
	return NULL;
}
/*
void *mythread(void *arg){
	printf("%s:begin\n",(char*) arg);
	int i;
	for(i=0 ; i<1e7 ; i++){
		counter=counter+1;
	}
	printf("%s:done\n",(char*) arg);
	return NULL;
}
*/
int main(){
	pthread_t p;
	int rc;
	
	myarg_t args;
	args.a=10;
	args.b=20;
	rc=my_pthread_create(&p,NULL,mythread,&args);
}
