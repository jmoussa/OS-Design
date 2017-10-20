#include "my_pthread_t.h"

typedef struct _myarg_t{
int a;
int b;
}myarg_t;

typedef struct _myret_t{
int x;
int y;
}myret_t; 

void * mythread(void * arg){
	myarg_t * m = (myarg_t *) arg;
	printf("%d %d\n", m->a, m->b);
	myret_t *r = malloc(sizeof(myret_t));
	r->x = 1;
	r->y = 2;
	pthread_yield();
	return (void*) r;
}

int main(int argc,char *argv[]){
	int rc;
	pthread_t p;
	myret_t *m;
	myarg_t args;
	args.a = 10;
	args.b = 20;
	pthread_create(&p ,NULL, mythread,&args);
	pthread_join(p,(void**) &m);
	pthread_yield();
	printf("returned %d %d\n",m->x,m->y);
	return 0;
}
