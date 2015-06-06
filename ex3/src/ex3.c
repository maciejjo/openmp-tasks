/* * * * *
 *
 * ex3
 *
 * A) Set affinity to single CPU for each OMP thread
 * B) Prepare time measurment method using RDTSC x86 instruction
 * 
 * * * * */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include <omp.h>
#include <pthread.h>

#define NUM_STEPS 1000000000
 
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

double compute_pi(unsigned long long num_steps)
{
	double x, sum = 0.0f, step = 0.0f;
	unsigned int i;

	step = 1.0f / num_steps;

	for(i = 0; i < num_steps; i++) {

		x = (i + 0.5f) * step;
		sum += 4.0f / (1.0f + (x * x));

	}

	return sum * step;

}

struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;

	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

int main()
{
	printf("ex2\n");

	omp_set_num_threads(2);

#pragma omp parallel
	{

		pthread_t pt_handle;
		int omp_thread_num;
		int cpunum;
		cpu_set_t cpuset;
		int cpu_amount;
		int i;
		
		double pi;
		struct timespec t1_ts, t2_ts;

		pt_handle = pthread_self();

		/* Get current affinity to count processors */
		CPU_ZERO(&cpuset);
		pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);
		cpu_amount = CPU_COUNT(&cpuset);

		/* Select processor based on OpenMP thread number */
		omp_thread_num = omp_get_thread_num();
		cpunum = omp_thread_num % cpu_amount;
		 
		/* Set the affinity to selected processor */
		CPU_ZERO(&cpuset);
		CPU_SET(cpunum, &cpuset);
		pthread_setaffinity_np(pt_handle, sizeof(cpuset), &cpuset);

		/* Check if affinity is set properly */
		CPU_ZERO(&cpuset);
		pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);
		for(i = 0; i < cpu_amount; i++)
			if(CPU_ISSET(i, &cpuset))
				printf("[%d] I am running on CPU %d!\n", omp_thread_num, i);


		/* Measure pi computation on each core */
		if(clock_gettime(CLOCK_REALTIME, &t1_ts) == -1) {
			perror("clock gettime");
			exit( EXIT_FAILURE );
		}

		pi = compute_pi(NUM_STEPS);

		if(clock_gettime(CLOCK_REALTIME, &t2_ts) == -1) {
			perror("clock gettime");
			exit( EXIT_FAILURE );
		}

		printf("[%d] Pi = %15.12f, computed in %ld.%ld  seconds\n",
				omp_thread_num, pi, diff(t1_ts, t2_ts).tv_sec,
				diff(t1_ts, t2_ts).tv_nsec);


	}

	return EXIT_SUCCESS;

}

