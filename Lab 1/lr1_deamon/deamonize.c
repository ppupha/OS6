#include "apue.h"
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>


void deamonize (const char *cmd) {
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit r1;
	struct sigaction sa;

	//(1)сбросить маску родителя создания файла, вдруг мы будем создавать файлы, а это запрещено. 
	umask(0);
	
	if (getrlimit(RLIMIT_NOFILE, &r1) < 0){
		err_quit("%s: невозможно получить максимальный номер дескриптора", cmd);
	}

	//(2)заставим ком.оболочку думать, что прцоесс завершился + для последующего вызова setsid необходимо не быть лидером группы.
	if ((pid = fork()) < 0) {
		err_quit("%s: ошибка вызова функции fork", cmd);
	} else if (pid != 0) {
		exit(0);
	}

	//(3) создаем новый сеанс (теперь мы лидер нового сеанса, новой группы процессов, лишаемся упр терминала)
	setsid();



	//(4) т.к. мы можем быть на смонтированной ФС, её нельзя будет отмонтировать, пока работает демон
	if (chdir("/home/ilyaps/bmstu/lr1_deamon/") < 0) {
		err_quit("%s: невозможно сделать текущим рабочим каталогом /", cmd);
	}

	//(5) предотвращаем удержание в открытом состоянии родительских дескрипторов
	if (r1.rlim_max == RLIM_INFINITY) {
		r1.rlim_max = 1024;
	}
	for (i = 0; i < r1.rlim_max; ++i) {
		close(i);
	}

	//сигнал утраты управляющего терминала надо подавить
	sa.sa_handler = SIG_IGN; 
	sigemptyset( &sa.sa_mask );
	sa.sa_flags = 0;
	if ( sigaction(SIGHUP, &sa, NULL) < 0 )
	{
		syslog(LOG_ERR, "ERROR: can't ignore SIGHUP");
		exit(-1);
	}

	//библ функции чтения/записи не будут влиять на работу
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
}
	

	
		
	
