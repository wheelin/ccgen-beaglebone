#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#include "EpmGen.h"

#define EPM_PATTERN_LEN    (7200u)
typedef struct {
    uint8_t Patten[EPM_PATTERN_LEN];
    uint32_t Speed;
    uint32_t Counter;
    uint16_t SigIndex;
    uint8_t Activated;
} T_Epm_SigGen;

#define PRU_ADDR        0x4A300000      // Start of PRU memory Page 184 am335x TRM
#define PRU_LEN         0x80000         // Length of PRU memory
#define PRU0_DRAM       0x00000         // Offset to DRAM
#define PRU1_DRAM       0x02000
#define PRU_SHAREDMEM   0x10000         // Offset to shared memory

T_Epm_SigGen *Epm_SigGen;        // Points to the start of local DRAM
static uint8_t Epm_SigPatternCpy[EPM_PATTERN_LEN];

int main(int argc, char *argv[])
{
    unsigned int    *pru;       // Points to start of PRU memory.

    int fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf ("ERROR: could not open /dev/mem.\n\n");
        return 1;
    }
    pru = mmap (0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PRU_ADDR);
    if (pru == MAP_FAILED) {
        printf ("ERROR: could not map memory.\n\n");
        return 1;
    }
    close(fd);
    printf ("Using /dev/mem.\n");

    Epm_SigGen =  (T_Epm_SigGen *)(pru + PRU0_DRAM/4 + 0x200/4);   // Points to 0x200 of PRU0 memory

    SpdGen_GenSig(&Epm_SigPatternCpy[0u], &EpmGen_Config, 0u);
    memcpy((void*)&Epm_SigGen->Patten[0u], (void*)&Epm_SigPatternCpy[0u], EPM_PATTERN_LEN);
    Epm_SigGen->Speed = 10000u;
    Epm_SigGen->Counter = 0u;
    Epm_SigGen->Activated = 1u;

    printf("Signal generation status: %d\n", Epm_SigGen->Activated);

    bool exit = false;
    while (true)
    {
        char input = getchar();
        switch (input)
        {
            case 'u':
            {
                Epm_SigGen->Speed = Epm_SigGen->Speed - 10u;
                break;
            }
            case 'd':
            {
                Epm_SigGen->Speed = Epm_SigGen->Speed + 10u;
                break;
            }
            case 'q':
            {
                exit = true;
                break;
            }
            case 'e':
            {
                Epm_SigGen->Activated = 1u;
                break;
            }
            case 'n':
            {
                Epm_SigGen->Activated = 0u;
                break;
            }
            default:
            {
                /*nothing*/
            }
        }

        if (exit == true)
        {
            break;
        }
        uint16_t Idx = Epm_SigGen->SigIndex;
        printf("Signal at %d: %d\n", Idx, Epm_SigGen->Patten[Idx]);
        printf("Status: %d\n", Epm_SigGen->Activated);
        printf("Speed: %d\n", Epm_SigGen->Speed);
    }

    if(munmap(pru, PRU_LEN)) {
        printf("munmap failed\n");
    } else {
        printf("munmap succeeded\n");
    }
}