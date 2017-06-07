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

int test(int pg_num){
    struct nvm_addr addrs[pg * geo -> nsectors];
    char *w_buf_pg1 = NULL;
    char *w_buf_pg2 = NULL;
    char *r_buf = NULL;
    int pmode = NVM_FLAG_PMODE_SNGL;
    int i;
    ssize_t res;
    w_buf_pg1 = nvm_buf_alloc(geo, geo->nsectors * geo->sector_nbytes);
    w_buf_pg2 = nvm_buf_alloc(geo, geo->nsectors * geo->sector_nbytes);
    r_buf = nvm_buf_alloc(geo, geo->nsectors * geo->sector_nbytes);
    if(w_buf_pg1 == NULL || w_buf_pg2 == NULL || r_buf == NULL)
    {
        printf("Fail alloc buf\n");
        free(w_buf);
        teardown();
        exit(-1);
    }
    buf_fill(w_buf_pg1, geo->nsectors * geo->sector_nbytes, 'a'); //pg1 set a
    buf_fill(w_buf_pg2, geo->nsectors * geo->sector_nbytes, 'b'); //pg1 set b
////////////////////////////////////////////////////////////////////////////////////
////                erase the block that we want to write                       ////
////////////////////////////////////////////////////////////////////////////////////
    for(i = 0; i < 2; i++){
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
////////////////////////////////////////////////////////////////////////////////////
////                write the page that we want to write                        ////
////////////////////////////////////////////////////////////////////////////////////
    for(i = 0; i < pg_num * geo->nesectors; i++){
        addrs[i].ppa = lun_addr.ppa;
        addrs[i].g.pg = 10;
        addrs[i].g.pl = i / geo->nesectors;
        addrs[i].g.sec = i % geo->nesectors;
    }
    nvm_addr_write(dev, addrs, geo->nsectors, w_buf_pg1, NULL, pmode, &ret);//write pg 1
    if(res < 0){
        printf("fail to write pg 1\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        teardown();
        exit(-2);
    }
    nvm_addr_write(dev, addrs + geo->nsectors, geo->nsectors, w_buf_pg2, NULL, pmode, &ret);//write pg 2
    if(res < 0){
        printf("fail to write pg 2\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        teardown();
        exit(-2);
    }
////////////////////////////////////////////////////////////////////////////////////
////                read the page that we wrote                                 ////
////////////////////////////////////////////////////////////////////////////////////
    printf("---------------------read-------------------------\n");
    memset(r_buf, 0, geo->nsectors * geo->geo->sector_nbytes);
    res = nvm_addr_read(dev, addrs, geo->nsectors, r_buf, NULL, pmode, &ret );
    if(res < 0) {
        printf("fail to read\n");
        nvm_ret_pr(&ret);
        free(r_buf);
        teardown();
        exit(-2);
    }
    if(memcmp(r_buf, w_buf_pg1, geo->nsectors * geo->geo->sector_nbytes) != 0){
        printf("write pg 1 and read it error!");
        nvm_ret_pr(&ret);
        free(r_buf);
        teardown();
        exit(-3);
    }
    else
        printf("read pg 1 succeed!\n");

    memset(r_buf, 0, geo->nsectors * geo->geo->sector_nbytes);
    res = nvm_addr_read(dev, addrs + geo->nsectors, geo->nsectors, r_buf, NULL, pmode, &ret );
    if(res < 0) {
        printf("fail to read\n");
        nvm_ret_pr(&ret);
        free(r_buf);
        teardown();
        exit(-2);
    }
    if(memcmp(r_buf, w_buf_pg2, geo->nsectors * geo->geo->sector_nbytes) != 0){
        printf("write pg 1 and read it error!");
        nvm_ret_pr(&ret);
        free(r_buf);
        teardown();
        exit(-3);
    }
    else
        printf("read pg 2 succeed!\n");
////////////////////////////////////////////////////////////////////////////////////
////                             erase a block                                  ////
////////////////////////////////////////////////////////////////////////////////////
    printf("---------------------erase the block that pg1 located-------------------------\n");
    res = nvm_addr_erase(dev, addrs, geo->nplanes, pmode, &ret); //erase the block that pg 1 located
    if(res < 0){
        printf("fail to erase\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        teardown();
        exit(-2);
    }
    printf("---------------------read pg 2-------------------------\n");
    memset(r_buf, 0, geo->nsectors * geo->geo->sector_nbytes);
    res = nvm_addr_read(dev, addrs + geo->nsectors, geo->nsectors, r_buf, NULL, pmode, &ret );
    if(res < 0) {
        printf("fail to read\n");
        nvm_ret_pr(&ret);
        free(r_buf);
        teardown();
        exit(-2);
    }
    if(memcmp(r_buf, w_buf_pg2, geo->nsectors * geo->geo->sector_nbytes) != 0){
        printf("write pg 1 and read it error!");
        nvm_ret_pr(&ret);
        free(r_buf);
        teardown();
        exit(-3);
    }
    else
        printf("read pg 2 succeed!\n");
    return 0;
}

int main(){
    setup();
    test(2);
    return 0;
}
