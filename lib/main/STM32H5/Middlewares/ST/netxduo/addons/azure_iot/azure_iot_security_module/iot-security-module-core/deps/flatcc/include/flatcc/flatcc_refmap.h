/*
 * The flatcc builder supports storing a pointer to a refmap
 * and wraps some operations to make them work as a dummy
 * even if no refmap has been set. This enables optional
 * DAG preservation possible during clone operations.
 *
 * A refmap maps a source address to a builder reference.
 *
 * This is just a map, but the semantics are important:
 *
 * The map thus preserves identity of the source. It is not a
 * cache because cache eviction would fail to properly track
 * identity.
 *
 * The map is used for memoization during object cloning are and
 * may also be used by user logic doing similar operations.
 * This ensures that identity is preserved so a source object is
 * not duplicated which could lead to either loss of semantic
 * information, or an explosion in size, or both. In some, or
 * even most, cases this concern may not be important, but when
 * it is important, it is important.
 *
 * The source address must not be reused for different content
 * for the lifetime of the map, although the content doest not
 * have to be valid or event exist at that location since source
 * address is just used as a key.
 *
 * The lifetime may be a single clone operation which then
 * tracks child object references as well, or it may be the
 * lifetime of the buffer builder.
 *
 * The map may be flushed explicitly when the source addresses
 * are no longer unique, such as when reusing a memory buffer,
 * and when identity preservation is no longer important.
 * Flushing a map is esentially the same as ending a lifetime.
 *
 * Multiple maps may exist concurrently for example if cloning
 * an object twice into two new objects that should have
 * separate identities. This is especially true and necessary
 * when creating a new nested buffer because the nested buffer
 * cannot share references with the parent. Cloning and object
 * that contains a nested buffer does not require multiple maps
 * because the nested buffer is then opaque.
 */

#ifndef FLATCC_REFMAP_H
#define FLATCC_REFMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "flatcc/flatcc_types.h"

#ifndef FLATCC_REFMAP_MIN_BUCKETS
/* 8 buckets gives us 5 useful initial entries with a load factor of 0.7 */
#define FLATCC_REFMAP_MIN_BUCKETS 8
#endif

#define FLATCC_REFMAP_LOAD_FACTOR 0.7f

typedef struct flatcc_refmap flatcc_refmap_t;
typedef flatbuffers_soffset_t flatcc_refmap_ref_t;

static const flatcc_refmap_ref_t flatcc_refmap_not_found = 0;

struct flatcc_refmap_item {
    const void *src;
    flatcc_refmap_ref_t ref;
};

struct flatcc_refmap {
    size_t count;
    size_t buckets;
    struct flatcc_refmap_item *table;
    /* Use stack allocation for small maps. */
    struct flatcc_refmap_item min_table[FLATCC_REFMAP_MIN_BUCKETS];
};

/*
 * Fast zero initialization - does not allocate any memory.
 * May be replaced by memset 0, but `init` avoids clearing the
 * stack allocated initial hash table until it is needed.
 */
static inline int flatcc_refmap_init(flatcc_refmap_t *refmap)
{
    refmap->count = 0;
    refmap->buckets = 0;
    refmap->table = 0;
    return 0;
}

/*
 * Removes all items and deallocates memory.
 * Not required unless `insert` or `resize` took place. The map can be
 * reused subsequently without calling `init`.
 */
void flatcc_refmap_clear(flatcc_refmap_t *refmap);

/*
 * Keeps allocated memory as is, but removes all items. The map
 * must intialized first.
 */
void flatcc_refmap_reset(flatcc_refmap_t *refmap);

/*
 * Returns the inserted reference if the `src` pointer was found,
 * without inspecting the content of the `src` pointer.
 *
 * Returns flatcc_refmap_not_found (default 0) if the `src` pointer was
 * not found.
 */
flatcc_refmap_ref_t flatcc_refmap_find(flatcc_refmap_t *refmap, const void *src);

/*
 * Inserts a `src` source pointer and its associated `ref` reference
 * into the refmap without inspecting the `src` pointer content. The
 * `ref` value will be replaced if the the `src` pointer already exists.
 *
 * Inserting null will just return the ref without updating the map.
 *
 * There is no delete operation which simplifies an open
 * addressing hash table, and it isn't needed for this use case.
 *
 * Returns the input ref or not_found on allocation error.
 */
flatcc_refmap_ref_t flatcc_refmap_insert(flatcc_refmap_t *refmap, const void *src, flatcc_refmap_ref_t ref);

/*
 * Set the hash table to accommodate at least `count` items while staying
 * within the predefined load factor.
 *
 * Resize is primarily an internal operation, but the user may resize
 * ahead of a large anticipated load, or after a large load to shrink
 * the table using 0 as the `count` argument. The table never shrinks
 * on its own account.
 */
int flatcc_refmap_resize(flatcc_refmap_t *refmap, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_REFMAP_H */
