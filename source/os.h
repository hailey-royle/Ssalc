#ifndef OSH
#define OSH

#if defined(_WIN32)
	#error Windows not yet supported.
	//#define OS_WINDOWS 1
#elif defined(__gnu_linux__) || defined(__linux__)
	#define OS_LINUX 1
	#include <unistd.h>
	#include <pthread.h>
#elif defined(__APPLE__) && defined(__MACH__)
	#error Mac not yet supported.
	//#define OS_MAC 1
#else
	#error Unknown operating system.
#endif

#if defined( OS_LINUX )

#define thread_type pthread_t
#define barrier_type pthread_barrier_t
#define mutex_type pthread_mutex_t

i64 find_cpu_count(){
	return sysconf( _SC_NPROCESSORS_ONLN );
}

void thread_create( thread_type* thread, void* (start_routine)( void* ), void* arg ){
	i32 error = pthread_create( thread, NULL, start_routine, arg );
	assert( error == 0, "Thread could not be created." );
}

void thread_join( thread_type thread, void* thread_return ){
	i32 error = pthread_join( thread, thread_return );
	assert( error == 0, "Thread could not be created." );
}

void barrier_init( pthread_barrier_t* barrier, i32 count ){
	pthread_barrier_init( barrier, NULL, count );
}

void barrier_wait( barrier_type* barrier ){
	pthread_barrier_wait( barrier );
}

void mutex_init( mutex_type* mutex ){
	pthread_mutex_init( mutex, NULL );
}

void mutex_lock( mutex_type* mutex ){
	pthread_mutex_lock( mutex );
}

i32 mutex_trylock( mutex_type* mutex ){
	return pthread_mutex_trylock( mutex );
}

void mutex_unlock( mutex_type* mutex ){
	pthread_mutex_unlock( mutex );
}

#endif // linux


#endif
