/*

   Copyright (c) 2011, Willem-Hendrik Thiart
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
 * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "linked_list_hashmap.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* when we call for more capacity */
#define SPACERATIO 0.5

typedef struct node_s node_t;

struct node_s
{
    ll_hashmap_entry_t ety;
    node_t *next;
};

static void __ensurecapacity(
    ll_hashmap_t * h
    );

/**
 * Allocate memory for nodes. Used for chained nodes. */
static node_t *__allocnodes(
    unsigned int count
    )
{
    // FIXME: make a chain node reservoir
    return calloc(count, sizeof(node_t));
}

ll_hashmap_t *ll_hashmap_new(
    func_longhash_f hash,
    func_longcmp_f cmp,
    unsigned int initial_capacity
    )
{
    ll_hashmap_t *h = calloc(1, sizeof(ll_hashmap_t));
    h->arraySize = initial_capacity;
    h->array = __allocnodes(h->arraySize);
    h->hash = hash;
    h->compare = cmp;
    return h;
}

int ll_hashmap_count(const ll_hashmap_t * h)
{
    return h->count;
}

int ll_hashmap_size(
    ll_hashmap_t * h
    )
{
    return h->arraySize;
}

/**
 * free all the nodes in a chain, recursively. */
static void __node_empty(ll_hashmap_t * h, node_t * node)
{
    if (node)
    {
        __node_empty(h, node->next);
        free(node);
        h->count--;
    }
}

void ll_hashmap_clear(ll_hashmap_t * h)
{
    int ii;

    for (ii = 0; ii < h->arraySize; ii++)
    {
        node_t *node = &((node_t*)h->array)[ii];

        if (NULL == node->ety.key)
            continue;

        /* normal actions will overwrite the value */
        node->ety.key = NULL;

        /* empty and free this chain */
        __node_empty(h, node->next);
        node->next = NULL;

        h->count--;
        assert(0 <= h->count);
    }

    assert(0 == ll_hashmap_count(h));
}

void ll_hashmap_free(ll_hashmap_t * h)
{
    assert(h);
    ll_hashmap_clear(h);
    free(h->array);
}

void ll_hashmap_freeall(ll_hashmap_t * h)
{
    assert(h);
    ll_hashmap_free(h);
    free(h);
}

inline static unsigned int __do_probe(ll_hashmap_t * h, const void *key)
{
    return h->hash(key) % h->arraySize;
}

void *ll_hashmap_get(
    ll_hashmap_t * h,
    const void *key
    )
{
    if (0 == ll_hashmap_count(h) || !key)
        return NULL;

    unsigned int probe = __do_probe(h, key);
    node_t *node = &((node_t*)h->array)[probe];

    if (NULL == node->ety.key)
        return NULL; /* we don't have this item */
    else
    {
        /* iterate down the node's linked list chain */
        do
            if (0 == h->compare(key, node->ety.key))
                return (void*)node->ety.val;
        while ((node = node->next));
    }

    return NULL;
}

int ll_hashmap_contains_key(
    ll_hashmap_t * h,
    const void *key
    )
{
    return NULL != ll_hashmap_get(h, key);
}

void ll_hashmap_remove_entry(
    ll_hashmap_t * h,
    ll_hashmap_entry_t * entry,
    const void *key
    )
{
    node_t *n, *n_parent;

    n = &((node_t*)h->array)[__do_probe(h, key)];

    if (!n->ety.key)
        goto notfound;

    n_parent = NULL;

    do
    {
        if (0 != h->compare(key, n->ety.key))
        {
            /* does not match, traverse the chain.. */
            n_parent = n;
            n = n->next;
            continue;
        }

        memcpy(entry, &n->ety, sizeof(ll_hashmap_entry_t));

        /* I am not a chain node */
        if (!n_parent)
        {
            /* I have a node on my chain. This node will replace me */
            if (n->next)
            {
                node_t *tmp = n->next;
                memcpy(&n->ety, &tmp->ety, sizeof(ll_hashmap_entry_t));
                /* Replace me with my next on chain */
                n->next = tmp->next;
                free(tmp);
            }
            else
                /* un-assign */
                n->ety.key = NULL;
        }
        else
        {
            /* Replace me with my next on chain */
            n_parent->next = n->next;
            free(n);
        }

        h->count--;
        return;

    }
    while (n);

notfound:
    entry->key = NULL;
    entry->val = NULL;
}

void *ll_hashmap_remove(ll_hashmap_t * h, const void *key)
{
    ll_hashmap_entry_t entry;
    ll_hashmap_remove_entry(h, &entry, key);
    return (void*)entry.val;
}

inline static void __nodeassign(
    ll_hashmap_t * h,
    node_t * node,
    void *key,
    void *val
    )
{
    /* Don't double increment if we are replacing an item */
    if (!node->ety.key)
        h->count++;

    assert(h->count < 32768);
    node->ety.key = key;
    node->ety.val = val;
}

