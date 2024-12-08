#include <stdlib.h>

#include <asc_config.h>

#ifdef ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR
#include "asc_security_core/serializer/page_allocator.h"

#define FLATCC_EMITTER_ALLOC serializer_page_alloc
#define FLATCC_EMITTER_FREE serializer_page_free
#endif

#include "flatcc/flatcc_rtconfig.h"
#include "flatcc/flatcc_emitter.h"

static int advance_front(flatcc_emitter_t *E)
{
    flatcc_emitter_page_t *p = 0;

    if (E->front && E->front->prev != E->back) {
        E->front->prev->page_offset = E->front->page_offset - FLATCC_EMITTER_PAGE_SIZE;
        E->front = E->front->prev;
        goto done;
    }
    if (!(p = FLATCC_EMITTER_ALLOC(sizeof(flatcc_emitter_page_t)))) {
        return -1;
    }
    E->capacity += FLATCC_EMITTER_PAGE_SIZE;
    if (E->front) {
        p->prev = E->back;
        p->next = E->front;
        E->front->prev = p;
        E->back->next = p;
        E->front = p;
        goto done;
    }
    /*
     * The first page is shared between front and back to avoid
     * double unecessary extra allocation.
     */
    E->front = p;
    E->back = p;
    p->next = p;
    p->prev = p;
    E->front_cursor = E->front->page + FLATCC_EMITTER_PAGE_SIZE / 2;
    E->back_cursor = E->front_cursor;
    E->front_left = FLATCC_EMITTER_PAGE_SIZE / 2;
    E->back_left = FLATCC_EMITTER_PAGE_SIZE - E->front_left;
    p->page_offset = -(flatbuffers_soffset_t)E->front_left;
    return 0;
done:
    E->front_cursor = E->front->page + FLATCC_EMITTER_PAGE_SIZE;
    E->front_left = FLATCC_EMITTER_PAGE_SIZE;
    E->front->page_offset = E->front->next->page_offset - FLATCC_EMITTER_PAGE_SIZE;
    return 0;
}

static int advance_back(flatcc_emitter_t *E)
{
    flatcc_emitter_page_t *p = 0;

    if (E->back && E->back->next != E->front) {
        E->back = E->back->next;
        goto done;
    }
    if (!(p = FLATCC_EMITTER_ALLOC(sizeof(flatcc_emitter_page_t)))) {
        return -1;
    }
    E->capacity += FLATCC_EMITTER_PAGE_SIZE;
    if (E->back) {
        p->prev = E->back;
        p->next = E->front;
        E->front->prev = p;
        E->back->next = p;
        E->back = p;
        goto done;
    }
    /*
     * The first page is shared between front and back to avoid
     * double unecessary extra allocation.
     */
    E->front = p;
    E->back = p;
    p->next = p;
    p->prev = p;
    E->front_cursor = E->front->page + FLATCC_EMITTER_PAGE_SIZE / 2;
    E->back_cursor = E->front_cursor;
    E->front_left = FLATCC_EMITTER_PAGE_SIZE / 2;
    E->back_left = FLATCC_EMITTER_PAGE_SIZE - E->front_left;
    p->page_offset = -(flatbuffers_soffset_t)E->front_left;
    return 0;
done:
    E->back_cursor = E->back->page;
    E->back_left = FLATCC_EMITTER_PAGE_SIZE;
    E->back->page_offset = E->back->prev->page_offset + FLATCC_EMITTER_PAGE_SIZE;
    return 0;
}

static int copy_front(flatcc_emitter_t *E, uint8_t *data, size_t size)
{
    size_t k;

    data += size;
    while (size) {
        k = size;
        if (k > E->front_left) {
            k = E->front_left;
            if (k == 0) {
                if (advance_front(E)) {
                    return -1;
                }
                continue;
            }
        }
        E->front_cursor -= k;
        E->front_left -= k;
        data -= k;
        size -= k;
        memcpy(E->front_cursor, data, k);
    };
    return 0;
}

static int copy_back(flatcc_emitter_t *E, uint8_t *data, size_t size)
{
    size_t k;

    while (size) {
        k = size;
        if (k > E->back_left) {
            k = E->back_left;
            if (k == 0) {
                if (advance_back(E)) {
                    return -1;
                }
                continue;
            }
        }
        memcpy(E->back_cursor, data, k);
        size -= k;
        data += k;
        E->back_cursor += k;
        E->back_left -= k;
    }
    return 0;
}

