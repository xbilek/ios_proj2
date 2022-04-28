#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h> /// dont know if needed
#include <sys/shm.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

typedef struct {
    int NO;     //pocet kysliku
    int NH;     //pocet vodiku
    int TI;     //max cas v ms na cekani atomu
    int TB;     //max cas v ms na cekani vytvoreni molekuly
} arguments;


sem_t *mutex = NULL;
sem_t *barrier = NULL;
sem_t *oxygen_queue = NULL;
sem_t *hydrogen_queue = NULL;
sem_t *barrier_mutex = NULL;
sem_t *turnstile = NULL;
sem_t *turnstile_2 = NULL;
sem_t *out = NULL;
sem_t *creating = NULL;

static int *action_counter;
static int *oxygen_counter;
static int *hydrogen_counter;
static int *molecule_counter;
static int *barrier_counter;/////////
static int *molecule_check;
static int *creating_counter;
static int *created_counter;

FILE *file;



void clean(){
	sem_unlink("/xbilek25_sem_mutex");
	sem_close(mutex);

	sem_unlink("/xbilek25_sem_barrier");
	sem_close(barrier);

	sem_unlink("/xbilek25_sem_oxygen_queue");
	sem_close(oxygen_queue);

	sem_unlink("/xbilek25_sem_hydrogen_queue");
	sem_close(hydrogen_queue);

	sem_unlink("/xbilek25_sem_barrier_mutex");
	sem_close(barrier_mutex);

	sem_unlink("/xbilek25_sem_turnstile");
	sem_close(turnstile);

	sem_unlink("/xbilek25_sem_turnstile_2");
	sem_close(turnstile_2);

	sem_unlink("/xbilek25_sem_out");
	sem_close(turnstile_2);

	sem_unlink("/xbilek25_sem_creating");
	sem_close(creating);

	munmap(action_counter, sizeof *action_counter);
	munmap(oxygen_counter, sizeof *oxygen_counter);
	munmap(hydrogen_counter, sizeof *hydrogen_counter);
	munmap(molecule_counter, sizeof *molecule_counter);
	munmap(barrier_counter, sizeof *barrier_counter);
	munmap(molecule_check, sizeof *molecule_check);
	munmap(creating_counter, sizeof *creating_counter);
	munmap(created_counter, sizeof *created_counter);
}

