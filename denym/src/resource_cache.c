#include "resource_cache.h"
#include "texture.h"
#include "logger.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include <stb_ds.h>

#pragma clang diagnostic pop


typedef struct resourceCacheValue
{
    void *ptr;
    uint32_t refCount;
    char name[FILENAME_MAX];
} resourceCacheValue;


typedef struct resourceCacheEntry
{
    size_t key;
    resourceCacheValue value;
} resourceCacheEntry;


typedef struct resourceCache_t
{
    resourceCacheEntry *hashmap;
} resourceCache_t;


static const size_t seed = 0xBADC0FFEE0DDF00D;


resourceCache resourceCacheCreate(void)
{
    resourceCache cache = malloc(sizeof *cache);
    cache->hashmap = NULL;

    return cache;
}


void resourceCacheDestroy(resourceCache cache)
{
    ptrdiff_t len = hmlen(cache->hashmap);

    for(ptrdiff_t i = 0; i < len; i++)
    {
        logWarning("Cache still containing resource \"%s\" (%u references)",
            cache->hashmap[i].value.name, cache->hashmap[i].value.refCount);
    }

    free(cache);
}


void *resourceCacheGet(resourceCache cache, char *key)
{
    size_t k = stbds_hash_string(key, seed);
    ptrdiff_t idx = hmgeti(cache->hashmap, k);

    if(idx == -1)
        return NULL;

    cache->hashmap[idx].value.refCount++;
    return cache->hashmap[idx].value.ptr;
}


void resourceCacheAdd(resourceCache cache, char *key, void *resource)
{
    size_t k = stbds_hash_string(key, seed);

    resourceCacheValue value = { .ptr = resource, .refCount = 1 };
    strncpy(value.name, key, FILENAME_MAX);
    hmput(cache->hashmap, k, value);
}


void resourceCacheRemove(resourceCache cache, char *key, VkBool32 *needDestruction)
{
    size_t k = stbds_hash_string(key, seed);
    ptrdiff_t idx = hmgeti(cache->hashmap, k);

    if(idx == -1)
    {
        logError("Trying to remove non cached element \"%s\"", key);

        return;
    }

    cache->hashmap[idx].value.refCount--;

    if(cache->hashmap[idx].value.refCount == 0)
    {
        *needDestruction = VK_TRUE;
        hmdel(cache->hashmap, k);
    }
    else
        *needDestruction = VK_FALSE;
}
