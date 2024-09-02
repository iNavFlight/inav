#ifndef FLATCC_EMITTER_H
#define FLATCC_EMITTER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR
#include "asc_security_core/serializer/page_allocator.h"
#endif
/*
 * Default implementation of a flatbuilder emitter.
 *
 * This may be used as a starting point for more advanced emitters,
 * for example writing completed pages to disk or network and
 * the recycling those pages.
 */

#include <stdlib.h>
#include <string.h>

#include <asc_config.h>

#include "flatcc/flatcc_types.h"
#include "flatcc/flatcc_iov.h"
#include "flatcc/flatcc_alloc.h"

/*
 * The buffer steadily grows during emission but the design allows for
 * an extension where individual pages can recycled before the buffer
 * is complete, for example because they have been transmitted.
 *
 * When done, the buffer can be cleared to free all memory, or reset to
 * maintain an adaptive page pool for next buffer construction.
 *
 * Unlike an exponentially growing buffer, each buffer page remains
 * stable in memory until reset, clear or recycle is called.
 *
 * Design notes for possible extensions:
 *
 * The buffer is a ring buffer marked by a front and a back page. The
 * front and back may be the same page and may initially be absent.
 * Anything outside these pages are unallocated pages for recycling.
 * Any page between (but excluding) the front and back pages may be
 * recycled by unlinking and relinking outside the front and back pages
 * but then copy operations no longer makes sense. Each page stores the
 * logical offset within the buffer but isn't otherwise used by the
 * implemention - it might be used for network transmission.  The buffer
 * is not explicitly designed for multithreaded access but any page
 * strictly between front and back is not touched unless recycled and in
 * this case aligned allocation is useful to prevent cache line sharing.
 */

/*
 * Memory is allocated in fixed size page units - the first page is
 * split between front and back so each get half the page size. If the
 * size is a multiple of 128 then each page offset will be a multiple of
 * 64, which may be useful for sequencing etc.
 */
#ifndef FLATCC_EMITTER_PAGE_SIZE
#define FLATCC_EMITTER_MAX_PAGE_SIZE 3000
#define FLATCC_EMITTER_PAGE_MULTIPLE 64
#define FLATCC_EMITTER_PAGE_SIZE ((FLATCC_EMITTER_MAX_PAGE_SIZE) &\
    ~(2 * (FLATCC_EMITTER_PAGE_MULTIPLE) - 1))
#endif

#ifndef FLATCC_EMITTER_ALLOC
#ifdef FLATCC_EMITTER_USE_ALIGNED_ALLOC
/*
 * <stdlib.h> does not always provide aligned_alloc, so include whatever
 * is required when enabling this feature.
 */
#define FLATCC_EMITTER_ALLOC(n) aligned_alloc(FLATCC_EMITTER_PAGE_MULTIPLE,\
        (((n) + FLATCC_EMITTER_PAGE_MULTIPLE - 1) & ~(FLATCC_EMITTER_PAGE_MULTIPLE - 1)))
#ifndef FLATCC_EMITTER_FREE
#define FLATCC_EMITTER_FREE(p) aligned_free(p)
#endif
#endif
#endif

#ifndef FLATCC_EMITTER_ALLOC
#define FLATCC_EMITTER_ALLOC(n) FLATCC_ALLOC(n)
#endif
#ifndef FLATCC_EMITTER_FREE
#define FLATCC_EMITTER_FREE(p) FLATCC_FREE(p)
#endif

typedef struct flatcc_emitter_page flatcc_emitter_page_t;
typedef struct flatcc_emitter flatcc_emitter_t;

struct flatcc_emitter_page {
    uint8_t page[FLATCC_EMITTER_PAGE_SIZE];
    flatcc_emitter_page_t *next;
    flatcc_emitter_page_t *prev;
    /*
     * The offset is relative to page start, but not necessarily
     * to any present content if part of front or back page,
     * and undefined for unused pages.
     */
    flatbuffers_soffset_t page_offset;
};

