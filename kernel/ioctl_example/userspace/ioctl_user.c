#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define MY_MACIG 'G'
#define READ_IOCTL _IOR(MY_MACIG, 0, int)
#define WRITE_IOCTL _IOW(MY_MACIG, 1, int)

struct buffer_struct {
    void *addr;
    size_t size;
};

/*
* Send the buffer struct through ioctl and receive the size in bytes
*/
int main(){
	struct buffer_struct buf;
	int buf2[4096*1/sizeof(int)];
	printf("sizeof(buf2): %ld\n", sizeof(buf2));
	buf.addr = &buf2;
	buf.size = sizeof(buf2);
	int fd = -2;
	if ((fd = open("/dev/gnuradio", O_RDWR)) < 0) {
		perror("open");
		return -1;
	}
	int num = 42;
	if(ioctl(fd, WRITE_IOCTL, &buf) < 0)
		perror("first ioctl");
	if(ioctl(fd, READ_IOCTL, &num) < 0)
		perror("second ioctl");

	printf("message: %d\n", num);
	return 0;
}

