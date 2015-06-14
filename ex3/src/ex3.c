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

#define NUM_STEPS 100000000L
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

/*
 * bind_omp_thread_to_one_cpu()
 *
 * Helper function to set CPU affinity so each OMP thread is bound to single core
 *
 */
int bind_omp_thread_to_one_cpu()
{

	pthread_t pt_handle;
	int omp_thread_num;
	int cpunum;
	cpu_set_t cpuset;
	unsigned int cpu_amount;

	unsigned int i;
	
	omp_thread_num = omp_get_thread_num();
	pt_handle = pthread_self();

	CPU_ZERO(&cpuset);
	pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);
	cpu_amount = CPU_COUNT(&cpuset);

	printf("[%d] I run on %u cpus\n", omp_thread_num, cpu_amount);

	if(cpu_amount != 1) {

		omp_thread_num = omp_get_thread_num();
		cpunum = omp_thread_num % cpu_amount;
		printf("[%d] setting cpunum to %d\n", omp_thread_num, cpunum);

		CPU_ZERO(&cpuset);
		CPU_SET(cpunum, &cpuset);
		pthread_setaffinity_np(pt_handle, sizeof(cpuset), &cpuset);

		CPU_ZERO(&cpuset);
		pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);


	}

	pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);
	for(i = 0; i < 4; i++)
		if(CPU_ISSET(i, &cpuset))
			printf("[%d] I am running on CPU %d!\n", omp_thread_num, i);

	return 0;

}


double compute_pi_serial(unsigned long long num_steps)
{
	double x = 0.0f, sum = 0.0f, step = 0.0f;
	unsigned int i;

	step = 1.0f / num_steps;

	for(i = 0; i < num_steps; i++) {

		x = (i + 0.5f) * step;
		sum += 4.0f / (1.0f + (x * x));

	}

	return sum * step;

}

/* * *
 * 
 * compute_pi_parallel_v1
 * 
 * Zapewnić atomowość uaktualnienia współdzielonej sumy za pomocą #pragma omp
 * atomic
 *
 * * */

double compute_pi_parallel_v1(const unsigned long long num_steps, unsigned int num_threads)
{

	double x = 0.0f, sum = 0.0f;
	const double step = 1.0f / num_steps;
	unsigned int i;

	omp_set_num_threads(num_threads);

#pragma omp parallel default(none) private(i, x) shared(sum)
	{

		bind_omp_thread_to_one_cpu();

		printf("[%d] My pthread id = %lu\n", omp_get_thread_num(), pthread_self());

		


#pragma omp for

		for(i = 0; i < num_steps; i++) {

			/*
			printf("in step %d in thread %d\n", i, omp_thread_num);
			*/

			x = (i + 0.5f) * step;

#pragma omp atomic update

			sum += 4.0f / (1.0f + (x * x));
		}
	}

	return sum * step;

}

/* * *
 * 
 * compute_pi_parallel_v2
 * 
 * Zapewnić atomowość uaktualnienia współdzielonej sumy za pomocą #pragma omp
 * atomic
 *
 * * */

double compute_pi_parallel_v2(const unsigned long long num_steps, unsigned int num_threads)
{

	double x = 0.0f, sum = 0.0f;
	const double step = 1.0f / num_steps;
	unsigned int i;

	omp_set_num_threads(num_threads);

#pragma omp parallel default(none) private(i, x) shared(sum)
	{

		bind_omp_thread_to_one_cpu();

		printf("[%d] My pthread id = %lu\n", omp_get_thread_num(), pthread_self());

#pragma omp for reduction(+ : sum)

		for(i = 0; i < num_steps; i++) {

			/*
			printf("in step %d in thread %d\n", i, omp_thread_num);
			*/

			x = (i + 0.5f) * step;
			sum += 4.0f / (1.0f + (x * x));
		}
	}

	return sum * step;

}

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
	struct timespec ts_start, ts_end, ts_diff;

	if(clock_gettime(CLOCK_REALTIME, &ts_start) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	pi = compute_pi_serial(NUM_STEPS);

	if(clock_gettime(CLOCK_REALTIME, &ts_end) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	ts_diff = get_timespec_diff(ts_start, ts_end);

	printf("Pi = %15.12f, computed in %ld.%09ld  seconds\n", pi,
			ts_diff.tv_sec, ts_diff.tv_nsec);


	if(clock_gettime(CLOCK_REALTIME, &ts_start) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	pi = compute_pi_parallel_v1(NUM_STEPS, NUM_THREADS);

	if(clock_gettime(CLOCK_REALTIME, &ts_end) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	ts_diff = get_timespec_diff(ts_start, ts_end);

	printf("Pi = %15.12f, computed in %ld.%09ld  seconds\n", pi,
			ts_diff.tv_sec, ts_diff.tv_nsec);

	if(clock_gettime(CLOCK_REALTIME, &ts_start) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	pi = compute_pi_parallel_v2(NUM_STEPS, NUM_THREADS);

	if(clock_gettime(CLOCK_REALTIME, &ts_end) == -1) {
		perror("clock gettime");
		exit( EXIT_FAILURE );
	}

	ts_diff = get_timespec_diff(ts_start, ts_end);

	printf("Pi = %15.12f, computed in %ld.%09ld  seconds\n", pi,
			ts_diff.tv_sec, ts_diff.tv_nsec);

	return EXIT_SUCCESS;

}

