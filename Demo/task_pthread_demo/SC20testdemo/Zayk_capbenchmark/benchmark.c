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

#include "data.h"
#include "atomic.h"
#include "misc.h"
#include "capabilities.h"

#include "SC20api.h"
#define BENCHMARK_MAX_HAND_CNT 128

#define CAP_DEV_S10         0x10
#define CAP_DEV_S20         0x20

#if 1
#define DEBUG_UNSUP_INFO(...)
#else
#define DEBUG_UNSUP_INFO printf
#endif

// extern void debug_printf(const char *fmt, ...);
#define PRINT_HEX_PRINTF(...) printf(__VA_ARGS__)

// [send]
// 0000: 7E 25 02 00 00 7D 02 02  00 00 2B 48 54 00 47 56  |~%...}.. ..+HT.GV|
// 0010: 35 55 5C 00 00 00 00 00  00 00 00 00 00 00 00 00  |5U\..... ........|
// 0020: 00 00 00 00 00 00 00 00  00 20 01 14 16 31 00 E1  |........ . ...1..|
// 0030: 0D 02 00 0F 0A 00 F0 58  53 15 0C DA BD 05 61 7D  |.......X S.....a}|
// 0040: 02 E8 5A 7F                                       |..Z.             |
// DEC:00068 HEX:0044
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



#if 0
#define BENCHMARK_DEBUG             printf
#else
#define BENCHMARK_DEBUG(...)
#endif

#define MAX_JOBS    128
#define MAX_LOOP    0xFFFFFFFF

/* capabilities related macros */
#define CAP_ALGO_BITMASK        1
#define CAP_KEK_BITMASK         2
#define CAP_KEY_INDEX_BITMASK   4
#define CAP_GCM_UPDATE_BITMASK  8
#define CAP_GCM_AAD_BITMASK     16

static atomic_t pack_cnt;
static atomic_t pack_failed;
static atomic_t thread_cnt;

static uint64_t thread_global_loop_count = 0;

/***************************************/

int symmMode = 0x00000401;

unsigned int uiInDataLen = 0, uiOutDataLen = 0;
unsigned char ucaInData[4096]  = { 0 };
unsigned char ucaSymmKey[32] = { 0 };
unsigned char ucaOutData[4096] = { 0 };
unsigned char ucaOutData1[4096] = { 0 };

unsigned char ucIv[16];
int symmKeyLen;
/***************************************/

#define MAX_DEV_NUM 66
char *cap_dev_name[MAX_DEV_NUM] = 
{
    "rsp_dev_01", "rsp_dev_02", "rsp_dev_03", "rsp_dev_04", "rsp_dev_05", "rsp_dev_06", "rsp_dev_07", "rsp_dev_08",
    "rsp_dev_09", "rsp_dev_10", "rsp_dev_11", "rsp_dev_12", "rsp_dev_13", "rsp_dev_14", "rsp_dev_15", "rsp_dev_16",
    "rsp_dev_17", "rsp_dev_18", "rsp_dev_19", "rsp_dev_20", "rsp_dev_21", "rsp_dev_22", "rsp_dev_23", "rsp_dev_24",
    "rsp_dev_25", "rsp_dev_26", "rsp_dev_27", "rsp_dev_28", "rsp_dev_29", "rsp_dev_30", "rsp_dev_31", "rsp_dev_32",
    "rsp_dev_33", "rsp_dev_34", "rsp_dev_35", "rsp_dev_36", "rsp_dev_37", "rsp_dev_38", "rsp_dev_39", "rsp_dev_40", 
    "rsp_dev_41", "rsp_dev_42", "rsp_dev_43", "rsp_dev_44", "rsp_dev_45", "rsp_dev_46", "rsp_dev_47", "rsp_dev_48", 
    "rsp_dev_49", "rsp_dev_50", "rsp_dev_51", "rsp_dev_52", "rsp_dev_53", "rsp_dev_54", "rsp_dev_55", "rsp_dev_56", 
    "rsp_dev_57", "rsp_dev_58", "rsp_dev_59", "rsp_dev_60", "rsp_dev_61", "rsp_dev_62", "rsp_dev_63", "rsp_dev_64",
    "rsp_dev_65", "rsp_dev_66" 
};

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

