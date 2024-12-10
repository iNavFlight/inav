/*
 * Codegenerator for C, building FlatBuffers.
 *
 * There are several approaches, some light, some requiring a library,
 * some with vectored I/O etc.
 *
 * Here we focus on a reasonable balance of light code and efficiency.
 *
 * Builder code is generated to a separate file that includes the
 * generated read-only code.
 *
 * Mutable buffers are not supported in this version.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "flatcc/flatcc_builder.h"
#include "flatcc/flatcc_emitter.h"
#include "flatcc/flatcc_assert.h"

#ifndef FLATCC_NO_ASSERT 
int __SET_ASSERT__ = 0;
int __ASSERT_VAL__ = 0;
const char *__ASSERT_REASON__ = "";
#endif
/*
 * `check` is designed to handle incorrect use errors that can be
 * ignored in production of a tested product.
 *
 * `check_error` fails if condition is false and is designed to return an
 * error code in production.
 */

#if FLATCC_BUILDER_ASSERT_ON_ERROR
#define check(cond, reason) FLATCC_BUILDER_ASSERT(cond, reason)
#else
#define check(cond, reason) ((void)0)
#endif

#if FLATCC_BUILDER_SKIP_CHECKS
#define check_error(cond, err, reason) ((void)0)
#else
#define check_error(cond, err, reason) if (!(cond)) { check(cond, reason); return err; }
#endif

/* `strnlen` not widely supported. */
static inline size_t pstrnlen(const char *s, size_t max_len)
{
    const char *end = memchr(s, 0, max_len);
    return end ? (size_t)(end - s) : max_len;
}
#undef strnlen
#define strnlen pstrnlen

/* Padding can be up to 255 zeroes, and 1 zero string termination byte.
 * When two paddings are combined at nested buffers, we need twice that.
 * Visible to emitter so it can test for zero padding in iov. */
const uint8_t flatcc_builder_padding_base[512] = { 0 };
#define _pad flatcc_builder_padding_base

#define uoffset_t flatbuffers_uoffset_t
#define soffset_t flatbuffers_soffset_t
#define voffset_t flatbuffers_voffset_t
#define utype_t flatbuffers_utype_t

#define write_uoffset __flatbuffers_uoffset_write_to_pe
#define write_voffset  __flatbuffers_voffset_write_to_pe
#define write_identifier __flatbuffers_uoffset_write_to_pe
#define write_utype __flatbuffers_utype_write_to_pe

#define field_size sizeof(uoffset_t)
#define max_offset_count FLATBUFFERS_COUNT_MAX(field_size)
#define union_size sizeof(flatcc_builder_union_ref_t)
#define max_union_count FLATBUFFERS_COUNT_MAX(union_size)
#define utype_size sizeof(utype_t)
#define max_utype_count FLATBUFFERS_COUNT_MAX(utype_size)

#define max_string_len FLATBUFFERS_COUNT_MAX(1)
#define identifier_size FLATBUFFERS_IDENTIFIER_SIZE


#define iovec_t flatcc_iovec_t
#define frame_size sizeof(__flatcc_builder_frame_t)
#define frame(x) (B->frame[0].x)


/* `align` must be a power of 2. */
static inline uoffset_t alignup_uoffset(uoffset_t x, size_t align)
{
    return (x + (uoffset_t)align - 1u) & ~((uoffset_t)align - 1u);
}

static inline size_t alignup_size(size_t x, size_t align)
{
    return (x + align - 1u) & ~(align - 1u);
}


typedef struct vtable_descriptor vtable_descriptor_t;
struct vtable_descriptor {
    /* Where the vtable is emitted. */
    flatcc_builder_ref_t vt_ref;
    /* Which buffer it was emitted to. */
    uoffset_t nest_id;
    /* Where the vtable is cached. */
    uoffset_t vb_start;
    /* Hash table collision chain. */
    uoffset_t next;
};

typedef struct flatcc_iov_state flatcc_iov_state_t;
struct flatcc_iov_state {
    size_t len;
    int count;
    flatcc_iovec_t iov[FLATCC_IOV_COUNT_MAX];
};

#define iov_state_t flatcc_iov_state_t

/* This assumes `iov_state_t iov;` has been declared in scope */
#define push_iov_cond(base, size, cond) if ((size) > 0 && (cond)) { iov.len += size;\
        iov.iov[iov.count].iov_base = (void *)(base); iov.iov[iov.count].iov_len = (size); ++iov.count; }
#define push_iov(base, size) push_iov_cond(base, size, 1)
#define init_iov() { iov.len = 0; iov.count = 0; }


int flatcc_builder_default_alloc(void *alloc_context, iovec_t *b, size_t request, int zero_fill, int hint)
{
    void *p;
    size_t n;

    (void)alloc_context;

    if (request == 0) {
        if (b->iov_base) {
            FLATCC_BUILDER_FREE(b->iov_base);
            b->iov_base = 0;
            b->iov_len = 0;
        }
        return 0;
    }
    switch (hint) {
    case flatcc_builder_alloc_ds:
        n = 256;
        break;
    case flatcc_builder_alloc_ht:
        /* Should be exact size, or space size is just wasted. */
        n = request;
        break;
    case flatcc_builder_alloc_fs:
        n = sizeof(__flatcc_builder_frame_t) * 8;
        break;
    case flatcc_builder_alloc_us:
        n = 64;
        break;
    default:
        /*
         * We have many small structures - vs stack for tables with few
         * elements, and few offset fields in patch log. No need to
         * overallocate in case of busy small messages.
         */
        n = 32;
        break;
    }
    while (n < request) {
        n *= 2;
    }
    if (request <= b->iov_len && b->iov_len / 2 >= n) {
        /* Add hysteresis to shrink. */
        return 0;
    }
    p = FLATCC_BUILDER_REALLOC(b->iov_base, n);
    if (!p) {
        return -1;
    }
    /* Realloc might also shrink. */
    if (zero_fill && b->iov_len < n) {
        memset((uint8_t *)p + b->iov_len, 0, n - b->iov_len);
    }
    b->iov_base = p;
    b->iov_len = n;
    return 0;
}

#define T_ptr(base, pos) ((void *)((uint8_t *)(base) + (uoffset_t)(pos)))
#define ds_ptr(pos) (T_ptr(B->buffers[flatcc_builder_alloc_ds].iov_base, (pos)))
#define vs_ptr(pos) (T_ptr(B->buffers[flatcc_builder_alloc_vs].iov_base, (pos)))
#define pl_ptr(pos) (T_ptr(B->buffers[flatcc_builder_alloc_pl].iov_base, (pos)))
#define us_ptr(pos) (T_ptr(B->buffers[flatcc_builder_alloc_us].iov_base, (pos)))
#define vd_ptr(pos) (T_ptr(B->buffers[flatcc_builder_alloc_vd].iov_base, (pos)))
#define vb_ptr(pos) (T_ptr(B->buffers[flatcc_builder_alloc_vb].iov_base, (pos)))
#define vs_offset(ptr) ((uoffset_t)((size_t)(ptr) - (size_t)B->buffers[flatcc_builder_alloc_vs].iov_base))
#define pl_offset(ptr) ((uoffset_t)((size_t)(ptr) - (size_t)B->buffers[flatcc_builder_alloc_pl].iov_base))
#define us_offset(ptr) ((uoffset_t)((size_t)(ptr) - (size_t)B->buffers[flatcc_builder_alloc_us].iov_base))

#define table_limit (FLATBUFFERS_VOFFSET_MAX - field_size + 1)
#define data_limit (FLATBUFFERS_UOFFSET_MAX - field_size + 1)

#define set_identifier(id) memcpy(&B->identifier, (id) ? (void *)(id) : (void *)_pad, identifier_size)

/* Must also return true when no buffer has been started. */
#define is_top_buffer(B) (B->nest_id == 0)

/*
 * Tables use a stack represention better suited for quickly adding
 * fields to tables, but it must occasionally be refreshed following
 * reallocation or reentry from child frame.
 */
static inline void refresh_ds(flatcc_builder_t *B, uoffset_t type_limit)
{
    iovec_t *buf = B->buffers + flatcc_builder_alloc_ds;

    B->ds = ds_ptr(B->ds_first);
    B->ds_limit = (uoffset_t)buf->iov_len - B->ds_first;
    /*
     * So we don't allocate outside tables representation size, nor our
     * current buffer size.
     */
    if (B->ds_limit > type_limit) {
        B->ds_limit = type_limit;
    }
    /* So exit frame can refresh fast. */
    frame(type_limit) = type_limit;
}

static int reserve_ds(flatcc_builder_t *B, size_t need, uoffset_t limit)
{
    iovec_t *buf = B->buffers + flatcc_builder_alloc_ds;

    if (B->alloc(B->alloc_context, buf, B->ds_first + need, 1, flatcc_builder_alloc_ds)) {
        return -1;
    }
    refresh_ds(B, limit);
    return 0;
}

/*
 * Make sure there is always an extra zero termination on stack
 * even if it isn't emitted such that string updates may count
 * on zero termination being present always.
 */
