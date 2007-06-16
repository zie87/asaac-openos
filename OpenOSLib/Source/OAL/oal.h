#ifndef OAL_H_
#define OAL_H_

// OS ABSTRACTION LAYER

#include <limits.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <sstream>
#include <string>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <arpa/inet.h>


//STRUCTS

#define oal_thread_t					pthread_t
#define oal_thread_attr_t				pthread_attr_t
#define oal_thread_mutex_t				pthread_mutex_t	
#define oal_thread_mutexattr_t			pthread_mutexattr_t
#define oal_thread_cond_t				pthread_cond_t 		
#define oal_thread_condattr_t			pthread_condattr_t	
#define oal_thread_spinlock_t			pthread_spinlock_t
#define oal_sem_t						sem_t
#define oal_mq_attr						mq_attr
#define oal_mqd_t						mqd_t
#define oal_off_t						off_t

//FUNCTIONS

#define oal_sem_init					sem_init
#define oal_sem_destroy					sem_destroy
#define oal_sem_wait 					sem_wait
#define oal_sem_timedwait 				sem_timedwait
#define oal_sem_post					sem_post
#define oal_sem_getvalue				sem_getvalue

#define oal_mq_send						mq_send
#define oal_mq_timedsend				mq_timedsend
#define oal_mq_receive					mq_receive
#define oal_mq_timedreceive				mq_timedreceive
#define oal_mq_unlink					mq_unlink
#define oal_mq_open						mq_open
#define oal_mq_close					mq_close

#define oal_shm_open					shm_open
#define oal_shm_unlink				 	shm_unlink
#define oal_truncate					truncate
#define oal_ftruncate					ftruncate
    
#define oal_open						open
#define oal_close						close
#define oal_remove						remove

#define oal_mmap						mmap
#define oal_munmap						munmap

#define oal_kill						kill

#define oal_creat						creat
#define oal_fcntl						fcntl
#define oal_read						read
#define oal_write						write
#define oal_lseek						lseek
#define oal_fchmod						fchmod
#define oal_mkstemp						mkstemp
#define oal_unlink						unlink

#define oal_sigaction					sigaction
#define oal_sigemptyset					sigemptyset
#define oal_sigqueue					sigqueue
#define oal_sigaddset					sigaddset
#define oal_sigtimedwait				sigtimedwait
#define oal_sigwaitinfo					sigwaitinfo

#define oal_waitpid						waitpid
#define oal_sched_yield					sched_yield

#define oal_thread_kill					pthread_kill
#define oal_thread_exit					pthread_exit
#define oal_thread_cancel				pthread_cancel
#define oal_thread_create				pthread_create
#define oal_thread_join					pthread_join

#define oal_thread_mutexattr_init		pthread_mutexattr_init
#define oal_thread_mutexattr_setpshared	pthread_mutexattr_setpshared
#define oal_thread_mutex_init			pthread_mutex_init
#define oal_thread_mutex_destroy		pthread_mutex_destroy
#define oal_thread_mutex_unlock			pthread_mutex_unlock
#define oal_thread_mutex_lock			pthread_mutex_lock
#define oal_thread_mutex_timedlock		pthread_mutex_timedlock

#define oal_thread_attr_setstacksize	pthread_attr_setstacksize
#define oal_thread_attr_getstacksize	pthread_attr_getstacksize
#define oal_thread_attr_init			pthread_attr_init
#define oal_thread_attr_destroy			pthread_attr_destroy

#define oal_thread_cond_wait 			pthread_cond_wait
#define oal_thread_cond_timedwait 		pthread_cond_timedwait
#define oal_thread_condattr_init		pthread_condattr_init
#define oal_thread_condattr_setpshared	pthread_condattr_setpshared
#define oal_thread_cond_init			pthread_cond_init
#define oal_thread_cond_broadcast		pthread_cond_broadcast
#define oal_thread_cond_destroy			pthread_cond_destroy

#define oal_thread_getcancelstate		pthread_getcancelstate
#define oal_thread_setcancelstate		pthread_setcancelstate
#define oal_thread_testcancel			pthread_testcancel

#define oal_thread_self					pthread_self
#define oal_thread_equal				pthread_equal

#define oal_thread_spin_init			pthread_spin_init
#define oal_thread_spin_destroy			pthread_spin_destroy
#define oal_thread_spin_lock			pthread_spin_lock
#define oal_thread_spin_unlock			pthread_spin_unlock

#define oal_stat						stat
       
#define oal_inet_addr					inet_addr       

long oal_process_fork();
long oal_process_exec();
long oal_process_kill();
long oal_process_config();
long oal_process_run();
long oal_process_stop();

long oal_thread_create();
long oal_thread_destroy();
long oal_thread_config();
long oal_thread_run();
long oal_thread_stop();

       
#endif /*OAL_H_*/
