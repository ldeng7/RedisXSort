#include "common.h"

void solverSortHeap(Solver* self, bool heapified) {
    self->sortAsc = !self->sortAsc;
    if (!heapified) {
        solverHeapify(self);
    }
    i64 l = self->listLen;
    for (i64 i = l - 1; i > 0; --i) {
        solverHeapRemove(self, 0);
    }
    self->sortAsc = !self->sortAsc;
    self->listLen = l;
}

void solverSortPartial(Solver* self, i64 n) {
    self->sortAsc = !self->sortAsc;
    i64 l = self->listLen;
    self->listLen = n;
    solverHeapify(self);
    self->sortAsc = !self->sortAsc;

    for (i64 i = n; i < l; ++i) {
        if (solverArrElemLessByIndex(self, i, 0)) {
            solverArrElemSwap(self, i, 0);
            solverHeapFix(self, 0);
        }
    }
    solverSortHeap(self, true);
    self->listLen = l;
}

int commandSortHeap(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    Solver solver;
    Solver* ps = &solver;
    if (!solverInit(ps, ctx, argv, argc, 5)) {
        goto end;
    }
    solverListRead(ps);
    solverSortHeap(ps, false);
    solverCommit(ps);

end:
    solverDeinit(ps);
    return REDISMODULE_OK;
}

int commandSortPartial(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    Solver solver;
    Solver* ps = &solver;
    i64 n = 0;
    if (!solverInit(ps, ctx, argv, argc, 6)) {
        goto end;
    }
    if (RedisModule_StringToLongLong(argv[3], &n) != REDISMODULE_OK || n > solver.listLen) {
        RedisModule_ReplyWithError(solver.redisCtx, errInvalidIndex);
        goto end;
    }
    solverListRead(ps);
    solverSortPartial(ps, n);
    solverCommit(ps);

end:
    solverDeinit(ps);
    return REDISMODULE_OK;
}
