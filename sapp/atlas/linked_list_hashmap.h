#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

typedef unsigned long (*func_longhash_f) (const void *);

typedef long (*func_longcmp_f) (const void *, const void *);

typedef struct
{
    void *key;
    void *val;
} ll_hashmap_entry_t;

typedef struct
{
    int count;
    int arraySize;
    void *array;
    func_longhash_f hash;
    func_longcmp_f compare;
} ll_hashmap_t;

typedef struct
{
    int cur;
    void *cur_linked;
} ll_hashmap_iterator_t;

ll_hashmap_t *ll_hashmap_new(
    func_longhash_f hash,
    func_longcmp_f cmp,
    unsigned int initial_capacity
);

/**
 * @return number of items within hash */
int ll_hashmap_count(const ll_hashmap_t * hmap);

/**
 * @return size of the array used within hash */
int ll_hashmap_size(
    ll_hashmap_t * hmap
);

/**
 * Empty this hash. */
void ll_hashmap_clear(
    ll_hashmap_t * hmap
);

/**
 * Free all the memory related to this hash. */
void ll_hashmap_free(
    ll_hashmap_t * hmap
);

/**
 * Free all the memory related to this hash.
 * This includes the actual h itself. */
void ll_hashmap_freeall(
    ll_hashmap_t * hmap
);

/**
 * Get this key's value.
 * @return key's item, otherwise NULL */
void *ll_hashmap_get(
    ll_hashmap_t * hmap,
    const void *key
);

/**
 * Is this key inside this map?
 * @return 1 if key is in hash, otherwise 0 */
int ll_hashmap_contains_key(
    ll_hashmap_t * hmap,
    const void *key
);

/**
 * Remove the value refrenced by this key from the hash. */
void ll_hashmap_remove_entry(
    ll_hashmap_t * hmap,
    ll_hashmap_entry_t * entry,
    const void *key
);

/**
 * Remove this key and value from the map.
 * @return value of key, or NULL on failure */
void *ll_hashmap_remove(
    ll_hashmap_t * hmap,
    const void *key
);

/**
 * Associate key with val.
 * Does not insert key if an equal key exists.
 * @return previous associated val; otherwise NULL */
void *ll_hashmap_put(
    ll_hashmap_t * hmap,
    void *key,
    void *val
);

/**
 * Put this key/value entry into the hash */
void ll_hashmap_put_entry(
    ll_hashmap_t * hmap,
    ll_hashmap_entry_t * entry
);

void* ll_hashmap_iterator_peek(
    ll_hashmap_t * hmap,
    ll_hashmap_iterator_t * iter);

void* ll_hashmap_iterator_peek_value(
    ll_hashmap_t * hmap,
    ll_hashmap_iterator_t * iter);

int ll_hashmap_iterator_has_next(
    ll_hashmap_t * hmap,
    ll_hashmap_iterator_t * iter
);

/**
 * Iterate to the next item on a hash iterator
 * @return next item key from iterator */
void *ll_hashmap_iterator_next(
    ll_hashmap_t * hmap,
    ll_hashmap_iterator_t * iter
);

/**
 * Iterate to the next item on a hash iterator
 * @return next item value from iterator */
void *ll_hashmap_iterator_next_value(
    ll_hashmap_t * hmap,
    ll_hashmap_iterator_t * iter);

/**
 * Initialise a new hash iterator over this hash
 * It is safe to remove items while iterating.  */
void ll_hashmap_iterator(
    ll_hashmap_t * hmap,
    ll_hashmap_iterator_t * iter
);

/**
 * Increase hash capacity.
 * @param factor : increase by this factor */
void ll_hashmap_increase_capacity(
    ll_hashmap_t * hmap,
    unsigned int factor);

#if defined(__cplusplus)
} // extern "C" {
#endif