static inline void *push_ds(flatcc_builder_t *B, uoffset_t size)
{
    size_t offset;

    offset = B->ds_offset;
    if ((B->ds_offset += size) >= B->ds_limit) {
        if (reserve_ds(B, B->ds_offset + 1, data_limit)) {
            return 0;
        }
    }
    return B->ds + offset;
}

static inline void unpush_ds(flatcc_builder_t *B, uoffset_t size)
{
    B->ds_offset -= size;
    memset(B->ds + B->ds_offset, 0, size);
}

static inline void *push_ds_copy(flatcc_builder_t *B, const void *data, uoffset_t size)
{
    void *p;

    if (!(p = push_ds(B, size))) {
        return 0;
    }
    memcpy(p, data, size);
    return p;
}

static inline void *push_ds_field(flatcc_builder_t *B, uoffset_t size, uint16_t align, voffset_t id)
{
    uoffset_t offset;

    /*
     * We calculate table field alignment relative to first entry, not
     * header field with vtable offset.
     *
     * Note: >= comparison handles special case where B->ds is not
     * allocated yet and size is 0 so the return value would be mistaken
     * for an error.
     */
    offset = alignup_uoffset(B->ds_offset, align);
    if ((B->ds_offset = offset + size) >= B->ds_limit) {
        if (reserve_ds(B, B->ds_offset + 1, table_limit)) {
            return 0;
        }
    }
    B->vs[id] = (voffset_t)(offset + field_size);
    if (id >= B->id_end) {
        B->id_end = id + 1u;
    }
    return B->ds + offset;
}

static inline void *push_ds_offset_field(flatcc_builder_t *B, voffset_t id)
{
    uoffset_t offset;

    offset = alignup_uoffset(B->ds_offset, field_size);
    if ((B->ds_offset = offset + field_size) > B->ds_limit) {
        if (reserve_ds(B, B->ds_offset, table_limit)) {
            return 0;
        }
    }
    B->vs[id] = (voffset_t)(offset + field_size);
    if (id >= B->id_end) {
        B->id_end = id + 1u;
    }
    *B->pl++ = (flatbuffers_voffset_t)offset;
    return B->ds + offset;
}

static inline void *reserve_buffer(flatcc_builder_t *B, int alloc_type, size_t used, size_t need, int zero_init)
{
    iovec_t *buf = B->buffers + alloc_type;

    if (used + need > buf->iov_len) {
        if (B->alloc(B->alloc_context, buf, used + need, zero_init, alloc_type)) {
            check(__SET_ASSERT__, "memory allocation failed");
            return 0;
        }
    }
    return (void *)((size_t)buf->iov_base + used);
}

static inline int reserve_fields(flatcc_builder_t *B, int count)
{
    size_t used, need;

    /* Provide faster stack operations for common table operations. */
    used = frame(container.table.vs_end) + frame(container.table.id_end) * sizeof(voffset_t);
    need = (size_t)(count + 2) * sizeof(voffset_t);
    if (!(B->vs = reserve_buffer(B, flatcc_builder_alloc_vs, used, need, 1))) {
        return -1;
    }
    /* Move past header for convenience. */
    B->vs += 2;
    used = frame(container.table.pl_end);
    /* Add one to handle special case of first table being empty. */
    need = (size_t)count * sizeof(*(B->pl)) + 1;
    if (!(B->pl = reserve_buffer(B, flatcc_builder_alloc_pl, used, need, 0))) {
        return -1;
    }
    return 0;
}

static int alloc_ht(flatcc_builder_t *B)
{
    iovec_t *buf = B->buffers + flatcc_builder_alloc_ht;

    size_t size, k;
    /* Allocate null entry so we can check for return errors. */
    FLATCC_ASSERT(B->vd_end == 0);
    if (!reserve_buffer(B, flatcc_builder_alloc_vd, B->vd_end, sizeof(vtable_descriptor_t), 0)) {
        return -1;
    }
    B->vd_end = sizeof(vtable_descriptor_t);
    size = field_size * FLATCC_BUILDER_MIN_HASH_COUNT;
    if (B->alloc(B->alloc_context, buf, size, 1, flatcc_builder_alloc_ht)) {
        return -1;
    }
    while (size * 2 <= buf->iov_len) {
        size *= 2;
    }
    size /= field_size;
    for (k = 0; (((size_t)1) << k) < size; ++k) {
    }
    B->ht_width = k;
    return 0;
}

static inline uoffset_t *lookup_ht(flatcc_builder_t *B, uint32_t hash)
{
    uoffset_t *T;

    if (B->ht_width == 0) {
        if (alloc_ht(B)) {
            return 0;
        }
    }
    T = B->buffers[flatcc_builder_alloc_ht].iov_base;

    return &T[FLATCC_BUILDER_BUCKET_VT_HASH(hash, B->ht_width)];
}

void flatcc_builder_flush_vtable_cache(flatcc_builder_t *B)
{
    iovec_t *buf = B->buffers + flatcc_builder_alloc_ht;

    if (B->ht_width == 0) {
        return;
    }
    memset(buf->iov_base, 0, buf->iov_len);
    /* Reserve the null entry. */
    B->vd_end = sizeof(vtable_descriptor_t);
    B->vb_end = 0;
}

int flatcc_builder_custom_init(flatcc_builder_t *B,
        flatcc_builder_emit_fun *emit, void *emit_context,
        flatcc_builder_alloc_fun *alloc, void *alloc_context)
{
    /*
     * Do not allocate anything here. Only the required buffers will be
     * allocated. For simple struct buffers, no allocation is required
     * at all.
     */
    memset(B, 0, sizeof(*B));

    if (emit == 0) {
        B->is_default_emitter = 1;
        emit = flatcc_emitter;
        emit_context = &B->default_emit_context;
    }
    if (alloc == 0) {
        alloc = flatcc_builder_default_alloc;
    }
    B->alloc_context = alloc_context;
    B->alloc = alloc;
    B->emit_context = emit_context;
    B->emit = emit;
    return 0;
}

int flatcc_builder_init(flatcc_builder_t *B)
{
    return flatcc_builder_custom_init(B, 0, 0, 0, 0);
}

int flatcc_builder_custom_reset(flatcc_builder_t *B, int set_defaults, int reduce_buffers)
{
    iovec_t *buf;
    int i;

    for (i = 0; i < FLATCC_BUILDER_ALLOC_BUFFER_COUNT; ++i) {
        buf = B->buffers + i;
        if (buf->iov_base) {
            /* Don't try to reduce the hash table. */
            if (i != flatcc_builder_alloc_ht &&
                reduce_buffers && B->alloc(B->alloc_context, buf, 1, 1, i)) {
                return -1;
            }
            memset(buf->iov_base, 0, buf->iov_len);
        } else {
            FLATCC_ASSERT(buf->iov_len == 0);
        }
    }
    B->vb_end = 0;
    if (B->vd_end > 0) {
        /* Reset past null entry. */
        B->vd_end = sizeof(vtable_descriptor_t);
    }
    B->min_align = 0;
    B->emit_start = 0;
    B->emit_end = 0;
    B->level = 0;
    B->limit_level = 0;
    B->ds_offset = 0;
    B->ds_limit = 0;
    B->nest_count = 0;
    B->nest_id = 0;
    /* Needed for correct offset calculation. */
    B->ds = B->buffers[flatcc_builder_alloc_ds].iov_base;
    B->pl = B->buffers[flatcc_builder_alloc_pl].iov_base;
    B->vs = B->buffers[flatcc_builder_alloc_vs].iov_base;
    B->frame = 0;
    if (set_defaults) {
        B->vb_flush_limit = 0;
        B->max_level = 0;
        B->disable_vt_clustering = 0;
    }
    if (B->is_default_emitter) {
        flatcc_emitter_reset(&B->default_emit_context);
    }
    if (B->refmap) {
        flatcc_refmap_reset(B->refmap);
    }
    return 0;
}

int flatcc_builder_reset(flatcc_builder_t *B)
{
    return flatcc_builder_custom_reset(B, 0, 0);
}

void flatcc_builder_clear(flatcc_builder_t *B)
{
    iovec_t *buf;
    int i;

    for (i = 0; i < FLATCC_BUILDER_ALLOC_BUFFER_COUNT; ++i) {
        buf = B->buffers + i;
        B->alloc(B->alloc_context, buf, 0, 0, i);
    }
    if (B->is_default_emitter) {
        flatcc_emitter_clear(&B->default_emit_context);
    }
    if (B->refmap) {
        flatcc_refmap_clear(B->refmap);
    }
    memset(B, 0, sizeof(*B));
}

static inline void set_min_align(flatcc_builder_t *B, uint16_t align)
{
    if (B->min_align < align) {
        B->min_align = align;
    }
}

/*
 * It's a max, but the minimum viable alignment is the largest observed
 * alignment requirement, but no larger.
 */
static inline void get_min_align(uint16_t *align, uint16_t b)
{
    if (*align < b) {
        *align = b;
    }
}

