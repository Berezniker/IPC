#include <stdio.h>			// printf()
#include <signal.h>			// signal()
#include <unistd.h>			// fork(), sleep()
#include <stdlib.h>			// srand(), rand()
#include <time.h>			// time()
#include <sys/types.h>
#include <sys/wait.h>		// wait()
#include <sys/ipc.h>		// IPC_PRIVATE, IPC_CREAT, IPC_RMID
#include <sys/sem.h>		// semget(), semctl(), semop()
#include <sys/shm.h>		// shmget(), shmat(), shmdt(), shmctl(), SETVAL, struct sembuf

#define MEAT 50				// количество кусков мяса, которое кладет смотритель в миску

int	*shmNum, dshm, semid;
struct sembuf sem;

/**
 * [tiger: процесс-тигр съедает из миски некоторое количество кусков мяса]
 */
void tiger(void);

/**
 * [keeper: процесс-смотритель добавляет MEAT кусков мяса в миску, когда тигр доест]
 */
void keeper(void);

/**
 * [cleanup: обработчика сигнала SIGINT для завершение процессов тигр и смотритель]
 */
void cleanup(int i) {
	shmdt(shmNum);
	_exit(0);
}

int main()
{
	// настроим обработку сигнала SIGINT, при нажатии <Ctrl+C>
	// для завершения работы процессов тигр и смотритель через cleanup()
	signal(SIGINT, cleanup);
	// выделим разделяющую память "миска с мясом"
	dshm = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0666);
	shmNum = shmat(dshm, NULL, 0);
	// создаем массив семафоров: 0 - для смотрителя, 1 - для тигра
	semid = semget(IPC_PRIVATE, 2, IPC_CREAT|0666);
	sem.sem_flg = 0;
	semctl(semid, 0, SETVAL, 1);
	semctl(semid, 1, SETVAL, 0);
	// запускаем процессы
	if (fork() == 0)
		keeper();
	if(fork() == 0)
		tiger();
	// игнорируем сигнал SIGINT для основного процесса
	signal(SIGINT, SIG_IGN);
	// дожидаемся завершения процессов тигр и смотритель
	wait(NULL);
	wait(NULL);
	// завершаем работу основного процесса, удаляя средства IPC
	shmdt(shmNum);
	shmctl(dshm, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
	return 0;
}

void keeper()
{
	// процесс заблокирован, пока тигр не доест мяса и не поднимет семафор смотрителю
	while(1) {
		// опустить семафор смотрителя
		sem.sem_num = 0;
		sem.sem_op = -1;
		semop(semid, &sem, 1);
		// добавить мяса в миску
		*shmNum = MEAT;
		sleep(1);
		printf("\033[1;32mkeeper\033[0m: \\__%d__/\n", MEAT);
		// поднять семафор тигру
		sem.sem_num = 1;
		sem.sem_op = 1;
		semop(semid, &sem, 1);
	}
}

void tiger()
{
	// процесс заблокирован, пока смотритель не положит мяса и не поднимет семафор тигру
	int eat, res; // eat - сколько тигр съедает, res - остаток в миске

	// устанавливает основу для новой последовательности псевдослучайных целых чисел,
	// возвращаемых функцией rand()
	srand(time(NULL));

	while (1) {
		// опустить семафор тигру
		sem.sem_num = 1;
		sem.sem_op = -1;
		semop(semid, &sem, 1);

		res = *shmNum;
		printf("\033[1;34m        yum--yum\033[0m\n");
		while (res != 0) {
			eat = 1 + rand() % res;
			sleep(1);
			res -= eat;
			printf("\033[1;31mtiger\033[0m : \\__%2d__/\n", res);
		}
		printf("\033[1;34m         zzzZZZ\033[0m\n");

		// поднять семафор смотрителю
		sem.sem_num = 0;
		sem.sem_op = 1;
		semop(semid, &sem, 1);
	}
}