#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h>

#define shmSIZE sizeof(int)
#define shmKEY "/ios-proj2h-xvanic09"
#define semCISLOVANIE "/ios-proj2_cislovanie-xvani09"
#define semVSTUP "/ios-proj2_vstup-xvanic09"
#define shmPOCITADLO "/ios-proj2_pocitadlo-xvanic09"
#define semSYNC "/ios-proj2_sync-xvanic09"
#define semWAITSYNC "/ios-proj2_waitsync-xvanic09"
#define semGEN1 "/ios-proj2_gen1-xvanic09"
#define semGEN2 "/ios-proj2_gen2-xvanic09"
#define FNAME "proj2.out"


pid_t consPID;
pid_t prodPID;
int *shm;
sem_t *sem_cislovanie;
sem_t *sem_vstup;
sem_t *sem_sync;
sem_t *sem_waitsync;
sem_t *sem_gen1;
sem_t *sem_gen2;
int A;
int C;
int AGT;
int CGT;
int AWT;
int CWT;
int AGT_R;
int CGT_R;
int AWT_R;
int CWT_R;
FILE *logFile;
int pocet_A; //pre sync
int pocet_C; //pre sync

struct pocitadlo {  //struktura obsahujuca premenne zdielanej pamate(ich inicializacia)
    int CA; 
    int CC;
    int AN; //Number of generated Adults
};

struct pocitadlo *shm_pocitadlo;

int string_na_cislo(char* str)
{
    int cislo = 0;
    int i = 0;
    while(str[i] != '\0') //
    {
        if(!(str[i] >= '0' && str[i] <= '9'))
            return -1;

        cislo = cislo*10;
        cislo = cislo + (str[i] - '0');
        i++;
    }

    return cislo;
}

int adult_proces(int generatedAdults)
{
    int shmID;
    int *shm;
    
    shmID = shm_open(shmKEY, O_RDWR, S_IRUSR | S_IWUSR);
    shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
    sem_wait(sem_cislovanie);
    fprintf(logFile,"%d     : A %d     : started \n",*shm,generatedAdults);
    fflush(logFile);
    *shm = *shm + 1;
    shm_pocitadlo->AN++;
    sem_post(sem_cislovanie);

    sem_wait(sem_cislovanie);
    fprintf(logFile,"%d     : A %d     : enter \n",*shm,generatedAdults);
    fflush(logFile);
    *shm = *shm + 1;
    shm_pocitadlo->CA++;
    sem_post(sem_vstup);
    sem_post(sem_vstup);
    sem_post(sem_vstup);
    sem_post(sem_cislovanie);
    if((AWT_R = rand() % (AWT + 1 - 0) + 0) > 0)
    {
        usleep(AWT_R);
    }


    sem_wait(sem_cislovanie);
    fprintf(logFile,"%d     : A %d     : trying to leave \n",*shm,generatedAdults);
    fflush(logFile);
    *shm = *shm + 1;
    sem_post(sem_cislovanie);

    sem_wait(sem_cislovanie);
    if (shm_pocitadlo->CC > (shm_pocitadlo->CA-1)*3)
    {
        fprintf(logFile,"%d     : A %d     : waiting: %d: %d \n",*shm,generatedAdults,shm_pocitadlo->CA,shm_pocitadlo->CC);
        fflush(logFile);
        *shm = *shm + 1;
    }
    sem_post(sem_cislovanie);

    sem_wait(sem_vstup);
    sem_wait(sem_vstup);
    sem_wait(sem_vstup);
    sem_wait(sem_cislovanie);
    shm_pocitadlo->CA--;
    fprintf(logFile,"%d     : A %d     : leave \n",*shm,generatedAdults);
    fflush(logFile);
    *shm = *shm + 1;
    if(shm_pocitadlo->AN == A && shm_pocitadlo->CA == 0)
    {
        int pocet = 0;
        while(C > pocet)
        {
            sem_post(sem_vstup);
            pocet++;
        }
    }
    sem_post(sem_cislovanie);

    sem_post(sem_waitsync);

    sem_wait(sem_sync);
    sem_wait(sem_cislovanie);
    fprintf(logFile,"%d     : A %d     : finished \n",*shm,generatedAdults);
    fflush(logFile);
    *shm = *shm + 1;
    sem_post(sem_cislovanie);


    munmap(shm, shmSIZE);
    exit(0);
}