void *flatcc_builder_enter_user_frame_ptr(flatcc_builder_t *B, size_t size)
{
    size_t *frame;

    size = alignup_size(size, sizeof(size_t)) + sizeof(size_t);

    if (!(frame = reserve_buffer(B, flatcc_builder_alloc_us, B->user_frame_end, size, 0))) {
        return 0;
    }
    memset(frame, 0, size);
    *frame++ = B->user_frame_offset;
    B->user_frame_offset = B->user_frame_end + sizeof(size_t);
    B->user_frame_end += size;
    return frame;
}

size_t flatcc_builder_enter_user_frame(flatcc_builder_t *B, size_t size)
{
    size_t *frame;

    size = alignup_size(size, sizeof(size_t)) + sizeof(size_t);

    if (!(frame = reserve_buffer(B, flatcc_builder_alloc_us, B->user_frame_end, size, 0))) {
        return 0;
    }
    memset(frame, 0, size);
    *frame++ = B->user_frame_offset;
    B->user_frame_offset = B->user_frame_end + sizeof(size_t);
    B->user_frame_end += size;
    return B->user_frame_offset;
}


size_t flatcc_builder_exit_user_frame(flatcc_builder_t *B)
{
    size_t *hdr;

    FLATCC_ASSERT(B->user_frame_offset > 0);

    hdr = us_ptr(B->user_frame_offset);
    B->user_frame_end = B->user_frame_offset - sizeof(size_t);
    return B->user_frame_offset = hdr[-1];
}

size_t flatcc_builder_exit_user_frame_at(flatcc_builder_t *B, size_t handle)
{
    FLATCC_ASSERT(B->user_frame_offset >= handle);

    B->user_frame_offset = handle;
    return flatcc_builder_exit_user_frame(B);
}

size_t flatcc_builder_get_current_user_frame(flatcc_builder_t *B)
{
    return B->user_frame_offset;
}

void *flatcc_builder_get_user_frame_ptr(flatcc_builder_t *B, size_t handle)
{
    return us_ptr(handle);
}

static int enter_frame(flatcc_builder_t *B, uint16_t align)
{
    if (++B->level > B->limit_level) {
        if (B->max_level > 0 && B->level > B->max_level) {
            return -1;
        }
        if (!(B->frame = reserve_buffer(B, flatcc_builder_alloc_fs,
                        (size_t)(B->level - 1) * frame_size, frame_size, 0))) {
            return -1;
        }
        B->limit_level = (int)(B->buffers[flatcc_builder_alloc_fs].iov_len / frame_size);
        if (B->max_level > 0 && B->max_level < B->limit_level) {
            B->limit_level = B->max_level;
        }
    } else {
        ++B->frame;
    }
    frame(ds_offset) = B->ds_offset;
    frame(align) = B->align;
    B->align = align;
    /* Note: do not assume padding before first has been allocated! */
    frame(ds_first) = B->ds_first;
    frame(type_limit) = data_limit;
    B->ds_first = alignup_uoffset(B->ds_first + B->ds_offset, 8);
    B->ds_offset = 0;
    return 0;
}

static inline void exit_frame(flatcc_builder_t *B)
{
    memset(B->ds, 0, B->ds_offset);
    B->ds_offset = frame(ds_offset);
    B->ds_first = frame(ds_first);
    refresh_ds(B, frame(type_limit));

    /*
     * Restore local alignment: e.g. a table should not change alignment
     * because a child table was just created elsewhere in the buffer,
     * but the overall alignment (min align), should be aware of it.
     * Each buffer has its own min align that then migrates up without
     * being affected by sibling or child buffers.
     */
    set_min_align(B, B->align);
    B->align = frame(align);

    --B->frame;
    --B->level;
}

static inline uoffset_t front_pad(flatcc_builder_t *B, uoffset_t size, uint16_t align)
{
    return (uoffset_t)(B->emit_start - (flatcc_builder_ref_t)size) & (align - 1u);
}

static inline uoffset_t back_pad(flatcc_builder_t *B, uint16_t align)
{
    return (uoffset_t)(B->emit_end) & (align - 1u);
}

static inline flatcc_builder_ref_t emit_front(flatcc_builder_t *B, iov_state_t *iov)
{
    flatcc_builder_ref_t ref;

    /*
     * We might have overflow when including headers, but without
     * headers we should have checks to prevent overflow in the
     * uoffset_t range, hence we subtract 16 to be safe. With that
     * guarantee we can also make a safe check on the soffset_t range.
     *
     * We only allow buffers half the theoritical size of
     * FLATBUFFERS_UOFFSET_MAX so we can safely use signed references.
     *
     * NOTE: vtables vt_offset field is signed, and the check in create
     * table only ensures the signed limit. The check would fail if the
     * total buffer size could grow beyond UOFFSET_MAX, and we prevent
     * that by limiting the lower end to SOFFSET_MIN, and the upper end
     * at emit_back to SOFFSET_MAX.
     */
    ref = B->emit_start - (flatcc_builder_ref_t)iov->len;
    
    if (
#if FLATBUFFERS_UOFFSET_MAX < UINT32_MAX
        (iov->len > 16 && iov->len - 16 > FLATBUFFERS_UOFFSET_MAX) || 
#endif  
        ref >= B->emit_start) {
        check(__SET_ASSERT__, "buffer too large to represent");
        return 0;
    }
    if (B->emit(B->emit_context, iov->iov, iov->count, ref, iov->len)) {
        check(__SET_ASSERT__, "emitter rejected buffer content");
        return 0;
    }
    return B->emit_start = ref;
}

static inline flatcc_builder_ref_t emit_back(flatcc_builder_t *B, iov_state_t *iov)
{
    flatcc_builder_ref_t ref;

    ref = B->emit_end;
    B->emit_end = ref + (flatcc_builder_ref_t)iov->len;
    /*
     * Similar to emit_front check, but since we only emit vtables and
     * padding at the back, we are not concerned with iov->len overflow,
     * only total buffer overflow.
     *
     * With this check, vtable soffset references at table header can
     * still overflow in extreme cases, so this must be checked
     * separately.
     */
    if (B->emit_end < ref) {
        check(__SET_ASSERT__, "buffer too large to represent");
        return 0;
    }
    if (B->emit(B->emit_context, iov->iov, iov->count, ref, iov->len)) {
        check(__SET_ASSERT__, "emitter rejected buffer content");
        return 0;
    }
    /*
     * Back references always return ref + 1 because ref == 0 is valid and
     * should not be mistaken for error. vtables understand this.
     */
    return ref + 1;
}

static int align_to_block(flatcc_builder_t *B, uint16_t *align, uint16_t block_align, int is_nested)
{
    size_t end_pad;
    iov_state_t iov;

    block_align = block_align ? block_align : B->block_align ? B->block_align : 1;
    get_min_align(align, field_size);
    get_min_align(align, block_align);
    /* Pad end of buffer to multiple. */
    if (!is_nested) {
        end_pad = back_pad(B, block_align);
        if (end_pad) {
            init_iov();
            push_iov(_pad, end_pad);
            if (0 == emit_back(B, &iov)) {
                check(__SET_ASSERT__, "emitter rejected buffer content");
                return -1;
            }
        }
    }
    return 0;
}

flatcc_builder_ref_t flatcc_builder_embed_buffer(flatcc_builder_t *B,
        uint16_t block_align,
        const void *data, size_t size, uint16_t align, int flags)
{
    uoffset_t size_field, pad;
    iov_state_t iov;
    int with_size = flags & flatcc_builder_with_size;

    if (align_to_block(B, &align, block_align, !is_top_buffer(B))) {
        return 0;
    }
    pad = front_pad(B, (uoffset_t)(size + (with_size ? field_size : 0)), align);
    write_uoffset(&size_field, (uoffset_t)size + pad);
    init_iov();
    /* Add ubyte vector size header if nested buffer. */
    push_iov_cond(&size_field, field_size, !is_top_buffer(B));
    push_iov(data, size);
    push_iov(_pad, pad);
    return emit_front(B, &iov);
}

