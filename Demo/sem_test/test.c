#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <stdlib.h>

void P(int sid)
{
    struct sembuf sem_p;
    sem_p.sem_num = 0;
    sem_p.sem_op = -1;
    sem_p.sem_flg = SEM_UNDO;

    if (semop(sid, &sem_p, 1) == -1) {
        perror("p op failed");
        exit(1);
    }
}

void V(int sid)
{
    struct sembuf sem_p;
    sem_p.sem_num = 0;
    sem_p.sem_op = 1;
    //sem_p.sem_flg = SEM_UNDO;
    sem_p.sem_flg = SEM_UNDO;

    if (semop(sid, &sem_p, 1) == -1) {
        perror("v op failed");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    pid_t pid;
    int fd;
    key_t key;
    int sid;

    if ((fd = open("semset", O_RDWR | O_CREAT, 0666)) == -1) {
        perror("open");
        exit(-1);
    }

    if ((key = ftok("semset", 'a')) == -1) {
        perror("ftok");
        return -1;
    }

    if ((sid = semget(key, 1, IPC_CREAT | 0666)) == -1) {
        perror("createSemset");
        exit(-1);
    }

    if (-1 == semctl(sid, 0, SETVAL, 1)) {
        perror("SETVAL");
        exit(1);
    }

    if ((pid = fork()) == -1) {
        perror("fork");
        exit(-1);
    } else if (0 == pid) {
        while (1) {
            P(sid);
            printf("child writing\n");
            sleep(1);
            printf("child finish post\n");

            V(sid);
        }
    } else {
        sleep(1);
        while (1) {
            P(sid);
            printf("parent writing\n");

            sleep(1);
            printf("parent writing finish post\n");

            V(sid);
        }
    }

    return 0;
}