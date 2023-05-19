#include "common.h"

static void up(Solver* self, i64 i) {
    while (true) {
        i64 j = (i - 1) / 2;
        if (j == i || !solverArrElemLessByIndex(self, i, j)) {
            break;
        }
        solverArrElemSwap(self, i, j);
        i = j;
    }
}

static bool down(Solver* self, i64 i0) {
    i64 i = i0, l = self->listLen;
    while (true) {
        i64 j = i * 2 + 1;
        if (j >= l) {
            break;
        }
        if (j + 1 < l && solverArrElemLessByIndex(self, j + 1, j)) {
            ++j;
        }
        if (!solverArrElemLessByIndex(self, j, i)) {
            break;
        }
        solverArrElemSwap(self, i, j);
        i = j;
    }
    return i > i0;
}

void solverHeapify(Solver* self) {
    i64 l = self->listLen;
    for (i64 i = l / 2 - 1; i >= 0; --i) {
        down(self, i);
    }
}

void solverHeapFix(Solver* self, i64 index) {
    if (!down(self, index) && index > 0) {
        up(self, index);
    }
}

void solverHeapRemove(Solver* self, i64 index) {
    --(self->listLen);
    if (index != self->listLen) {
        solverArrElemSwap(self, index, self->listLen);
        solverHeapFix(self, index);
    }
}

// ldeng7: xsort.heap.heapify key keyOut sortConfig [sortKey]
int commandHeapify(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    Solver solver;
    Solver* ps = &solver;
    if (!solverInit(ps, ctx, argv, argc, 5)) {
        goto end;
    }
    solverListRead(ps);
    solverHeapify(ps);
    solverCommit(ps);

end:
    solverDeinit(ps);
    return REDISMODULE_OK;
}

// ldeng7: xsort.heap.fix key keyOut index sortConfig [sortKey]
int commandHeapFix(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    Solver solver;
    Solver* ps = &solver;
    i64 index = 0;
    if (!solverInit(ps, ctx, argv, argc, 6)) {
        goto end;
    }
    if (RedisModule_StringToLongLong(argv[3], &index) != REDISMODULE_OK || index >= solver.listLen) {
        RedisModule_ReplyWithError(solver.redisCtx, errInvalidIndex);
        goto end;
    }
    solverHeapFix(ps, index);
    solverCommit(ps);

end:
    solverDeinit(ps);
    return REDISMODULE_OK;
}

int commandHeapRemove(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    Solver solver;
    Solver* ps = &solver;
    i64 index = 0;
    if (!solverInit(ps, ctx, argv, argc, 6)) {
        goto end;
    }
    if (RedisModule_StringToLongLong(argv[3], &index) != REDISMODULE_OK || index >= solver.listLen) {
        RedisModule_ReplyWithError(solver.redisCtx, errInvalidIndex);
        goto end;
    }
    solverHeapRemove(ps, index);
    solverCommit(ps);

end:
    solverDeinit(ps);
    return REDISMODULE_OK;
}
