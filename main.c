/*
 * VUT FIT - IOS project 2
 * Author: Patrik Cerbak (xcerba00)
 * Date: 2.5.2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <pthread.h>

// declare semaphores and shared memory
sem_t *mutex, *oxygen_sem, *hydrogen_sem, *mol_created, *printing, *barrier_mutex, *barrier_sem1, *barrier_sem2;
int *no, *nh, *ti, *tb, *oxy, *hyd, *oxygen_id, *hydrogen_id, *molecule_id, *line_num, *barrier_count;
FILE *file;

// a function that initializes semaphores, shared memory etc.
void initialize() {
    srand(time(0)); // seed runtime random generator

    // initialize the memory for semaphores
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    oxygen_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    hydrogen_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    mol_created = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    printing = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    barrier_mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    barrier_sem1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    barrier_sem2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

    // check the mapped memory
    if(mutex == MAP_FAILED || oxygen_sem == MAP_FAILED || hydrogen_sem == MAP_FAILED || mol_created == MAP_FAILED ||
       printing == MAP_FAILED || barrier_mutex == MAP_FAILED || barrier_sem1 == MAP_FAILED || barrier_sem2 == MAP_FAILED) {
        fprintf(stderr, "Error: cannot map the memory!\n");
        exit(1);
    }

    // initialize semaphores and check if it was succesful
    if(sem_init(mutex, 1, 1) == -1 ||
    sem_init(oxygen_sem, 1, 0) == -1 ||
    sem_init(hydrogen_sem, 1, 0) == -1 ||
    sem_init(mol_created, 1, 0) == -1 ||
    sem_init(printing, 1, 1) == -1 ||
    sem_init(barrier_mutex, 1, 1) == -1 ||
    sem_init(barrier_sem1, 1, 0) == -1 ||
    sem_init(barrier_sem2, 1, 1) == -1) {
        fprintf(stderr, "Error: cannot initialize semaphores!\n");
        exit(1);
    }

    // initialize shared memory for variables
    no =mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    nh = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    ti = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    tb = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    oxy = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    hyd = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    oxygen_id = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    hydrogen_id = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    molecule_id = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    line_num = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    barrier_count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

    // check the mapped memory
    if(no == MAP_FAILED || nh == MAP_FAILED || ti == MAP_FAILED || tb == MAP_FAILED ||
       oxy == MAP_FAILED || hyd == MAP_FAILED || oxygen_id == MAP_FAILED || hydrogen_id == MAP_FAILED ||
       molecule_id == MAP_FAILED || line_num == MAP_FAILED || barrier_count == MAP_FAILED) {
        fprintf(stderr, "Error: cannot map the memory!\n");
        exit(1);
    }

    // initial values
    *no = 0, *nh = 0, *ti = 0, *tb = 0;
    *oxy = 0, *hyd = 0;
    *oxygen_id = 1, *hydrogen_id = 1, *molecule_id = 1;
    *line_num = 1;
    *barrier_count = 0;

    // open the output file
    if((file = fopen("proj2.out","w")) == NULL){
        fprintf(stderr, "Error: cannot open proj2.out!\n");
        exit(1);
    }
}

// a function that destroys semaphores and frees the memory
void destroy() {
    // destroy the semaphores and check if it was succesful
    if(sem_destroy(mutex) == -1 ||
    sem_destroy(oxygen_sem) == -1 ||
    sem_destroy(hydrogen_sem) == -1 ||
    sem_destroy(mol_created) == -1 ||
    sem_destroy(printing) == -1 ||
    sem_destroy(barrier_mutex) == -1 ||
    sem_destroy(barrier_sem1) == -1 ||
    sem_destroy(barrier_sem2) == -1) {
        fprintf(stderr, "Error: cannot destroy semaphores!\n");
        exit(1);
    }

    // unmap the semaphore shared memory and check
    if(munmap(mutex, sizeof(sem_t)) == -1 ||
    munmap(oxygen_sem, sizeof(sem_t)) == -1 ||
    munmap(hydrogen_sem, sizeof(sem_t)) == -1 ||
    munmap(mol_created, sizeof(sem_t)) == -1 ||
    munmap(printing, sizeof(sem_t)) == -1 ||
    munmap(barrier_mutex, sizeof(sem_t)) == -1 ||
    munmap(barrier_sem1, sizeof(sem_t)) == -1 ||
    munmap(barrier_sem2, sizeof(sem_t)) == -1) {
        fprintf(stderr, "Error: cannot unmap the memory!\n");
        exit(1);
    }

    // unmap shared memory of the variables
    if(munmap(no, sizeof(int)) == -1 ||
    munmap(nh, sizeof(int)) == -1 ||
    munmap(ti, sizeof(int)) == -1 ||
    munmap(tb, sizeof(int)) == -1 ||
    munmap(oxy, sizeof(int)) == -1 ||
    munmap(hyd, sizeof(int)) == -1 ||
    munmap(oxygen_id, sizeof(int)) == -1 ||
    munmap(hydrogen_id, sizeof(int)) == -1 ||
    munmap(molecule_id, sizeof(int)) == -1 ||
    munmap(line_num, sizeof(int)) == -1 ||
    munmap(barrier_count, sizeof(int)) == -1) {
        fprintf(stderr, "Error: cannot unmap the memory!\n");
        exit(1);
    }

    // close the file
    fclose(file);
}

// parses the arguments
void parse_args(int argc, char *argv[]) {
    // check for the correct number of arguments
    if(argc != 5) {
        fprintf(stderr, "Error: wrong number of arguments!\n");
        fprintf(stderr, "Usage: %s [NO] [NH] [TI] [TB]\n", argv[0]);
        destroy();
        exit(1);
    }

    // no
    *no = atoi(argv[1]);
    if(*no <= 0) {
        fprintf(stderr, "Error: NO must be positive!\n");
        destroy();
        exit(1);
    }
    // nh
    *nh = atoi(argv[2]);
    if(*nh <= 0) {
        fprintf(stderr, "Error: NH must be positive!\n");
        destroy();
        exit(1);
    }
    // ti
    *ti = atoi(argv[3]);
    if(*ti < 0 || *ti > 1000) {
        fprintf(stderr, "Error: TI is out of range! (0 <= TI <= 1000)\n");
        destroy();
        exit(1);
    }
    // tb
    *tb = atoi(argv[4]);
    if(*tb < 0 || *tb > 1000) {
        fprintf(stderr, "Error: TB is out of range! (0 <= TB <= 1000)\n");
        destroy();
        exit(1);
    }
}

// this function takes the same arguments as printf and prints to the output file with the line number
void print_process(const char *fmt, ...) {
    sem_wait(printing); // wait for semaphore
    va_list args;
    va_start(args, fmt);
    fprintf(file, "%d: ", (*line_num)++); // print the line number
    vfprintf(file, fmt, args);
    fflush(file); // flush the output buffer directly to the file
    va_end(args);
    sem_post(printing);
}

/* 
 * the barrier and parts of the oxygen and hydrogen functions were inspired by the book
 * "Allen B. Downey: The Little Book of Semaphores"
 */

