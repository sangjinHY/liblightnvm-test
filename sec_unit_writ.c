//
// Created by victor on 6/7/17.
//

//
// Created by victor on 6/7/17.
//
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

int setup(){
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

int teardown(){
    nvm_dev_close(dev);
    return 0;

}

void buf_fill(char *buf, int length, char flag){
    int i;
    for(i = 0; i < length; i++){
        buf[i] = flag;
    }
}

int test(){
    struct nvm_addr addrs[geo->nsectors];
    struct nvm_ret ret;
    int pmode = NVM_FLAG_PMODE_SNGL;
    char *w_buf = NULL;
    char *r_buf = NULL;
    int res;
    int i;

    w_buf = nvm_buf_alloc(geo, geo->nsectors * geo->sector_nbytes);
    r_buf = nvm_buf_alloc(geo, geo->nsectors * geo->sector_nbytes);
    if(w_buf == NULL || r_buf == NULL) {
        printf("Fail alloc w_buf\n");
        teardown();
        exit(-1);
    }
    buf_fill(w_buf, geo->nsectors * geo->sector_nbytes, 'a');

    printf("------------------unit is sector(Write)-------------------\n");
    for(i = 0; i < geo->nplanes; i++){
        addrs[i].ppa = lun_addr.ppa;
        addrs[i].g.pl = i;
    }
    res = nvm_addr_erase(dev, addrs, geo->nplanes, pmode, &ret);
    if(res < 0){
        printf("fail to erase\n");
        nvm_ret_pr(&ret);
        teardown();
        exit(-2);
    }

    for(i = 0; i < geo->nsectors; i++){
        addrs[i].ppa = lun_addr.ppa;
        addrs[i].g.pl = 0;
        addrs[i].g.sec = i;
        addrs[i].g.pg = 0;
    }
    res = nvm_addr_write(dev, addrs, 1, w_buf, NULL, pmode, &ret);//write pg 1
    if(res < 0){
        printf("fail to write a pg\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        free(r_buf);
        teardown();
        exit(-2);
    }
    else
        printf("Write a sector succeed!\n");
    memset(r_buf, 0, geo->nsectors * geo->sector_nbytes);
    res = nvm_addr_read(dev, addrs, 1, r_buf, NULL, pmode, &ret );
    if(res < 0) {
        printf("fail to read sector\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        free(r_buf);
        teardown();
        exit(-2);
    }
    if(memcmp(r_buf, w_buf, geo->sector_nbytes) != 0){
        printf("write pg and read it error!");
        nvm_ret_pr(&ret);
        free(w_buf);
        free(r_buf);
        teardown();
        exit(-3);
    }
    else
        printf("read pg succeed!\n");
}

int main(){
    setup();
    test();
    teardown();
    return 0;
}
