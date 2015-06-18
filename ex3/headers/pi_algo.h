#ifndef _PI_ALGO_H
#define _PI_ALGO_H

struct pi_algo {

	double (*function)(unsigned long long num_steps, unsigned int num_threads);
	char name[128];
	char description[1024];

};

double compute_pi_serial(const unsigned long long num_steps, unsigned int num_threads);
double compute_pi_parallel_v1(const unsigned long long num_steps, unsigned int num_threads);
double compute_pi_parallel_v2(const unsigned long long num_steps, unsigned int num_threads);
double compute_pi_parallel_v3(const unsigned long long num_steps, unsigned int num_threads);
double compute_pi_parallel_v4(const unsigned long long num_steps, unsigned int num_threads);

#endif

