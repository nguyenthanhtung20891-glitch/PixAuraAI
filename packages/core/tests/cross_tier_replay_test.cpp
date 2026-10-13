#include "certification/include/cross_tier_certification.h"
#include <cstdio>
int main(int argc,char** argv) {
    if(argc!=2)return 1;
    const auto result=pixaura_test_cross_tier_replay(argv[1],0);
    if(result)std::fprintf(stderr,"cross-tier certification failed at %d\n",result);
    return result?1:0;
}
