#define _GNU_SOURCE
#include <sched.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/time.h>

#include <pthread.h>
#include <assert.h>
#include <sys/eventfd.h>

#include "atomic.h"
#include "misc.h"

// extern void debug_printf(const char *fmt, ...);
#define PRINT_HEX_PRINTF(...) printf(__VA_ARGS__)

void print_hex(char *title, int offset, unsigned char *data, int dataLen)
{
    int   i = 0;
    int   lineLen;
    char  tmpBuf[20];
    char *buf;
    int   len = dataLen;

    if (title != NULL) {
        PRINT_HEX_PRINTF("%s", title);
    }

    while (1) {
        buf = tmpBuf;

        lineLen = (dataLen > 16 ? 16 : dataLen);
        PRINT_HEX_PRINTF("%06X:", offset);

        for (i = 0; i < 16; i++) {
            if (i < lineLen) {
                PRINT_HEX_PRINTF(" %02X", data[i]);
                if (data[i] < 0x20 || data[i] > 0x7e)
                    *buf++ = '.';
                else
                    *buf++ = data[i];
            } else {
                PRINT_HEX_PRINTF("   ");
                *buf++ = ' ';
            }

            if (i == 7) {
                PRINT_HEX_PRINTF(" ");
                *buf++ = ' ';
            }
        }

        *buf++ = '\0';
        PRINT_HEX_PRINTF("  |%s|\r\n", tmpBuf);

        offset += lineLen;
        dataLen -= lineLen;
        data += lineLen;

        if (lineLen < 16 || dataLen == 0) {
            PRINT_HEX_PRINTF("DEC:%05d HEX:%04X\r\n\r\n", len, len);
            return;
        }
    }
}

#define MAX_LOOP    0xFFFFFFFF

static atomic_t pack_cnt;
static atomic_t pack_failed;
static atomic_t thread_cnt;

static uint64_t thread_global_loop_count = 0;

typedef struct benchmark_
{
    void *hdev;
    int algo;
    int api_mode;
    int process_num;
    int thread_num;
    int test_time; /* per minute*/
    uint64_t size;
    uint64_t loop;
    uint8_t key_mode;
    uint8_t dev;
    uint8_t check_rst;
    uint8_t check_perf;
    uint8_t dev_type;
    uint32_t handle_cnt;
} benchmark_t;

static benchmark_t g_benchmark;

/*
 * algo_info is to show the all supported algo as list.
 * algo_info is mapped with bechmark_algo_e.
 */
static char *algo_info[] =
{
    "Test-1:","Test-2","Test-3","Test-4","Test-5"
};


static uint64_t get_current_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec*1000*1000 + tv.tv_usec);
}

static void print_performance_info(uint8_t algo, uint64_t start, uint64_t stop, uint64_t size, uint64_t loop)
{
    //float bits = (float)((size*loop*8)/1024.0/1024.0);
    float time_ms = (float)(stop - start)/1000.0;
    float time_s = time_ms/1000.0;
    float tps = ((loop))/((float)(stop - start)/1000000.0);
    //tps = (g_benchmark.api_mode == 0) ? tps*MAX_JOBS : tps;
    //float mbps = (g_benchmark.api_mode == 1) ? (bits/time_s)*MAX_JOBS : bits/time_s;
    int pack_failed_cnt = atomic_read(&pack_failed);
    // printf("%-15s : %f(s), %f(Mb), %f(tps), %6.2f(Mbps), %s\n", 
    //     algo_info[algo], time_s, bits, tps, mbps, pack_failed_cnt ? "failed" : "Success");
     printf("%-15s : %f(s), %f(tps), %d(loop), %s\n", 
        algo_info[0], time_s, tps, loop, pack_failed_cnt ? "failed" : "Success");
    fflush(stdout);
}
static num = 0;

void Task_test(void *hdev)
{   
    uint64_t i, *loop_count;
    uint64_t *loop_status = shm_get_global_loop_status();
    uint64_t *param = (uint64_t *)hdev;
    for (i = 0; (i < g_benchmark.loop) && (*loop_status); ++i)
    {
      //  printf("param:%d\r\n", *param);
      //TODO
    }
    /* record the loop count for performance statistics */
    loop_count_lock();
    thread_global_loop_count += i;
    loop_count = shm_get_global_loop_count();
    shm_set_global_loop_count((*loop_count)+i);
    loop_count_unlock();
}

void *TestFun(void* param)
{   
    Task_test(param);

    atomic_inc(&thread_cnt);

    return NULL;
}