flatcc_builder_ref_t flatcc_builder_create_buffer(flatcc_builder_t *B,
        const char identifier[identifier_size], uint16_t block_align,
        flatcc_builder_ref_t object_ref, uint16_t align, int flags)
{
    flatcc_builder_ref_t buffer_ref;
    uoffset_t header_pad, id_size = 0;
    uoffset_t object_offset, buffer_size, buffer_base;
    iov_state_t iov;
    flatcc_builder_identifier_t id_out = 0;
    int is_nested = (flags & flatcc_builder_is_nested) != 0;
    int with_size = (flags & flatcc_builder_with_size) != 0;

    if (align_to_block(B, &align, block_align, is_nested)) {
        return 0;
    }
    set_min_align(B, align);
    if (identifier) {
        FLATCC_ASSERT(sizeof(flatcc_builder_identifier_t) == identifier_size);
        FLATCC_ASSERT(sizeof(flatcc_builder_identifier_t) == field_size);
        memcpy(&id_out, identifier, identifier_size);
        id_out = __flatbuffers_thash_read_from_le(&id_out);
        write_identifier(&id_out, id_out);
    }
    id_size = id_out ? identifier_size : 0;
    header_pad = front_pad(B, field_size + id_size + (uoffset_t)(with_size ? field_size : 0), align);
    init_iov();
    /* ubyte vectors size field wrapping nested buffer. */
    push_iov_cond(&buffer_size, field_size, is_nested || with_size);
    push_iov(&object_offset, field_size);
    /* Identifiers are not always present in buffer. */
    push_iov(&id_out, id_size);
    push_iov(_pad, header_pad);
    buffer_base = (uoffset_t)B->emit_start - (uoffset_t)iov.len + (uoffset_t)((is_nested || with_size) ? field_size : 0);
    if (is_nested) {
        write_uoffset(&buffer_size, (uoffset_t)B->buffer_mark - buffer_base);
    } else {
        /* Also include clustered vtables. */
        write_uoffset(&buffer_size, (uoffset_t)B->emit_end - buffer_base);
    }
    write_uoffset(&object_offset, (uoffset_t)object_ref - buffer_base);
    if (0 == (buffer_ref = emit_front(B, &iov))) {
        check(__SET_ASSERT__, "emitter rejected buffer content");
        return 0;
    }
    return buffer_ref;
}

flatcc_builder_ref_t flatcc_builder_create_struct(flatcc_builder_t *B, const void *data, size_t size, uint16_t align)
{
    size_t pad;
    iov_state_t iov;

    check(align >= 1, "align cannot be 0");
    set_min_align(B, align);
    pad = front_pad(B, (uoffset_t)size, align);
    init_iov();
    push_iov(data, size);
    /*
     * Normally structs will already be a multiple of their alignment,
     * so this padding will not likely be emitted.
     */
    push_iov(_pad, pad);
    return emit_front(B, &iov);
}

int flatcc_builder_start_buffer(flatcc_builder_t *B,
        const char identifier[identifier_size], uint16_t block_align, int flags)
{
    /*
     * This saves the parent `min_align` in the align field since we
     * shouldn't use that for the current buffer. `exit_frame`
     * automatically aggregates align up, so it is updated when the
     * buffer frame exits.
     */
    if (enter_frame(B, B->min_align)) {
        return -1;
    }
    /* B->align now has parent min_align, and child frames will save it. */
    B->min_align = 1;
    /* Save the parent block align, and set proper defaults for this buffer. */
    frame(container.buffer.block_align) = B->block_align;
    B->block_align = block_align;
    frame(container.buffer.flags = B->buffer_flags);
    B->buffer_flags = (uint16_t)flags;
    frame(container.buffer.mark) = B->buffer_mark;
    frame(container.buffer.nest_id) = B->nest_id;
    /*
     * End of buffer when nested. Not defined for top-level because we
     * here (on only here) permit strings etc. to be created before buffer start and
     * because top-level buffer vtables can be clustered.
     */
    B->buffer_mark = B->emit_start;
    /* Must be 0 before and after entering top-level buffer, and unique otherwise. */
    B->nest_id = B->nest_count++;
    frame(container.buffer.identifier) = B->identifier;
    set_identifier(identifier);
    frame(type) = flatcc_builder_buffer;
    return 0;
}

flatcc_builder_ref_t flatcc_builder_end_buffer(flatcc_builder_t *B, flatcc_builder_ref_t root)
{
    flatcc_builder_ref_t buffer_ref;
    int flags;

    flags = B->buffer_flags & flatcc_builder_with_size;
    flags |= is_top_buffer(B) ? 0 : flatcc_builder_is_nested;
    check(frame(type) == flatcc_builder_buffer, "expected buffer frame");
    set_min_align(B, B->block_align);
    if (0 == (buffer_ref = flatcc_builder_create_buffer(B, (void *)&B->identifier,
            B->block_align, root, B->min_align, flags))) {
        return 0;
    }
    B->buffer_mark = frame(container.buffer.mark);
    B->nest_id = frame(container.buffer.nest_id);
    B->identifier = frame(container.buffer.identifier);
    B->buffer_flags = frame(container.buffer.flags);
    exit_frame(B);
    return buffer_ref;
}

void *flatcc_builder_start_struct(flatcc_builder_t *B, size_t size, uint16_t align)
{
    /* Allocate space for the struct on the ds stack. */
    if (enter_frame(B, align)) {
        return 0;
    }
    frame(type) = flatcc_builder_struct;
    refresh_ds(B, data_limit);
    return push_ds(B, (uoffset_t)size);
}

void *flatcc_builder_struct_edit(flatcc_builder_t *B)
{
    return B->ds;
}

flatcc_builder_ref_t flatcc_builder_end_struct(flatcc_builder_t *B)
{
    flatcc_builder_ref_t object_ref;

    check(frame(type) == flatcc_builder_struct, "expected struct frame");
    if (0 == (object_ref = flatcc_builder_create_struct(B, B->ds, B->ds_offset, B->align))) {
        return 0;
    }
    exit_frame(B);
    return object_ref;
}

static inline int vector_count_add(flatcc_builder_t *B, uoffset_t count, uoffset_t max_count)
{
    uoffset_t n, n1;
    n = frame(container.vector.count);
    n1 = n + count;
    /*
     * This prevents elem_size * count from overflowing iff max_vector
     * has been set sensible. Without this check we might allocate to
     * little on the ds stack and return a buffer the user thinks is
     * much larger which of course is bad even though the buffer eventually
     * would fail anyway.
     */
    check_error(n <= n1 && n1 <= max_count, -1, "vector too large to represent");
    frame(container.vector.count) = n1;
    return 0;
}

void *flatcc_builder_extend_vector(flatcc_builder_t *B, size_t count)
{
    if (vector_count_add(B, (uoffset_t)count, frame(container.vector.max_count))) {
        return 0;
    }
    return push_ds(B, frame(container.vector.elem_size) * (uoffset_t)count);
}

void *flatcc_builder_vector_push(flatcc_builder_t *B, const void *data)
{
    check(frame(type) == flatcc_builder_vector, "expected vector frame");
    check_error(frame(container.vector.count) <= frame(container.vector.max_count), 0, "vector max count exceeded");
    frame(container.vector.count) += 1;
    return push_ds_copy(B, data, frame(container.vector.elem_size));
}

void *flatcc_builder_append_vector(flatcc_builder_t *B, const void *data, size_t count)
{
    check(frame(type) == flatcc_builder_vector, "expected vector frame");
    if (vector_count_add(B, (uoffset_t)count, frame(container.vector.max_count))) {
        return 0;
    }
    return push_ds_copy(B, data, frame(container.vector.elem_size) * (uoffset_t)count);
}

flatcc_builder_ref_t *flatcc_builder_extend_offset_vector(flatcc_builder_t *B, size_t count)
{
    if (vector_count_add(B, (uoffset_t)count, max_offset_count)) {
        return 0;
    }
    return push_ds(B, (uoffset_t)(field_size * count));
}

flatcc_builder_ref_t *flatcc_builder_offset_vector_push(flatcc_builder_t *B, flatcc_builder_ref_t ref)
{
    flatcc_builder_ref_t *p;

    check(frame(type) == flatcc_builder_offset_vector, "expected offset vector frame");
    if (frame(container.vector.count) == max_offset_count) {
        return 0;
    }
    frame(container.vector.count) += 1;
    if (0 == (p = push_ds(B, field_size))) {
        return 0;
    }
    *p = ref;
    return p;
}

flatcc_builder_ref_t *flatcc_builder_append_offset_vector(flatcc_builder_t *B, const flatcc_builder_ref_t *refs, size_t count)
{
    check(frame(type) == flatcc_builder_offset_vector, "expected offset vector frame");
    if (vector_count_add(B, (uoffset_t)count, max_offset_count)) {
        return 0;
    }
    return push_ds_copy(B, refs, (uoffset_t)(field_size * count));
}

char *flatcc_builder_extend_string(flatcc_builder_t *B, size_t len)
{
    check(frame(type) == flatcc_builder_string, "expected string frame");
    if (vector_count_add(B, (uoffset_t)len, max_string_len)) {
        return 0;
    }
    return push_ds(B, (uoffset_t)len);
}

char *flatcc_builder_append_string(flatcc_builder_t *B, const char *s, size_t len)
{
    check(frame(type) == flatcc_builder_string, "expected string frame");
    if (vector_count_add(B, (uoffset_t)len, max_string_len)) {
        return 0;
    }
    return push_ds_copy(B, s, (uoffset_t)len);
}

char *flatcc_builder_append_string_str(flatcc_builder_t *B, const char *s)
{
    return flatcc_builder_append_string(B, s, strlen(s));
}

char *flatcc_builder_append_string_strn(flatcc_builder_t *B, const char *s, size_t max_len)
{
    return flatcc_builder_append_string(B, s, strnlen(s, max_len));
}

