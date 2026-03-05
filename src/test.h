#pragma once

struct __m128 {
    float v[4];
};
inline __m128 _mm_set1_ps(float f){
    return __m128{{f,f,f,f}};
}