void init(){
	action_counter = mmap(NULL, sizeof *action_counter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (action_counter == MAP_FAILED){
		fprintf(stderr, "ERROR, mmap failed");
		clean();
		fclose(file);
		exit(1);
	}

	*action_counter = 1;

	oxygen_counter = mmap(NULL, sizeof *oxygen_counter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (oxygen_counter == MAP_FAILED){
		fprintf(stderr, "ERROR, mmap failed");
		clean();
		fclose(file);
		exit(1);
	}

	*oxygen_counter = 0;

	hydrogen_counter = mmap(NULL, sizeof *hydrogen_counter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (hydrogen_counter == MAP_FAILED){
		fprintf(stderr, "ERROR, mmap failed");
		clean();
		fclose(file);
		exit(1);
	}

	*hydrogen_counter = 0;

	molecule_counter = mmap(NULL, sizeof *molecule_counter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (molecule_counter == MAP_FAILED){
		fprintf(stderr, "ERROR, mmap failed");
		clean();
		fclose(file);
		exit(1);
	}

	*molecule_counter = 1;

	barrier_counter = mmap(NULL, sizeof *barrier_counter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (barrier_counter == MAP_FAILED){
		fprintf(stderr, "ERROR, mmap failed");
		clean();
		fclose(file);
		exit(1);
	}

	*barrier_counter = 0;

	molecule_check = mmap(NULL, sizeof *molecule_check, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (molecule_check == MAP_FAILED){
		fprintf(stderr, "ERROR, mmap failed");
		clean();
		fclose(file);
		exit(1);
	}

	*molecule_check = 0;

	creating_counter = mmap(NULL, sizeof *creating_counter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (molecule_check == MAP_FAILED){
		fprintf(stderr, "ERROR, mmap failed");
		clean();
		fclose(file);
		exit(1);
	}

	*creating_counter = 0;

	created_counter = mmap(NULL, sizeof *created_counter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (oxygen_counter == MAP_FAILED){
		fprintf(stderr, "ERROR, mmap failed");
		clean();
		fclose(file);
		exit(1);
	}

	*created_counter = 0;


	//SEMAPHORES

	mutex = sem_open("/xbilek25_sem_mutex", O_CREAT | O_EXCL, 0666, 1);
	if (mutex == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open mutex failed");
		clean();
		fclose(file);
		exit(1);
	}

	barrier = sem_open("/xbilek25_sem_barrier", O_CREAT | O_EXCL, 0666, 3);
	if (barrier == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open bar failed");
		clean();
		fclose(file);
		exit(1);
	}

	oxygen_queue = sem_open("/xbilek25_sem_oxygen_queue", O_CREAT | O_EXCL, 0666, 0);
	if (oxygen_queue == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open oq failed");
		clean();
		fclose(file);
		exit(1);
	}

	hydrogen_queue = sem_open("/xbilek25_sem_hydrogen_queue", O_CREAT | O_EXCL, 0666, 0);
	if (hydrogen_queue == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open hq failed");
		clean();
		fclose(file);
		exit(1);
	}

	barrier_mutex = sem_open("/xbilek25_sem_barrier_mutex", O_CREAT | O_EXCL, 0666, 1);
	if (barrier_mutex == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open barmutex failed");
		clean();
		fclose(file);
		exit(1);
	}

	turnstile = sem_open("/xbilek25_sem_turnstile", O_CREAT | O_EXCL, 0666, 0);
	if (turnstile == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open turn failed");
		clean();
		fclose(file);
		exit(1);
	}

	turnstile_2 = sem_open("/xbilek25_sem_turnstile_2", O_CREAT | O_EXCL, 0666, 1);
	if (turnstile_2 == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open turn2 failed");
		clean();
		fclose(file);
		exit(1);
	}

	out = sem_open("/xbilek25_sem_out", O_CREAT | O_EXCL, 0666, 1);
	if (turnstile_2 == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open out failed");
		clean();
		fclose(file);
		exit(1);
	}

	creating = sem_open("/xbilek25_sem_creating", O_CREAT | O_EXCL, 0666, 0);
	if (mutex == SEM_FAILED){
		fprintf(stderr, "ERROR, sem_open creating failed");
		clean();
		fclose(file);
		exit(1);
	}

	file = fopen("proj2.out", "w");
	if(file == NULL){
		fprintf(stderr, "ERROR cant open file \n");
		clean();
		fclose(file);
		exit(1);
	}

}

void process_oxygen(int i, int ti, int tb){		// a je cislo procesu

	sem_wait(mutex);

	sem_wait(out);
	fprintf(file, "%d: O %d: started\n", *action_counter, i);
	fflush(file);
	++*(action_counter);
	sem_post(out);

	srand(time(NULL));
	int random = rand() % (ti+1);
	usleep(random);

	sem_wait(out);
	fprintf(file, "%d: O %d: going to queue\n", *action_counter, i);
	fflush(file);
	(*action_counter)++;
	sem_post(out);

	(*oxygen_counter)++;
	if (*hydrogen_counter >= 2){
		sem_post(hydrogen_queue);
		sem_post(hydrogen_queue);
		(*hydrogen_counter) -= 2;
		sem_post(oxygen_queue);
		(*oxygen_counter)--;
	}else{
		sem_post(mutex);
	}

	sem_wait(oxygen_queue);
	
	sem_wait(out);
	
	if(*creating_counter == 3){
		(*creating_counter) = 0;
		sem_wait(creating);
		fprintf(file, "cekam na semafor\n");
		fflush(file);
	}
	
	fprintf(file, "%d: O %d: creating molecule %d ________________creating_counter %d \n", *action_counter, i, *molecule_counter, *creating_counter);
	fflush(file);
	(*molecule_check)++;
	if(*molecule_check == 3){
		(*molecule_counter)++;
		(*molecule_check) = 0;
	}
	(*action_counter)++;
	(*creating_counter)++;
	sem_post(out);

	
	//usleep simuluici doby

	random = rand() % (tb+1);
	usleep(random);

	
	///////////bariera
	sem_wait(barrier_mutex);
	(*barrier_counter)++;
	if (*barrier_counter == 3){
		sem_wait(turnstile_2);
		sem_post(turnstile);
	}
	sem_post(barrier_mutex);
	
	sem_wait(turnstile);
	sem_post(turnstile);

	sem_wait(barrier_mutex);
	(*barrier_counter)--;
	if(*barrier_counter == 0){
		sem_wait(turnstile);
		sem_post(turnstile_2);
	}
	sem_post(barrier_mutex);

	sem_wait(turnstile_2);
	sem_post(turnstile_2);

	////////////////
	//asi vypis ze je created
	sem_wait(out);
	fprintf(file, "%d: O %d: molecule %d created _________________created_counter: %d\n", *action_counter, i, *molecule_counter-1, *created_counter);
	fflush(file);
	(*action_counter)++;
	(*created_counter)++;
	sem_post(out);
	if(*created_counter == 3){
		sem_post(creating);
		*created_counter = 0;
		fprintf(file, "oteviram creating semafor\n");
		fflush(file);
	}

	sem_post(mutex);

	exit(0);

}

void process_hydrogen(int i, int ti, int tb){

	sem_wait(mutex);

	sem_wait(out);
	fprintf(file, "%d: H %d: started\n", *action_counter, i);
	fflush(file);
	++*(action_counter);
	sem_post(out);

	srand(time(NULL));
	int random = rand() % (ti+1);
	usleep(random);

	
	sem_wait(out);
	fprintf(file, "%d: H %d: going to queue\n", *action_counter, i);
	fflush(file);
	++*(action_counter);
	sem_post(out);

	(*hydrogen_counter)++;

	if (*hydrogen_counter >= 2 && *oxygen_counter >= 1){
		sem_post(hydrogen_queue);
		sem_post(hydrogen_queue);
		(*hydrogen_counter) -= 2;
		sem_post(oxygen_queue);
		(*oxygen_counter)--;
	}else{
		sem_post(mutex);
	}

	sem_wait(hydrogen_queue);
	///////bond()
	sem_wait(out);
	if(*creating_counter == 3){
		(*creating_counter) = 0;
		sem_wait(creating);
		fprintf(file, "cekam na semafor\n");
		fflush(file);
	}
	
	fprintf(file, "%d: H %d: creating molecule %d ________________ creating_counter: %d \n", *action_counter, i, *molecule_counter, *creating_counter);
	fflush(file);
	(*molecule_check)++;
	if(*molecule_check == 3){
		(*molecule_counter)++;
		(*molecule_check) = 0;
	}
	(*action_counter)++;
	(*creating_counter)++;

	sem_post(out);

	
	//usleep simuluici doby

	random = rand() % (tb+1);
	usleep(random);

	///////////////////

	

	///////////bariera
	sem_wait(barrier_mutex);
	(*barrier_counter)++;
	if (*barrier_counter == 3){
		sem_wait(turnstile_2);
		sem_post(turnstile);
	}
	sem_post(barrier_mutex);
	
	sem_wait(turnstile);
	sem_post(turnstile);


	sem_wait(barrier_mutex);
	(*barrier_counter)--;
	if(*barrier_counter == 0){
		sem_wait(turnstile);
		sem_post(turnstile_2);
	}
	sem_post(barrier_mutex);

	sem_wait(turnstile_2);
	sem_post(turnstile_2);


	////////////////

	//asi vypis ze je created
	sem_wait(out);
	fprintf(file, "%d: H %d: molecule %d created _________created counter: %d\n", *action_counter, i, *molecule_counter-1, *created_counter);
	fflush(file);
	(*action_counter)++;
	(*created_counter)++;
	sem_post(out);
	if (*created_counter == 3){
		sem_post(creating);
		*created_counter = 0;
		fprintf(file, "oteviram creating semafor\n");
		fflush(file);
	}
	
	sem_post(mutex);

	exit(0);
}


arguments parse_arguments(int args, char **argv, arguments a){
    char *ptr;
    if (args != 5){     // ocekavanycho 5 argumentu
        fprintf(stderr, "ERROR wrong amout of arguments");
        exit(1);
    }

    a.NO = strtol(argv[1], &ptr, 10);     
    if(*ptr != '\0' || ptr == argv[1]){
        fprintf(stderr, "ERROR argument is not a number");
        exit(1);
    }             

    a.NH = strtol(argv[2], &ptr, 10);   
    if(*ptr != '\0' || ptr == argv[2]){
        fprintf(stderr, "ERROR argument is not a number");
        exit(1);
    }     

    a.TI = strtol(argv[3], &ptr, 10);     
    if(*ptr != '\0' || ptr == argv[3]){
        fprintf(stderr, "ERROR argument is not a number");
        exit(1);
    }            

    a.TB = strtol(argv[4], &ptr, 10);     
    if(*ptr != '\0' || ptr == argv[4]){
        fprintf(stderr, "ERROR argument is not a number");
        exit(1);
    }         

    if (a.NO < 0){
        fprintf(stderr, "ERROR arguments have to be non-negative");
        exit(1);
    }

    if (a.NH < 0){
        fprintf(stderr, "ERROR arguments have to be non-negative");
        exit(1);
    }

    if ((a.TI < 0) || a.TI > 1000){
        fprintf(stderr, "ERROR argument out of bounds");
        exit(1);
    }

    if ((a.TB < 0) || a.TI > 1000){
        fprintf(stderr, "ERROR argument out of bounds");
        exit(1);;
    }


	return a;
}

int main(int argc, char **argv){


    arguments a = parse_arguments(argc, argv, a);
	clean();
	fprintf(stdout, "jsem tu\n");
	fflush(stdout);
	init();


	for(int i = 1; i <= a.NO; ++i){
		pid_t process_o = fork();
		if (process_o < 0){
			fprintf(stderr, "ERROR, fork failed\n");
			exit(1);
		}
		if(process_o == 0){
			process_oxygen(i, a.TI, a.TB);
			exit(0);
		}
	}

	for(int i = 1; i <= a.NH; ++i){
		pid_t process_h = fork();
		if (process_h < 0){
			fprintf(stderr, "ERROR, fork failed\n");
			exit(1);
		}
		if(process_h == 0){
			process_hydrogen(i, a.TI, a.TB);
			exit(0);
		}

	}
	while(wait(NULL) > 0);
	clean();
	fclose(file);
    return 0;
}