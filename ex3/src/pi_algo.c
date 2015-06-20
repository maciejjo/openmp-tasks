#define _GNU_SOURCE

#include <omp.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

int is_cpu_hyperthreaded()
{
	uint32_t registers[4];

	__asm__ __volatile__ ("cpuid " :
			      "=a" (registers[0]),
			      "=b" (registers[1]),
			      "=c" (registers[2]),
			      "=d" (registers[3])
			      : "a" (1), "c" (0));

	int hyperthreading = registers[3] & (1 << 28);

	if(hyperthreading)
		return 1;

	else
		return 0;
}


/*
 * bind_omp_thread_to_one_cpu()
 *
 * Helper function to set CPU affinity so each OMP thread is bound to single core
 *
 */
static int bind_omp_thread_to_one_cpu()
{

	pthread_t pt_handle;
	int omp_thread_num;
	int cpunum;
	cpu_set_t cpuset;
	unsigned int cpu_amount;

	/*
	unsigned int i;
	*/

	omp_thread_num = omp_get_thread_num();
	pt_handle = pthread_self();

	CPU_ZERO(&cpuset);
	pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);
	cpu_amount = CPU_COUNT(&cpuset);

	/*
	printf("[%d] I run on %u cpus\n", omp_thread_num, cpu_amount);
	*/

	if(cpu_amount != 1) {

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


	}

	pthread_getaffinity_np(pt_handle, sizeof(cpuset), &cpuset);
	/*
	for(i = 0; i < 4; i++)
		if(CPU_ISSET(i, &cpuset))
			printf("[%d] I am running on CPU %d!\n", omp_thread_num, i);
	*/

	return 0;

}

double compute_pi_serial(const unsigned long long num_steps, unsigned int num_threads)
{
	double x = 0.0f, sum = 0.0f, step = 0.0f;
	unsigned int i;

	/* Ignore threads in serial computation */
	(void)num_threads;

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

/* * *
 *
 * compute_pi_parallel_v3
 *
 * Zapewnić atomowość uaktualnienia współdzielonej sumy za pomocą #pragma omp
 * atomic
 *
 * * */

double compute_pi_parallel_v3(const unsigned long long num_steps, unsigned int num_threads)
{

	double x = 0.0f, sum = 0.0f;
	volatile double *sum_partial = NULL;
	const double step = 1.0f / num_steps;
	unsigned int i;

	omp_set_num_threads(num_threads);

	if((sum_partial = calloc(num_threads, sizeof(double))) == NULL) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}

#pragma omp parallel default(none) private(i, x) shared(sum_partial)
	{

		int omp_thread_num = 0;

		bind_omp_thread_to_one_cpu();
		omp_thread_num = omp_get_thread_num();

#pragma omp for

		for(i = 0; i < num_steps; i++) {

			/*
			printf("in step %d in thread %d\n", i, omp_thread_num);
			*/

			x = (i + 0.5f) * step;
			sum_partial[omp_thread_num] += 4.0f / (1.0f + (x * x));
		}

	}


	for(i = 0; i < num_threads; i++)
		sum += sum_partial[i];

	free((double *) sum_partial);
	return sum * step;

}