void *ll_hashmap_put(ll_hashmap_t * h, void *key, void *val_new)
{
    if (!key || !val_new)
        return NULL;

    assert(key);
    assert(val_new);
    assert(h->array);

    __ensurecapacity(h);

    node_t *node = &((node_t*)h->array)[__do_probe(h, key)];

    assert(node);

    /* this one wasn't assigned */
    if (NULL == node->ety.key)
        __nodeassign(h, node, key, val_new);
    else
    {
        /* check the linked list */
        do
        {
            /* if same key, then we are just replacing val */
            if (0 == h->compare(key, node->ety.key))
            {
                void *val_prev = node->ety.val;
                node->ety.val = val_new;
                return val_prev;
            }
        }
        while (node->next && (node = node->next));

        node->next = __allocnodes(1);
        __nodeassign(h, node->next, key, val_new);
    }

    return NULL;
}

void ll_hashmap_put_entry(ll_hashmap_t * h, ll_hashmap_entry_t * entry)
{
    ll_hashmap_put(h, entry->key, entry->val);
}

void ll_hashmap_increase_capacity(ll_hashmap_t * h, unsigned int factor)
{
    node_t *array_old;
    int ii, asize_old;

    /*  stored old array */
    array_old = h->array;
    asize_old = h->arraySize;

    /*  double array capacity */
    h->arraySize *= factor;
    h->array = __allocnodes(h->arraySize);
    h->count = 0;

    for (ii = 0; ii < asize_old; ii++)
    {
        node_t *node = &((node_t*)array_old)[ii];

        /*  if key is null */
        if (NULL == node->ety.key)
            continue;

        ll_hashmap_put(h, node->ety.key, node->ety.val);

        /* re-add chained hash nodes */
        node = node->next;

        while (node)
        {
            node_t *next = node->next;
            ll_hashmap_put(h, node->ety.key, node->ety.val);
            assert(NULL != node->ety.key);
            free(node);
            node = next;
        }
    }

    free(array_old);
}

static void __ensurecapacity(ll_hashmap_t * h)
{
    if ((float)h->count / h->arraySize < SPACERATIO)
        return;
    else
        ll_hashmap_increase_capacity(h, 2);
}

void* ll_hashmap_iterator_peek(
    ll_hashmap_t * h,
    ll_hashmap_iterator_t * iter
    )
{
    if (NULL == iter->cur_linked)
    {
        for (; iter->cur < h->arraySize; iter->cur++)
        {
            node_t *node = &((node_t*)h->array)[iter->cur];

            if (node->ety.key)
                return node->ety.key;
        }

        return NULL;
    }
    else
    {
        node_t *node = iter->cur_linked;
        return node->ety.key;
    }
}

void* ll_hashmap_iterator_peek_value(ll_hashmap_t * h, ll_hashmap_iterator_t * iter)
{
    return ll_hashmap_get(h, ll_hashmap_iterator_peek(h, iter));
}

int ll_hashmap_iterator_has_next(ll_hashmap_t * h, ll_hashmap_iterator_t * iter)
{
    return NULL != ll_hashmap_iterator_peek(h, iter);
}

void *ll_hashmap_iterator_next_value(ll_hashmap_t * h, ll_hashmap_iterator_t * iter)
{
    void* k = ll_hashmap_iterator_next(h, iter);
    if (!k)
        return NULL;
    return ll_hashmap_get(h, k);
}

void *ll_hashmap_iterator_next(ll_hashmap_t * h, ll_hashmap_iterator_t * iter)
{
    assert(iter);

    node_t *n = iter->cur_linked;

    /* if we have a node ready to look at on the chain.. */
    if (n)
    {
        node_t *n_parent = &((node_t*)h->array)[iter->cur];

        /* check that we aren't following a dangling pointer.
         * There is a chance that cur_linked is now on the array. */
        if (!n_parent->next)
        {
            n = n_parent;
            iter->cur_linked = NULL;
            iter->cur++;
        }
        else
        {
            /*  it's safe to increment */
            if (NULL == n->next)
                iter->cur++;
            iter->cur_linked = n->next;
        }
        return n->ety.key;
    }
    /*  otherwise check if we have a node to look at */
    else
    {
        for (; iter->cur < h->arraySize; iter->cur++)
        {
            n = &((node_t*)h->array)[iter->cur];

            if (n->ety.key)
                break;
        }

        /*  exit if we are at the end */
        if (h->arraySize == iter->cur)
            return NULL;

        n = &((node_t*)h->array)[iter->cur];

        if (n->next)
            iter->cur_linked = n->next;
        else
            /*  Only increment if we aren't following the chain.
             *  This is so that if the user is removing while iterating
             *  we don't follow the dangling iter->cur_linked pointer
             *  if the node got placed on the array. */
            iter->cur += 1;

        return n->ety.key;
    }
}

void ll_hashmap_iterator(
    ll_hashmap_t * h,
    ll_hashmap_iterator_t * iter
    )
{
    (void*)h;
    iter->cur = 0;
    iter->cur_linked = NULL;
}

#if defined(__cplusplus)
} // extern "C" {
#endif
