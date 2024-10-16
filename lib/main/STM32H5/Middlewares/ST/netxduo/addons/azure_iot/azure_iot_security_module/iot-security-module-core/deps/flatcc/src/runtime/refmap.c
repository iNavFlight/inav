/*
 * Optional file that can be included in runtime library to support DAG
 * cloning with the builder and may also be used for custom purposes
 * standalone. See also comments in `flatcc/flatcc_builder.h`.
 *
 * Note that dynamic construction takes place and that large offset
 * vectors might consume significant space if there are not many shared
 * references. In the basic use case no allocation takes place because a
 * few references can be held using only a small stack allocated hash
 * table.
 */

#include <stdlib.h>
#include <string.h>

#include "flatcc/flatcc_rtconfig.h"
#include "flatcc/flatcc_refmap.h"
#include "flatcc/flatcc_alloc.h"
#include "flatcc/flatcc_assert.h"

#define _flatcc_refmap_calloc FLATCC_CALLOC
#define _flatcc_refmap_free FLATCC_FREE

/* Can be used as a primitive defense against collision attacks. */
#ifdef FLATCC_HASH_SEED
#define _flatcc_refmap_seed FLATCC_HASH_SEED
#else
#define _flatcc_refmap_seed 0x2f693b52
#endif

static inline size_t _flatcc_refmap_above_load_factor(size_t count, size_t buckets)
{
    static const size_t d = 256;
    static const size_t n = (size_t)((FLATCC_REFMAP_LOAD_FACTOR) * 256.0f);

    return count >= buckets * n / d;
}

#define _flatcc_refmap_probe(k, i, N) ((k + i) & N)

void flatcc_refmap_clear(flatcc_refmap_t *refmap)
{
    if (refmap->table && refmap->table != refmap->min_table) {
        _flatcc_refmap_free(refmap->table);
    }
    flatcc_refmap_init(refmap);
}

static inline size_t _flatcc_refmap_hash(const void *src)
{
    /* MurmurHash3 64-bit finalizer */
    uint64_t x;

    x = (uint64_t)((size_t)src) ^ _flatcc_refmap_seed;

    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return (size_t)x;
}

void flatcc_refmap_reset(flatcc_refmap_t *refmap)
{
    if (refmap->count) {
        memset(refmap->table, 0, sizeof(refmap->table[0]) * refmap->buckets);
    }
    refmap->count = 0;
}

/*
 * Technically resize also supports shrinking which may be useful for
 * adapations, but the current hash table never deletes individual items.
 */
int flatcc_refmap_resize(flatcc_refmap_t *refmap, size_t count)
{
    const size_t min_buckets = sizeof(refmap->min_table) / sizeof(refmap->min_table[0]);

    size_t i;
    size_t buckets;
    size_t buckets_old;
    struct flatcc_refmap_item *T_old;

    if (count < refmap->count) {
        count = refmap->count;
    }
    buckets = min_buckets;

    while (_flatcc_refmap_above_load_factor(count, buckets)) {
        buckets *= 2;
    }
    if (refmap->buckets == buckets) {
        return 0;
    }
    T_old = refmap->table;
    buckets_old = refmap->buckets;
    if (buckets == min_buckets) {
        memset(refmap->min_table, 0, sizeof(refmap->min_table));
        refmap->table = refmap->min_table;
    } else {
        refmap->table = _flatcc_refmap_calloc(buckets, sizeof(refmap->table[0]));
        if (refmap->table == 0) {
            refmap->table = T_old;
            FLATCC_ASSERT(__SET_ASSERT__); /* out of memory */
            return -1;
        }
    }
    refmap->buckets = buckets;
    refmap->count = 0;
    for (i = 0; i < buckets_old; ++i) {
        if (T_old[i].src) {
            flatcc_refmap_insert(refmap, T_old[i].src, T_old[i].ref);
        }
    }
    if (T_old && T_old != refmap->min_table) {
        _flatcc_refmap_free(T_old);
    }
    return 0;
}

flatcc_refmap_ref_t flatcc_refmap_insert(flatcc_refmap_t *refmap, const void *src, flatcc_refmap_ref_t ref)
{
    struct flatcc_refmap_item *T;
    size_t N, i, j, k;

    if (src == 0) return ref;
    if (_flatcc_refmap_above_load_factor(refmap->count, refmap->buckets)) {
        if (flatcc_refmap_resize(refmap, refmap->count * 2)) {
            return flatcc_refmap_not_found; /* alloc failed */
        }
    }
    T = refmap->table;
    N = refmap->buckets - 1;
    k = _flatcc_refmap_hash(src);
    i = 0;
    j = _flatcc_refmap_probe(k, i, N);
    while (T[j].src) {
        if (T[j].src == src) {
            return T[j].ref = ref;
        }
        ++i;
        j = _flatcc_refmap_probe(k, i, N);
    }
    ++refmap->count;
    T[j].src = src;
    return T[j].ref = ref;
}