typedef enum benchmark_algo_
{
    /* hash/md */
    BENCHMARK_HASH_SM3=0,
    BENCHMARK_HASH_SHA1,
    BENCHMARK_HASH_SHA256,
    BENCHMARK_HASH_SHA384,
    BENCHMARK_HASH_SHA512,

    /* prf */
    BENCHMARK_PRF_SM3,
    BENCHMARK_PRF_SHA256,
    BENCHMARK_PRF_SHA384,

    /* cipher: aes-128*/
    BENCHMARK_AES_128_ECB,
    BENCHMARK_AES_128_CBC,
    BENCHMARK_AES_128_CTR,
    BENCHMARK_AES_128_GCM,

    /* cipher: aes-256 */
    BENCHMARK_AES_256_ECB,
    BENCHMARK_AES_256_CBC,
    BENCHMARK_AES_256_CTR,
    BENCHMARK_AES_256_GCM,

    /* cipher: sm1 */
    BENCHMARK_SM1_ECB,
    BENCHMARK_SM1_CBC,
    BENCHMARK_SM1_CTR,

    /* cipher: sm4 */
    BENCHMARK_SM4_ECB,
    BENCHMARK_SM4_CBC,
    BENCHMARK_SM4_CTR,
    BENCHMARK_SM4_GCM,

    /* DES      */
    BENCHMARK_DES_ECB,
    BENCHMARK_DES_CBC,

    BENCHMARK_3DES_ECB,
    BENCHMARK_3DES_CBC,

    /* ECC: sm2 */
    BENCHMARK_SM2_KG,
    BENCHMARK_SM2_KP,
    BENCHMARK_SM2_GEN,
    BENCHMARK_SM2_ENC,
    BENCHMARK_SM2_DEC,
    BENCHMARK_SM2_SIGN,
    BENCHMARK_SM2_VERIFY,

    /* RSA-1024 */
    BENCHMARK_RSA_1024_GEN,
    BENCHMARK_RSA_1024_ENC,
    BENCHMARK_RSA_1024_DEC,
    BENCHMARK_RSA_1024_DEC_CRT,
    BENCHMARK_RSA_1024_SIGN,
    BENCHMARK_RSA_1024_SIGN_CRT,
    BENCHMARK_RSA_1024_VERIFY,

    /* RSA-2048 */
    BENCHMARK_RSA_2048_GEN,
    BENCHMARK_RSA_2048_ENC,
    BENCHMARK_RSA_2048_DEC,
    BENCHMARK_RSA_2048_DEC_CRT,
    BENCHMARK_RSA_2048_SIGN,
    BENCHMARK_RSA_2048_SIGN_CRT,
    BENCHMARK_RSA_2048_VERIFY,

    /* ECC-curve: 25519 */
    BENCHMARK_25519_KG,
    BENCHMARK_25519_KP,

    /* ECC-curve: 256r1 */
    BENCHMARK_256R1_KG,
    BENCHMARK_256R1_KP,
    BENCHMARK_256R1_SIGN,
    BENCHMARK_256R1_VERIFY,

    /* rand */
    BENCHMARK_RAND,
    BENCHMARK_MAX,
} benchmark_algo_e;

/*
 * algo_info is to show the all supported algo as list.
 * algo_info is mapped with bechmark_algo_e.
 */
static char *algo_info[] =
{
    "HASH-SM3","HASH-SHA1", "HASH-SHA256", "HASH-SHA384", "HASH-SHA512",
    "PRF-SM3" , "PRF-SHA256", "PRF-SHA384", 

    "AES-128-ECB", "AES-128-CBC", "AES-128-CTR", "AES-128-GCM",
    "AES-256-ECB", "AES-256-CBC", "AES-256-CTR", "AES-256-GCM",
    "SM1-ECB", "SM1-CBC", "SM1-CTR", "SM4-ECB", "SM4-CBC", "SM4-CTR", "SM4-GCM",

    "DES_ECB", "DES_CBC", "3DES_ECB", "3DES_CBC",
    "SM2-KG", "SM2-KP",  "SM2-GEN", "SM2-ENC", "SM2-DEC","SM2-SIGN", "SM2-VERIFY",

    "RSA-1024-GEN", "RSA-1024-ENC", "RSA-1024-DEC", "RSA-1024-DEC_CRT","RSA-1024-SIGN", "RSA-1024-SIGN_CRT", "RSA-1024-VERIFY", 
    "RSA-2048-GEN", "RSA-2048-ENC", "RSA-2048-DEC", "RSA-2048-DEC_CRT", "RSA-2048-SIGN", "RSA-2048-SIGN_CRT", "RSA-2048-VERIFY", 

    "25519-KG", "25519-KP", "256R1-KG", "256R1-KP", "256R1-SIGN", "256R1-VERIFY", 
    "RAND", 
};
 
