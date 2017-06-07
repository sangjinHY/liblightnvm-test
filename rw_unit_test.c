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
    lun_addr.g.blk = 30;
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
    struct nvm_addr addrs_pg[geo->nsectors];
    struct nvm_ret ret;
    int pmode = NVM_FLAG_PMODE_SNGL;
    char *w_buf;
    char *r_buf;
    int i;

    w_buf = nvm_buf_alloc(geo, geo->nsectors * geo->sector_nbytes);
    r_buf = nvm_buf_alloc(geo, geo->nsectors * geo->sector_nbytes);

    buf_fill(w_buf, geo->nsectors * geo->sector_nbytes, 'a');

////////////////////////////////////////////////////////////////////////////////////
////                erase the block that we want to write                       ////
////////////////////////////////////////////////////////////////////////////////////
    for(i = 0; i < geo->nsectors; i++){
        addrs_pg[i].ppa = lun_addr.ppa;
        addrs_pg[i].g.pl = 0;
        addrs_pg[i].g.sec = i;
        addrs_pg[i].g.pg = 10;
    }

    res = nvm_addr_erase(dev, addrs, 1, pmode, &ret);
    if(res < 0){
        printf("fail to erase\n");
        nvm_ret_pr(&ret);
        teardown();
        exit(-2);
    }
////////////////////////////////////////////////////////////////////////////////////
////                           test unit is pg                                  ////
////////////////////////////////////////////////////////////////////////////////////
    printf("------------------unit is pg(Write)-------------------\n");
    nvm_addr_write(dev, addrs, geo->nsectors, w_buf, NULL, pmode, &ret);//write pg 1
    if(res < 0){
        printf("fail to write a pg\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        free(r_buf);
        teardown();
        exit(-2);
    }
    else
        printf("Write a page succeed!\n");
    printf("------------------unit is pg(Read)-------------------\n");
    memset(r_buf, 0, geo->nsectors * geo->sector_nbytes);
    res = nvm_addr_read(dev, addrs, geo->nsectors, r_buf, NULL, pmode, &ret );
    if(res < 0) {
        printf("fail to read pg\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        free(r_buf);
        teardown();
        exit(-2);
    }
    if(memcmp(r_buf, w_buf, geo->nsectors * geo->sector_nbytes) != 0){
        printf("write pg and read it error!");
        nvm_ret_pr(&ret);
        free(w_buf_pg1);
        free(w_buf_pg2);
        free(r_buf);
        teardown();
        exit(-3);
    }
    else
        printf("read pg succeed!\n");
////////////////////////////////////////////////////////////////////////////////////
////                           test unit is sector                              ////
////////////////////////////////////////////////////////////////////////////////////
    printf("------------------unit is sector(Read)-------------------\n");
    memset(r_buf, 0, geo->nsectors * geo->sector_nbytes);
    res = nvm_addr_read(dev, addrs, 1, r_buf, NULL, pmode, &ret );
    if(res < 0) {
        printf("fail to read pg\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        free(r_buf);
        teardown();
        exit(-2);
    }
    if(memcmp(r_buf, w_buf, geo->sector_nbytes) != 0){
        printf("read sector error!");
    }
    else
        printf("read sec succeed!\n");

    if(memcmp(r_buf, w_buf, geo->nsectors * geo->sector_nbytes) != 0){
        printf("read sector and compare pg reference error!");
    }
    else
        printf("read sec and compare pg reference succeed!\n");

    res = nvm_addr_erase(dev, addrs, 1, pmode, &ret);
    if(res < 0){
        printf("fail to erase\n");
        nvm_ret_pr(&ret);
        teardown();
        exit(-2);
    }
    printf("------------------unit is sector(Write)-------------------\n");
    nvm_addr_write(dev, addrs, 1, w_buf, NULL, pmode, &ret);//write pg 1
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
    res = nvm_addr_read(dev, addrs, 1, r_buf, NULL, pmode, &ret );
    if(res < 0) {
        printf("fail to read sector\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        free(r_buf);
        teardown();
        exit(-2);
    }
    if(memcmp(r_buf, w_buf_pg1, geo->sector_nbytes) != 0){
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