int flatcc_builder_truncate_vector(flatcc_builder_t *B, size_t count)
{
    check(frame(type) == flatcc_builder_vector, "expected vector frame");
    check_error(frame(container.vector.count) >= count, -1, "cannot truncate vector past empty");
    frame(container.vector.count) -= (uoffset_t)count;
    unpush_ds(B, frame(container.vector.elem_size) * (uoffset_t)count);
    return 0;
}

int flatcc_builder_truncate_offset_vector(flatcc_builder_t *B, size_t count)
{
    check(frame(type) == flatcc_builder_offset_vector, "expected offset vector frame");
    check_error(frame(container.vector.count) >= (uoffset_t)count, -1, "cannot truncate vector past empty");
    frame(container.vector.count) -= (uoffset_t)count;
    unpush_ds(B, frame(container.vector.elem_size) * (uoffset_t)count);
    return 0;
}

int flatcc_builder_truncate_string(flatcc_builder_t *B, size_t len)
{
    check(frame(type) == flatcc_builder_string, "expected string frame");
    check_error(frame(container.vector.count) >= len, -1, "cannot truncate string past empty");
    frame(container.vector.count) -= (uoffset_t)len;
    unpush_ds(B, (uoffset_t)len);
    return 0;
}

int flatcc_builder_start_vector(flatcc_builder_t *B, size_t elem_size, uint16_t align, size_t max_count)
{
    get_min_align(&align, field_size);
    if (enter_frame(B, align)) {
        return -1;
    }
    frame(container.vector.elem_size) = (uoffset_t)elem_size;
    frame(container.vector.count) = 0;
    frame(container.vector.max_count) = (uoffset_t)max_count;
    frame(type) = flatcc_builder_vector;
    refresh_ds(B, data_limit);
    return 0;
}

int flatcc_builder_start_offset_vector(flatcc_builder_t *B)
{
    if (enter_frame(B, field_size)) {
        return -1;
    }
    frame(container.vector.elem_size) = field_size;
    frame(container.vector.count) = 0;
    frame(type) = flatcc_builder_offset_vector;
    refresh_ds(B, data_limit);
    return 0;
}

flatcc_builder_ref_t flatcc_builder_create_offset_vector(flatcc_builder_t *B,
        const flatcc_builder_ref_t *vec, size_t count)
{
    flatcc_builder_ref_t *_vec;

    if (flatcc_builder_start_offset_vector(B)) {
        return 0;
    }
    if (!(_vec = flatcc_builder_extend_offset_vector(B, count))) {
        return 0;
    }
    memcpy(_vec, vec, count * field_size);
    return flatcc_builder_end_offset_vector(B);
}

int flatcc_builder_start_string(flatcc_builder_t *B)
{
    if (enter_frame(B, 1)) {
        return -1;
    }
    frame(container.vector.elem_size) = 1;
    frame(container.vector.count) = 0;
    frame(type) = flatcc_builder_string;
    refresh_ds(B, data_limit);
    return 0;
}

int flatcc_builder_reserve_table(flatcc_builder_t *B, int count)
{
    check(count >= 0, "cannot reserve negative count");
    return reserve_fields(B, count);
}

int flatcc_builder_start_table(flatcc_builder_t *B, int count)
{
    if (enter_frame(B, field_size)) {
        return -1;
    }
    frame(container.table.vs_end) = vs_offset(B->vs);
    frame(container.table.pl_end) = pl_offset(B->pl);
    frame(container.table.vt_hash) = B->vt_hash;
    frame(container.table.id_end) = B->id_end;
    B->vt_hash = 0;
    FLATCC_BUILDER_INIT_VT_HASH(B->vt_hash);
    B->id_end = 0;
    frame(type) = flatcc_builder_table;
    if (reserve_fields(B, count)) {
        return -1;
    }
    refresh_ds(B, table_limit);
    return 0;
}

flatcc_builder_vt_ref_t flatcc_builder_create_vtable(flatcc_builder_t *B,
        const voffset_t *vt, voffset_t vt_size)
{
    flatcc_builder_vt_ref_t vt_ref;
    iov_state_t iov;
    voffset_t *vt_;
    size_t i;

    /*
     * Only top-level buffer can cluster vtables because only it can
     * extend beyond the end.
     *
     * We write the vtable after the referencing table to maintain
     * the construction invariant that any offset reference has
     * valid emitted data at a higher address, and also that any
     * issued negative emit address represents an offset reference
     * to some flatbuffer object or vector (or possibly a root
     * struct).
     *
     * The vt_ref is stored as the reference + 1 to avoid having 0 as a
     * valid reference (which usally means error). It also idententifies
     * vtable references as the only uneven references, and the only
     * references that can be used multiple times in the same buffer.
     *
     * We do the vtable conversion here so cached vtables can be built
     * hashed and compared more efficiently, and so end users with
     * direct vtable construction don't have to worry about endianness.
     * This also ensures the hash function works the same wrt.
     * collision frequency.
     */

    if (!flatbuffers_is_native_pe()) {
        /* Make space in vtable cache for temporary endian conversion. */
        if (!(vt_ = reserve_buffer(B, flatcc_builder_alloc_vb, B->vb_end, vt_size, 0))) {
            return 0;
        }
        for (i = 0; i < vt_size / sizeof(voffset_t); ++i) {
            write_voffset(&vt_[i], vt[i]);
        }
        vt = vt_;
        /* We don't need to free the reservation since we don't advance any base pointer. */
    }

    init_iov();
    push_iov(vt, vt_size);
    if (is_top_buffer(B) && !B->disable_vt_clustering) {
        /* Note that `emit_back` already returns ref + 1 as we require for vtables. */
        if (0 == (vt_ref = emit_back(B, &iov))) {
            return 0;
        }
    } else {
        if (0 == (vt_ref = emit_front(B, &iov))) {
            return 0;
        }
        /*
         * We don't have a valid 0 ref here, but to be consistent with
         * clustered vtables we offset by one. This cannot be zero
         * either.
         */
        vt_ref += 1;
    }
    return vt_ref;
}

flatcc_builder_vt_ref_t flatcc_builder_create_cached_vtable(flatcc_builder_t *B,
        const voffset_t *vt, voffset_t vt_size, uint32_t vt_hash)
{
    vtable_descriptor_t *vd, *vd2;
    uoffset_t *pvd, *pvd_head;
    uoffset_t next;
    voffset_t *vt_;

    /* This just gets the hash table slot, we still have to inspect it. */
    if (!(pvd_head = lookup_ht(B, vt_hash))) {
        return 0;
    }
    pvd = pvd_head;
    next = *pvd;
    /* Tracks if there already is a cached copy. */
    vd2 = 0;
    while (next) {
        vd = vd_ptr(next);
        vt_ = vb_ptr(vd->vb_start);
        if (vt_[0] != vt_size || 0 != memcmp(vt, vt_, vt_size)) {
            pvd = &vd->next;
            next = vd->next;
            continue;
        }
        /* Can't share emitted vtables between buffers, */
        if (vd->nest_id != B->nest_id) {
            /* but we don't have to resubmit to cache. */
            vd2 = vd;
            /* See if there is a better match. */
            pvd = &vd->next;
            next = vd->next;
            continue;
        }
        /* Move to front hash strategy. */
        if (pvd != pvd_head) {
            *pvd = vd->next;
            vd->next = *pvd_head;
            *pvd_head = next;
        }
        /* vtable exists and has been emitted within current buffer. */
        return vd->vt_ref;
    }
    /* Allocate new descriptor. */
    if (!(vd = reserve_buffer(B, flatcc_builder_alloc_vd, B->vd_end, sizeof(vtable_descriptor_t), 0))) {
        return 0;
    }
    next = B->vd_end;
    B->vd_end += (uoffset_t)sizeof(vtable_descriptor_t);

    /* Identify the buffer this vtable descriptor belongs to. */
    vd->nest_id = B->nest_id;

    /* Move to front hash strategy. */
    vd->next = *pvd_head;
    *pvd_head = next;
    if (0 == (vd->vt_ref = flatcc_builder_create_vtable(B, vt, vt_size))) {
        return 0;
    }
    if (vd2) {
        /* Reuse cached copy. */
        vd->vb_start = vd2->vb_start;
    } else {
        if (B->vb_flush_limit && B->vb_flush_limit < B->vb_end + vt_size) {
            flatcc_builder_flush_vtable_cache(B);
        } else {
            /* Make space in vtable cache. */
            if (!(vt_ = reserve_buffer(B, flatcc_builder_alloc_vb, B->vb_end, vt_size, 0))) {
                return -1;
            }
            vd->vb_start = B->vb_end;
            B->vb_end += vt_size;
            memcpy(vt_, vt, vt_size);
        }
    }
    return vd->vt_ref;
}