// the barrier function - it waits for three processes before releasing them
void barrier() {
    sem_wait(barrier_mutex);
    (*barrier_count)++; // increase the count
    if(*barrier_count == 3) {
        sem_wait(barrier_sem2);
        sem_post(barrier_sem1);
    }
    sem_post(barrier_mutex);

    sem_wait(barrier_sem1);
    sem_post(barrier_sem1);

    sem_wait(barrier_mutex);
    (*barrier_count)--; // decrease the count
    if(*barrier_count == 0) {
        sem_wait(barrier_sem1);
        sem_post(barrier_sem2);
    }
    sem_post(barrier_mutex);

    sem_wait(barrier_sem2);
    sem_post(barrier_sem2);
}

// the oxygen function
void oxygen() {
    sem_wait(mutex);
    
    int oxy_count = (*oxygen_id)++; // copy the oxygen_id into local variable so it doesn't change
    print_process("O %d: started\n", oxy_count);
    usleep((rand() % (*ti + 1)) * 1000); // sleep for a random time
    print_process("O %d: going to queue\n", oxy_count);

    (*oxy)++;
    if(*hyd >= 2) {
        // if there are enough hydrogens, a molecule will be created, reset the hyd and oxy counters
        // and post the semaphores
        sem_post(hydrogen_sem);
        sem_post(hydrogen_sem);
        *hyd -= 2;
        sem_post(oxygen_sem);
        (*oxy)--;
    } else if((oxy_count * 2) > *nh) {
        // if the oxygen cannot form a molecule (not enough hydrogens), exit the process
        print_process("O %d: not enough H\n", oxy_count);
        sem_post(mutex);
        exit(0);
    } else {
        sem_post(mutex);
    }

    sem_wait(oxygen_sem); // wait for the oxygen semaphore

    // "bond" the molecule
    print_process("O %d: creating molecule %d\n", oxy_count, (*molecule_id));
    usleep((rand() % (*tb + 1)) * 1000); // the "creation" of the molucule
    sem_post(mol_created);
    sem_post(mol_created);
    print_process("O %d: molecule %d created\n", oxy_count, *molecule_id);

    barrier(); // wait for two hydrogens

    (*molecule_id)++; // increase the molecule count

    sem_post(mutex);
    exit(0);
}

