#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <time.h>

extern void deamonize(const char * cmd);
extern int already_running();

int main(int argc, char *argv[]) {
	
	deamonize("new_deamon");

	if (already_running() != 0) {
		printf("already_running");
		exit(0);
	}

	FILE *fp;

	if ((fp = fopen("test", "w"))== NULL) {
		exit(1);
	}
	
	while (1) {
		//fprintf(fp, "I am working %d takts \n", clock());
		syslog(LOG_ERR, "I am working \n");
		fflush(fp);
		sleep(5);
	}
	return 0;
}