flatcc_builder_ref_t flatcc_builder_create_table(flatcc_builder_t *B, const void *data, size_t size, uint16_t align,
        flatbuffers_voffset_t *offsets, int offset_count, flatcc_builder_vt_ref_t vt_ref)
{
    int i;
    uoffset_t pad, vt_offset, vt_offset_field, vt_base, base, offset, *offset_field;
    iov_state_t iov;

    check(offset_count >= 0, "expected non-negative offset_count");
    /*
     * vtable references are offset by 1 to avoid confusion with
     * 0 as an error reference. It also uniquely identifies them
     * as vtables being the only uneven reference type.
     */
    check(vt_ref & 1, "invalid vtable referenc");
    get_min_align(&align, field_size);
    set_min_align(B, align);
    /* Alignment is calculated for the first element, not the header. */
    pad = front_pad(B, (uoffset_t)size, align);
    base = (uoffset_t)B->emit_start - (uoffset_t)(pad + size + field_size);
    /* Adjust by 1 to get unencoded vtable reference. */
    vt_base = (uoffset_t)(vt_ref - 1);
    vt_offset = base - vt_base;
    /* Avoid overflow. */
    if (base - vt_offset != vt_base) {
        return -1;
    }
    /* Protocol endian encoding. */
    write_uoffset(&vt_offset_field, vt_offset);
    for (i = 0; i < offset_count; ++i) {
        offset_field = (uoffset_t *)((size_t)data + offsets[i]);
        offset = *offset_field - base - offsets[i] - (uoffset_t)field_size;
        write_uoffset(offset_field, offset);
    }
    init_iov();
    push_iov(&vt_offset_field, field_size);
    push_iov(data, size);
    push_iov(_pad, pad);
    return emit_front(B, &iov);
}

int flatcc_builder_check_required_field(flatcc_builder_t *B, flatbuffers_voffset_t id)
{
    check(frame(type) == flatcc_builder_table, "expected table frame");

    return id < B->id_end && B->vs[id] != 0;
}

int flatcc_builder_check_union_field(flatcc_builder_t *B, flatbuffers_voffset_t id)
{
    check(frame(type) == flatcc_builder_table, "expected table frame");

    if (id == 0 || id >= B->id_end) {
        return 0;
    }
    if (B->vs[id - 1] == 0) {
        return B->vs[id] == 0;
    }
    if (*(uint8_t *)(B->ds + B->vs[id - 1])) {
        return B->vs[id] != 0;
    }
    return B->vs[id] == 0;
}

int flatcc_builder_check_required(flatcc_builder_t *B, const flatbuffers_voffset_t *required, int count)
{
    int i;

    check(frame(type) == flatcc_builder_table, "expected table frame");

    if (B->id_end < count) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (B->vs[required[i]] == 0) {
            return 0;
        }
    }
    return 1;
}

flatcc_builder_ref_t flatcc_builder_end_table(flatcc_builder_t *B)
{
    voffset_t *vt, vt_size;
    flatcc_builder_ref_t table_ref, vt_ref;
    int pl_count;
    voffset_t *pl;

    check(frame(type) == flatcc_builder_table, "expected table frame");

    /* We have `ds_limit`, so we should not have to check for overflow here. */

    vt = B->vs - 2;
    vt_size = (voffset_t)(sizeof(voffset_t) * (B->id_end + 2u));
    /* Update vtable header fields, first vtable size, then object table size. */
    vt[0] = vt_size;
    /*
     * The `ds` buffer is always at least `field_size` aligned but excludes the
     * initial vtable offset field. Therefore `field_size` is added here
     * to the total table size in the vtable.
     */
    vt[1] = (voffset_t)(B->ds_offset + field_size);
    FLATCC_BUILDER_UPDATE_VT_HASH(B->vt_hash, (uint32_t)vt[0], (uint32_t)vt[1]);
    /* Find already emitted vtable, or emit a new one. */
    if (!(vt_ref = flatcc_builder_create_cached_vtable(B, vt, vt_size, B->vt_hash))) {
        return 0;
    }
    /* Clear vs stack so it is ready for the next vtable (ds stack is cleared by exit frame). */
    memset(vt, 0, vt_size);

    pl = pl_ptr(frame(container.table.pl_end));
    pl_count = (int)(B->pl - pl);
    if (0 == (table_ref = flatcc_builder_create_table(B, B->ds, B->ds_offset, B->align, pl, pl_count, vt_ref))) {
        return 0;
    }
    B->vt_hash = frame(container.table.vt_hash);
    B->id_end = frame(container.table.id_end);
    B->vs = vs_ptr(frame(container.table.vs_end));
    B->pl = pl_ptr(frame(container.table.pl_end));
    exit_frame(B);
    return table_ref;
}

flatcc_builder_ref_t flatcc_builder_create_vector(flatcc_builder_t *B,
        const void *data, size_t count, size_t elem_size, uint16_t align, size_t max_count)
{
    /*
     * Note: it is important that vec_size is uoffset not size_t
     * in case sizeof(uoffset_t) > sizeof(size_t) because max_count is
     * defined in terms of uoffset_t representation size, and also
     * because we risk accepting too large a vector even if max_count is
     * not violated.
     */
    uoffset_t vec_size, vec_pad, length_prefix;
    iov_state_t iov;

    check_error(count <= max_count, 0, "vector max_count violated");
    get_min_align(&align, field_size);
    set_min_align(B, align);
    vec_size = (uoffset_t)count * (uoffset_t)elem_size;
    /*
     * That can happen on 32 bit systems when uoffset_t is defined as 64-bit.
     * `emit_front/back` captures overflow, but not if our size type wraps first.
     */
#if FLATBUFFERS_UOFFSET_MAX > SIZE_MAX
    check_error(vec_size < SIZE_MAX, 0, "vector larger than address space");
#endif
    write_uoffset(&length_prefix, (uoffset_t)count);
    /* Alignment is calculated for the first element, not the header. */
    vec_pad = front_pad(B, vec_size, align);
    init_iov();
    push_iov(&length_prefix, field_size);
    push_iov(data, vec_size);
    push_iov(_pad, vec_pad);
    return emit_front(B, &iov);
}

/*
 * Note: FlatBuffers official documentation states that the size field of a
 * vector is a 32-bit element count. It is not quite clear if the
 * intention is to have the size field be of type uoffset_t since tables
 * also have a uoffset_t sized header, or if the vector size should
 * remain unchanged if uoffset is changed to 16- or 64-bits
 * respectively. Since it makes most sense to have a vector compatible
 * with the addressable space, we choose to use uoffset_t as size field,
 * which remains compatible with the default 32-bit version of uoffset_t.
 */
flatcc_builder_ref_t flatcc_builder_end_vector(flatcc_builder_t *B)
{
    flatcc_builder_ref_t vector_ref;

    check(frame(type) == flatcc_builder_vector, "expected vector frame");

    if (0 == (vector_ref = flatcc_builder_create_vector(B, B->ds,
            frame(container.vector.count), frame(container.vector.elem_size),
            B->align, frame(container.vector.max_count)))) {
        return 0;
    }
    exit_frame(B);
    return vector_ref;
}

size_t flatcc_builder_vector_count(flatcc_builder_t *B)
{
    return frame(container.vector.count);
}

void *flatcc_builder_vector_edit(flatcc_builder_t *B)
{
    return B->ds;
}

/* This function destroys the source content but avoids stack allocation. */
static flatcc_builder_ref_t _create_offset_vector_direct(flatcc_builder_t *B,
        flatcc_builder_ref_t *vec, size_t count, const utype_t *types)
{
    uoffset_t vec_size, vec_pad;
    uoffset_t length_prefix, offset;
    uoffset_t i;
    soffset_t base;
    iov_state_t iov;

    if ((uoffset_t)count > max_offset_count) {
        return 0;
    }
    set_min_align(B, field_size);
    vec_size = (uoffset_t)(count * field_size);
    write_uoffset(&length_prefix, (uoffset_t)count);
    /* Alignment is calculated for the first element, not the header. */
    vec_pad = front_pad(B, vec_size, field_size);
    init_iov();
    push_iov(&length_prefix, field_size);
    push_iov(vec, vec_size);
    push_iov(_pad, vec_pad);
    base = B->emit_start - (soffset_t)iov.len;
    for (i = 0; i < (uoffset_t)count; ++i) {
        /*
         * 0 is either end of buffer, start of vtables, or start of
         * buffer depending on the direction in which the buffer is
         * built. None of these can create a valid 0 reference but it
         * is easy to create by mistake when manually building offset
         * vectors.
         *
         * Unions do permit nulls, but only when the type is NONE.
         */
        if (vec[i] != 0) {
            offset = (uoffset_t)
                (vec[i] - base - (soffset_t)(i * field_size) - (soffset_t)field_size);
            write_uoffset(&vec[i], offset);
            if (types) {
                check(types[i] != 0, "union vector cannot have non-null element with type NONE");
            }
        } else {
            if (types) {
                check(types[i] == 0, "union vector cannot have null element without type NONE");
            } else {
                check(__SET_ASSERT__, "offset vector cannot have null element");
            }
        }
    }
    return emit_front(B, &iov);
}

flatcc_builder_ref_t flatcc_builder_create_offset_vector_direct(flatcc_builder_t *B,
        flatcc_builder_ref_t *vec, size_t count)
{
    return _create_offset_vector_direct(B, vec, count, 0);
}

