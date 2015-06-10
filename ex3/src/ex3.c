/*
 *
 * ex3
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

#define NUM_STEPS 1000000000L
#define NUM_THREADS 4
 
/*
 *
 * From Intel® 64 and IA-32 Architectures Software Developer’s Manual:
 *
 * RDTSC — Read Time-Stamp Counter
 *
 * Opcode  Instruction  Op/En  64-Bit Mode  Compat/Leg Mode  Description
 * 0F 31   RDTSC        NP     Valid        Valid            Read time-stamp counter into EDX:EAX.
 * 

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}
 */

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

double compute_pi_parallel_1(unsigned long long num_steps, unsigned int num_threads)
{

	double x, sum = 0.0f, step = 0.0f;
	double *results = NULL;
	step = 1.0f / num_steps;
	unsigned int i;

	if((results = malloc(sizeof(double) * num_steps)) == NULL)
		perror("malloc");

	omp_set_num_threads(num_threads);

#pragma omp parallel
	{

		pthread_t pt_handle;
		int omp_thread_num;
		int cpunum;
		cpu_set_t cpuset;
		unsigned int cpu_amount;
		
		omp_thread_num = omp_get_thread_num();
		pt_handle = pthread_self();

		CPU_ZERO(&cpuset);
		pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);
		cpu_amount = CPU_COUNT(&cpuset);

		omp_thread_num = omp_get_thread_num();
		cpunum = omp_thread_num % cpu_amount;
		/*
		printf("[%d] setting cpunum to %d\n", omp_thread_num, cpunum);
		*/

		CPU_ZERO(&cpuset);
		CPU_SET(cpunum, &cpuset);
		pthread_setaffinity_np(pt_handle, sizeof(cpuset), &cpuset);

		CPU_ZERO(&cpuset);
		pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);

		/*
		pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);
		for(i = 0; i < cpu_amount; i++)
			if(CPU_ISSET(i, &cpuset))
				printf("[%d] I am running on CPU %d!\n", omp_thread_num, i);
		*/
#pragma omp for private(i)
		for(i = 0; i < num_steps; i++) {
			/*
			printf("in step %d in thread %d\n", i, omp_thread_num);
			*/

			x = (i + 0.5f) * step;
			results[i] = 4.0f / (1.0f + (x * x));

		}
	}

	for(i = 0; i < num_steps; i++)
		sum += results[i];

	return sum * step;
	return 1.0f;

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

	double pi;
	struct timespec t1_ts, t2_ts;


	/* Measure pi computation on each core */
	if(clock_gettime(CLOCK_REALTIME, &t1_ts) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	pi = compute_pi_parallel_1(NUM_STEPS, NUM_THREADS);

	if(clock_gettime(CLOCK_REALTIME, &t2_ts) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	printf("Pi = %15.12f, computed in %ld.%09ld  seconds\n",
			pi, diff(t1_ts, t2_ts).tv_sec,
			diff(t1_ts, t2_ts).tv_nsec);

	if(clock_gettime(CLOCK_REALTIME, &t1_ts) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	pi = compute_pi(NUM_STEPS);

	if(clock_gettime(CLOCK_REALTIME, &t2_ts) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	printf("Pi = %15.12f, computed in %ld.%09ld  seconds\n",
			pi, diff(t1_ts, t2_ts).tv_sec,
			diff(t1_ts, t2_ts).tv_nsec);



	return EXIT_SUCCESS;

}

