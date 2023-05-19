#pragma once

//#include <stdint.h>
#include <stdbool.h>
#include "redismodule.h"

//typedef char bool;
//static const bool true = 1;
//static const bool false = 0;
typedef long long i64;
typedef unsigned long long u64;
typedef size_t usize;

extern const char* errInvalidSortConfig;
extern const char* errInvalidKeyType;
extern const char* errInvalidIndex;

typedef enum SortValueType {
    SortValueTypeInt64,
    SortValueTypeUint64,
    SortValueTypeDouble,
} SortValueType;

typedef union SortValue {
    i64 i;
    u64 u;
    double d;
} SortValue;

typedef struct SolverArrElem {
    i64 srcIndex;
    bool valueCached;
    SortValue value;
} SolverArrElem;

typedef struct Solver {
    RedisModuleCtx* redisCtx;
    SortValueType sortValueType;
    bool sortAsc;
    int sortKeyType;

    RedisModuleKey* listKey;
    i64 listLen;
    RedisModuleKey* listOutKey;
    RedisModuleKey* sortHashkey;
    SolverArrElem* arr;
} Solver;

extern bool solverInit(Solver* self, RedisModuleCtx* redisCtx, RedisModuleString** argv, int argc, int fullArgc);
extern void solverDeinit(Solver* self);
extern void solverListRead(Solver* self);
extern bool solverArrElemLess(Solver* self, SolverArrElem* a, SolverArrElem* b);
extern bool solverArrElemLessByIndex(Solver* self, i64 i, i64 j);
extern void solverArrElemSwap(Solver* self, i64 i, i64 j);
extern void solverCommit(Solver* self);

extern void solverHeapify(Solver* self);
extern void solverHeapFix(Solver* self, i64 index);
extern void solverHeapRemove(Solver* self, i64 index);