flatcc_builder_ref_t flatcc_builder_end_offset_vector(flatcc_builder_t *B)
{
    flatcc_builder_ref_t vector_ref;

    check(frame(type) == flatcc_builder_offset_vector, "expected offset vector frame");
    if (0 == (vector_ref = flatcc_builder_create_offset_vector_direct(B,
            (flatcc_builder_ref_t *)B->ds, frame(container.vector.count)))) {
        return 0;
    }
    exit_frame(B);
    return vector_ref;
}

flatcc_builder_ref_t flatcc_builder_end_offset_vector_for_unions(flatcc_builder_t *B, const utype_t *types)
{
    flatcc_builder_ref_t vector_ref;

    check(frame(type) == flatcc_builder_offset_vector, "expected offset vector frame");
    if (0 == (vector_ref = _create_offset_vector_direct(B,
            (flatcc_builder_ref_t *)B->ds, frame(container.vector.count), types))) {
        return 0;
    }
    exit_frame(B);
    return vector_ref;
}

void *flatcc_builder_offset_vector_edit(flatcc_builder_t *B)
{
    return B->ds;
}

size_t flatcc_builder_offset_vector_count(flatcc_builder_t *B)
{
    return frame(container.vector.count);
}

int flatcc_builder_table_add_union(flatcc_builder_t *B, int id,
    flatcc_builder_union_ref_t uref)
{
    flatcc_builder_ref_t *pref;
    flatcc_builder_utype_t *putype;

    check(frame(type) == flatcc_builder_table, "expected table frame");
    check_error(uref.type != 0 || uref.value == 0, -1, "expected null value for type NONE");
    if (uref.value != 0) {
        pref = flatcc_builder_table_add_offset(B, id);
        check_error(pref != 0, -1, "unable to add union value");
        *pref = uref.value;
    }
    putype = flatcc_builder_table_add(B, id - 1, utype_size, utype_size);
    check_error(putype != 0, -1, "unable to add union type");
    write_utype(putype, uref.type);
    return 0;
}

int flatcc_builder_table_add_union_vector(flatcc_builder_t *B, int id,
        flatcc_builder_union_vec_ref_t uvref)
{
    flatcc_builder_ref_t *pref;

    check(frame(type) == flatcc_builder_table, "expected table frame");
    check_error((uvref.type == 0) == (uvref.value == 0), -1, "expected both type and value vector, or neither");
    if (uvref.type != 0) {
        pref = flatcc_builder_table_add_offset(B, id - 1);
        check_error(pref != 0, -1, "unable to add union member");
        *pref = uvref.type;

        pref = flatcc_builder_table_add_offset(B, id);
        check_error(pref != 0, -1, "unable to add union member");
        *pref = uvref.value;
    }
    return 0;
}

flatcc_builder_union_vec_ref_t flatcc_builder_create_union_vector(flatcc_builder_t *B,
        const flatcc_builder_union_ref_t *urefs, size_t count)
{
    flatcc_builder_union_vec_ref_t uvref = { 0, 0 };
    flatcc_builder_utype_t *types;
    flatcc_builder_ref_t *refs;
    size_t i;

    if (flatcc_builder_start_offset_vector(B)) {
        return uvref;
    }
    if (0 == flatcc_builder_extend_offset_vector(B, count)) {
        return uvref;
    }
    if (0 == (types = push_ds(B, (uoffset_t)(utype_size * count)))) {
        return uvref;
    }

    /* Safe even if push_ds caused stack reallocation. */
    refs = flatcc_builder_offset_vector_edit(B);

    for (i = 0; i < count; ++i) {
        types[i] = urefs[i].type;
        refs[i] = urefs[i].value;
    }
    uvref = flatcc_builder_create_union_vector_direct(B,
            types, refs, count);
    /* No need to clean up after out temporary types vector. */
    exit_frame(B);
    return uvref;
}

flatcc_builder_union_vec_ref_t flatcc_builder_create_union_vector_direct(flatcc_builder_t *B,
        const flatcc_builder_utype_t *types, flatcc_builder_ref_t *data, size_t count)
{
    flatcc_builder_union_vec_ref_t uvref = { 0, 0 };

    if (0 == (uvref.value = _create_offset_vector_direct(B, data, count, types))) {
        return uvref;
    }
    if (0 == (uvref.type = flatcc_builder_create_type_vector(B, types, count))) {
        return uvref;
    }
    return uvref;
}

flatcc_builder_ref_t flatcc_builder_create_type_vector(flatcc_builder_t *B,
        const flatcc_builder_utype_t *types, size_t count)
{
    return flatcc_builder_create_vector(B, types, count,
                    utype_size, utype_size, max_utype_count);
}

int flatcc_builder_start_union_vector(flatcc_builder_t *B)
{
    if (enter_frame(B, field_size)) {
        return -1;
    }
    frame(container.vector.elem_size) = union_size;
    frame(container.vector.count) = 0;
    frame(type) = flatcc_builder_union_vector;
    refresh_ds(B, data_limit);
    return 0;
}

flatcc_builder_union_vec_ref_t flatcc_builder_end_union_vector(flatcc_builder_t *B)
{
    flatcc_builder_union_vec_ref_t uvref = { 0, 0 };
    flatcc_builder_utype_t *types;
    flatcc_builder_union_ref_t *urefs;
    flatcc_builder_ref_t *refs;
    size_t i, count;

    check(frame(type) == flatcc_builder_union_vector, "expected union vector frame");

    /*
     * We could split the union vector in-place, but then we would have
     * to deal with strict pointer aliasing rules which is not worthwhile
     * so we create a new offset and type vector on the stack.
     *
     * We assume the stack is sufficiently aligned as is.
     */
    count = flatcc_builder_union_vector_count(B);
    if (0 == (refs = push_ds(B, (uoffset_t)(count * (utype_size + field_size))))) {
        return uvref;
    }
    types = (flatcc_builder_utype_t *)(refs + count);

    /* Safe even if push_ds caused stack reallocation. */
    urefs = flatcc_builder_union_vector_edit(B);

    for (i = 0; i < count; ++i) {
        types[i] = urefs[i].type;
        refs[i] = urefs[i].value;
    }
    uvref = flatcc_builder_create_union_vector_direct(B, types, refs, count);
    /* No need to clean up after out temporary types vector. */
    exit_frame(B);
    return uvref;
}

void *flatcc_builder_union_vector_edit(flatcc_builder_t *B)
{
    return B->ds;
}

size_t flatcc_builder_union_vector_count(flatcc_builder_t *B)
{
    return frame(container.vector.count);
}

flatcc_builder_union_ref_t *flatcc_builder_extend_union_vector(flatcc_builder_t *B, size_t count)
{
    if (vector_count_add(B, (uoffset_t)count, max_union_count)) {
        return 0;
    }
    return push_ds(B, (uoffset_t)(union_size * count));
}

int flatcc_builder_truncate_union_vector(flatcc_builder_t *B, size_t count)
{
    check(frame(type) == flatcc_builder_union_vector, "expected union vector frame");
    check_error(frame(container.vector.count) >= (uoffset_t)count, -1, "cannot truncate vector past empty");
    frame(container.vector.count) -= (uoffset_t)count;
    unpush_ds(B, frame(container.vector.elem_size) * (uoffset_t)count);
    return 0;
}

flatcc_builder_union_ref_t *flatcc_builder_union_vector_push(flatcc_builder_t *B,
        flatcc_builder_union_ref_t uref)
{
    flatcc_builder_union_ref_t *p;

    check(frame(type) == flatcc_builder_union_vector, "expected union vector frame");
    if (frame(container.vector.count) == max_union_count) {
        return 0;
    }
    frame(container.vector.count) += 1;
    if (0 == (p = push_ds(B, union_size))) {
        return 0;
    }
    *p = uref;
    return p;
}

flatcc_builder_union_ref_t *flatcc_builder_append_union_vector(flatcc_builder_t *B,
        const flatcc_builder_union_ref_t *urefs, size_t count)
{
    check(frame(type) == flatcc_builder_union_vector, "expected union vector frame");
    if (vector_count_add(B, (uoffset_t)count, max_union_count)) {
        return 0;
    }
    return push_ds_copy(B, urefs, (uoffset_t)(union_size * count));
}

flatcc_builder_ref_t flatcc_builder_create_string(flatcc_builder_t *B, const char *s, size_t len)
{
    uoffset_t s_pad;
    uoffset_t length_prefix;
    iov_state_t iov;

#if max_string_len < UINT32_MAX
    if (len > max_string_len) {
        return 0;
    }
#endif
    write_uoffset(&length_prefix, (uoffset_t)len);
    /* Add 1 for zero termination. */
    s_pad = front_pad(B, (uoffset_t)len + 1, field_size) + 1;
    init_iov();
    push_iov(&length_prefix, field_size);
    push_iov(s, len);
    push_iov(_pad, s_pad);
    return emit_front(B, &iov);
}

flatcc_builder_ref_t flatcc_builder_create_string_str(flatcc_builder_t *B, const char *s)
{
    return flatcc_builder_create_string(B, s, strlen(s));
}

