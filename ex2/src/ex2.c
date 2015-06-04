/*
 *
 * ex2
 *
 * A) Set affinity to single CPU for each OMP thread
 * B) Prepare time measurment method using RDTSC x86 instruction
 * 
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include <omp.h>
#include <pthread.h>
 
/*
 *
 * From Intel® 64 and IA-32 Architectures Software Developer’s Manual:
 *
 * RDTSC — Read Time-Stamp Counter
 *
 * Opcode  Instruction  Op/En  64-Bit Mode  Compat/Leg Mode  Description
 * 0F 31   RDTSC        NP     Valid        Valid            Read time-stamp counter into EDX:EAX.
 * 
 */

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

int main()
{
	printf("ex2\n");

#pragma omp parallel
	{
		pthread_t pt_handle;
		int omp_thread_num;
		int cpunum;
		cpu_set_t cpuset;
		int cpu_amount;
		int i, j;
		unsigned long long t1, t2;

		pt_handle = pthread_self();
		cpu_amount = CPU_COUNT(&cpuset);
		omp_thread_num = omp_get_thread_num();
		cpunum = omp_thread_num % cpu_amount;

		CPU_ZERO(&cpuset);
		CPU_SET(cpunum, &cpuset);
		pthread_setaffinity_np(pt_handle, sizeof(cpuset), &cpuset);

		CPU_ZERO(&cpuset);
		pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);

		for(i = 0; i < cpu_amount; i++)
			if(CPU_ISSET(i, &cpuset))
				printf("[%d] I am running on CPU %d!\n", omp_thread_num, i);

		j = 0;

		t1 = rdtsc();

		for(i = 0; i < 99999; i++)
			j += 1;

		t2 = rdtsc();

		printf("Loop run for %llu cycles\n", t2 - t1);

	}

	return EXIT_SUCCESS;

}

