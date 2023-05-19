#include <algorithm>

extern "C" {
#include "common.h"

void solverNth(Solver* self, i64 n) {
    std::nth_element(self->arr, self->arr + n, self->arr + self->listLen, [self](auto& a, auto& b) {
        return solverArrElemLess(self, &a, &b);
    });
}

int commandNth(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
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
    solverNth(ps, n);
    solverCommit(ps);
    RedisModule_ReplyWithSimpleString(ps->redisCtx, "OK");
end:
    solverDeinit(ps);
    return REDISMODULE_OK;
}

}
