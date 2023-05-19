#include <string.h>
#include "common.h"

const char* errInvalidSortConfig = "ERR invalid argument sortConfig";
const char* errInvalidKeyType = "ERR invalid key type or key not exist";
const char* errInvalidIndex = "ERR invalid index";

static bool readSortConfig(Solver* self, const RedisModuleString* str) {
    usize len;
    const char* cStr = RedisModule_StringPtrLen(str, &len);
    if (len != 3) {
        RedisModule_ReplyWithError(self->redisCtx, errInvalidSortConfig);
        return false;
    }

    switch (cStr[0]) {
    case 'l':
        self->sortValueType = SortValueTypeInt64; break;
    case 'u':
        self->sortValueType = SortValueTypeUint64; break;
    case 'd':
        self->sortValueType = SortValueTypeDouble; break;
    default:
        RedisModule_ReplyWithError(self->redisCtx, errInvalidSortConfig);
        return false;
    }
    switch (cStr[1]) {
    case 'a':
        self->sortAsc = true; break;
    case 'd':
        self->sortAsc = false; break;
    default:
        RedisModule_ReplyWithError(self->redisCtx, errInvalidSortConfig);
        return false;
    }
    switch (cStr[2]) {
    case 'l':
        self->sortKeyType = REDISMODULE_KEYTYPE_LIST; break;
    case 'h':
        self->sortKeyType = REDISMODULE_KEYTYPE_HASH; break;
    default:
        RedisModule_ReplyWithError(self->redisCtx, errInvalidSortConfig);
        return false;
    }

    return true;
}

bool solverInit(Solver* self, RedisModuleCtx* redisCtx, RedisModuleString** argv, int argc, int fullArgc) {
    memset(self, 0, sizeof(Solver));
    self->redisCtx = redisCtx;

    if (argc < fullArgc - 1) {
        RedisModule_WrongArity(self->redisCtx);
        return false;
    }

    if (!readSortConfig(self, argv[fullArgc - 2])) {
        return false;
    }

    self->listKey = RedisModule_OpenKey(self->redisCtx, argv[1], REDISMODULE_READ);
    if (RedisModule_KeyType(self->listKey) != REDISMODULE_KEYTYPE_LIST) {
        RedisModule_ReplyWithError(self->redisCtx, errInvalidKeyType);
        return false;
    }
    self->listLen = RedisModule_ValueLength(self->listKey);
    self->listOutKey = RedisModule_OpenKey(self->redisCtx, argv[2], REDISMODULE_WRITE);
    if (RedisModule_KeyType(self->listOutKey) != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_ReplyWithError(self->redisCtx, "fuckkkk");//ldeng7 errInvalidKeyType);
        return false;
    }

    switch (self->sortKeyType) {
    case REDISMODULE_KEYTYPE_HASH:
        if (argc < fullArgc) {
            RedisModule_WrongArity(self->redisCtx);
            return false;
        }
        self->sortHashkey = RedisModule_OpenKey(self->redisCtx, argv[fullArgc - 1], REDISMODULE_READ);
        if (RedisModule_KeyType(self->sortHashkey) != REDISMODULE_KEYTYPE_HASH) {
            RedisModule_ReplyWithError(self->redisCtx, errInvalidKeyType);
            return false;
        }
        break;
    }

    self->arr = RedisModule_Alloc(sizeof(SolverArrElem) * (usize)self->listLen);
    for (i64 i = 0; i < self->listLen; ++i) {
        SolverArrElem* elem = self->arr + i;
        memset(elem, 0, sizeof(SolverArrElem));
        elem->srcIndex = i;
    }

    return true;
}

void solverDeinit(Solver* self) {
    if (self->listKey != NULL) {
        RedisModule_CloseKey(self->listKey);
        self->listKey = NULL;
    }
    if (self->listOutKey != NULL) {
        RedisModule_CloseKey(self->listOutKey);
        self->listOutKey = NULL;
    }
    if (self->sortHashkey != NULL) {
        RedisModule_CloseKey(self->sortHashkey);
        self->sortHashkey = NULL;
    }
    if (self->arr != NULL) {
        RedisModule_Free(self->arr);
        self->arr = NULL;
    }
}

static SortValue solverArrElemGetValue(Solver* self, SolverArrElem* elem) {
    if (elem->valueCached) {
        return elem->value;
    }
    elem->valueCached = true;

    RedisModuleString* listValueStr = RedisModule_ListGet(self->listKey, (long)elem->srcIndex);
    RedisModuleString* valueStr = NULL;
    switch (self->sortKeyType) {
    case REDISMODULE_KEYTYPE_LIST:
        valueStr = listValueStr;
        break;
    case REDISMODULE_KEYTYPE_HASH:
        RedisModule_HashGet(self->sortHashkey, REDISMODULE_HASH_NONE, listValueStr, &valueStr, NULL);
        break;
    }
    if (valueStr == NULL) {
        goto end;
    }

    switch (self->sortValueType) {
    case SortValueTypeInt64:
        RedisModule_StringToLongLong(valueStr, &(elem->value.i));
        break;
    case SortValueTypeUint64:
        RedisModule_StringToULongLong(valueStr, &(elem->value.u));
        break;
    case SortValueTypeDouble:
        RedisModule_StringToDouble(valueStr, &(elem->value.d));
        break;
    }

    switch (self->sortKeyType) {
    case REDISMODULE_KEYTYPE_HASH:
        RedisModule_FreeString(self->redisCtx, valueStr);
        break;
    }

end:
    RedisModule_FreeString(self->redisCtx, listValueStr);
    return elem->value;
}

void solverListRead(Solver* self) {
    SolverArrElem* elem = self->arr;
    for (i64 i = 0; i < self->listLen; ++i, ++elem) {
        solverArrElemGetValue(self, elem);
    }
}

bool solverArrElemLess(Solver* self, SolverArrElem* a, SolverArrElem* b) {
    switch (self->sortValueType) {
    case SortValueTypeInt64:
        if (self->sortAsc) {
            return a->value.i < b->value.i;
        } else {
            return a->value.i > b->value.i;
        }
    case SortValueTypeUint64:
        if (self->sortAsc) {
            return a->value.u < b->value.u;
        } else {
            return a->value.u > b->value.u;
        }
    case SortValueTypeDouble:
        if (self->sortAsc) {
            return a->value.d < b->value.d;
        } else {
            return a->value.d > b->value.d;
        }
    }
    RedisModule__Assert("impossible", __FILE__, __LINE__);
    return false;
}

bool solverArrElemLessByIndex(Solver* self, i64 i, i64 j) {
    return solverArrElemLess(self, self->arr + i, self->arr + j);
}

void solverArrElemSwap(Solver* self, i64 i, i64 j) {
    SolverArrElem* a = self->arr + i;
    SolverArrElem* b = self->arr + j;
    SolverArrElem t;
    memcpy(&t, a, sizeof(SolverArrElem));
    memcpy(a, b, sizeof(SolverArrElem));
    memcpy(b, &t, sizeof(SolverArrElem));
}

void solverCommit(Solver* self) {
    for (i64 i = 0; i < self->listLen; ++i) {
        RedisModuleString* listValueStr = RedisModule_ListGet(self->listKey, (long)self->arr[i].srcIndex);
        RedisModule_ListPush(self->listOutKey, REDISMODULE_LIST_TAIL, listValueStr);
        RedisModule_FreeString(self->redisCtx, listValueStr);
    }
}
