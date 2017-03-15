#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <liblightnvm.h>

static char nvm_dev_path[NVM_DEV_PATH_LEN] = "/dev/nvme0n1";
static int channel = 0;
static int lun = 0;
static int block = 10;

static struct nvm_dev *dev;


int show_geo(void)
{
	nvm_dev_pr(dev);
	return 1;
}

int main(void)
{
	show_geo();
	return 1;
}


