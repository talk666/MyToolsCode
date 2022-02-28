#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/shm.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <assert.h>
#include <syslog.h>

#include "misc.h"

extern int errno;

pthread_mutex_t *mutex;
pthread_mutexattr_t mutexattr;

int loop_lock_init(void)
{
    mutex = (pthread_mutex_t *)mmap(NULL,sizeof(pthread_mutex_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANON,-1,0);
    if(mutex == MAP_FAILED)
    {
        printf("loop_lock_init: mmap failed.\n");
        return -1;
    }

    memset(mutex, 0, sizeof(pthread_mutex_t));

    assert(pthread_mutexattr_init(&mutexattr) == 0);
    assert(pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED) == 0);
    assert(pthread_mutex_init(mutex, &mutexattr) == 0);

    return 0;
}

int loop_lock_release(void)
{
    pthread_mutexattr_destroy(&mutexattr);
    pthread_mutex_destroy(mutex);
    munmap(mutex, sizeof(pthread_mutex_t));
    return 0;
}

int loop_count_lock(void)
{
    assert(pthread_mutex_lock(mutex) == 0);

    return 0;
}

int loop_count_unlock(void)
{
    assert(pthread_mutex_unlock(mutex) == 0);
    return 0;
}

/* shared memory operation : get and set */
#define SHM_GLOBAL_LOOP_COUNT_MODE (SHM_R|SHM_W|IPC_CREAT)

static int shm_loop_key = 0;

typedef struct shm_loop_data
{
    uint64_t shm_loop_count;
    uint64_t shm_loop_status;
}shm_loop_data_t;

static int shmid;
static void *shmptr;

int shm_loop_init(void)
{
    shm_loop_key = getpid();
    shmid = shmget(shm_loop_key, sizeof(shm_loop_data_t), SHM_GLOBAL_LOOP_COUNT_MODE);
    if(shmid < 0)
    {
        printf("ERROR: shm_loop_init: shmget failed. errno=%d.\n", errno);
        return -1;
    }

    return 0;
}

int shm_loop_attach(void)
{
    if((shmptr = shmat(shmid, 0, 0)) == (void *)-1)
    {
        printf("ERROR: shm_loop_release: shmat failed. errno=%d.\n", errno);
        return -1;
    }

    return 0;
}

int shm_loop_detach(void)
{
    if(shmdt(shmptr) == -1)
    {
        printf("ERROR: shm_loop_release: shmat failed. errno=%d.\n", errno);
        return -1;
    }

    return 0;
}

int shm_loop_release(void)
{
    int ret;

    ret = shmctl(shmid, IPC_RMID, NULL);
    if(ret < 0)
    {
        printf("ERROR: shm_loop_release: shmat failed. errno=%d.\n", errno);
        return -1;
    }

    return 0;
}

uint64_t *shm_get_variable(void)
{
    return shmptr;
}

int shm_set_variable(shm_loop_data_t *data)
{
    memcpy(shmptr, data, sizeof(shm_loop_data_t));

    return 0;
}


uint64_t *shm_get_global_loop_count(void)
{
    shm_loop_data_t *data;
    data = (shm_loop_data_t *)shm_get_variable();
    if(data == (shm_loop_data_t *)-1)
    {
        return (uint64_t *)-1;
    }

    return &(data->shm_loop_count);
}

int shm_set_global_loop_count(uint64_t loop_count)
{
    shm_loop_data_t *data;
    data = (shm_loop_data_t *)shm_get_variable();
    if(data == (shm_loop_data_t *)-1)
    {
        return -1;
    }

    memcpy(&(data->shm_loop_count), &loop_count, sizeof(uint64_t));
    return 0;
}

uint64_t *shm_get_global_loop_status(void)
{
    shm_loop_data_t *data;
    data = (shm_loop_data_t *)shm_get_variable();
    if(data == (shm_loop_data_t *)-1)
    {
        return (uint64_t *)-1;
    }

    return &(data->shm_loop_status);
}

int shm_set_global_loop_status(uint64_t loop_status)
{
    shm_loop_data_t *data;
    data = (shm_loop_data_t *)shm_get_variable();
    if(data == (shm_loop_data_t *)-1)
    {
        return -1;
    }

    memcpy(&(data->shm_loop_status), &loop_status, sizeof(uint64_t));
    return 0;
}

timer_t timer_id;
/* set alarm timer to limit test time */
void rsp_alarm_clock_handler(union sigval v)
{
    shm_set_global_loop_status(0);
    timer_delete(timer_id);
}

void create_alarm_clock_thread(int test_time)
{
    struct sigevent evp;

    memset(&evp, 0, sizeof(struct sigevent));
    evp.sigev_value.sival_int = 111;
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = rsp_alarm_clock_handler;

    if(timer_create(CLOCK_REALTIME, &evp, &timer_id) == -1)
    {
        printf("ERROR: timer_create failed.\n");
        return;
    }


    struct itimerspec it;
    it.it_interval.tv_sec = test_time;
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = test_time;
    it.it_value.tv_nsec = 0;

    if(timer_settime(timer_id, 0, &it, NULL) == -1)
    {
        printf("ERROR: timer_settime failed.\n");
        return;
    }

    return;
}

void delete_alarm_clock_thread(void)
{
    int ret;
    struct itimerspec it;

    ret = timer_gettime(timer_id, &it);
    if(ret == -1)
    {
        /* timer_gettime failed. */
        return;
    }else{
        if((it.it_value.tv_sec == 0) &&(it.it_value.tv_nsec == 0))
        {
            return;
        }else{
            timer_delete(timer_id);
            return;
        }
    }
}