void hydrogen() {
    sem_wait(mutex);

    int hyd_count = (*hydrogen_id)++; // copy the id into local variable
    print_process("H %d: started\n", hyd_count);
    usleep((rand() % (*ti + 1)) * 1000); // sleep for a random time
    print_process("H %d: going to queue\n", hyd_count);

    (*hyd)++;
    if(*hyd >= 2 && *oxy >= 1) {
        // check if there are enough atom to form a molecule, if so, reset the counters
        // and post the semaphores
        sem_post(hydrogen_sem);
        sem_post(hydrogen_sem);
        (*hyd) -= 2;
        sem_post(oxygen_sem);
        (*oxy)--;
    } else if(hyd_count > (*no * 2) || (hyd_count == *nh && *nh % 2 == 1)) {
        // if there are not enough atom to form a molecule woth this hydrogen, exit
        print_process("H %d: not enough O or H\n", hyd_count);
        sem_post(mutex);
        exit(0);
    } else {
        sem_post(mutex);
    }

    sem_wait(hydrogen_sem); // wait for the hydrogen semaphore

    // "bonding" of the molecule
    print_process("H %d: creating molecule %d\n", hyd_count, *molecule_id);
    sem_wait(mol_created); // wait for a signal from oxygen
    print_process("H %d: molecule %d created\n", hyd_count, *molecule_id);

    barrier();

    exit(0);
}

int main(int argc, char *argv[]) {
    initialize();
    parse_args(argc, argv);

    // the oxygen processes
    pid_t oxygen_processes[*no + 1];
    for(int o = 0; o < *no; o++) {
        pid_t id = fork();
        oxygen_processes[o] = id;
        if(id == 0) {
            oxygen();
        } else if(id < 0) {
            // if fork fails, exit
            fprintf(stderr, "Error: fork failed!\n");
            for(int i = 0; i < o; i++) {
                kill(oxygen_processes[i], SIGKILL);
            }
            destroy();
            exit(1);
        }
    }
    // the hydrogen processes
    pid_t hydrogen_processes[*nh + 1];
    for(int h = 1; h <= *nh; h++) {
        pid_t id = fork();
        if(id == 0) {
            hydrogen();
        } else if(id < 0) {
            // if fork fails, exit
            fprintf(stderr, "Error: fork failed!\n");
            for(int i = 0; i < h; i++) {
                kill(hydrogen_processes[i], SIGKILL);
            }
            destroy();
            exit(1);
        }
    }

    while(wait(NULL) > 0); // wait for all the processes to finish

    destroy();

    return 0;
}
