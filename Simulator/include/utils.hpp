#pragma once
#include<bits/stdc++.h>
#include <stdlib.h>
using namespace std;
#define ALWAYS_INLINE inline __attribute__((always_inline)) 
#define UNROLL __attribute__((optimize("unroll-loops")))
#define NOINLINE __attribute__((noinline))
#define vector_size 4
#define all(x) begin(x), end(x)
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;

using i_ptr = intptr_t;
using u_ptr = uintptr_t;

#define REP(i,a,b) for(int i=(a),i##_end_=(b);i<i##_end_;++i)
#define REP_EQ(i,a,b) for(int i=(a),i##_end_=(b);i<=i##_end_;++i)
#define DREP(i,a,b) for(int i=(a),i##_end_=(b);i>i##_end_;--i)
#define DREP_EQ(i,a,b) for(int i=(a),i##_end_=(b);i>=i##_end_;--i)
#define __lg(x) (31 - __builtin_clz(x))