//测试数据结构体 start
void *hdev_t;
void *phsess_t;

void *hashhdl_test[1024];

ECCrefPublicKey sm2Pubkey;
ECCrefPrivateKey sm2Prikey;
ECCSignature sm2SignData;
ECCCipher sm2CipherData;
RSArefPublicKey rsaPubkey;
RSArefPrivateKey rsaPrikey;
RSArefPublicKey rsaPubkey_2048;
RSArefPrivateKey rsaPrikey_2048;
//测试数据结构体 end

static uint64_t get_current_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec*1000*1000 + tv.tv_usec);
}

static void print_performance_info(uint8_t algo, uint64_t start, uint64_t stop, uint64_t size, uint64_t loop)
{
    float bits = (float)((size*loop*8)/1024.0/1024.0);
    float time_ms = (float)(stop - start)/1000.0;
    float time_s = time_ms/1000.0;
    float tps = ((loop))/((float)(stop - start)/1000000.0);
    tps = (g_benchmark.api_mode == 0) ? tps*MAX_JOBS : tps;
    float mbps = (g_benchmark.api_mode == 1) ? (bits/time_s)*MAX_JOBS : bits/time_s;
    int pack_failed_cnt = atomic_read(&pack_failed);
    printf("%-15s : %f(s), %f(Mb), %f(tps), %6.2f(Mbps), %s\n", 
        algo_info[algo], time_s, bits, tps, mbps, pack_failed_cnt ? "failed" : "Success");
    fflush(stdout);
}


void *hdev[BENCHMARK_MAX_HAND_CNT];
void *phsess[1024];
// void cap_benchmark_s20_test(void *sess)
void cap_benchmark_s20_test(int num)