/*
 * Must be allocated and zeroed externally, e.g. on the stack
 * then provided as emit_context to the flatbuilder along
 * with the `flatcc_emitter` function.
 */
struct flatcc_emitter {
    flatcc_emitter_page_t *front, *back;
    uint8_t *front_cursor;
    size_t front_left;
    uint8_t *back_cursor;
    size_t back_left;
    size_t used;
    size_t capacity;
    size_t used_average;
};

/* Optional helper to ensure emitter is zeroed initially. */
static inline void flatcc_emitter_init(flatcc_emitter_t *E)
{
    memset(E, 0, sizeof(*E));
}

/* Deallocates all buffer memory making the emitter ready for next use. */
void flatcc_emitter_clear(flatcc_emitter_t *E);

/*
 * Similar to `clear_flatcc_emitter` but heuristacally keeps some allocated
 * memory between uses while gradually reducing peak allocations.
 * For small buffers, a single page will remain available with no
 * additional allocations or deallocations after first use.
 */
void flatcc_emitter_reset(flatcc_emitter_t *E);

/*
 * Helper function that allows a page between front and back to be
 * recycled while the buffer is still being constructed - most likely as part
 * of partial copy or transmission. Attempting to recycle front or back
 * pages will result will result in an error. Recycling pages outside the
 * front and back will be valid but pointless. After recycling and copy
 * operations are no longer well-defined and should be replaced with
 * whatever logic is recycling the pages.  The reset operation
 * automatically recycles all (remaining) pages when emission is
 * complete. After recycling, the `flatcc_emitter_size` function will
 * return as if recycle was not called, but will only represent the
 * logical size, not the size of the active buffer. Because a recycled
 * page is fully utilized, it is fairly easy to compensate for this if
 * required.
 *
 * Returns 0 on success.
 */
int flatcc_emitter_recycle_page(flatcc_emitter_t *E, flatcc_emitter_page_t *p);

/*
 * The amount of data copied with `flatcc_emitter_copy_buffer` and related
 * functions. Normally called at end of buffer construction but is
 * always valid, as is the copy functions. The size is a direct
 * function of the amount emitted data so the flatbuilder itself can
 * also provide this information.
 */
static inline size_t flatcc_emitter_get_buffer_size(flatcc_emitter_t *E)
{
    return E->used;
}

/*
 * Returns buffer start iff the buffer fits on a single internal page.
 * Only useful for fairly small buffers - about half the page size since
 * one half of first page goes to vtables that likely use little space.
 * Returns null if request could not be served.
 *
 * If `size_out` is not null, it is set to the buffer size, or 0 if
 * operation failed.
 */
static inline void *flatcc_emitter_get_direct_buffer(flatcc_emitter_t *E, size_t *size_out)
{
    if (E->front == E->back) {
        if (size_out) {
            *size_out = E->used;
        }
        return E->front_cursor;
    }
    if (size_out) {
        *size_out = 0;
    }
    return 0;
}

/*
 * Copies the internal flatcc_emitter representation to an externally
 * provided linear buffer that must have size `flatcc_emitter_get_size`.
 *
 * If pages have been recycled, only the remaining pages will be copied
 * and thus less data than what `flatcc_emitter_get_size` would suggest. It
 * makes more sense to provide a customized copy operation when
 * recycling pages.
 *
 * If the buffer is too small, nothing is copied, otherwise the
 * full buffer is copied and the input buffer is returned.
 */
void *flatcc_emitter_copy_buffer(flatcc_emitter_t *E, void *buf, size_t size);

/*
 * The emitter interface function to the flatbuilder API.
 * `emit_context` should be of type `flatcc_emitter_t` for this
 * particular implementation.
 *
 * This function is compatible with the `flatbuilder_emit_fun`
 * type defined in "flatbuilder.h".
 */
int flatcc_emitter(void *emit_context,
        const flatcc_iovec_t *iov, int iov_count,
        flatbuffers_soffset_t offset, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_EMITTER_H */
