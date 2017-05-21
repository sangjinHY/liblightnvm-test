#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <liblightnvm.h>
#include <inttypes.h>

static char nvm_dev_path[NVM_DEV_PATH_LEN] = "/dev/nvme0n1";
static struct nvm_dev *dev;
static const struct nvm_geo *geo;
static struct nvm_addr lun_addr;

int setup()
{
	dev = nvm_dev_open(nvm_dev_path);
        if (!dev) {
                perror("nvm_dev_open");
                return -1;
        }
        geo = nvm_dev_get_geo(dev);
        lun_addr.ppa = 0;
        lun_addr.g.ch = 0;
        lun_addr.g.lun = 0;
        return nvm_addr_check(lun_addr, geo);
}

int teardown()
{
	nvm_dev_close(dev);
        return 0;

}

int read_function(struct nvm_addr read_nvm)
{
	struct nvm_ret ret;
	int pmode = NVM_FLAG_PMODE_SNGL; //set the option mode
	struct nvm_addr addrs[1];
	char *r_buf = NULL;
	ssize_t res;

	printf("1\n");
	r_buf = nvm_buf_alloc(geo, 4*geo->sector_nbytes);
	if(r_buf == NULL)
	{
		printf("Fail alloc r_buf\n");
		free(r_buf);
		teardown();
		exit(-1);
	}
	printf("2\n");
	addrs[0].ppa = lun_addr.ppa;
	addrs[0].g.blk = 0;
	addrs[0].g.pg = 0;
	addrs[0].g.pl = 0;
	addrs[0].g.sec = 0;
	res = nvm_addr_read(dev, &read_nvm, 1, r_buf, NULL, pmode, &ret );
	printf("3\n");
	if(res < 0)
	{
		printf("fail to read\n");
		nvm_ret_pr(&ret);
		free(r_buf);
                teardown();
		exit(-2);
	}
	printf("%s\n", r_buf);
	free(r_buf);
	return 1;
	
}

int test_read()
{
	struct nvm_addr read_addr;
	read_addr.ppa = lun_addr.ppa;
	read_addr.g.ch = lun_addr.g.ch;
	read_addr.g.pl = 0;
	read_addr.g.blk = 0;
	read_addr.g.pg = 0;	
	read_function(read_addr);
}

int main()
{
	setup();
	test_read();
	teardown();	
	return 1;
}