{   
    void *sess = phsess[num];
    uint64_t i, *loop_count;
    uint64_t *loop_status = shm_get_global_loop_status();

    for (i = 0; (i < g_benchmark.loop) && (*loop_status); ++i)
    {
        switch(g_benchmark.algo)
        {   
//hash [0 1 2 4]
            case BENCHMARK_HASH_SM3:
                HASH_SM3(sess, num);
                break;

            case BENCHMARK_HASH_SHA1:
                HASH_SHA1(sess, num);
                break;

            case BENCHMARK_HASH_SHA256:
                HASH_SHA256(sess, num);
                break;

            case BENCHMARK_HASH_SHA512:
                HASH_SHA512(sess, num);
                break;
//sm2 29-33
            case BENCHMARK_SM2_GEN:
                cipher_SM2_GEN(sess, 0);//sess句柄 密钥索引
                break;

            case BENCHMARK_SM2_ENC:
                cipher_SM2_ENCRYPT(sess, 0);
                break;

            case BENCHMARK_SM2_DEC:
                cipher_SM2_DECRYPT(sess, 0);
                break;

            case BENCHMARK_SM2_SIGN:
                cipher_SM2_SIGN(sess, 0);//sess句柄 密钥索引
                break;

            case BENCHMARK_SM2_VERIFY:
                cipher_SM2_VERITY(sess, 0);
                break;
//RSA  [34 38 40 41 45 47]
            case BENCHMARK_RSA_1024_GEN:
                cipher_RSA_1024_GEN(sess, 0);
                break;

            case BENCHMARK_RSA_1024_SIGN:
                cipher_RSA_1024_SIGN(sess, 0);
                break;

            case BENCHMARK_RSA_1024_VERIFY:
                cipher_RSA_1024_VERITY(sess, 0);
                break;

            case BENCHMARK_RSA_2048_GEN:
                cipher_RSA_2048_GEN(sess, 0);
                break;

            case BENCHMARK_RSA_2048_SIGN:
                cipher_RSA_2048_SIGN(sess, 0);
                break;

            case BENCHMARK_RSA_2048_VERIFY:
                cipher_RSA_2048_VERITY(sess, 0);
                break;
//随机数 54
            case BENCHMARK_RAND:
                RANG(sess);
                break;
//对称算法 [8,9,12,13,16,17,19,20,23,24,25,26]
            case BENCHMARK_SM1_ECB:
                cipher_SM1_ECB(sess, 0, 0, 16, 0);
                break;

            case BENCHMARK_SM1_CBC:
                cipher_SM1_CBC(sess, 0, 0, 16, 16);
                break;

            case BENCHMARK_SM4_ECB:
                cipher_SM4_ECB(sess, 0, 0, 16, 0);
                break;

            case BENCHMARK_SM4_CBC:
                cipher_SM4_CBC(sess, 0, 0, 16, 16);
                break;

            case BENCHMARK_AES_128_ECB:
                cipher_AES128_ECB(sess, 0, 0, 16, 0);
                break;

            case BENCHMARK_AES_128_CBC:
                cipher_AES128_CBC(sess, 0, 0, 16, 16);
                break;

            case BENCHMARK_AES_256_ECB:
                cipher_AES256_ECB(sess, 0, 0, 32, 0);
                break;

            case BENCHMARK_AES_256_CBC:
                cipher_AES256_CBC(sess, 0, 0, 32, 16);
                break;

            case BENCHMARK_DES_ECB:
                cipher_DES_ECB(sess, 0, 0, 8, 0);
                break;

            case BENCHMARK_DES_CBC:
                cipher_DES_CBC(sess, 0, 0, 8, 16);
                break;
                
            case BENCHMARK_3DES_ECB:
                cipher_3DES_ECB(sess, 0, 0, 24, 0);
                break;
                
            case BENCHMARK_3DES_CBC:
                cipher_3DES_CBC(sess, 0, 0, 24, 16);
                break;

            default:
                DEBUG_UNSUP_INFO("ERROR: This algo isn't supported now.\n");
                return;
        }
    }

    /* record the loop count for performance statistics */
    loop_count_lock();
    thread_global_loop_count += i;
    loop_count = shm_get_global_loop_count();
    shm_set_global_loop_count((*loop_count)+i);
    loop_count_unlock();
}

