/*
 * ex3
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

#include "../headers/pi_algo.h"

#define NUM_STEPS 10000L
#define NUM_THREADS 4
 
#if 0
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
#endif

struct timespec get_timespec_diff(struct timespec start, struct timespec end)
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
	printf("ex3\n");

	double pi;
	unsigned int i;
	struct timespec ts_start, ts_end, ts_diff;
	/*
	double (* pi_algos[])(const unsigned long long num_steps, unsigned int num_threads) = {
		compute_pi_serial,
		compute_pi_parallel_v1,
		compute_pi_parallel_v2,
		compute_pi_parallel_v3
	};
	*/

	struct pi_algo algos[] = {
		{
			.function = compute_pi_serial,
			.name = "Serial Pi computation",
			.description = "Compute Pi using integrals",
		},
		{
			.function = compute_pi_parallel_v1,
			.name = "Parallel Pi computation v1",
			.description = "Parallelism with #pragma omp atomic on shared sum",
		},
		{
			.function = compute_pi_parallel_v2,
			.name = "Parallel Pi computation v2",
			.description = "Parallelism with #pragma omp reduction on sum",
		},
		{
			.function = compute_pi_parallel_v3,
			.name = "Parallel Pi computation v3",
			.description = "Parallelism with partial sums array per thread",
		},
	};

	for(i = 0; i < sizeof(algos)/sizeof(algos[0]); i++) {

		printf("Method: %s\n", algos[i].name);
		printf("Description: %s\n", algos[i].description);

		if(clock_gettime(CLOCK_REALTIME, &ts_start) == -1) {
			perror("clock gettime");
			exit( EXIT_FAILURE );
		}

		pi = algos[i].function(NUM_STEPS, NUM_THREADS);

		if(clock_gettime(CLOCK_REALTIME, &ts_end) == -1) {
			perror("clock gettime");
			exit( EXIT_FAILURE );
		}

		ts_diff = get_timespec_diff(ts_start, ts_end);

		printf("Pi = %15.12f, computed in %ld.%09ld  seconds\n", pi,
				ts_diff.tv_sec, ts_diff.tv_nsec);

	}

	return EXIT_SUCCESS;

}