int childs_proces(int generatedChilds)
{
    int shmID;
    int *shm;
    
    shmID = shm_open(shmKEY, O_RDWR, S_IRUSR | S_IWUSR);
    shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
    sem_wait(sem_cislovanie);
    fprintf(logFile,"%d     : C %d     : started \n",*shm,generatedChilds);
    fflush(logFile);
    *shm = *shm + 1;
    sem_post(sem_cislovanie);

    sem_wait(sem_cislovanie);
    if (shm_pocitadlo->CC+1 > (shm_pocitadlo->CA)*3)
    {
        fprintf(logFile,"%d     : C %d     : waiting: %d: %d \n",*shm,generatedChilds,shm_pocitadlo->CA,shm_pocitadlo->CC);
        fflush(logFile);
        *shm = *shm + 1;
    }
    sem_post(sem_cislovanie);

    sem_wait(sem_vstup);
    sem_wait(sem_cislovanie);
    fprintf(logFile,"%d     : C %d     : enter \n",*shm,generatedChilds);
    fflush(logFile);
    *shm = *shm + 1;
    shm_pocitadlo->CC++;
    sem_post(sem_cislovanie);
    if((CWT_R = rand() % (CWT + 1 - 0) + 0) > 0)
    {
        usleep(CWT_R);
    }

    sem_wait(sem_cislovanie);
    fprintf(logFile,"%d     : C %d     : trying to leave \n",*shm,generatedChilds);
    fflush(logFile);
    *shm = *shm + 1;
    sem_post(sem_cislovanie);

    sem_post(sem_vstup);
    sem_wait(sem_cislovanie);
    shm_pocitadlo->CC--;
    fprintf(logFile,"%d     : C %d     : leave \n",*shm,generatedChilds);
    fflush(logFile);
    *shm = *shm + 1;
    sem_post(sem_cislovanie);

    sem_post(sem_waitsync);

    sem_wait(sem_sync);
    sem_wait(sem_cislovanie);
    fprintf(logFile,"%d     : C %d     : finished \n",*shm,generatedChilds);
    fflush(logFile);
    *shm = *shm + 1;
    sem_post(sem_cislovanie);


    munmap(shm, shmSIZE);
    exit(0);
}

void konec() { //vyhadzuje chybovy kod a ukoncuje
    kill(consPID,SIGTERM);
    kill(prodPID,SIGTERM);
    waitpid(consPID, NULL, 0);
    waitpid(prodPID, NULL, 0);
    fclose(logFile);
    sem_close(sem_vstup);
    sem_unlink(semVSTUP);
    sem_close(sem_cislovanie);
    sem_unlink(semCISLOVANIE);
    sem_close(sem_sync);
    sem_unlink(semSYNC);
    sem_close(sem_waitsync);
    sem_unlink(semWAITSYNC);
    sem_close(sem_gen1);
    sem_unlink(semGEN1);
    sem_close(sem_gen2);
    sem_unlink(semGEN2);
    munmap(shm_pocitadlo, sizeof(struct pocitadlo));
    shm_unlink(shmKEY);
    shm_unlink(shmPOCITADLO);
    exit(2);
}

