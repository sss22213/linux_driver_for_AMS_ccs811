#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>

#define	IOCTL_WRITE	_IOW('c', 1, struct _ccs811_ioctl)

typedef enum {
    _CCS811_SET_MODE = 1,
    _CCS811_SET_TEMPERATURE,
    _CCS811_SET_HUMIDITY,
} _CCS811_SET;

struct _ccs811_ioctl {
    uint8_t ccs811_cmd;
    uint8_t low_val;
    uint8_t high_val;
};

int main()
{
    uint8_t buffer[6] = {0};

    // Read info from ccs811
    int fd = open("/dev/ccs811", O_RDWR);
    if (fd < 0) {
    	printf("Error: cannot open file.\n");
	    return -1;
    }

    printf("Open success\n");

    struct _ccs811_ioctl ccs811_ioctl;

    // Config ccs811 mode(Spec page 17).
    ccs811_ioctl.ccs811_cmd = _CCS811_SET_MODE;
    ccs811_ioctl.low_val = 1;

    ioctl(fd, IOCTL_WRITE, &ccs811_ioctl);

    while(1) {
        read(fd, &buffer, 8);

        // Status register
        if (buffer[4] == 0x98) {
            printf("ECO2= %d, TVOC= %d", buffer[0] << 8 | buffer[1], buffer[2] << 8 | buffer[3]);
        } else {
            printf("Waiting for measure.");
        }

        printf("\n");

        sleep(1);
    }
        
    close(fd);
    return 0;
}
