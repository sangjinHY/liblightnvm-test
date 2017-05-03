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
static const struct nvm_geo *geo;

int setup(void)
{
    dev = nvm_dev_open(nvm_dev_path);
    if(!dev){
        perror("nvm_dev_open");
        exit(0);
    }
    geo = nvm_dev_get_geo(dev);
    return 1;
}

int ini_addr(struct nvm_addr *addr)
{
    addr->ppa = 0;
    addr->g.ch = 0;
    addr->g.lun = 0;
    addr->g.blk = 0;
    return 1;
}

void error_exit(void *w_buf, void *r_buf)
{
    if(w_buf)
        free(w_buf);
    if(r_buf)
        free(r_buf);
    nvm_dev_close(dev);
}

int rw_to_ocs(struct nvm_addr *addr, int length)
{
    char *buf, *r_buf;
    int pmode = NVM_FLAG_PMODE_SNGL;
    int res;
    struct nvm_ret ret;

    buf = nvm_buf_alloc(geo, length*geo->sector_nbytes);
    if(!buf){
        printf("alloc buf error\n");
        error_exit(buf, NULL);
        exit(-1);
    }
    buf[0]='h';
    buf[1]='e';
    buf[2]='l';
//    nvm_buf_fill(buf, length*geo->sector_nbytes);
//erase
    printf("Write sting:%d\n", (int)strlen(buf));
    
    res = nvm_addr_erase(dev, addr, length, pmode, &ret);
    if(res < 0){
        error_exit(buf, NULL);
        printf("erase error\n");
        exit(-1);
    }
//write
//    addr->g.pg = 1;
    res = nvm_addr_write(dev, addr, length, buf, NULL, pmode, &ret);
    printf("%s\n",buf);
    printf("res=%d\n", res);
    if(res < 0){
        error_exit(buf, NULL);
        printf("write error\n");
        exit(-1);
    }
//read
    r_buf = nvm_buf_alloc(geo, 2*length*geo->sector_nbytes);
    if(!r_buf){
        printf("alloc r_buf error\n");
        error_exit(buf, r_buf);
        exit(-1);
    }
    memset(r_buf, 0, 2*length*geo->sector_nbytes);
    res = nvm_addr_read(dev, addr, length, r_buf, NULL, pmode, &ret);
    if(res < 0){
        printf("read error\n");
        error_exit(buf, r_buf);
        exit(-1);
    }
    printf("Read string:%d\n", (int)strlen(r_buf));
    
    printf("%s\n",r_buf);
    free(buf);
    free(r_buf);
    return 1;
}

void shutdown()
{
    nvm_dev_close(dev);
}

int main()
{
    struct nvm_addr addr[1];
    int length;
    setup();
    length = 1*geo->nsectors;
    ini_addr(addr);
    rw_to_ocs(addr, length);
    shutdown();
    return 1;
}
