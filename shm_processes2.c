#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

#define LOOPS 25

// Function prototypes
void ParentProcess(int *shm_ptr);
void ChildProcess(int *shm_ptr);

int main() {
    srand(time(0));  // Seed for random numbers

    // Shared memory setup
    int shm_id = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget error");
        exit(1);
    }
    int *shm_ptr = (int *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (int *)-1) {
        perror("shmat error");
        exit(1);
    }

    // Initialize shared variables
    shm_ptr[0] = 0;  // BankAccount
    shm_ptr[1] = 0;  // Turn

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork error");
        exit(1);
    } else if (pid == 0) {
        ChildProcess(shm_ptr);  // Child process
    } else {
        ParentProcess(shm_ptr); // Parent process
        wait(NULL);             // Wait for child to complete

        // Detach and remove shared memory
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
    }
    return 0;
}

void ParentProcess(int *shm_ptr) {
    int *BankAccount = &shm_ptr[0];
    int *Turn = &shm_ptr[1];

    for (int i = 0; i < LOOPS; i++) {
        sleep(rand() % 6);  // Sleep for 0-5 seconds

        int account = *BankAccount;

        // Wait for Turn to be 0
        while (*Turn != 0);

        if (account <= 100) {
            int balance = rand() % 101;  // Random amount between 0 and 100
            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        // Update BankAccount and Turn
        *BankAccount = account;
        *Turn = 1;
    }
}

void ChildProcess(int *shm_ptr) {
    int *BankAccount = &shm_ptr[0];
    int *Turn = &shm_ptr[1];

    for (int i = 0; i < LOOPS; i++) {
        sleep(rand() % 6);  // Sleep for 0-5 seconds

        int account = *BankAccount;

        // Wait for Turn to be 1
        while (*Turn != 1);

        int balance = rand() % 51;  // Random amount between 0 and 50
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // Update BankAccount and Turn
        *BankAccount = account;
        *Turn = 0;
    }
}