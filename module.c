#include "common.h"

extern int commandHeapify(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
extern int commandHeapFix(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
extern int commandHeapRemove(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
extern int commandSortHeap(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
extern int commandSortPartial(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
extern int commandNth(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

typedef struct CommandTableItem {
    RedisModuleCmdFunc func;
    const char* name;
} CommandTableItem;

static CommandTableItem commandTable[] = {
    { commandHeapify, "xsort.heap.heapify" },
    { commandHeapFix, "xsort.heap.fix" },
    { commandHeapRemove, "xsort.heap.remove" },
    { commandSortHeap, "xsort.sort.heap" },
    { commandSortPartial, "xsort.sort.partial" },
    { commandNth, "xsort.nth" },
};

int RedisModule_OnLoad(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    if (RedisModule_Init(ctx, "xsort", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }
    for (usize i = 0; i < sizeof(commandTable) / sizeof(CommandTableItem); ++i) {
        if (RedisModule_CreateCommand(ctx, commandTable[i].name, commandTable[i].func,
            "write no-cluster", 1, 2, 1) == REDISMODULE_ERR) {
            return REDISMODULE_ERR;
        }
    }
    return REDISMODULE_OK;
}//ldeng7: -shared -W -Wall -fno-common -fPIC -std=c17 -std=c++20 -O2
