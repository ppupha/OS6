#include <unistd.h>
#include <fcntl.h>

int lockfile(int fd) {
	struct flock f1;
	f1.l_type = F_WRLCK;
	f1.l_start = 0;
	f1.l_whence = SEEK_SET;
	f1.l_len = 0;
	return(fcntl(fd, F_SETLK, &f1));
}
