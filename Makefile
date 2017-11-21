CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib




Target: my_pthread.a

my_pthread.a: my_pthread.o my_alloc.o
	$(AR) libmy_pthread.a my_pthread.o my_alloc.o
	$(RANLIB) libmy_pthread.a

my_alloc.o: my_alloc.h  my_pthread_t.h 
	$(CC) -pthread $(CFLAGS) my_pthread.c
	$(CC) $(CFLAGS) my_alloc.c  
	
my_pthread.o: my_pthread_t.h my_alloc.h
	$(CC) -pthread $(CFLAGS) my_pthread.c
	$(CC) $(CFLAGS) my_alloc.c  
	



clean:
	rm -rf testfile *.o *.a