void *cap_benchmark(void* param)
{
    // void *sess = *((long *)param);
    int num = (int)param;
    cap_benchmark_s20_test(num);
   // cap_benchmark_s20_test(param);

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
    pthread_t ptid;

    if(argc < 9)
    {
        printf("\n$benchmark <algo> <api mode> <pcnt> <tcnt> <bsize> <loop/stime> <key mode> <dev>\n");
        for (int i = 0; i < BENCHMARK_MAX; ++i)
        {
            printf("algo %d : %s\n", i, algo_info[i]);
        }

        fflush(stdout);
        return 0;
    }

    for (i = 0; i < 4096; i++) {
        ucaInData[i] = i;
    }
    memset(&ucaSymmKey, 0x11, 32);
    g_benchmark.algo = atoi(argv[1]) % BENCHMARK_MAX;
    g_benchmark.api_mode = atoi(argv[2]);
    g_benchmark.process_num = atoi(argv[3]);
    g_benchmark.thread_num = atoi(argv[4]);
    g_benchmark.size = atoi(argv[5]);
    g_benchmark.key_mode = atoi(argv[7]);
    g_benchmark.dev = atoi(argv[8]);

    /* Judgement the argv[6] input is <loop> or <mtime> by UOM "ms" */
    if (argv[6])
    {
        if(strstr(argv[6], "s"))
        {
            g_benchmark.loop = MAX_LOOP;
            sscanf(argv[6], "%ds", &(g_benchmark.test_time));
        }else{
            g_benchmark.loop = atoi(argv[6]);
            g_benchmark.test_time = 0;
        }
    }

    if(argc >= 10)
       g_benchmark.check_perf = atoi(argv[9]);

    if(argc >= 11)
       g_benchmark.check_rst = atoi(argv[10]);

    char cap[4096] = {0};
    
    g_benchmark.dev_type = CAP_DEV_S20;

//数据准备start
    Zayk_OpenDevice(&hdev_t);
    Zayk_OpenSession(hdev_t, &phsess_t);

    cipher_SM2_GEN(phsess_t, 0);//SM2加解密签名使用
    cipher_SM2_SIGN(phsess_t, 0);//sm2单独验签测试使用
    cipher_SM2_ENCRYPT(phsess_t, 0);//sm2单独解密使用
    cipher_RSA_1024_GEN(phsess_t, 0);//rsa签名验签使用
    cipher_RSA_2048_GEN(phsess_t, 0);

    Zayk_CloseSession(phsess_t);
    Zayk_CloseDevice(hdev_t);
//数据准备end

    int *efd = malloc(g_benchmark.process_num * sizeof(int));
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

    printf("[p/t]-[%d-%d]\r\n",g_benchmark.thread_num,g_benchmark.process_num);
    for(int fnum = 0; fnum < g_benchmark.process_num; fnum++)
    {
        pid_t fpid = fork();
        if(fpid == 0)
        {
            uint64_t u = 1;
            pthread_t tid;
            // void *hdev[BENCHMARK_MAX_HAND_CNT];
            // void *phsess[1024];
#if 0
            ret = Zayk_OpenDevice(&hdev[0]);//单设备句柄多session模式
            if (ret != 0) {
                printf("Zayk_OpenDevice fail\r\n");
                fflush(stdout);
                shm_loop_detach();
                exit(0);
            }
            for (int i = 0; i < g_benchmark.thread_num; ++i)//BENCHMARK_MAX_HAND_CNT 单进程最大打开设备数限制128
            {   
                ret = Zayk_OpenSession(hdev[0], &phsess[i]);
                if (ret != 0) {
                    printf("open session failed %x\r\n", ret);
                    fflush(stdout);
                    shm_loop_detach();
                    exit(0);
                }
#else
            for (int i = 0; i < BENCHMARK_MAX_HAND_CNT; ++i)//BENCHMARK_MAX_HAND_CNT 单进程最大打开设备数限制128
            {   
                ret = Zayk_OpenDevice(&hdev[i]);//多设备句柄多session模式 最大打开设备数限制128
                if (ret != 0) {
                    printf("Zayk_OpenDevice fail\r\n");
                    fflush(stdout);
                    shm_loop_detach();
                    exit(0);
                }

                ret = Zayk_OpenSession(hdev[i], &phsess[i]);
                if (ret != 0) {
                    printf("open session failed %x\r\n", ret);
                    fflush(stdout);
                    shm_loop_detach();
                    exit(0);
                }
#endif
                // ret = cap_open_device(cap_dev_name[g_benchmark.dev - 1], &hdev[i]);
                // if(ret != CAP_RET_SUCCESS)
                // {
                //     printf("Device Open Filed.\n");
                //     fflush(stdout);
                //     shm_loop_detach();
                //     exit(0);
                // }
                g_benchmark.handle_cnt++;
                //printf("open handle: %d-%d\n", i, g_benchmark.handle_cnt);

                if(g_benchmark.thread_num == (i+1))
                {
                    break;
                }
            }
//在这里进行哈希初始化 start 1 2 4 8
            if(g_benchmark.algo == BENCHMARK_HASH_SM3 || g_benchmark.algo == BENCHMARK_HASH_SHA1 ||
                g_benchmark.algo == BENCHMARK_HASH_SHA256 || g_benchmark.algo == BENCHMARK_HASH_SHA512)
            {   
                int Algo_Mode = 0;
                switch (g_benchmark.algo)
                {
                case BENCHMARK_HASH_SM3:
                    Algo_Mode = 1;
                    break;
                case BENCHMARK_HASH_SHA1:
                    Algo_Mode = 2;
                    break;
                case BENCHMARK_HASH_SHA256:
                    Algo_Mode = 4;
                    break;
                case BENCHMARK_HASH_SHA512:
                    Algo_Mode = 8;
                    break;
                default:
                    break;
                }

                unsigned char ucId[32];
                for (i = 0; i < g_benchmark.thread_num; i++) {
                    ret = Zayk_HashInit(phsess[i], Algo_Mode, 2, ucId, 32, 0, &sm2Pubkey, &hashhdl_test[i]);
                    if (ret != 0) {
                            printf("ret:%x\r\n", ret);
                            return 0;
                        }
                }
            }
//在这里进行哈希初始化 end

            for (int tnum = 0; tnum < g_benchmark.thread_num; ++tnum)
            {   
                //int ret = pthread_create(&tid, NULL, cap_benchmark, (void *)(phsess[tnum%g_benchmark.handle_cnt]));
                int ret = pthread_create(&tid, NULL, cap_benchmark, tnum%g_benchmark.handle_cnt);
                //int ret = pthread_create(&tid, NULL, cap_benchmark, NULL);//单设备句柄使用
                if(ret != 0)
                {
                    printf("creat cap_benchmark pthread error. ret = %d\n", ret);
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
                        //printf("write secceed\r\n");

                    if(g_benchmark.algo == BENCHMARK_HASH_SM3 || g_benchmark.algo == BENCHMARK_HASH_SHA1 ||
                            g_benchmark.algo == BENCHMARK_HASH_SHA256 || g_benchmark.algo == BENCHMARK_HASH_SHA512)
                    {   //在这里进行哈希结束
                        for (int i = 0; i < g_benchmark.handle_cnt; ++i)
                            Zayk_HashFinal(phsess[i], hashhdl_test[i], ucaOutData, &uiOutDataLen);
                    }

                    for (int i = 0; i < g_benchmark.handle_cnt; ++i)
                    {
                        Zayk_CloseSession(phsess[i]);
                        Zayk_CloseDevice(hdev[i]);
                    }

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
            //printf("read secceed\r\n");

    }
    uint64_t stop = get_current_time();
//哈希 测试
    //unsigned char ucId[32];
    //Zayk_HashInit(phsess_t, 1, 2, ucId, 32, 0, &sm2Pubkey, &hashhdl_SM3);
    //Zayk_HashInit(phsess_t, 2, 2, ucId, 32, 0, &sm2Pubkey, &hashhdl_SHA1);
    // Zayk_HashInit(phsess_t, 4, 2, ucId, 32, 0, &sm2Pubkey, &hashhdl_SHA256);
    // Zayk_HashInit(phsess_t, 8, 2, ucId, 32, 0, &sm2Pubkey, &hashhdl_SHA512);

    //Zayk_HashFinal(phsess_t, hashhdl_SM3, ucaOutData, &uiOutDataLen);

    //Zayk_HashFinal(phsess_t, hashhdl_SHA1, ucaOutData, &uiOutDataLen);
    // Zayk_HashFinal(phsess_t, hashhdl_SHA256, ucaOutData, &uiOutDataLen);
    // Zayk_HashFinal(phsess_t, hashhdl_SHA512, ucaOutData, &uiOutDataLen);
   // free(hashhdl_SM3);

//
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


uint8_t iv[16] =
{ 
    0x21, 0x22, 0x8F, 0x7D, 0x26, 0x04, 0xB3, 0x93, 0xB0, 0xD0, 0x0D, 0x3F, 0x01, 0x02
};

//HASH ！！！！！！！！！！！！！！代码都都相同 可以合并 方便后续添加 先保留接口不进行合并代码
int HASH_SM3(void *sess, int num)
{   
    int ret = 0;
    ret = Zayk_HashUpdate(sess, hashhdl_test[num], ucaInData, g_benchmark.size);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return 0;
}

int HASH_SHA1(void *sess, int num)
{
    int ret = 0;
    ret = Zayk_HashUpdate(sess, hashhdl_test[num], ucaInData, g_benchmark.size);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return 0;
}

int HASH_SHA256(void *sess, int num)
{   
    int ret = 0;
    ret = Zayk_HashUpdate(sess, hashhdl_test[num], ucaInData, g_benchmark.size);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return 0;
}

int HASH_SHA512(void *sess, int num)
{   
    int ret = 0;
    ret = Zayk_HashUpdate(sess, hashhdl_test[num], ucaInData, g_benchmark.size);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return 0;
}

//SM2

int cipher_SM2_GEN(void *sess, int index)
{
    int ret = 0;
    ret = ZaykCSM_GenerateECCKeyPair(sess, index, &sm2Pubkey, &sm2Prikey);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}
int cipher_SM2_SIGN(void *sess, int index)
{
    int ret = 0;
    ret = Zayk_Sign_ECC(sess, index, &sm2Prikey, ucaInData, 32, &sm2SignData);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}
int cipher_SM2_VERITY(void *sess, int index)
{
    int ret = 0;
    ret = Zayk_Verify_ECC(sess, 0, &sm2Pubkey, &sm2SignData, ucaInData, 32);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}

int cipher_SM2_ENCRYPT(void *sess, int index)
{   
    int ret = 0;
    ret = Zayk_Encrypt_ECC(sess, 0, &sm2Pubkey, ucaInData, 32, &sm2CipherData);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}
int cipher_SM2_DECRYPT(void *sess, int index)
{   
    int ret = 0;
    ret = Zayk_Decrypt_ECC(sess, 0, &sm2Prikey, &sm2CipherData, ucaOutData, &uiOutDataLen);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
        return 0;
    }
    return ret;
}

//RSA

int cipher_RSA_1024_GEN(void *sess, int index)
{   
    int ret = 0;
    ret = ZaykCSM_GenerateRSAKeyPair(sess, index, 1024, 65537, &rsaPubkey, &rsaPrikey);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}
int cipher_RSA_1024_SIGN(void *sess, int index)
{   
    int ret = 0;
    ret = Zayk_PrivateKeyOperation_RSA(sess, index, &rsaPrikey, ucaInData, 128, ucaOutData, &uiOutDataLen);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}
int cipher_RSA_1024_VERITY(void *sess, int index)
{   
    int ret = 0;
    ret = Zayk_PublicKeyOperation_RSA(sess, index, &rsaPubkey, ucaInData + 512, 128, ucaOutData, &uiOutDataLen);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}


int cipher_RSA_2048_GEN(void *sess, int index)
{   
    int ret = 0;
     ret = ZaykCSM_GenerateRSAKeyPair(sess, index, 2048, 65537, &rsaPubkey_2048, &rsaPrikey_2048);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}
int cipher_RSA_2048_SIGN(void *sess, int index)
{   
    int ret = 0;
    ret = Zayk_PrivateKeyOperation_RSA(sess, index, &rsaPrikey_2048, ucaInData, 256, ucaOutData, &uiOutDataLen);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}
int cipher_RSA_2048_VERITY(void *sess, int index)
{   
    int ret = 0;
    ret = Zayk_PublicKeyOperation_RSA(sess, 0, &rsaPubkey_2048, ucaInData + 512, 256, ucaOutData, &uiOutDataLen);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
    }
    return ret;
}
//RANG
int RANG(void *sess)
{   
    int ret = 0;
    ret = Zayk_GenerateRandom(sess, g_benchmark.size, ucaOutData);
    if (ret != 0) {
        printf("ret:%x\r\n", ret);
        return 0;
    }
}

//SM1
int cipher_SM1_ECB(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_SM1_ECB, 0, ucaSymmKey, keylen, NULL, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}

int cipher_SM1_CBC(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_SM1_CBC, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
//SM4
int cipher_SM4_ECB(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_SM4_ECB, 0, ucaSymmKey, keylen, NULL, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}

int cipher_SM4_CBC(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_SM4_ECB, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
//AES
int cipher_AES128_ECB(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_AES128_ECB, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
int cipher_AES128_CBC(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_AES128_CBC, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
int cipher_AES256_ECB(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_AES256_ECB, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
int cipher_AES256_CBC(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_AES256_CBC, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
//des
int cipher_DES_ECB(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_DES_ECB, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
int cipher_DES_CBC(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_DES_CBC, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
int cipher_3DES_ECB(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_3DES_ECB, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}
int cipher_3DES_CBC(void *sess, int algo, int mode, int keylen, int ivlen)
{
    int ret = 0;
    ret = Zayk_Encrypt(sess, SGD_3DES_CBC, 0, ucaSymmKey, keylen, iv, ivlen, ucaInData, g_benchmark.size, ucaOutData, &uiOutDataLen);
    if (ret != 0)
    {
        printf("Zayk API faild [%s:%d] %04X\n\n", __FUNCTION__, __LINE__, ret);
        exit(1);
    }
    return ret;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("\n$benchmark <algo> <api mode> <pcnt> <tcnt> <bsize> <loop/stime> <key mode> <dev>\n");
        for (int i = 0; i < BENCHMARK_MAX; ++i)
        {
            printf("algo %d : %s\n", i, algo_info[i]);
        }
        return 0;
    }

    memset(&g_benchmark, 0, sizeof(benchmark_t));

  
    test(argc, argv, 0);
    

    return 0;
}
