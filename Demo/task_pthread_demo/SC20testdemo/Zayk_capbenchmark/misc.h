#ifndef _MISC_H_
#define _MISC_H_

int loop_lock_init(void);

int loop_lock_release(void);

int loop_count_lock(void);

int loop_count_unlock(void);

/* global varabiles shared among progresses */
int shm_loop_init(void);

int shm_loop_attach(void);

int shm_loop_detach(void);

int shm_loop_release(void);

uint64_t *shm_get_global_loop_count(void);

int shm_set_global_loop_count(uint64_t loop_count);

uint64_t *shm_get_global_loop_status(void);

int shm_set_global_loop_status(uint64_t loop_status);

/* alarm functions */
void rsp_alarm_clock_handler(union sigval v);

void create_alarm_clock_thread(int test_time);

void delete_alarm_clock_thread(void);
#endif
