#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <errno.h>

static int dopath(const char *filename, int depth) {
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	
	if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
		return 0;

	if (lstat(filename, &statbuf) < 0) {
		switch(errno)
		{
			case EBADF:
				printf("Неверный описатель файлового дескриптора.");
				break;
			case ENOENT:
				printf("Компонент полного имени файла filename не существует или полное имя является пустой строкой.");
				break;
			case ENOTDIR:
				printf("Компонент пути не является каталогом. ");
				break;	
			case ELOOP:
				printf("При поиске файла встретилась символьная ссылка.");
				break;	
			case EFAULT:
				printf("Некорректный адрес. ");
				break;	
			case EACCES:
				printf("Запрещен доступ. ");
				break;		
			case ENOMEM:
				printf("Недостаточно памяти в системе.");
				break;		
			case ENAMETOOLONG:
				printf("Слишком длинное название файла. ");
				break;	
			case EOVERFLOW:
				printf("Некоторые значения были слишком большими, чтобы быть представленными в возвращаемой структуре.s");
				break;	
		}		
		printf("Ошибка вызова функции stat");
		return -1;

	}
	
	for (int i = 0; i < depth; ++i)
		printf("       |");
	

	if (S_ISDIR(statbuf.st_mode) == 0) {
		printf("%s\n", filename);
		return 0;
	}

	printf("%s\n", filename);
	if ((dp = opendir(filename)) == NULL) {
		printf("couldn't open directory '%s'\n", filename);
		return 1;
	}
	
	chdir(filename);
	
	while ((dirp = readdir(dp)) != NULL) 
		dopath(dirp -> d_name, depth+1);
	chdir("..");

	closedir(dp);
} 

int main(int argc, char *argv[]) {
	int ret;

	if (argc != 2)
		ret = dopath("./",0);
	else
		ret = dopath(argv[1],0);

	return ret;
}
