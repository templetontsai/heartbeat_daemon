#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>

#include <utils/Atomic.h>
#include <utils/Log.h>


#define LOG_TAG "HEARTBEAT" 
#define HEARTBEAT_SYS_PATH "/sys/class/sensors/di_sensors/hsensor"
#define HEARTBEAT_PXDATA_PATH "/sys/class/sensors/di_sensors/pxvalue"
#define HEARTBEAT_DATA_PATH "/sys/class/sensors/di_sensors/heartbeat"
#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define AP3426_SYS_HS_ENABLE 9
extern float cal(int);
int getFD(char *path);
int sys_read(char* path);
/*=====================================================================*/

int main() {

    int ret;
    int mode_fd;
    int heartbeat_fd = 0;
    char buf_w[2];
    char h[1000];
    struct timeval start, end;
    int heartbeat = 0, pxvalue = 0;


    heartbeat_fd = getFD(HEARTBEAT_DATA_PATH);

    if (heartbeat_fd < 0) {
	ALOGE("couldn't open motion_fd sysfs device");
	return -1;     
    }

    HBM_Init();

    while(1) {

	if(sys_read(HEARTBEAT_SYS_PATH) == AP3426_SYS_HS_ENABLE) {
	    //ALOGI("time: start = %ld\n", start.tv_sec * 1000000 + start.tv_usec);
	    gettimeofday(&start, NULL);

	    pxvalue = sys_read(HEARTBEAT_PXDATA_PATH);
	    ALOGI("px raw data = %d\n", pxvalue);

	    //usleep(1000 * 1000 * 5);
	    heartbeat = round(cal(pxvalue));
	    ALOGI("cal data = %d\n", heartbeat);
	    sprintf(h, "%d", heartbeat);
	    ALOGI("h = %s\n", h);
	    //	buf_w[0] = round(heartbeat);
	    //	buf_w[1] = '\0';
	    write(heartbeat_fd, h, sizeof(h));

	    gettimeofday(&end, NULL);
	    //ALOGI("time: end = %ld\n", end.tv_sec * 1000000 + end.tv_usec);
	    ALOGI("time: diff = %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
			- (start.tv_sec * 1000000 + start.tv_usec)));


	} else {
	    usleep(1000 * 1000 * 5);//sleep 5 ms
	}
    }

    return 0;
}
int getFD(char *path) {
    int fd = -1;
    char sysfs[PATH_MAX];
    strcpy(sysfs, path);

    fd = open(sysfs, O_RDWR);

    return fd;
}
int sys_read(char* path) 
{
    int ret = 0;
    int fd = 0;
    char buf_r[4] = {0};

    fd = getFD(path);


    if (fd < 0) {
	ALOGE("couldn't open sysfs device");
	return -1;     
    }
    ret = read(fd, buf_r, sizeof(buf_r));
    if (ret < 0) {
	ALOGE("sysfs read failed");
	return -1;     
    }


    ret = (int)strtol(buf_r, NULL, 10);


    close(fd);
    return ret;

}