flatcc_builder_ref_t flatcc_builder_create_string_strn(flatcc_builder_t *B, const char *s, size_t max_len)
{
    return flatcc_builder_create_string(B, s, strnlen(s, max_len));
}

flatcc_builder_ref_t flatcc_builder_end_string(flatcc_builder_t *B)
{
    flatcc_builder_ref_t string_ref;

    check(frame(type) == flatcc_builder_string, "expected string frame");
    FLATCC_ASSERT(frame(container.vector.count) == B->ds_offset);
    if (0 == (string_ref = flatcc_builder_create_string(B,
            (const char *)B->ds, B->ds_offset))) {
        return 0;
    }
    exit_frame(B);
    return string_ref;
}

char *flatcc_builder_string_edit(flatcc_builder_t *B)
{
    return (char *)B->ds;
}

size_t flatcc_builder_string_len(flatcc_builder_t *B)
{
    return frame(container.vector.count);
}

void *flatcc_builder_table_add(flatcc_builder_t *B, int id, size_t size, uint16_t align)
{
    /*
     * We align the offset relative to the first table field, excluding
     * the header holding the vtable reference. On the stack, `ds_first`
     * is aligned to 8 bytes thanks to the `enter_frame` logic, and this
     * provides a safe way to update the fields on the stack, but here
     * we are concerned with the target buffer alignment.
     *
     * We could also have aligned relative to the end of the table which
     * would allow us to emit each field immediately, but it would be a
     * confusing user experience wrt. field ordering, and it would add
     * more variability to vtable layouts, thus reducing reuse, and
     * frequent emissions to external emitter interface would be
     * sub-optimal. Also, with that appoach, the vtable offsets would
     * have to be adjusted at table end.
     *
     * As we have it, each emit occur at table end, vector end, string
     * end, or buffer end, which might be helpful to various backend
     * processors.
     */
    check(frame(type) == flatcc_builder_table, "expected table frame");
    check(id >= 0 && id <= (int)FLATBUFFERS_ID_MAX, "table id out of range");
    if (align > B->align) {
        B->align = align;
    }
#if FLATCC_BUILDER_ALLOW_REPEAT_TABLE_ADD
    if (B->vs[id] != 0) {
        return B->ds + B->vs[id] - field_size;
    }
#else
    if (B->vs[id] != 0) {
        check(__SET_ASSERT__, "table field already set");
        return 0;
    }
#endif
    FLATCC_BUILDER_UPDATE_VT_HASH(B->vt_hash, (uint32_t)id, (uint32_t)size);
    return push_ds_field(B, (uoffset_t)size, align, (voffset_t)id);
}

void *flatcc_builder_table_edit(flatcc_builder_t *B, size_t size)
{
    check(frame(type) == flatcc_builder_table, "expected table frame");

    return B->ds + B->ds_offset - size;
}

void *flatcc_builder_table_add_copy(flatcc_builder_t *B, int id, const void *data, size_t size, uint16_t align)
{
    void *p;

    if ((p = flatcc_builder_table_add(B, id, size, align))) {
        memcpy(p, data, size);
    }
    return p;
}

flatcc_builder_ref_t *flatcc_builder_table_add_offset(flatcc_builder_t *B, int id)
{
    check(frame(type) == flatcc_builder_table, "expected table frame");
    check(id >= 0 && id <= (int)FLATBUFFERS_ID_MAX, "table id out of range");
#if FLATCC_BUILDER_ALLOW_REPEAT_TABLE_ADD
    if (B->vs[id] != 0) {
        return B->ds + B->vs[id] - field_size;
    }
#else
    if (B->vs[id] != 0) {
        check(__SET_ASSERT__, "table field already set");
        return 0;
    }
#endif
    FLATCC_BUILDER_UPDATE_VT_HASH(B->vt_hash, (uint32_t)id, (uint32_t)field_size);
    return push_ds_offset_field(B, (voffset_t)id);
}

uint16_t flatcc_builder_push_buffer_alignment(flatcc_builder_t *B)
{
    uint16_t old_min_align = B->min_align;

    B->min_align = field_size;
    return old_min_align;
}

void flatcc_builder_pop_buffer_alignment(flatcc_builder_t *B, uint16_t pushed_align)
{
    set_min_align(B, pushed_align);
}

uint16_t flatcc_builder_get_buffer_alignment(flatcc_builder_t *B)
{
    return B->min_align;
}

void flatcc_builder_set_vtable_clustering(flatcc_builder_t *B, int enable)
{
    /* Inverted because we zero all memory in B on init. */
    B->disable_vt_clustering = !enable;
}

void flatcc_builder_set_block_align(flatcc_builder_t *B, uint16_t align)
{
    B->block_align = align;
}

int flatcc_builder_get_level(flatcc_builder_t *B)
{
    return B->level;
}

void flatcc_builder_set_max_level(flatcc_builder_t *B, int max_level)
{
    B->max_level = max_level;
    if (B->limit_level < B->max_level) {
        B->limit_level = B->max_level;
    }
}

size_t flatcc_builder_get_buffer_size(flatcc_builder_t *B)
{
    return (size_t)(B->emit_end - B->emit_start);
}

flatcc_builder_ref_t flatcc_builder_get_buffer_start(flatcc_builder_t *B)
{
    return B->emit_start;
}

flatcc_builder_ref_t flatcc_builder_get_buffer_end(flatcc_builder_t *B)
{
    return B->emit_end;
}

void flatcc_builder_set_vtable_cache_limit(flatcc_builder_t *B, size_t size)
{
    B->vb_flush_limit = size;
}

void flatcc_builder_set_identifier(flatcc_builder_t *B, const char identifier[identifier_size])
{
    set_identifier(identifier);
}

enum flatcc_builder_type flatcc_builder_get_type(flatcc_builder_t *B)
{
    return B->frame ? (enum flatcc_builder_type)frame(type) : flatcc_builder_empty;
}

enum flatcc_builder_type flatcc_builder_get_type_at(flatcc_builder_t *B, int level)
{
    if (level < 1 || level > B->level) {
        return flatcc_builder_empty;
    }
    return (enum flatcc_builder_type)(B->frame[level - B->level].type);
}

void *flatcc_builder_get_direct_buffer(flatcc_builder_t *B, size_t *size_out)
{
    if (B->is_default_emitter) {
        return flatcc_emitter_get_direct_buffer(&B->default_emit_context, size_out);
    } else {
        if (size_out) {
            *size_out = 0;
        }
    }
    return 0;
}

void *flatcc_builder_copy_buffer(flatcc_builder_t *B, void *buffer, size_t size)
{
    /* User is allowed to call tentatively to see if there is support. */
    if (!B->is_default_emitter) {
        return 0;
    }
    buffer = flatcc_emitter_copy_buffer(&B->default_emit_context, buffer, size);
    check(buffer, "default emitter declined to copy buffer");
    return buffer;
}

void *flatcc_builder_finalize_buffer(flatcc_builder_t *B, size_t *size_out)
{
    void * buffer;
    size_t size;

    size = flatcc_builder_get_buffer_size(B);

    if (size_out) {
        *size_out = size;
    }

    buffer = FLATCC_BUILDER_ALLOC(size);

    if (!buffer) {
        check(__SET_ASSERT__, "failed to allocated memory for finalized buffer");
        goto done;
    }
    if (!flatcc_builder_copy_buffer(B, buffer, size)) {
        check(__SET_ASSERT__, "default emitter declined to copy buffer");
        FLATCC_BUILDER_FREE(buffer);
        buffer = 0;
    }
done:
    if (!buffer && size_out) {
        *size_out = 0;
    }
    return buffer;
}

void *flatcc_builder_finalize_aligned_buffer(flatcc_builder_t *B, size_t *size_out)
{
    void * buffer;
    size_t align;
    size_t size;

    size = flatcc_builder_get_buffer_size(B);

    if (size_out) {
        *size_out = size;
    }
    align = flatcc_builder_get_buffer_alignment(B);

    size = (size + align - 1) & ~(align - 1);
    buffer = FLATCC_BUILDER_ALIGNED_ALLOC(align, size);

    if (!buffer) {
        goto done;
    }
    if (!flatcc_builder_copy_buffer(B, buffer, size)) {
        FLATCC_BUILDER_ALIGNED_FREE(buffer);
        buffer = 0;
        goto done;
    }
done:
    if (!buffer && size_out) {
        *size_out = 0;
    }
    return buffer;
}

void *flatcc_builder_aligned_alloc(size_t alignment, size_t size)
{
    return FLATCC_BUILDER_ALIGNED_ALLOC(alignment, size);
}

void flatcc_builder_aligned_free(void *p)
{
    FLATCC_BUILDER_ALIGNED_FREE(p);
}

void *flatcc_builder_alloc(size_t size)
{
    return FLATCC_BUILDER_ALLOC(size);
}

void flatcc_builder_free(void *p)
{
    FLATCC_BUILDER_FREE(p);
}

void *flatcc_builder_get_emit_context(flatcc_builder_t *B)
{
    return B->emit_context;
}
