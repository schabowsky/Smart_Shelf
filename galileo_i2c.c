#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

static const int ARDUINO_ADDRESS = 0x6;

int main(int argc, char** argv) {

	int file = -1;
    int oldweight = 0;
    int weight = 0;
    
	if ((file = open("/dev/i2c-0", O_RDWR)) < 0) {
		printf("open error!\n");
		exit(1);
	}
	if (ioctl(file, I2C_SLAVE_FORCE, ARDUINO_ADDRESS) < 0) {
		printf("address error!\n");
		exit(1);
	}
	while (true) {
		
		char buf[8] = {0};
		
		if (read(file, buf, 8) == -1) {
			printf("Failed to read from the i2c bus.\n");
		}
		else {
            printf("Odczytano 8 bajtow\n");
			int i = 0;
			int numberOfBytes = 0;
			for (i=0; i<8; i++) {
				if (buf[i] == '0' || buf[i] == '1' || buf[i] == '2' || buf[i] == '3' || buf[i] == '4' || buf[i] == '5' || buf[i] == '6' || buf[i] == '7' || buf[i] == '8' || buf[i] == '9' || buf[i] == '-')
					numberOfBytes++;
			}
			i = 0;
			char w[numberOfBytes];
			for (i; i < numberOfBytes; i++)
				w[i] = buf[i];
			weight = atoi(w);
			printf("%d\n", weight);
			if (((oldweight - weight > 100) || (oldweight - weight < -100)) && weight > 0) {
			//if (oldweight != weight) {
                FILE *fp;
                char weightstr[12];
                sprintf(weightstr, "%d", weight);
				//strcpy(weightstr, w);
                if ((fp=fopen("weight.txt", "w"))==NULL) {
                    printf ("Nie moge otworzyc pliku test.txt do zapisu!\n");
                    exit(1);
                }
                fprintf(fp, "%s", weightstr);
                fclose(fp);
            }
            oldweight = weight;
		}
		sleep(2);
	}
}