void handle_sigchld2(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


void Stop(int signo) 
{
    loop_lock_release();
    shm_loop_release();
    _exit(0);
}


int test(int argc, char const *argv[], int rand_test)
{
    int ret = 0, i;
    int *efd = NULL;
    pthread_t ptid;

    g_benchmark.process_num = atoi(argv[1]);
    g_benchmark.thread_num = atoi(argv[2]);

    /* Judgement the argv[6] input is <loop> or <mtime> by UOM "ms" */
    if(argc >= 6 && atoi(argv[5]) == 100)
    {   
        g_benchmark.test_time = 0xFFFF;
        g_benchmark.loop = MAX_LOOP;
        //sscanf("65535s", "%ds", &(g_benchmark.test_time));
        goto go;
    } 
    if(argv[3])
    {
        if(strstr(argv[3], "s"))
        {   
            g_benchmark.loop = MAX_LOOP;
            sscanf(argv[3], "%ds", &(g_benchmark.test_time));
        }else{
            g_benchmark.loop = atoi(argv[3]);
            g_benchmark.test_time = 0;
        }
    }

go:
    efd = malloc(g_benchmark.process_num * sizeof(int));
    for(int i = 0; i < g_benchmark.process_num; i++)
    {
        efd[i] = eventfd(0, 0);
        if(efd[i] == -1)
        {
            printf("eventfd creat error. efd[%d] = %d\n", efd[i], i);
            fflush(stdout);
            goto free_efd;
        }
    }

    signal(SIGCHLD, handle_sigchld2);
    signal(SIGINT, Stop); 
    signal(SIGTERM, Stop); 

    /* initial the variables through shared memory for inter-progress-communicate */
    assert(loop_lock_init() == 0);
    shm_loop_init();
    shm_loop_attach();
    shm_set_global_loop_status(1);
    shm_set_global_loop_count(0);

    uint64_t e_val = 0;
    atomic_set(&pack_cnt , 0);
    atomic_set(&pack_failed , 0);
    atomic_set(&thread_cnt , 0);

    for(int fnum = 0; fnum < g_benchmark.process_num; fnum++)
    {   
        int testparam = 10;
        pid_t fpid = fork();
        if(fpid == 0)
        {
            uint64_t u = 1;
            pthread_t tid;

            for (int tnum = 0; tnum < g_benchmark.thread_num; ++tnum)
            {
                int ret = pthread_create(&tid, NULL, TestFun, &testparam);
                if(ret != 0)
                {
                    printf("creat  pthread error. ret = %d\n", ret);
                    fflush(stdout);
                    shm_loop_detach();
                    exit(0);
                }
            }

            while(1)
            {
                usleep(10);
                if(atomic_read(&thread_cnt) == 1 * g_benchmark.thread_num)
                {
                    if(write(efd[fnum], &u, sizeof(uint64_t)) != sizeof(uint64_t))
                    {
                        printf("write efd[%d] error\n", fnum);
                        fflush(stdout);
                    }else
                        printf("write secceed\r\n");

                    shm_loop_detach();
                    exit(0);
                }
            }
        }else if(fpid < 0)
        {
            printf("fork error. fpid = %d\n", fpid);
            fflush(stdout);
        }
    }

    fflush(stdout);
    uint64_t start = get_current_time();

    if(g_benchmark.test_time)
    {
        /* set the alarm to stop the test loop */
        create_alarm_clock_thread(g_benchmark.test_time);
    }

    for(int i = 0; i < g_benchmark.process_num; i++)
    {
        read(efd[i], &e_val, sizeof(uint64_t));
        if(e_val != 1)
        {
            printf("read efd[%d] error\n", i);
            fflush(stdout);
            goto release_shm;
        }
    }
    uint64_t stop = get_current_time();

    uint64_t *total_loop = shm_get_global_loop_count();
    print_performance_info(g_benchmark.algo, start, stop, g_benchmark.size, *total_loop);

release_shm:
    loop_lock_release();
    shm_loop_detach();
    shm_loop_release();

    if(g_benchmark.test_time)
    {
        delete_alarm_clock_thread();
    }

free_efd:
    for(int i = 0; i < g_benchmark.process_num; i++)
    {
        close(efd[i]);
    }

    free(efd);

    return 0;
}

int main(int argc, char const *argv[])
{
    if (argc <= 2)
    {
        printf("%s\n", algo_info[3]);
        return 0;
    }
    memset(&g_benchmark, 0, sizeof(benchmark_t));

    test(argc, argv, 0);
    
    return 0;
}
