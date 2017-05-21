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
char *test_str = "Hello";

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
	lun_addr.g.blk = 0;
        return nvm_addr_check(lun_addr, geo);
}

int teardown()
{
	nvm_dev_close(dev);
        return 0;

}

int write_function(struct nvm_addr write_nvm)
{
	struct nvm_ret ret;
	int pmode = NVM_FLAG_PMODE_SNGL; //set the option mode
	char *w_buf = NULL;
	struct nvm_addr addrs[4];
	ssize_t res;

	w_buf = nvm_buf_alloc(geo, 4*geo->sector_nbytes);
	if(w_buf == NULL)
	{
		printf("Fail alloc w_buf\n");
		free(w_buf);
		teardown();
		exit(-1);
	}
	nvm_buf_fill(w_buf, 4*geo->sector_nbytes);
	printf("1\n");
	for( int i = 0; i < geo -> nplanes; i++){
		addrs[i].ppa = lun_addr.ppa;
		addrs[i].g.pl = i;
		
	}
	res = nvm_addr_erase(dev, addrs, geo->nplanes, pmode, &ret);
	if(res < 0){
		printf("fail to erase\n");
		nvm_ret_pr(&ret);
		free(w_buf);
                teardown();
                exit(-2);
			
	}
	for( int i = 0; i < geo -> nsectors; i++){
		addrs[i].ppa = lun_addr.ppa;
		addrs[i].g.pg = 0;
		addrs[i].g.sec = i % geo->nsectors;
		addrs[i].g.pl = 0;
	}
	res = nvm_addr_write(dev, addrs, 4, w_buf, NULL, pmode, &ret );
	printf("2\n");
	if(res < 0)
	{
		printf("fail to write\n");
		nvm_ret_pr(&ret);
		free(w_buf);
                teardown();
		exit(-2);
	}
	free(w_buf);
	return 1;
	
}

int test_write()
{
	struct nvm_addr write_addr;
	write_addr.ppa = lun_addr.ppa;
	write_addr.g.ch = lun_addr.g.ch;
	write_addr.g.pl = 0;
	write_addr.g.blk = 1;
	write_addr.g.pg = 0;	
	write_function(write_addr);
}

int main()
{
	setup();
	test_write();
	teardown();	
	return 1;
}
