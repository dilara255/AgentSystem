#pragma once

#include "miscStdHeaders.h"

namespace AZ{
    //on test system tends to about 13,5 nanos per prn + float mult + assignment for large n
	void draw4spcg32s(uint64_t* s0, uint64_t* s1, uint64_t* s2, uint64_t* s3,
                      uint32_t* dest0, uint32_t* dest1, uint32_t* dest2, uint32_t* dest3){ 
        uint64_t m = 0x9b60933458e17d7d;
        uint64_t a = 0xd737232eeccdf7ed;

        *s0 = *s0 * m + a;
        *s1 = *s1 * m + a;
        *s2 = *s2 * m + a;
        *s3 = *s3 * m + a;
        
        int shift0 = 29 - (*s0 >> 61);
        int shift1 = 29 - (*s1 >> 61);
        int shift2 = 29 - (*s2 >> 61);
        int shift3 = 29 - (*s3 >> 61); 

        *dest0 = *s0 >> shift0;
        *dest1 = *s1 >> shift1;
        *dest2 = *s2 >> shift2;
        *dest3 = *s3 >> shift3;
    }


    //from https://nullprogram.com/blog/2017/09/21/
    //on test system tends to about 16,5 nanos per prn + float mult + assignment for large n
	uint32_t spcg32(uint64_t* s) {
        uint64_t m = 0x9b60933458e17d7d;
        uint64_t a = 0xd737232eeccdf7ed;
        *s = *s * m + a;
        int shift = 29 - (*s >> 61);
        return *s >> shift;
    }

}


