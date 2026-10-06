#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

int main()
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;

    // Create shared memory for two integers:
    // ShmPTR[0] = BankAccount
    // ShmPTR[1] = Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);

    if (ShmID < 0)
    {
        perror("shmget");
        exit(1);
    }

    ShmPTR = (int *)shmat(ShmID, NULL, 0);

    if (ShmPTR == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }

    // Initialize shared variables
    ShmPTR[0] = 0;   // BankAccount
    ShmPTR[1] = 0;   // Turn

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    // CHILD PROCESS - Poor Student
    if (pid == 0)
    {
        int i;
        int account;
        int balance;

        // Seed child's random number generator
        srand((unsigned int)(time(NULL) + getpid()));

        for (i = 0; i < 25; i++)
        {
            // Sleep randomly from 0 to 5 seconds
            sleep(rand() % 6);

            // Wait for child's turn
            while (ShmPTR[1] != 1)
            {
                // no-op for strict alternation
            }

            // Copy BankAccount after receiving the turn
            account = ShmPTR[0];

            // Student needs between $1 and $50
            balance = (rand() % 50) + 1;

            printf("Poor Student needs $%d\n", balance);

            if (balance <= account)
            {
                account -= balance;

                printf(
                    "Poor Student: Withdraws $%d / Balance = $%d\n",
                    balance,
                    account
                );
            }
            else
            {
                printf(
                    "Poor Student: Not Enough Cash ($%d)\n",
                    account
                );
            }

            // Update shared BankAccount
            ShmPTR[0] = account;

            // Give turn to Dad
            ShmPTR[1] = 0;
        }

        shmdt(ShmPTR);
        exit(0);
    }

    // PARENT PROCESS - Dear Old Dad
    else
    {
        int i;
        int account;
        int balance;

        // Seed parent's random number generator differently
        srand((unsigned int)(time(NULL) + getpid() * 2));

        for (i = 0; i < 25; i++)
        {
            // Sleep randomly from 0 to 5 seconds
            sleep(rand() % 6);

            // Wait for parent's turn
            while (ShmPTR[1] != 0)
            {
                // no-op for strict alternation
            }

            // Copy BankAccount after receiving the turn
            account = ShmPTR[0];

            if (account <= 100)
            {
                // Dad considers an amount between $1 and $100
                balance = (rand() % 100) + 1;

                // Deposit only when the random number is even
                if (balance % 2 == 0)
                {
                    account += balance;

                    printf(
                        "Dear old Dad: Deposits $%d / Balance = $%d\n",
                        balance,
                        account
                    );
                }
                else
                {
                    printf(
                        "Dear old Dad: Doesn't have any money to give\n"
                    );
                }
            }
            else
            {
                printf(
                    "Dear old Dad: Thinks Student has enough Cash ($%d)\n",
                    account
                );
            }

            // Update shared BankAccount
            ShmPTR[0] = account;

            // Give turn to Student
            ShmPTR[1] = 1;
        }

        // Wait for child to finish
        wait(NULL);

        // Detach and remove shared memory
        shmdt(ShmPTR);
        shmctl(ShmID, IPC_RMID, NULL);
    }

    return 0;
}