flatcc_refmap_ref_t flatcc_refmap_find(flatcc_refmap_t *refmap, const void *src)
{
    struct flatcc_refmap_item *T;
    size_t N, i, j, k;

    if (refmap->count == 0) {
        return flatcc_refmap_not_found;
    }
    T = refmap->table;
    N = refmap->buckets - 1;
    k = _flatcc_refmap_hash(src);
    i = 0;
    j = _flatcc_refmap_probe(k, i, N);
    while (T[j].src) {
        if (T[j].src == src) return T[j].ref;
        ++i;
        j = _flatcc_refmap_probe(k, i, N);
    }
    return flatcc_refmap_not_found;
}

/*
 * To run test from project root:
 *
 *  cc -D FLATCC_REFMAP_TEST -I include src/runtime/refmap.c -o test_refmap && ./test_refmap
 *
 */
#ifdef FLATCC_REFMAP_TEST

#include <stdio.h>

#ifndef FLATCC_REFMAP_H
#include "flatcc/flatcc_refmap.h"
#endif

#define test(x) do { if (!(x)) { fprintf(stderr, "%02d: refmap test failed\n", __LINE__); exit(-1); } } while (0)
#define test_start() fprintf(stderr, "starting refmap test ...\n")
#define test_ok() fprintf(stderr, "refmap test succeeded\n")

int main()
{
    int i;
    int data[1000];
    int a = 1;
    int b = 2;
    int c = 3;
    flatcc_refmap_t refmap;

    flatcc_refmap_init(&refmap);

    test(flatcc_refmap_find(&refmap, &a) == flatcc_refmap_not_found);
    test(flatcc_refmap_find(&refmap, &b) == flatcc_refmap_not_found);
    test(flatcc_refmap_find(&refmap, &c) == flatcc_refmap_not_found);
    test(flatcc_refmap_find(&refmap, 0) == flatcc_refmap_not_found);
    test(flatcc_refmap_find(&refmap, &a) == 0);

    test(flatcc_refmap_insert(&refmap, &a, 42) == 42);
    test(flatcc_refmap_find(&refmap, &a) == 42);
    test(flatcc_refmap_find(&refmap, &b) == flatcc_refmap_not_found);
    test(flatcc_refmap_find(&refmap, &c) == flatcc_refmap_not_found);
    test(flatcc_refmap_insert(&refmap, &a, 42) == 42);
    test(flatcc_refmap_find(&refmap, &a) == 42);
    test(refmap.count == 1);
    test(flatcc_refmap_insert(&refmap, &a, 43) == 43);
    test(flatcc_refmap_find(&refmap, &a) == 43);
    test(refmap.count == 1);
    test(flatcc_refmap_insert(&refmap, &b, -10) == -10);
    test(flatcc_refmap_insert(&refmap, &c, 100) == 100);
    test(refmap.count == 3);
    test(flatcc_refmap_find(&refmap, &a) == 43);
    test(flatcc_refmap_find(&refmap, &b) == -10);
    test(flatcc_refmap_find(&refmap, &c) == 100);

    test(flatcc_refmap_insert(&refmap, 0, 1000) == 1000);
    test(flatcc_refmap_find(&refmap, 0) == 0);
    test(refmap.count == 3);

    test(flatcc_refmap_insert(&refmap, &b, 0) == 0);
    test(flatcc_refmap_find(&refmap, &b) == 0);
    test(refmap.count == 3);

    flatcc_refmap_reset(&refmap);
    test(refmap.count == 0);
    test(refmap.buckets > 0);
    for (i = 0; i < 1000; ++i) {
        test(flatcc_refmap_insert(&refmap, data + i, i + 42) == i + 42);
    }
    test(refmap.count == 1000);
    for (i = 0; i < 1000; ++i) {
        test(flatcc_refmap_find(&refmap, data + i) == i + 42);
    }
    flatcc_refmap_clear(&refmap);
    test(refmap.count == 0);
    test(refmap.buckets == 0);
    test_ok();
    return 0;
}

#endif /* FLATCC_REFMAP_TEST */
