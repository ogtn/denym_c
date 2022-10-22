#ifndef _resource_cache_h_
#define _resource_cache_h_


#include "denym_common.h"


typedef struct resourceCache_t *resourceCache;


resourceCache resourceCacheCreate(void);

void resourceCacheDestroy(resourceCache cache);

void *resourceCacheGet(resourceCache cache, char *key);

void resourceCacheAdd(resourceCache cache, char *key, void *resource);

void resourceCacheRemove(resourceCache cache, char *key, VkBool32 *needDestruction);


#endif
