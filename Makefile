CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib


Target: my_pthread.a

my_pthread.a: my_pthread.o
	$(AR) libmy_pthread.a my_pthread.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h my_alloc.h
	$(CC) -pthread $(CFLAGS) my_pthread.c 
	$(CC) $(CFLAGS) my_alloc.c 
	

clean:
	rm -rf testfile *.o *.a