int flatcc_emitter_recycle_page(flatcc_emitter_t *E, flatcc_emitter_page_t *p)
{
    if (p == E->front || p == E->back) {
        return -1;
    }
    p->next->prev = p->prev;
    p->prev->next = p->next;
    p->prev = E->front->prev;
    p->next = E->front;
    p->prev->next = p;
    p->next->prev = p;
    return 0;
}

void flatcc_emitter_reset(flatcc_emitter_t *E)
{
    flatcc_emitter_page_t *p = E->front;

    if (!E->front) {
        return;
    }
    E->back = E->front;
    E->front_cursor = E->front->page + FLATCC_EMITTER_PAGE_SIZE / 2;
    E->back_cursor = E->front_cursor;
    E->front_left = FLATCC_EMITTER_PAGE_SIZE / 2;
    E->back_left = FLATCC_EMITTER_PAGE_SIZE - FLATCC_EMITTER_PAGE_SIZE / 2;
    E->front->page_offset = -(flatbuffers_soffset_t)E->front_left;
    /* Heuristic to reduce peak allocation over time. */
    if (E->used_average == 0) {
        E->used_average = E->used;
    }
    E->used_average = E->used_average * 3 / 4 + E->used / 4;
    E->used = 0;
    while (E->used_average * 2 < E->capacity && E->back->next != E->front) {
        /* We deallocate the page after back since it is less likely to be hot in cache. */
        p = E->back->next;
        E->back->next = p->next;
        p->next->prev = E->back;
        FLATCC_EMITTER_FREE(p);
        E->capacity -= FLATCC_EMITTER_PAGE_SIZE;
    }
}

void flatcc_emitter_clear(flatcc_emitter_t *E)
{
    flatcc_emitter_page_t *p = E->front;

    if (!p) {
        return;
    }
    p->prev->next = 0;
    while (p->next) {
        p = p->next;
        FLATCC_EMITTER_FREE(p->prev);
    }
    FLATCC_EMITTER_FREE(p);
    memset(E, 0, sizeof(*E));
}

int flatcc_emitter(void *emit_context,
        const flatcc_iovec_t *iov, int iov_count,
        flatbuffers_soffset_t offset, size_t len)
{
    flatcc_emitter_t *E = emit_context;
    uint8_t *p;

    E->used += len;
    if (offset < 0) {
        if (len <= E->front_left) {
            E->front_cursor -= len;
            E->front_left -= len;
            p = E->front_cursor;
            goto copy;
        }
        iov += iov_count;
        while (iov_count--) {
            --iov;
            if (copy_front(E, iov->iov_base, iov->iov_len)) {
                return -1;
            }
        }
    } else {
        if (len <= E->back_left) {
            p = E->back_cursor;
            E->back_cursor += len;
            E->back_left -= len;
            goto copy;
        }
        while (iov_count--) {
            if (copy_back(E, iov->iov_base, iov->iov_len)) {
                return -1;
            }
            ++iov;
        }
    }
    return 0;
copy:
    while (iov_count--) {
        memcpy(p, iov->iov_base, iov->iov_len);
        p += iov->iov_len;
        ++iov;
    }
    return 0;
}

void *flatcc_emitter_copy_buffer(flatcc_emitter_t *E, void *buf, size_t size)
{
    flatcc_emitter_page_t *p;
    size_t len;

    if (size < E->used) {
        return 0;
    }
    if (!E->front) {
        return 0;
    }
    if (E->front == E->back) {
        memcpy(buf, E->front_cursor, E->used);
        return buf;
    }
    len = FLATCC_EMITTER_PAGE_SIZE - E->front_left;
    memcpy(buf, E->front_cursor, len);
    buf = (uint8_t *)buf + len;
    p = E->front->next;
    while (p != E->back) {
        memcpy(buf, p->page, FLATCC_EMITTER_PAGE_SIZE);
        buf = (uint8_t *)buf + FLATCC_EMITTER_PAGE_SIZE;
        p = p->next;
    }
    memcpy(buf, p->page, FLATCC_EMITTER_PAGE_SIZE - E->back_left);
    return buf;
}
