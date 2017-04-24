#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <liblightnvm.h>

int main()
{
    struct nvm_addr addr = {.ppa=1};
    nvm_addr_pr(addr);
    return 1;
}
