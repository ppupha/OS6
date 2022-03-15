#include <syslog.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h> // char * getlogin ( void );
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/wait.h> //wait
#include <pthread.h>

#define LOCKFILE "/var/run/lab1_daemon.pid"


#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

sigset_t mask; 


void daemonize(const char *cmd) 
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl; 
    struct sigaction sa; 
    
    umask(0);   // 1


    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) 
    {
        printf("Невозможно получить максимальный номер дескриптора\n");
        exit(1);
    }


    if ((pid = fork()) < 0)   // 2
    { 
        printf("Ошибка вызова fork\n");
        exit(1);
    }
    else if (pid != 0) 
    { 
        exit(0);
    }                                                          

	if (setsid() == -1)  // 3
    {
    	printf("Проблема с setsid\n");
    	exit(1);
    }
    
    sa.sa_handler = SIG_IGN; 

    sigemptyset(&sa.sa_mask); 
    sa.sa_flags = 0; 

    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        printf("Невозможно игнорировтаь сигнал sighup\n");
        exit(1);
    }


    if (chdir("/") < 0) {       // 4
        printf("Невозможно сделать корневой каталог текущим рабочим каталогом \n");
        exit(1);
    }


    if (rl.rlim_max == RLIM_INFINITY){
        rl.rlim_max = 1024; 
    }

    for (i = 0; i < rl.rlim_max; i++) {    // 5
        close(i); 
    }


    fd0 = open("dev/null", O_RDWR); 
    
    fd1 = dup(0);
    fd2 = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != -0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
     syslog(LOG_WARNING, "Демон запущен!");
}


int already_running(void)
{
    syslog(LOG_ERR, "Check ALready Running Daemon");

    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);

    if (fd < 0)
    {
        syslog(LOG_ERR, "Cant open File %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    syslog(LOG_WARNING, "File Opened");

    if (flock(fd, LOCK_EX | LOCK_NB) != 0)                
    {
    	if (errno == EWOULDBLOCK)                                                            
    	{
	        syslog(LOG_ERR, "Deamon is Running %s: %s", LOCKFILE, strerror(errno));
	        close(fd);
	        exit(1);
    	}

    	syslog(LOG_ERR, "Error fd"); 
    	exit(1);
	}

	
    ftruncate(fd, 0);  
    sprintf(buf, "%ld", (long)getpid()); 
    write(fd, buf, strlen(buf) + 1); 

    syslog(LOG_WARNING, "Wrote PID to File");
     
    return 0;
}


void *thr_fn(void *arg)
{
    int signo;
    int code_error = 0;
    int i = 0;
    while(code_error == 0)
    {
    	syslog(LOG_INFO, "From thr_fn\n");
        code_error = sigwait(&mask, &signo);
        switch (signo)
        {
		    case SIGHUP:
				syslog(LOG_INFO, "Received Signal SIGHUP\n");
		        time_t raw_time;
				struct tm *time_info;
				char buf[70];

				time(&raw_time);
				time_info = localtime(&raw_time);
				
				syslog(LOG_INFO, "User: %s; Time: %s", getlogin(), asctime(time_info));
		        break;
		    case SIGTERM:
		        syslog(LOG_INFO, "Received Signal SIGTERM\n");
		        return 0;
		        break;
		    default:
		        syslog(LOG_INFO, "Received Another Signal - %d\n", signo);
		        break;
        }
    }
}


int main(int argc, char *argv[]) 
{
    int err;
    pthread_t tid;    
    char *cmd;
    struct sigaction sa;


    if ((cmd = strrchr(argv[0], '/')) == NULL)
    {
        cmd = argv[0];
    }
    else
        cmd++;
        
    daemonize(cmd);


    if (already_running() != 0) {
        syslog(LOG_ERR, "Damon is Running\n");
        exit(1);
    }

    
    sa.sa_handler = SIG_DFL; 
    sigemptyset(&sa.sa_mask);  
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0){
        syslog(LOG_ERR, "Не удалось установить SIG_DFL на SIGHUP\n");
        exit(1);
    }
    
    syslog(LOG_INFO, "Reset SIGHUP");


    sigfillset(&mask);

    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
    {
        syslog(LOG_ERR, "Error: pthread_sigmask SIG_BLOCK\n");
        exit(1);
    }

    err = pthread_create(&tid, NULL, thr_fn, NULL);
    if (err != 0){
        syslog(LOG_ERR, "Error: Create Thread\n");
        exit(1); 
    }

    //pthread_join(tid, NULL);
    
    
    while (1)
    {
    	time_t t;
		struct tm *time_info;
		char buf[70];

		time(&t);
		time_info = localtime(&t);
			
		syslog(LOG_INFO, "User: %s; Time: %s", getlogin(), asctime(time_info));
		sleep(3);
    }
	
    exit(0);
}

