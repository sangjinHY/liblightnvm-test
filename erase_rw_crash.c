//
// Created by victor on 6/7/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <liblightnvm.h>
#include <inttypes.h>

double get_time(void) {
    struct	timeval	mytime;
    gettimeofday(&mytime,NULL);
    return (mytime.tv_sec*1.0+mytime.tv_usec/1000000.0);
}

static char nvm_dev_path[NVM_DEV_PATH_LEN] = "/dev/nvme0n1";
static struct nvm_dev *dev;
static const struct nvm_geo *geo;
static struct nvm_addr lun_addr;

double e_start_t = 0.0;
double e_end_t = 0.0;
double w_start_t = 0.0;
double w_end_t = 0.0;

int flag = 1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_l = PTHREAD_COND_INITIALIZER;

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

void single_lun_set(struct nvm_addr *single_addr){
    (*single_addr).ppa = 0;
    (*single_addr).g.ch = 0;
    (*single_addr).g.lun = 0;
    (*single_addr).g.blk = 30;
}




void buf_fill(char *buf, int length, char flag){
    int i;
    for(i = 0; i < length; i++){
        buf[i] = flag;
    }
}

void write_pg(void *w_addrs){
    struct nvm_ret ret;
    int pmode = NVM_FLAG_PMODE_SNGL; //set the option mode
    char *w_buf = NULL;
    ssize_t res;
    struct nvm_addr *addrs = (struct nvm_addr *)w_addrs;

    w_buf = nvm_buf_alloc(geo, 4*geo->sector_nbytes);
    if(w_buf == NULL)
    {
        printf("Fail alloc w_buf\n");
        free(w_buf);
        teardown();
        exit(-1);
    }
    nvm_buf_fill(w_buf, 4*geo->sector_nbytes);

/*    for( int i = 0; i < geo -> nplanes; i++){
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
    }*/
/*    res = nvm_addr_erase(dev, addrs, 1, pmode, &ret);
    if(res < 0){
        printf("fail to erase\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        teardown();
        exit(-2);

    }*/
    pthread_mutex_lock(&mutex);
    while(flag == 1){
        printf("Write pthread start!\n");
        pthread_cond_wait(&cond_l, &mutex);
    }
    w_start_t = get_time();
    res = nvm_addr_write(dev, addrs, 4, w_buf, NULL, pmode, &ret );
//    printf("write\n");
    if(res < 0)
    {
        printf("fail to write\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        teardown();
        exit(-2);
    }
    w_end_t = get_time();
    pthread_mutex_unlock(&mutex);
    free(w_buf);


}

void erase_blk(void *e_addrs){
    struct nvm_ret ret;
    int pmode = NVM_FLAG_PMODE_SNGL; //set the option mode
    ssize_t res;
    struct nvm_addr *addrs = (struct nvm_addr *)e_addrs;
    pthread_mutex_lock(&mutex);
    while(flag == 1){
        printf("Erase pthread start!\n");
        pthread_cond_wait(&cond_l, &mutex);
    }
    e_start_t = get_time();
    res = nvm_addr_erase(dev, addrs, 1, pmode, &ret);
    if(res < 0){
        printf("fail to erase\n");
        nvm_ret_pr(&ret);

        teardown();
        exit(-2);

    }
    e_end_t = get_time();
    pthread_mutex_unlock(&mutex);
}

int main(){
    struct nvm_ret ret;
    int pmode = NVM_FLAG_PMODE_SNGL; //set the option mode
    char *w_buf = NULL;
    ssize_t res;

    pthread_t pid_w;
    pthread_t pid_e;
    setup();
    struct nvm_addr w_addr[geo -> nsectors];
    for( int i = 0; i < geo -> nsectors; i++){
        w_addr[i].ppa = lun_addr.ppa;
        w_addr[i].g.pg = 0;
        w_addr[i].g.sec = i % geo->nsectors;
        w_addr[i].g.pl = 0;
        w_addr[i].g.blk = 10;
    }
    res = nvm_addr_erase(dev, w_addr, 1, pmode, &ret);
    if(res < 0){
        printf("fail to erase\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        teardown();
        exit(-2);

    }

    struct nvm_addr e_addr;
    e_addr.ppa = lun_addr.ppa;
    e_addr.g.pl = 1;
////////////////////////////////////////////////////////////////////////////////////
////                single lun different plane test                             ////
////////////////////////////////////////////////////////////////////////////////////
    printf("-----------------erase block and write a page at different plane------------------\n");
    pthread_create(&pid_w, NULL, (void *)write_pg, w_addr);
    pthread_create(&pid_e, NULL, (void *)erase_blk, &e_addr);
    sleep(1);
    flag = 0;
    pthread_cond_broadcast(&cond_l);
    pthread_join(pid_e, NULL);
    pthread_join(pid_w, NULL);
    printf("Write start time:%.6lf ms   end time:%.6lf ms\n", w_start_t * 1000, w_end_t * 1000);
    printf("Write time is:%.6lf ms\n", 1000 * (w_end_t - w_start_t));
    printf("Erase start time:%.6lf ms   end time:%.6lf ms\n", e_start_t * 1000, e_end_t * 1000);
    printf("Erase time is:%.6lf ms\n", 1000 * (e_end_t - e_start_t));

////////////////////////////////////////////////////////////////////////////////////
////                 different lun test                                         ////
////////////////////////////////////////////////////////////////////////////////////
    printf("-----------------erase block and write a page at different lun------------------\n");
    flag = 1;
    for( int i = 0; i < geo -> nsectors; i++){
        w_addr[i].ppa = lun_addr.ppa;
        w_addr[i].g.lun = 1;
        w_addr[i].g.pg = 0;
        w_addr[i].g.sec = i % geo->nsectors;
        w_addr[i].g.pl = 0;
        w_addr[i].g.blk = 10;
    }
    res = nvm_addr_erase(dev, w_addr, 1, pmode, &ret);
    if(res < 0){
        printf("fail to erase\n");
        nvm_ret_pr(&ret);
        free(w_buf);
        teardown();
        exit(-2);

    }

    e_addr.ppa = lun_addr.ppa;
    e_addr.g.pl = 0;

    pthread_create(&pid_w, NULL, (void *)write_pg, w_addr);
    pthread_create(&pid_e, NULL, (void *)erase_blk, &e_addr);
    sleep(1);
    flag = 0;
    pthread_cond_broadcast(&cond_l);
    pthread_join(pid_e, NULL);
    pthread_join(pid_w, NULL);
    printf("Write start time:%.6lf ms   end time:%.6lf ms\n", w_start_t * 1000, w_end_t * 1000);
    printf("Write time is:%.6lf ms\n", 1000 * (w_end_t - w_start_t));
    printf("Erase start time:%.6lf ms   end time:%.6lf ms\n", e_start_t * 1000, e_end_t * 1000);
    printf("Erase time is:%.6lf ms\n", 1000 * (e_end_t - e_start_t));


}

