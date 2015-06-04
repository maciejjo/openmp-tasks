#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include <omp.h>

struct dupa {

	int member;
	float liczba;
	char znaczek;

};

int main()
{
	cpu_set_t cpuset;

	struct dupa dupamiso;

	printf("ex2\n");


#pragma omp parallel
	{
		printf("Running in parallel...\n");
		printf("My pid is %d\n", getpid());
		printf("My thread num is %d\n", omp_get_thread_num());
	}

	printf("CPU_SETSIZE = %d\n", CPU_SETSIZE);

	printf("Zeroing cpuset...\n");
	CPU_ZERO(&cpuset);
	printf("Done!\n");

	printf("Getting CPU affinity...\n");
	sched_getaffinity(getpid(), sizeof(cpuset), &cpuset);
	printf("Done!\n");

	printf("%d CPUs in cpuset.\n", CPU_COUNT(&cpuset));

	dupamiso.member = 150;
	dupamiso.znaczek = 'z';
	dupamiso.liczba = 12312.312312;
	
	return EXIT_SUCCESS;

}