int main(int argc, char *argv[])
{
    signal(SIGTERM,konec);
    signal(SIGINT, konec);;
    
    printf("Zacal main!\n");

    if(argc != 7)
    {
        fprintf(stderr,"Nespravny pocet argumentov!");
        return EXIT_FAILURE;
    }
    else
    {
        A = string_na_cislo(argv[1]);
        C = string_na_cislo(argv[2]);
        AGT = string_na_cislo(argv[3]);
        CGT = string_na_cislo(argv[4]);
        AWT = string_na_cislo(argv[5]);
        CWT = string_na_cislo(argv[6]);
        int shmID;

        pocet_A = A;
        pocet_C = C;

        if(A <= 0)
        {
            fprintf(stderr,"Neplatna hodnota argumentu A!");
            return EXIT_FAILURE;
        }
        if(C <= 0)
        {
            fprintf(stderr,"Neplatna hodnota argumentu C!");
            return EXIT_FAILURE;
        }
        if(!(AGT >= 0 && AGT < 5001))
        {
            fprintf(stderr,"Neplatna hodnota argumentu AGT!");
            return EXIT_FAILURE;
        }
        if(!(CGT >= 0 && CGT < 5001))
        {
            fprintf(stderr,"Neplatna hodnota argumentu CGT!");
            return EXIT_FAILURE;
        }
        if(!(AWT >= 0 && AWT < 5001))
        {
            fprintf(stderr,"Neplatna hodnota argumentu AWT!");
            return EXIT_FAILURE;
        }
        if(!(CWT >= 0 && CWT < 5001))
        {
            fprintf(stderr,"Neplatna hodnota argumentu CWT!");
            return EXIT_FAILURE;
        }

        // vytvoreni souboru pro zapis
        logFile = fopen(FNAME, "w");
        if (logFile == NULL)
            {
                fprintf(stderr,"Nepodarilo sa vytvorit subor");
                return EXIT_FAILURE; 
            }

        // vytvorime a inicializujeme semafory
        sem_cislovanie = sem_open(semCISLOVANIE, O_CREAT|O_EXCL, 0666, 1);
        if (sem_cislovanie == SEM_FAILED) {
            fprintf(stderr, "Zlyhala inicializacia semaforu");
            konec();
        }
        sem_vstup = sem_open(semVSTUP, O_CREAT|O_EXCL, 0666, 0);
        if (sem_vstup == SEM_FAILED) {
            fprintf(stderr, "Zlyhala inicializacia semaforu");
            konec();
        }
        sem_sync = sem_open(semSYNC, O_CREAT|O_EXCL, 0666, 0);
        if (sem_sync == SEM_FAILED) {
            fprintf(stderr, "Zlyhala inicializacia semaforu");
            konec();
        }
        sem_waitsync = sem_open(semWAITSYNC, O_CREAT|O_EXCL, 0666, 0);
        if (sem_waitsync == SEM_FAILED) {
            fprintf(stderr, "Zlyhala inicializacia semaforu");
            konec();
        }
        sem_gen1 = sem_open(semGEN1, O_CREAT|O_EXCL, 0666, 0);
        if (sem_gen1 == SEM_FAILED) {
            fprintf(stderr, "Zlyhala inicializacia semaforu");
            konec();
        }
        sem_gen2 = sem_open(semGEN2, O_CREAT|O_EXCL, 0666, 0);
        if (sem_gen2 == SEM_FAILED) {
            fprintf(stderr, "Zlyhala inicializacia semaforu");
            konec();
        }


        // vytvoreni sdilene pameti - opat overime uspesnost ...
        // inicializacia ...
        shmID = shm_open(shmKEY, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
        if (shmID < 0) {
            perror("shm");
            konec();
        }
        ftruncate(shmID, shmSIZE);
        shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
        *shm = 1;
        munmap(shm, shmSIZE);
        close(shmID);

        shmID = shm_open(shmPOCITADLO, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
        if (shmID < 0) {
            perror("shm");
            konec();
        }
        ftruncate(shmID, sizeof(struct pocitadlo));
        shm_pocitadlo = (struct pocitadlo*)mmap(NULL,sizeof(struct pocitadlo), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
        shm_pocitadlo->CA=0;
        shm_pocitadlo->CC=0;
        shm_pocitadlo->AN=0;
        close(shmID);

        int pid;
        if ((pid = fork()) < 0)
        {
            perror("fork");
            konec();
        }

        if (pid == 0)   // novy proces pod main - generator childs
        {
            //kod pro generator childs
            printf("generator childs started\n");
            int generatedChilds = 0;
            while(generatedChilds < C) //loop generovania poctu zadanych childs
            {
                if((CGT_R = rand() % (CGT + 1 - 0) + 0) > 0)
                {
                    usleep(CGT_R);
                }
                int pid_newChild;
                if ((pid_newChild = fork()) == 0) // novy proces pod generatorem childs (novy child proces)
                {
                    childs_proces(generatedChilds);

                }
                else if(pid_newChild < 0) //kdyz nastala chyba pri generovani child
                {
                    perror("fork");
                    kill(getppid(),SIGTERM);
                    exit(2);
                }
                generatedChilds++;
            }
            int pomoc_C = pocet_C;
            while(pocet_C > 0)
            {
                sem_wait(sem_waitsync);
                pocet_C--;
            }
            sem_wait(sem_gen2); //synchronizace finished
            sem_post(sem_gen1);
            while(pomoc_C > 0)
                {
                    sem_post(sem_sync);
                    pomoc_C--;

                }
            printf("generator childs ended\n");
            //konec kodu pre generator childs
            exit(0);
        }
        else     // main proces (parent)
        {
            consPID = pid;
            //--
            pid = fork();
            if(pid < 0){
                perror("fork");
                konec();
            }
            if (pid == 0)   // novy proces pod main - generator adults
            {
                //kod pro generator adults
                printf("generator adults started\n");
                int generatedAdults = 0;
                while(generatedAdults < A) //loop generovania poctu zadanych adults
                {
                    if((AGT_R = rand() % (AGT + 1 - 0) + 0) > 0)
                    {
                        usleep(AGT_R);
                    }
                    int pid_newAdult;
                    if ((pid_newAdult = fork()) == 0) // novy proces pod generatorem adults (novy adult proces)
                    {
                        adult_proces(generatedAdults);
                    }
                    else if(pid_newAdult < 0) //kdyz nastala chyba pri generovani adult
                    {
                    perror("fork");
                    kill(getppid(),SIGTERM);
                    exit(2);
                    }
                    generatedAdults++;
                }
                int pomoc_A = pocet_A;
                while(pocet_A > 0)
                {
                    sem_wait(sem_waitsync);
                    pocet_A--;
                }
                sem_post(sem_gen2);
                sem_wait(sem_gen1); //synchronizace finished
                
                while(pomoc_A > 0)
                {
                    sem_post(sem_sync);
                    pomoc_A--;

                }
                printf("generator adults ended\n");
                //konec kodu pre generaotr adults
                exit(0);
            }
            else     // main proces (parent)
            {
                prodPID = pid;
            }
        }

        // pockame az vsichni skonci
        waitpid(consPID, NULL, 0);
        waitpid(prodPID, NULL, 0);
    }
    fclose(logFile);
    shm_unlink(shmPOCITADLO);
    sem_close(sem_vstup);
    sem_unlink(semVSTUP);
    sem_close(sem_cislovanie);
    sem_unlink(semCISLOVANIE);
    sem_close(sem_sync);
    sem_unlink(semSYNC);
    sem_close(sem_waitsync);
    sem_unlink(semWAITSYNC);
    sem_close(sem_gen1);
    sem_unlink(semGEN1);
    sem_close(sem_gen2);
    sem_unlink(semGEN2);
    shm_unlink(shmKEY);
    munmap(shm_pocitadlo, sizeof(struct pocitadlo));
    printf("Skoncil main!\n");
}
