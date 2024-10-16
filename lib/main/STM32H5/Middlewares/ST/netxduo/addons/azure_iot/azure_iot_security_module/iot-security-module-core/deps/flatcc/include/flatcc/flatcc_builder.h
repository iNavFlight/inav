#ifndef FLATCC_BUILDER_H
#define FLATCC_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Library for building untyped FlatBuffers. Intended as a support
 * library for generated C code to produce typed builders, but might
 * also be useful in runtime environments and as support for scripting
 * languages.
 *
 * The builder has two API layers: a stack based `start/end` approach,
 * and a direct `create`, and they may be fixed freely. The direct
 * approach may be used as part of more specialized optimizations such
 * as rewriting buffers while the stack approach is convenient for state
 * machine driven parsers without a stack, or with a very simple stack
 * without extra allocations.
 *
 * The builder emits partial buffer sequences to a user provided emitter
 * function and does not require a full buffer reprensenation in memory.
 * For this reason it also does not support sorting or other operations
 * that requires representing the buffer, but post-processors can easily
 * do this, and the generated schema specific code and provide functions
 * to handle this.
 *
 * A custom allocator with a default realloc implementation can place
 * restraints on resource consumption and provide initial allocation
 * sizes for various buffers and stacks in use.
 *
 * A buffer under construction uses a virtual address space for the
 * completed part of the buffer, starting at 0 and growing in both
 * directions, or just down depending on whether vtables should be
 * clustered at the end or not. Clustering may help caching and
 * preshipping that part of the buffer.
 *
 * Because an offset cannot be known before its reference location is
 * defined, every completed table, vector, etc. returns a reference into
 * the virtual address range. If the final buffer keeps the 0 offset,
 * these references remain stable an may be used for external references
 * into the buffer.
 *
 * The maximum buffer than can be constructed is in praxis limited to
 * half the UOFFSET_MAX size, typically 2^31 bytes, not counting
 * clustered vtables that may consume and additional 2^31 bytes
 * (positive address range), but in praxis cannot because vtable
 * references are signed and thus limited to 2^31 bytes (or equivalent
 * depending on the flatbuffer types chosen).
 *
 * CORRECTION: in various places rules are mentioned about nesting and using
 * a reference at most once. In fact, DAG's are also valid flatbuffers.
 * This means a reference may be reused as long as each individual use
 * obeys the rules and, for example, circular references are not
 * constructed (circular types are ok, but objects graphs with cycles
 * are not permitted). Be especially aware of the offset vector create
 * call which translates the references into offsets - this can be
 * reverted by noting the reference in vector and calculate the base
 * used for the offset to restore the original references after the
 * vector has been emitted.
 */

#include <stdlib.h>
#ifndef UINT8_MAX
#include <stdint.h>
#endif

#include "flatcc_flatbuffers.h"
#include "flatcc_emitter.h"
#include "flatcc_refmap.h"

/* It is possible to enable logging here. */
#ifndef FLATCC_BUILDER_ASSERT
#define FLATCC_BUILDER_ASSERT(cond, reason) FLATCC_ASSERT(cond)
#endif

/*
 * Eror handling is not convenient and correct use should not cause
 * errors beyond possibly memory allocation, but assertions are a
 * good way to trace problems.
 *
 * Note: some internal assertion will remain if disabled.
 */
#ifndef FLATCC_BUILDER_ASSERT_ON_ERROR
#define FLATCC_BUILDER_ASSERT_ON_ERROR 1
#endif

/*
 * If set, checks user input agains state and returns error,
 * otherwise errors are ignored (assuming they won't happen).
 * Errors will be asserted if enabled and checks are not skipped.
 */
#ifndef FLATCC_BUILDER_SKIP_CHECKS
#define FLATCC_BUILDER_SKIP_CHECKS 0
#endif


/*
 * When adding the same field to a table twice this is either an error
 * or the existing field is returned, potentially introducing garbage
 * if the type is a vector, table, or string. When implementing parsers
 * it may be convenient to not treat this as an error.
 */
#ifndef FLATCC_BUILDER_ALLOW_REPEAT_TABLE_ADD
#define FLATCC_BUILDER_ALLOW_REPEAT_TABLE_ADD 0
#endif

/**
 * This type must have same size as `flatbuffers_uoffset_t`
 * and must be a signed type.
 */
typedef flatbuffers_soffset_t flatcc_builder_ref_t;
typedef flatbuffers_utype_t flatcc_builder_utype_t;

/**
 * This type must be compatible with code generation that
 * creates union specific ref types.
 */
typedef struct flatcc_builder_union_ref {
    flatcc_builder_utype_t type;
    flatcc_builder_ref_t value;
} flatcc_builder_union_ref_t;

typedef struct flatcc_builder_union_vec_ref {
    flatcc_builder_ref_t type;
    flatcc_builder_ref_t value;
} flatcc_builder_union_vec_ref_t;

/**
 * Virtual tables are off by one to avoid being mistaken for error at
 * position 0, and it makes them detectable as such because no other
 * reference is uneven. Vtables are emitted at their actual location
 * which is one less than the reference value.
 */
typedef flatbuffers_soffset_t flatcc_builder_vt_ref_t;

typedef flatbuffers_uoffset_t flatcc_builder_identifier_t;

/**
 * Hints to custom allocators so they can provide initial alloc sizes
 * etc. There will be at most one buffer for each allocation type per
 * flatcc_builder instance. Buffers containing only structs may avoid
 * allocation altogether using a `create` call. The vs stack must hold
 * vtable entries for all open tables up to their requested max id, but
 * unused max id overlap on the stack. The final vtables only store the
 * largest id actually added. The fs stack must hold stack frames for
 * the nesting levels expected in the buffer, each about 50-100 bytes.
 * The ds stack holds open vectors, table data, and nested buffer state.
 * `create` calls bypass the `ds` and `fs` stack and are thus faster.
 * The vb buffer holds a copy of all vtables seen and emitted since last
 * vtable flush. The patch log holds a uoffset for every table field
 * added to currently open tables. The hash table holds a uoffset entry
 * for each hash slot where the allocator decides how many to provide
 * above a certain minimum. The vd buffer allocates vtable descriptors
 * which is a reference to an emitted vtable, an offset to a cached
 * vtable, and a link to next descriptor with same hash. Calling `reset`
 * after build can either keep the allocation levels for the next
 * buffer, or reduce the buffers already allocated by requesting 1 byte
 * allocations (meaning provide a default).
 *
 * The user stack is not automatically allocated, but when entered
 * explicitly, the boundary is rembered in the current live
 * frame.
 */
enum flatcc_builder_alloc_type {
    /* The stack where vtables are build. */
    flatcc_builder_alloc_vs,
    /* The stack where data structures are build. */
    flatcc_builder_alloc_ds,
    /* The virtual table buffer cache, holds a copy of each vt seen. */
    flatcc_builder_alloc_vb,
    /* The patch log, remembers table fields with outstanding offset refs. */
    flatcc_builder_alloc_pl,
    /* The stack of frames for nested types. */
    flatcc_builder_alloc_fs,
    /* The hash table part of the virtual table cache. */
    flatcc_builder_alloc_ht,
    /* The vtable descriptor buffer, i.e. list elements for emitted vtables. */
    flatcc_builder_alloc_vd,
    /* User stack frame for custom data. */
    flatcc_builder_alloc_us,

    /* Number of allocation buffers. */
    flatcc_builder_alloc_buffer_count
};

/** Must reflect the `flatcc_builder_alloc_type` enum. */
#define FLATCC_BUILDER_ALLOC_BUFFER_COUNT flatcc_builder_alloc_buffer_count

#ifndef FLATCC_BUILDER_ALLOC
#define FLATCC_BUILDER_ALLOC(n) FLATCC_ALLOC(n)
#endif

#ifndef FLATCC_BUILDER_FREE
#define FLATCC_BUILDER_FREE(p) FLATCC_FREE(p)
#endif

#ifndef FLATCC_BUILDER_REALLOC
#define FLATCC_BUILDER_REALLOC(p, n) FLATCC_REALLOC(p, n)
#endif

#ifndef FLATCC_BUILDER_ALIGNED_ALLOC
#define FLATCC_BUILDER_ALIGNED_ALLOC(a, n) FLATCC_ALIGNED_ALLOC(a, n)
#endif

#ifndef FLATCC_BUILDER_ALIGNED_FREE
#define FLATCC_BUILDER_ALIGNED_FREE(p) FLATCC_ALIGNED_FREE(p)
#endif

/**
 * Emits data to a conceptual deque by appending to either front or
 * back, starting from offset 0.
 *
 * Each emit call appends a strictly later or earlier sequence than the
 * last emit with same offset sign. Thus a buffer is gradually grown at
 * both ends. `len` is the combined length of all iov entries such that
 * `offset + len` yields the former offset for negative offsets and
 * `offset + len` yields the next offset for non-negative offsets.
 * The bulk of the data will be in the negative range, possibly all of
 * it. The first emitted emitted range will either start or end at
 * offset 0. If offset 0 is emitted, it indicates the start of clustered
 * vtables. The last positive (non-zero) offset may be zero padding to
 * place the buffer in a full multiple of `block_align`, if set.
 *
 * No iov entry is empty, 0 < iov_count <= FLATCC_IOV_COUNT_MAX.
 *
 * The source data are in general ephemeral and should be consumed
 * immediately, as opposed to caching iov.
 *
 * For high performance applications:
 *
 * The `create` calls may reference longer living data, but header
 * fields etc. will still be short lived. If an emitter wants to
 * reference data in another buffer rather than copying, it should
 * inspect the memory range. The length of an iov entry may also be used
 * since headers are never very long (anything starting at 16 bytes can
 * safely be assumed to be user provided, or static zero padding). It is
 * guaranteed that data pointers in `create` calls receive a unique slot
 * separate from temporary headers, in the iov table which may be used
 * for range checking or hashing (`create_table` is the only call that
 * mutates the data buffer). It is also guaranteed (with the exception
 * of `create_table` and `create_cached_vtable`) that data provided to
 * create calls are not referenced at all by the builder, and these data
 * may therefore de-facto be handles rather than direct pointers when
 * the emitter and data provider can agree on such a protocol. This does
 * NOT apply to any start/end/add/etc. calls which do copy to stack.
 * `flatcc_builder_padding_base` may be used to test if an iov entry is
 * zero padding which always begins at that address.
 *
 * Future: the emit interface could be extended with a type code
 * and return an existing object insted of the emitted if, for
 * example, they are identical. Outside this api level, generated
 * code could provide a table comparison function to help such
 * deduplication. It would be optional because two equal objects
 * are not necessarily identical. The emitter already receives
 * one object at time.
 *
 * Returns 0 on success and otherwise causes the flatcc_builder
 * to fail.
 */
typedef int flatcc_builder_emit_fun(void *emit_context,
        const flatcc_iovec_t *iov, int iov_count, flatbuffers_soffset_t offset, size_t len);

/*
 * Returns a pointer to static padding used in emitter calls. May
 * sometimes also be used for empty defaults such as identifier.
 */
extern const uint8_t flatcc_builder_padding_base[];

/**
 * `request` is a minimum size to be returned, but allocation is
 * expected to grow exponentially or in reasonable chunks. Notably,
 * `alloc_type = flatcc_builder_alloc_ht` will only use highest available
 * power of 2. The allocator may shrink if `request` is well below
 * current size but should avoid repeated resizing on small changes in
 * request sizes. If `zero_fill` is non-zero, allocated data beyond
 * the current size must be zeroed. The buffer `b` may be null with 0
 * length initially. `alloc_context` is completely implementation
 * dependendent, and not needed when just relying on realloc. The
 * resulting buffer may be the same or different with moved data, like
 * realloc. Returns -1 with unmodified buffer on failure or 0 on
 * success. The `alloc_type` identifies the buffer type. This may be
 * used to cache buffers between instances of builders, or to decide a
 * default allocation size larger than requested. If `need` is zero the
 * buffer should be deallocate if non-zero, and return success (0)
 * regardless.
 */
typedef int flatcc_builder_alloc_fun(void *alloc_context,
        flatcc_iovec_t *b, size_t request, int zero_fill, int alloc_type);

/*
 * The number of hash slots there will be allocated space for. The
 * allocator may provide more. The size returned should be
 * `sizeof(flatbuffers_uoffset_t) * count`, where the size is a power of
 * 2 (or the rest is wasted). The hash table can store many more entries
 * than slots using linear search. The table does not resize.
 */
#ifndef FLATCC_BUILDER_MIN_HASH_COUNT
#define FLATCC_BUILDER_MIN_HASH_COUNT 64
#endif

typedef struct __flatcc_builder_buffer_frame __flatcc_builder_buffer_frame_t;
struct __flatcc_builder_buffer_frame {
    flatcc_builder_identifier_t identifier;
    flatcc_builder_ref_t mark;
    flatbuffers_uoffset_t vs_end;
    flatbuffers_uoffset_t nest_id;
    uint16_t flags;
    uint16_t block_align;
};

typedef struct __flatcc_builder_vector_frame __flatcc_builder_vector_frame_t;
struct __flatcc_builder_vector_frame {
    flatbuffers_uoffset_t elem_size;
    flatbuffers_uoffset_t count;
    flatbuffers_uoffset_t max_count;
};

typedef struct __flatcc_builder_table_frame __flatcc_builder_table_frame_t;
struct __flatcc_builder_table_frame {
    flatbuffers_uoffset_t vs_end;
    flatbuffers_uoffset_t pl_end;
    uint32_t vt_hash;
    flatbuffers_voffset_t id_end;
};

/*
 * Store state for nested structures such as buffers, tables and vectors.
 *
 * For less busy data and data where access to a previous state is
 * irrelevant, the frame may store the current state directly. Otherwise
 * the current state is maintained in the flatcc_builder_t structure in a
 * possibly derived form (e.g. ds pointer instead of ds_end offset) and
 * the frame is used to store the previous state when the frame is
 * entered.
 *
 * Most operations have a start/update/end cycle the decides the
 * liftetime of a frame, but these generally also have a direct form
 * (create) that does not use a frame at all. These still do some
 * state updates notably passing min_align to parent which may also be
 * an operation without a frame following the child level operation
 * (e.g. create struct, create buffer). Ending a frame results in the
 * same kind of updates.
 */
typedef struct __flatcc_builder_frame __flatcc_builder_frame_t;
struct __flatcc_builder_frame {
    flatbuffers_uoffset_t ds_first;
    flatbuffers_uoffset_t type_limit;
    flatbuffers_uoffset_t ds_offset;
    uint16_t align;
    uint16_t type;
    union {
        __flatcc_builder_table_frame_t table;
        __flatcc_builder_vector_frame_t vector;
        __flatcc_builder_buffer_frame_t buffer;
    } container;
};

/**
 * The main flatcc_builder structure. Can be stack allocated and must
 * be initialized with `flatcc_builder_init` and cleared with
 * `flatcc_builder_clear` to reclaim memory. Between buffer builds,
 * `flatcc_builder_reset` may be used.
 */
typedef struct flatcc_builder flatcc_builder_t;

struct flatcc_builder {
    /* Next entry on reserved stack in `alloc_pl` buffer. */
    flatbuffers_voffset_t *pl;
    /* Next entry on reserved stack in `alloc_vs` buffer. */
    flatbuffers_voffset_t *vs;
    /* One above the highest entry in vs, used to track vt_size. */
    flatbuffers_voffset_t id_end;
    /* The evolving vtable hash updated with every new field. */
    uint32_t vt_hash;

    /* Pointer to ds_first. */
    uint8_t *ds;
    /* Offset from `ds` on current frame. */
    flatbuffers_uoffset_t ds_offset;
    /* ds buffer size relative to ds_first, clamped to max size of current type. */
    flatbuffers_uoffset_t ds_limit;

    /* ds_first, ds_first + ds_offset is current ds stack range. */
    flatbuffers_uoffset_t ds_first;
    /* Points to currently open frame in `alloc_fs` buffer. */
    __flatcc_builder_frame_t *frame;

    /* Only significant to emitter function, if at all. */
    void *emit_context;
    /* Only significant to allocator function, if at all. */
    void *alloc_context;
    /* Customizable write function that both appends and prepends data. */
    flatcc_builder_emit_fun *emit;
    /* Customizable allocator that also deallocates. */
    flatcc_builder_alloc_fun *alloc;
    /* Buffers indexed by `alloc_type` */
    flatcc_iovec_t buffers[FLATCC_BUILDER_ALLOC_BUFFER_COUNT];
    /* Number of slots in ht given as 1 << ht_width. */
    size_t ht_width;

    /* The location in vb to add next cached vtable. */
    flatbuffers_uoffset_t vb_end;
    /* Where to allocate next vtable descriptor for hash table. */
    flatbuffers_uoffset_t vd_end;
    /* Ensure final buffer is aligned to at least this. Nested buffers get their own `min_align`. */
    uint16_t min_align;
    /* The current active objects alignment isolated from nested activity. */
    uint16_t align;
    /* The current buffers block alignment used when emitting buffer. */
    uint16_t block_align;
    /* Signed virtual address range used for `flatcc_builder_ref_t` and emitter. */
    flatcc_builder_ref_t emit_start;
    flatcc_builder_ref_t emit_end;
    /* 0 for top level, and end of buffer ref for nested buffers (can also be 0). */
    flatcc_builder_ref_t buffer_mark;
    /* Next nest_id. */
    flatbuffers_uoffset_t nest_count;
    /* Unique id to prevent sharing of vtables across buffers. */
    flatbuffers_uoffset_t nest_id;
    /* Current nesting level. Helpful to state-machines with explicit stack and to check `max_level`. */
    int level;
    /* Aggregate check for allocated frame and max_level. */
    int limit_level;
    /* Track size prefixed buffer. */
    uint16_t buffer_flags;

    /* Settings that may happen with no frame allocated. */

    flatcc_builder_identifier_t identifier;

    /* Settings that survive reset (emitter, alloc, and contexts also survive): */

    /* If non-zero, vtable cache gets flushed periodically. */
    size_t vb_flush_limit;
    /* If non-zero, fails on deep nesting to help drivers with a stack, such as recursive parsers etc. */
    int max_level;
    /* If non-zero, do not cluster vtables at end, only emit negative offsets (0 by default). */
    int disable_vt_clustering;

    /* Set if the default emitter is being used. */
    int is_default_emitter;
    /* Only used with default emitter. */
    flatcc_emitter_t default_emit_context;

    /* Offset to the last entered user frame on the user frame stack, after frame header, or 0. */
    size_t user_frame_offset;

    /* The offset to the end of the most recent user frame. */
    size_t user_frame_end;

    /* The optional user supplied refmap for cloning DAG's - not shared with nested buffers. */
    flatcc_refmap_t *refmap;
};

/**
 * Call this before any other API call.
 *
 * The emitter handles the completed chunks of the buffer that will no
 * longer be required by the builder. It is largely a `write` function
 * that can append to both positive and negative offsets.
 *
 * No memory is allocated during init. Buffers will be allocated as
 * needed. The `emit_context` is only used by the emitter, if at all.
 *
 * `flatcc_builder_reset/clear` calls are automtically forwarded to the
 * default emitter.
 *
 * Returns -1 on failure, 0 on success.
 */
int flatcc_builder_init(flatcc_builder_t *B);

/**
 * Use instead of `flatcc_builder_init` when providing a custom allocator
 * or emitter. Leave emitter or allocator null to use default.
 * Cleanup of emit and alloc context must be handled manually after
 * the builder is cleared or reset, except if emitter is null the
 * default will be automatically cleared and reset.
 *
 * Returns -1 on failure, 0 on success.
 */
int flatcc_builder_custom_init(flatcc_builder_t *B,
        flatcc_builder_emit_fun *emit, void *emit_context,
        flatcc_builder_alloc_fun *alloc, void *alloc_context);

/*
 * Returns (flatcc_emitter_t *) if the default context is used.
 * Other emitter might have null contexts.
 */
void *flatcc_builder_get_emit_context(flatcc_builder_t *B);

/**
 * Prepares builder for a new build. The emitter is not told when a
 * buffer is finished or when a new begins, and must be told so
 * separately. Allocated buffers will be zeroed, but may optionally be
 * reduced to their defaults (signalled by reallocating each non-empty
 * buffer to a single byte). General settings are cleared optionally,
 * such as cache flushing. Buffer specific settings such as buffer
 * identifier are always cleared.
 *
 * Returns -1 if allocator complains during buffer reduction, 0 on
 * success.
 */
int flatcc_builder_custom_reset(flatcc_builder_t *B,
        int reduce_buffers, int set_defaults);

/*
 * Same as `flatcc_builder_custom_reset` with default arguments
 * where buffers are not reduced and default settings are not reset.
 */
int flatcc_builder_reset(flatcc_builder_t *B);

/**
 * Deallocates all memory by calling allocate with a zero size request
 * on each buffer, then zeroing the builder structure itself.
 */
void flatcc_builder_clear(flatcc_builder_t *B);

/**
 * Allocates to next higher power of 2 using system realloc and ignores
 * `alloc_context`. Only reduces size if a small subsequent increase in
 * size would not trigger a reallocation. `alloc_type` is used to
 * set minimum sizes. Hash tables are allocated to the exact requested
 * size. See also `alloc_fun`.
 */
int flatcc_builder_default_alloc(void *alloc_context,
        flatcc_iovec_t *b, size_t request, int zero_fill, int alloc_type);

/**
 * If non-zero, the vtable cache will get flushed whenever it reaches
 * the given limit at a point in time where more space is needed. The
 * limit is not exact as it is only tested when reallocation is
 * required.
 */
void flatcc_builder_set_vtable_cache_limit(flatcc_builder_t *B, size_t size);

/**
 * Manual flushing of vtable for long running tasks. Mostly used
 * internally to deal with nested buffers.
 */
void flatcc_builder_flush_vtable_cache(flatcc_builder_t *B);

/**
 * Low-level support function to aid in constructing nested buffers without
 * allocation. Not for regular use.
 *
 * Call where `start_buffer` would have been placed when using
 * `create_buffer` in a nested context. Save the return value on a stack
 * as argument to `pop_buffer_alignment`.
 *
 * The call resets the current derived buffer alignment so the nested
 * buffer will not be aligned to more than required.
 *
 * Often it will not be necessary to be so careful with alignment since
 * the alignment cannot be invalid by failing to use push and pop, but
 * for code generation it will ensure the correct result every time.
 */
uint16_t flatcc_builder_push_buffer_alignment(flatcc_builder_t *B);

/**
 * Low-level call.
 *
 * Call with the return value from push_buffer_alignment after a nested
 * `create_buffer_call`. The alignments merge back up in the buffer
 * hierarchy so the top level buffer gets the largest of all aligments.
 */
void flatcc_builder_pop_buffer_alignment(flatcc_builder_t *B, uint16_t buffer_align);

/**
 * This value may be of interest when the buffer has been ended, for
 * example when subsequently allocating memory for the buffer to ensure
 * that memory is properly aligned.
 */
uint16_t flatcc_builder_get_buffer_alignment(flatcc_builder_t *B);

/**
 * Level 0 means no buffer is started, otherwise it increments with
 * start calls and decrements with end calls (approximately for
 * optimized operations such as table vectors).
 *
 * If `max_level` has been set, `get_level` always returns a value <=
 * `max_level` provided no start call has failed.
 *
 * Level continues to increment inside nested buffers.
 */
int flatcc_builder_get_level(flatcc_builder_t *B);

/**
 * Setting the max level triggers a failure on start of new nestings
 * when the level is reached. May be used to protect recursive descend
 * parsers etc. or later buffer readers.
 *
 * The builder itself is not sensitive to depth, and the allocator is a
 * better way to protect resource abuse.
 *
 * `max_level` is not reset inside nested buffers.
 */
void flatcc_builder_set_max_level(flatcc_builder_t *B, int level);

/**
 * By default ordinary data such as tables are placed in front of
 * earlier produced content and vtables are placed at the very end thus
 * clustering vtables together. This can be disabled so all content is
 * placed in front. Nested buffers ignores this setting because they can
 * only place content in front because they cannot blend with the
 * containing buffers content. Clustering could be more cache friendly
 * and also enables pre-shipping of the vtables during transmission.
 */
void flatcc_builder_set_vtable_clustering(flatcc_builder_t *B, int enable);

/**
 * Sets a new user supplied refmap which maps source pointers to
 * references and returns the old refmap, or null. It is also
 * possible to disable an existing refmap by setting a null
 * refmap.
 *
 * A clone or pick operation may use this map when present,
 * depending on the data type. If a hit is found, the stored
 * reference will be used instead of performing a new clone or
 * pick operation. It is also possible to manually populate the
 * refmap. Note that the builder does not have a concept of
 * clone or pick - these are higher level recursive operations
 * to add data from one buffer to another - but such code may
 * rely on the builder to provide the current refmap during
 * recursive operations. For this reason, the builder makes no
 * calls to the refmap interface on its own - it just stores the
 * current refmap such that recursive operations can find it.
 *
 * Refmaps MUST be reset, replaced or disabled if a source
 * pointer may be reused for different purposes - for example if
 * repeatedly reading FlatBuffers into the same memory buffer
 * and performing a clone into a buffer under construction.
 * Refmaps may also be replaced if the same object is to be
 * cloned several times keeping the internal DAG structure
 * intact with every new clone being an independent object.
 *
 * Refmaps must also be replaced or disabled prior to starting a
 * nested buffer and after stopping it, or when cloning a object
 * as a nested root. THIS IS VERY EASY TO GET WRONG!  The
 * builder does a lot of bookkeeping for nested buffers but not
 * in this case. Shared references may happen and they WILL fail
 * verification and they WILL break when copying out a nested
 * buffer to somewhere else. The user_frame stack may be used
 * for pushing refmaps, but often user codes recursive stack
 * will work just as well.
 *
 * It is entirely optional to use refmaps when cloning - they
 * preserve DAG structure and may speed up operations or slow
 * them down, depending on the source material.
 *
 * Refmaps may consume a lot of space when large offset vectors
 * are cloned when these do not have significant shared
 * references. They may also be very cheap to use without any
 * dynamic allocation when objects are small and have at most a
 * few references.
 *
 * Refmaps only support init, insert, find, reset, clear but not
 * delete. There is a standard implementation in the runtime
 * source tree but it can easily be replaced compile time and it
 * may also be left out if unused. The builder wraps reset, insert,
 * and find so the user does not have to check if a refmap is
 * present but other operations must be done direcly on the
 * refmap.
 *
 * The builder wrapped refmap operations are valid on a null
 * refmap which will find nothing and insert nothing.
 *
 * The builder will reset the refmap during a builder reset and
 * clear the refmap during a builder clear operation. If the
 * refmap goes out of scope before that happens it is important
 * to call set_refmap with null and manually clear the refmap.
 */
static inline flatcc_refmap_t *flatcc_builder_set_refmap(flatcc_builder_t *B, flatcc_refmap_t *refmap)
{
    flatcc_refmap_t *refmap_old;

    refmap_old = B->refmap;
    B->refmap = refmap;
    return refmap_old;
}

/* Retrieves the current refmap, or null. */
static inline flatcc_refmap_t *flatcc_builder_get_refmap(flatcc_builder_t *B)
{
    return B->refmap;
}

/* Finds a reference, or a null reference if no refmap is active.  * */
static inline flatcc_builder_ref_t flatcc_builder_refmap_find(flatcc_builder_t *B, const void *src)
{
    return B->refmap ? flatcc_refmap_find(B->refmap, src) : flatcc_refmap_not_found;
}

/*
 * Inserts into the current refmap with the inseted ref upon
 * upon success, or not_found on failure (default 0), or just
 * returns ref if refmap is absent.
 *
 * Note that if an existing item exists, the ref is replaced
 * and the new, not the old, ref is returned.
 */
static inline flatcc_builder_ref_t flatcc_builder_refmap_insert(flatcc_builder_t *B, const void *src, flatcc_builder_ref_t ref)
{
    return B->refmap ? flatcc_refmap_insert(B->refmap, src, ref) : ref;
}

static inline void flatcc_builder_refmap_reset(flatcc_builder_t *B)
{
    if (B->refmap) flatcc_refmap_reset(B->refmap);
}


enum flatcc_builder_buffer_flags {
    flatcc_builder_is_nested = 1,
    flatcc_builder_with_size = 2,
};

/**
 * An alternative to start buffer, start struct/table ... end buffer.
 *
 * This call is mostly of interest as a means to quicly create a zero
 * allocation top-level buffer header following a call to create_struct,
 * or to create_vtable/create_table. For that, it is quite simple to
 * use. For general buffer construction without allocation, more care is
 * needed, as discussed below.
 *
 * If the content is created with `start/end_table` calls, or similar,
 * it is better to use `start/end_buffer` since stack allocation is used
 * anyway.
 *
 * The buffer alignment must be provided manually as it is not derived
 * from constructed content, unlike `start/end_buffer`. Typically
 * `align` would be same argument as provided to `create_struct`.
 * `get_buffer_alignment` may also used (note: `get_buffer_alignment`
 * may return different after the call because it will be updated with
 * the `block_align` argument to `create_buffer` but that is ok).
 *
 * The buffer may be constructed as a nested buffer with the `is_nested
 * = 1` flag. As a nested buffer a ubyte vector header is placed before
 * the aligned buffer header. A top-level buffer will normally have
 * flags set to 0.
 *
 * A top-level buffer may also be constructed with the `with_size = 2`
 * flag for top level buffers. It adds a size prefix similar to
 * `is_nested` but the size is part of the aligned buffer. A size
 * prefixed top level buffer must be accessed with a size prefix aware
 * reader, or the buffer given to a standard reader must point to after
 * the size field while keeping the buffer aligned to the size field
 * (this will depend on the readers API which may be an arbitrary other
 * language).
 *
 * If the `with_size` is used with the `is_nested` flag, the size is
 * added as usual and all fields remain aligned as before, but padding
 * is adjusted to ensure the buffer is aligned to the size field so
 * that, for example, the nested buffer with size can safely be copied
 * to a new memory buffer for consumption.
 *
 * Generally, references may only be used within the same buffer
 * context. With `create_buffer` this becomes less precise. The rule
 * here is that anything that would be valid with start/end_buffer
 * nestings is also valid when removing the `start_buffer` call and
 * replacing `end_buffer` with `create_buffer`.
 *
 * Note the additional burden of tracking buffer alignment manually -
 * To help with this use `push_buffer_alignment` where `start_buffer`
 * would have been placed, and  `pop_buffer_alignment after the
 * `create_buffer` call, and use `get_buffer_alignemnt` as described
 * above.
 *
 * `create_buffer` is not suitable as a container for buffers created
 * with `start/end_buffer` as these make assumptions about context that
 * create buffer does not provide. Also, there is no point in doing so,
 * since the idea of `create_buffer` is to avoid allocation in the first
 * place.
 */
flatcc_builder_ref_t flatcc_builder_create_buffer(flatcc_builder_t *B,
        const char identifier[FLATBUFFERS_IDENTIFIER_SIZE],
        uint16_t block_align,
        flatcc_builder_ref_t ref, uint16_t align, int flags);

/**
 * Creates a struct within the current buffer without using any
 * allocation.
 *
 * The struct should be used as a root in the `end_buffer` call or as a
 * union value as there are no other ways to use struct while conforming
 * to the FlatBuffer format - noting that tables embed structs in their
 * own data area except in union fields.
 *
 * The struct should be in little endian format and follow the usual
 * FlatBuffers alignment rules, although this API won't care about what
 * is being stored.
 *
 * May also be used to simply emit a struct through the emitter
 * interface without being in a buffer and without being a valid
 * FlatBuffer.
 */
flatcc_builder_ref_t flatcc_builder_create_struct(flatcc_builder_t *B,
        const void *data, size_t size, uint16_t align);

/**
 * Starts a struct and returns a pointer that should be used immediately
 * to fill in the struct in protocol endian format, and when done,
 * `end_struct` should be called. The returned reference should be used
 * as argument to `end_buffer` or as a union value. See also
 * `create_struct`.
 */
void *flatcc_builder_start_struct(flatcc_builder_t *B,
        size_t size, uint16_t align);

/**
 * Return a pointer also returned at start struct, e.g. for endian
 * conversion.
 */
void *flatcc_builder_struct_edit(flatcc_builder_t *B);

/**
 * Emits the struct started by `start_struct` and returns a reference to
 * be used as root in an enclosing `end_buffer` call or as a union
 * value.  As mentioned in `create_struct`, these can also be used more
 * freely, but not while being conformant FlatBuffers.
 */
flatcc_builder_ref_t flatcc_builder_end_struct(flatcc_builder_t *B);

/**
 * The buffer always aligns to at least the offset size (typically 4)
 * and the internal alignment requirements of the buffer content which
 * is derived as content is added.
 *
 * In addition, block_align can be specified. This ensures the resulting
 * buffer is at least aligned to the block size and that the total size
 * is zero padded to fill a block multiple if necessary. Because the
 * emitter operates on a virtual address range before the full buffer is
 * aligned, it may have to make assumptions based on that: For example,
 * it may be processing encryption blocks in the fly, and the resulting
 * buffer should be aligned to the encryption block size, even if the
 * content is just a byte aligned struct. Block align helps ensure this.
 * If the block align as 1 there will be no attempt to zero pad at the
 * end, but the content may still warrant padding after the header. End
 * padding is only needed with clustered vtables (which is the default).
 *
 * `block_align` is allowed to be 0 meaning it will inherit from parent if
 * present, and otherwise it defaults to 1.
 *
 * The identifier may be null, and it may optionally be set later with
 * `set_identifier` before the `end_buffer` call.
 *
 * General note:
 *
 * Only references returned with this buffer as current (i.e. last
 * unended buffer) can be stored in other objects (tables, offset
 * vectors) also belonging to this buffer, or used as the root argument
 * to `end_buffer`. A reference may be stored at most once, and unused
 * references will result in buffer garbage. All calls must be balanced
 * around the respective start / end operations, but may otherwise nest
 * freely, including nested buffers. Nested buffers are supposed to be
 * stored in a table offset field to comply with FlatBuffers, but the
 * API does not place any restrictions on where references are stored,
 * as long as they are indicated as offset fields.
 *
 * All alignment in all API calls must be between 1 and 256 and must be a
 * power of 2. This is not checked. Only if explicitly documented can it
 * also be 0 for a default value.
 *
 * `flags` can be `with_size` but `is_nested` is derived from context
 * see also `create_buffer`.
 */
int flatcc_builder_start_buffer(flatcc_builder_t *B,
        const char identifier[FLATBUFFERS_IDENTIFIER_SIZE],
        uint16_t block_align, int flags);

/**
 * The root object should be a struct or a table to conform to the
 * FlatBuffers format, but technically it can also be a vector or a
 * string, or even a child buffer (which is also vector as seen by the
 * buffer). The object must be created within the current buffer
 * context, that is, while the current buffer is the deepest nested
 * buffer on the stack.
 */
flatcc_builder_ref_t flatcc_builder_end_buffer(flatcc_builder_t *B, flatcc_builder_ref_t root);

/**
 * The embed buffer is mostly intended to add an existing buffer as a
 * nested buffer. The buffer will be wrapped in a ubyte vector such that
 * the buffer is aligned at vector start, after the size field.
 *
 * If `align` is 0 it will default to 8 so that all FlatBuffer numeric
 * types will be readable. NOTE: generally do not count on align 0 being
 * valid or even checked by the API, but in this case it may be
 * difficult to know the internal buffer alignment, and 1 would be the wrong
 * choice.
 *
 * If `block_align` is set (non-zero), the buffer is placed in an isolated
 * block multiple. This may cost up to almost 2 block sizes in padding.
 * If the `block_align` argument is 0, it inherits from the parent
 * buffer block_size, or defaults to 1.
 *
 * The `align` argument must be set to respect the buffers internal
 * alignment requirements, but if the buffer is smaller it will not be
 * padded to isolate the buffer. For example a buffer of with
 * `align = 64` and `size = 65` may share its last 64 byte block with
 * other content, but not if `block_align = 64`.
 *
 * Because the ubyte size field is not, by default, part of the aligned
 * buffer, significant space can be wasted if multiple blocks are added
 * in sequence with a large block size.
 *
 * In most cases the distinction between the two alignments is not
 * important, but it allows separate configuration of block internal
 * alignment and block size, which can be important for auto-generated
 * code that may know the alignment of the buffer, but not the users
 * operational requirements.
 *
 * If the buffer is embedded without a parent buffer, it will simply
 * emit the buffer through the emit interface, but may also add padding
 * up to block alignment. At top-level there will be no size field
 * header.
 *
 * If `with_size` flag is set, the buffer is aligned to size field and
 * the above note about padding space no longer applies. The size field
 * is added regardless. The `is_nested` flag has no effect since it is
 * impplied.
 */
flatcc_builder_ref_t flatcc_builder_embed_buffer(flatcc_builder_t *B,
        uint16_t block_align,
        const void *data, size_t size, uint16_t align, int flags);

/**
 * Applies to the innermost open buffer. The identifier may be null or
 * contain all zero. Overrides any identifier given to the start buffer
 * call.
 */
void flatcc_builder_set_identifier(flatcc_builder_t *B,
        const char identifier[FLATBUFFERS_IDENTIFIER_SIZE]);

enum flatcc_builder_type {
    flatcc_builder_empty = 0,
    flatcc_builder_buffer,
    flatcc_builder_struct,
    flatcc_builder_table,
    flatcc_builder_vector,
    flatcc_builder_offset_vector,
    flatcc_builder_string,
    flatcc_builder_union_vector
};

/**
 * Returns the object type currently on the stack, for example if
 * needing to decide how to close a buffer. Because a table is
 * automatically added when starting a table buffer,
 * `flatcc_builder_table_buffer` should not normally be seen and the level
 * should be 2 before when closing a top-level table buffer, and 0
 * after. A `flatcc_builder_struct_buffer` will be visible at level 1.
 *
 */
enum flatcc_builder_type flatcc_builder_get_type(flatcc_builder_t *B);

/**
 * Similar to `get_type` but for a specific level. `get_type_at(B, 1)`
 * will return `flatcc_builder_table_buffer` if this is the root buffer
 * type. get_type_at(B, 0) is always `flatcc_builder_empty` and so are any
 * level above `get_level`.
 */
enum flatcc_builder_type flatcc_builder_get_type_at(flatcc_builder_t *B, int level);

/**
 * The user stack is available for custom data. It may be used as
 * a simple stack by extending or reducing the inner-most frame.
 *
 * A frame has a size and a location on the user stack. Entering
 * a frame ensures the start is aligned to sizeof(size_t) and
 * ensures the requested space is available without reallocation.
 * When exiting a frame, the previous frame is restored.
 *
 * A user frame works completely independently of the builders
 * frame stack for tracking tables vectors etc. and does not have
 * to be completely at exit, but obviously it is not valid to
 * exit more often the entered.
 *
 * The frame is zeroed when entered.
 *
 * Returns a non-zero handle to the user frame upon success or
 * 0 on allocation failure.
 */
size_t flatcc_builder_enter_user_frame(flatcc_builder_t *B, size_t size);

/**
 * Makes the parent user frame current, if any. It is not valid to call
 * if there isn't any current frame. Returns handle to parent frame if
 * any, or 0.
 */
size_t flatcc_builder_exit_user_frame(flatcc_builder_t *B);

/**
 * Exits the frame represented by the given handle. All more
 * recently entered frames will also be exited. Returns the parent
 * frame handle if any, or 0.
 */
size_t flatcc_builder_exit_user_frame_at(flatcc_builder_t *B, size_t handle);

/**
 * Returns a non-zero handle to the current inner-most user frame if
 * any, or 0.
 */
size_t flatcc_builder_get_current_user_frame(flatcc_builder_t *B);

/*
 * Returns a pointer to the user frame at the given handle. Any active
 * frame can be accessed in this manner but the pointer is invalidated
 * by user frame enter and exit operations.
 */
void *flatcc_builder_get_user_frame_ptr(flatcc_builder_t *B, size_t handle);

/**
 * Returns the size of the buffer and the logical start and end address
 * of with respect to the emitters address range. `end` - `start` also
 * yields the size. During construction `size` is the emitted number of
 * bytes and after buffer close it is the actual buffer size - by then
 * the start is also the return value of close buffer. End marks the end
 * of the virtual table cluster block.
 *
 * NOTE: there is no guarantee that all vtables end up in the cluster
 * block if there is placed a limit on the vtable size, or if nested
 * buffers are being used. On the other hand, if these conditions are
 * met, it is guaranteed that all vtables are present if the vtable
 * block is available (this depends on external transmission - the
 * vtables are always emitted before tables using them). In all cases
 * the vtables will behave as valid vtables in a flatbuffer.
 */
size_t flatcc_builder_get_buffer_size(flatcc_builder_t *B);

/**
 * Returns the reference to the start of the emitter buffer so far, or
 * in total after buffer end, in the virtual address range used
 * by the emitter. Start is also returned by buffer end.
 */
flatcc_builder_ref_t flatcc_builder_get_buffer_start(flatcc_builder_t *B);

/**
 * Returns the reference to the end of buffer emitted so far. When
 * clustering vtables, this is the end of tables, or after buffer end,
 * also zero padding if block aligned. If clustering is disabled, this
 * method will return 0 as the buffer only grows down then.
 */
flatcc_builder_ref_t flatcc_builder_get_buffer_mark(flatcc_builder_t *B);

/**
 * Creates the vtable in the current buffer context, somewhat similar to
 * how create_vector operates. Each call results in a new table even if
 * an identical has already been emitted.
 *
 * Also consider `create_cached_vtable` which will reuse existing
 * vtables.
 *
 * This is low-low-level function intended to support
 * `create_cached_vtable` or equivalent, and `create_table`, both of
 * which are normally used indirectly via `start_table`, `table_add`,
 * `table_add_offset`..., `table_end`.
 *
 * Creates a vtable as a verbatim copy. This means the vtable must
 * include the header fields containing the vtable size and the table
 * size in little endian voffset_t encoding followed by the vtable
 * entries in same encoding.
 *
 * The function may be used to copy vtables from other other buffers
 * since they are directly transferable.
 *
 * The returned reference is actually the emitted location + 1. This
 * ensures the vtable is not mistaken for error because 0 is a valid
 * vtable reference. `create_table` is aware of this and substracts one
 * before computing the final offset relative to the table. This also
 * means vtable references are uniquely identifiable by having the
 * lowest bit set.
 *
 * vtable references may be reused within the same buffer, not any
 * parent or other related buffer (technically this is possible though,
 * as long as it is within same builder context, but it will not construct
 * valid FlatBuffers because the buffer cannot be extracted in isolation).
 */
flatcc_builder_vt_ref_t flatcc_builder_create_vtable(flatcc_builder_t *B,
        const flatbuffers_voffset_t *vt,
        flatbuffers_voffset_t vt_size);

/**
 * Support function to `create_vtable`. See also the uncached version
 * `create_vtable`.
 *
 * Looks up the constructed vtable on the vs stack too see if it matches
 * a cached entry. If not, it emits a new vtable either at the end if
 * top-level and clustering is enabled, or at the front (always for
 * nested buffers).
 *
 * If the same vtable was already emitted in a different buffer, but not
 * in the current buffer, the cache entry will be reused, but a new
 * table will be emitted the first it happens in the same table.
 *
 * The returned reference is + 1 relative to the emitted address range
 * to identify it as a vtable and to avoid mistaking the valid 0
 * reference for an error (clustered vtables tend to start at the end at
 * the virtual address 0, and up).
 *
 * The hash function can be chosen arbitrarily but may result in
 * duplicate emitted vtables if different hash functions are being used
 * concurrently, such as mixing the default used by `start/end table`
 * with a custom function (this is not incorrect, it only increases the
 * buffer size and cache pressure).
 *
 * If a vtable has a unique ID by other means than hashing the content,
 * such as an integer id, and offset into another buffer, or a pointer,
 * a good hash may be multiplication by a 32-bit prime number. The hash
 * table is not very sensitive to collissions as it uses externally
 * chained hashing with move to front semantics.
 */
flatcc_builder_vt_ref_t flatcc_builder_create_cached_vtable(flatcc_builder_t *B,
        const flatbuffers_voffset_t *vt,
        flatbuffers_voffset_t vt_size, uint32_t vt_hash);

/*
 * Based on Knuth's prime multiplier.
 *
 * This is an incremental hash that is called with id and size of each
 * non-empty field, and finally with the two vtable header fields
 * when vtables are constructed via `table_add/table_add_offset`.
 *
 */
#ifndef FLATCC_SLOW_MUL
#ifndef FLATCC_BUILDER_INIT_VT_HASH
#define FLATCC_BUILDER_INIT_VT_HASH(hash) { (hash) = (uint32_t)0x2f693b52UL; }
#endif
#ifndef FLATCC_BUILDER_UPDATE_VT_HASH
#define FLATCC_BUILDER_UPDATE_VT_HASH(hash, id, offset) \
        { (hash) = (((((uint32_t)id ^ (hash)) * (uint32_t)2654435761UL)\
                ^ (uint32_t)(offset)) * (uint32_t)2654435761UL); }
#endif
#ifndef FLATCC_BUILDER_BUCKET_VT_HASH
#define FLATCC_BUILDER_BUCKET_VT_HASH(hash, width) (((uint32_t)(hash)) >> (32 - (width)))
#endif
#endif

/*
 * By default we use Bernsteins hash as fallback if multiplication is slow.
 *
 * This just have to be simple, fast, and work on devices without fast
 * multiplication. We are not too sensitive to collisions. Feel free to
 * experiment and replace.
 */
#ifndef FLATCC_BUILDER_INIT_VT_HASH
#define FLATCC_BUILDER_INIT_VT_HASH(hash) { (hash) = 5381; }
#endif
#ifndef FLATCC_BUILDER_UPDATE_VT_HASH
#define FLATCC_BUILDER_UPDATE_VT_HASH(hash, id, offset) \
        { (hash) = ((((hash) << 5) ^ (id)) << 5) ^ (offset); }
#endif
#ifndef FLATCC_BUILDER_BUCKET_VT_HASH
#define FLATCC_BUILDER_BUCKET_VT_HASH(hash, width) (((1 << (width)) - 1) & (hash))
#endif



/**
 * Normally use `start_table` instead of this call.
 *
 * This is a low-level call only intended for high-performance
 * applications that repeatedly churn about similar tables of known
 * layout, or as a support layer for other builders that maintain their
 * own allocation rather than using the stack of this builder.
 *
 * Creates a table from an already emitted vtable, actual data that is
 * properly aligned relative to data start and in little endian
 * encoding. Unlike structs, tables can have offset fields. These must
 * be stored as flatcc_builder_ref_t types (which have uoffset_t size) as
 * returned by the api in native encoding. The `offsets` table contain
 * voffsets relative to `data` start (this is different from how vtables
 * store offsets because they are relative to a table header). The
 * `offsets` table is only used temporarily to translate the stored
 * references and is not part of final buffer content. `offsets` may be
 * null if `offset_count` is 0. `align` should be the highest aligned
 * field in the table, but `size` need not be a multiple of `align`.
 * Aside from endian encoding, the vtable must record a table size equal
 * to `size + sizeof(flatbuffers_uoffset_t)` because it includes the
 * table header field size. The vtable is not accessed by this call (nor
 * is it available). Unlike other references, the vtable reference may
 * be shared between tables in the same buffer (not with any related
 * buffer such as a parent buffer).
 *
 * The operation will not use any allocation, but will update the
 * alignment of the containing buffer if any.
 *
 * Note: unlike other create calls, except `create_offset_vector`,
 * the source data is modified in order to translate references intok
 * offsets before emitting the table.
 */
flatcc_builder_ref_t flatcc_builder_create_table(flatcc_builder_t *B,
        const void *data, size_t size, uint16_t align,
        flatbuffers_voffset_t *offsets, int offset_count,
        flatcc_builder_vt_ref_t vt_ref);

/**
 * Starts a table, typically following a start_buffer call as an
 * alternative to starting a struct, or to create table fields to be
 * stored in a parent table, or in an offset vector.
 * A number of `table_add` and table_add_offset` call may be placed
 * before the `end_table` call. Struct fields should NOT use `struct`
 * related call (because table structs are in-place), rather they should
 * use the `table_add` call with the appropriate size and alignment.
 *
 * A table, like other reference returning calls, may also be started
 * outside a buffer if the buffer header and alignment is of no
 * interest to the application, for example as part of an externally
 * built buffer.
 *
 * `count` must be larger than the largest id used for this table
 * instance. Normally it is set to the number of fields defined in the
 * schema, but it may be less if memory is constrained and only few
 * fields with low valued id's are in use. The count can extended later
 * with `reserve_table` if necessary. `count` may be also be set to a
 * large enough value such as FLATBUFFERS_ID_MAX + 1 if memory is not a
 * concern (reserves about twice the maximum vtable size to track the
 * current vtable and voffsets where references must be translated to
 * offsets at table end). `count` may be zero if for example
 * `reserve_table` is being used.
 *
 * Returns -1 on error, 0 on success.
 */
int flatcc_builder_start_table(flatcc_builder_t *B, int count);

/**
 * Call before adding a field with an id that is not below the count set
 * at table start. Not needed in most cases. For performance reasons
 * the builder does not check all bounds all the the time, but the user
 * can do so if memory constraints prevent start_table from using a
 * conservative value. See also `table_start`.
 *
 * Note: this call has absolutely no effect on the table layout, it just
 * prevents internal buffer overruns.
 *
 * Returns -1 on error, 0 on success.
 */
int flatcc_builder_reserve_table(flatcc_builder_t *B, int count);

/**
 * Completes the table constructed on the internal stack including
 * emitting a vtable, or finding a matching vtable that has already been
 * emitted to the same buffer. (Vtables cannot be shared between
 * buffers, but they can between tables of the same buffer).
 *
 * Note: there is a considerable, but necessary, amount of bookkeeping
 * involved in constructing tables. The `create_table` call is much
 * faster, but it also expects a lot of work to be done already.
 *
 * Tables can be created with no fields added. This will result in an
 * empty vtable and a table with just a vtable reference. If a table is
 * used as a sub-table, such a table might also not be stored at all,
 * but we do not return a special reference for that, nor do we provide
 * and option to not create the table in this case. This may be
 * interpreted as the difference between a null table (not stored in
 * parent), and an empty table with a unique offset (and thus identity)
 * different from other empty tables.
 */
flatcc_builder_ref_t flatcc_builder_end_table(flatcc_builder_t *B);

/**
 * Optionally this method can be called just before `flatcc_builder_end_table`
 * to verify that all required fields have been set.
 * Each entry is a table field id.
 *
 * Union fields should use the type field when checking for presence and
 * may also want to check the soundness of the union field overall using
 * `check_union_field` with the id one higher than the type field id.
 *
 * This funcion is typically called by an assertion in generated builder
 * interfaces while release builds may want to avoid this performance
 * overhead.
 *
 * Returns 1 if all fields are matched, 0 otherwise.
 */
int flatcc_builder_check_required(flatcc_builder_t *B, const flatbuffers_voffset_t *required, int count);

/**
 * Same as `check_required` when called with a single element.
 *
 * Typically used when direct calls are more convenient than building an
 * array first. Useful when dealing with untrusted intput such as parsed
 * text from an external source.
 */
int flatcc_builder_check_required_field(flatcc_builder_t *B, flatbuffers_voffset_t id);

/**
 * Checks that a union field is valid.
 *
 * The criteria is:
 *
 * If the type field is not present (at id - 1), or it holds a zero value,
 * then the table field (at id) must be present.
 *
 * Generated builder code may be able to enforce valid unions without
 * this check by setting both type and table together, but e.g. parsers
 * may receive the type and the table independently and then it makes
 * sense to validate the union fields before table completion.
 *
 * Note that an absent union field is perfectly valid. If a union is
 * required, the type field (id - 1), should be checked separately
 * while the table field should only be checked here because it can
 * (and must) be absent when the type is NONE (= 0).
 */
int flatcc_builder_check_union_field(flatcc_builder_t *B, flatbuffers_voffset_t id);

/**
 * A struct, enum or scalar added should be stored in little endian in
 * the return pointer location. The pointer is short lived and will
 * not necessarily survive other builder calls.
 *
 * A union type field can also be set using this call. In fact, this is
 * the only way to deal with unions via this API. Consequently, it is
 * the users repsonsibility to ensure the appropriate type is added
 * at the next higher id.
 *
 * Null and default values:
 *
 * FlatBuffers does not officially  provide an option for null values
 * because it does not distinguish between default values and values
 * that are not present. At this api level, we do not deal with defaults
 * at all. Callee should test the stored value against the default value
 * and only add the field if it does not match the default. This only
 * applies to scalar and enum values. Structs cannot have defaults so
 * their absence means null, and strings, vectors and subtables do have
 * natural null values different from the empty object and empty objects
 * with different identity is also possible.
 *
 * To handle Null for scalars, the following approach is recommended:
 *
 * Provide a schema-specific `add` operation that only calls this
 * low-level add method if the default does not match, and also provide
 * another `set` operation that always stores the value, regardless of
 * default. For most readers this will be transparent, except for extra
 * space used, but for Null aware readers, these can support operations
 * to test for Null/default/other value while still supporting the
 * normal read operation that returns default when a value is absent
 * (i.e. Null).
 *
 * It is valid to call with a size of 0 - the effect being adding the
 * vtable entry. The call may also be dropped in this case to reduce
 * the vtable size - the difference will be in null detection.
 */
void *flatcc_builder_table_add(flatcc_builder_t *B, int id, size_t size, uint16_t align);

/**
 * Returns a pointer to the buffer holding the last field added. The
 * size argument must match the field size added. May, for example, be
 * used to perform endian conversion after initially updating field
 * as a native struct. Must be called before the table is ended.
 */
void *flatcc_builder_table_edit(flatcc_builder_t *B, size_t size);

/**
 * Similar to `table_add` but copies source data into the buffer before
 * it is returned. Useful when adding a larger struct already encoded in
 * little endian.
 */
void *flatcc_builder_table_add_copy(flatcc_builder_t *B, int id, const void *data, size_t size, uint16_t align);

/**
 * Add a string, vector, or sub-table depending on the type if the
 * field identifier. The offset ref obtained when the field object was
 * closed should be stored as is in the given pointer. The pointer
 * is only valid short term, so create the object before calling
 * add to table, but the owner table can be started earlier. Never mix
 * refs from nested buffers with parent buffers.
 *
 * Also uses this method to add nested buffers. A nested buffer is
 * simple a buffer created while another buffer is open. The buffer
 * close operation provides the necessary reference.
 *
 * When the table closes, all references get converted into offsets.
 * Before that point, it is not required that the offset is written
 * to.
 */
flatcc_builder_ref_t *flatcc_builder_table_add_offset(flatcc_builder_t *B, int id);

/*
 * Adds a union type and reference in a single operation and returns 0
 * on success. Stores the type field at `id - 1` and the value at
 * `id`. The `value` is a reference to a table, to a string, or to a
 * standalone `struct` outside the table.
 *
 * If the type is 0, the value field must also be 0.
 *
 * Unions can also be added as separate calls to the type and the offset
 * separately which can lead to better packing when the type is placed
 * together will other small fields.
 */
int flatcc_builder_table_add_union(flatcc_builder_t *B, int id,
        flatcc_builder_union_ref_t uref);

/*
 * Adds a union type vector and value vector in a single operations
 * and returns 0 on success.
 *
 * If both the type and value vector is null, nothing is added.
 * Otherwise both must be present and have the same length.
 *
 * Any 0 entry in the type vector must also have a 0 entry in
 * the value vector.
 */
int flatcc_builder_table_add_union_vector(flatcc_builder_t *B, int id,
        flatcc_builder_union_vec_ref_t uvref);
/**
 * Creates a vector in a single operation using an externally supplied
 * buffer. This completely bypasses the stack, but the size must be
 * known and the content must be little endian. Do not use for strings
 * and offset vectors. Other flatbuffer vectors could be used as a
 * source, but the length prefix is not required.
 *
 * Set `max_count` to `FLATBUFFERS_COUNT_MAX(elem_size)` before a call
 * to any string or vector operation to the get maximum safe vector
 * size, or use (size_t)-1 if overflow is not a concern.
 *
 * The max count property is a global property that remains until
 * explicitly changed.
 *
 * `max_count` is to prevent malicous or accidental overflow which is
 * difficult to detect by multiplication alone, depending on the type
 * sizes being used and having `max_count` thus avoids a division for
 * every vector created. `max_count` does not guarantee a vector will
 * fit in an empty buffer, it just ensures the internal size checks do
 * not overflow. A safe, sane limit woud be max_count / 4 because that
 * is half the maximum buffer size that can realistically be
 * constructed, corresponding to a vector size of `UOFFSET_MAX / 4`
 * which can always hold the vector in 1GB excluding the size field when
 * sizeof(uoffset_t) = 4.
 */
flatcc_builder_ref_t flatcc_builder_create_vector(flatcc_builder_t *B,
        const void *data, size_t count, size_t elem_size, uint16_t align, size_t max_count);

/**
 * Starts a vector on the stack.
 *
 * Do not use these calls for string or offset vectors, but do store
 * scalars, enums and structs, always in little endian encoding.
 *
 * Use `extend_vector` subsequently to add zero, one or more elements
 * at time.
 *
 * See `create_vector` for `max_count` argument (strings and offset
 * vectors have a fixed element size and does not need this argument).
 *
 * Returns 0 on success.
 */
int flatcc_builder_start_vector(flatcc_builder_t *B, size_t elem_size,
        uint16_t align, size_t max_count);

/**
 * Emits the vector constructed on the stack by start_vector.
 *
 * The vector may be accessed in the emitted stream using the returned
 * reference, even if the containing buffer is still under construction.
 * This may be useful for sorting. This api does not support sorting
 * because offset vectors cannot read their references after emission,
 * and while plain vectors could be sorted, it has been chosen that this
 * task is better left as a separate processing step. Generated code can
 * provide sorting functions that work on final in-memory buffers.
 */
flatcc_builder_ref_t flatcc_builder_end_vector(flatcc_builder_t *B);

/** Returns the number of elements currently on the stack. */
size_t flatcc_builder_vector_count(flatcc_builder_t *B);

/**
 * Returns a pointer ot the first vector element on stack,
 * accessible up to the number of elements currently on stack.
 */
void *flatcc_builder_vector_edit(flatcc_builder_t *B);

/**
 * Returns a zero initialized buffer to a new region of the vector which
 * is extended at the end. The buffer must be consumed before other api
 * calls that may affect the stack, including `extend_vector`.
 *
 * Do not use for strings, offset or union vectors. May be used for nested
 * buffers, but these have dedicated calls to provide better alignment.
 */
void *flatcc_builder_extend_vector(flatcc_builder_t *B, size_t count);

/**
 * A specialized `vector_extend` that pushes a single element.
 *
 * Returns the buffer holding a modifiable copy of the added content,
 * or null on error. Note: for structs, care must be taken to ensure
 * the source has been zero padded. For this reason it may be better to
 * use extend(B, 1) and assign specific fields instead.
 */
void *flatcc_builder_vector_push(flatcc_builder_t *B, const void *data);

/**
 * Pushes multiple elements at a time.
 *
 * Returns the buffer holding a modifiable copy of the added content,
 * or null on error.
 */
void *flatcc_builder_append_vector(flatcc_builder_t *B, const void *data, size_t count);

/**
 * Removes elements already added to vector that has not been ended.
 * For example, a vector of parsed list may remove the trailing comma,
 * or the vector may simply overallocate to get some temporary working
 * space. The total vector size must never become negative.
 *
 * Returns -1 if the count as larger than current count, or 0 on success.
 */
int flatcc_builder_truncate_vector(flatcc_builder_t *B, size_t count);

/*
 * Similar to `create_vector` but with references that get translated
 * into offsets. The references must, as usual, belong to the current
 * buffer. Strings, scalar and struct vectors can emit directly without
 * stack allocation, but offset vectors must translate the offsets
 * and therefore need the temporary space. Thus, this function is
 * roughly equivalent to to start, append, end offset vector.
 *
 * See also `flatcc_builder_create_offset_vector_direct`.
 */
flatcc_builder_ref_t flatcc_builder_create_offset_vector(flatcc_builder_t *B,
        const flatcc_builder_ref_t *data, size_t count);

/*
 * NOTE: this call takes non-const source array of references
 * and destroys the content.
 *
 * This is a faster version of `create_offset_vector` where the
 * source references are destroyed. In return the vector can be
 * emitted directly without passing over the stack.
 */
flatcc_builder_ref_t flatcc_builder_create_offset_vector_direct(flatcc_builder_t *B,
        flatcc_builder_ref_t *data, size_t count);


/**
 * Starts a vector holding offsets to tables or strings. Before
 * completion it will hold `flatcc_builder_ref_t` references because the
 * offset is not known until the vector start location is known, which
 * depends to the final size, which for parsers is generally unknown.
 */
int flatcc_builder_start_offset_vector(flatcc_builder_t *B);

/**
 * Similar to `end_vector` but updates all stored references so they
 * become offsets to the vector start.
 */
flatcc_builder_ref_t flatcc_builder_end_offset_vector(flatcc_builder_t *B);

/**
 * Same as `flatcc_builder_end_offset_vector` except null references are
 * permitted when the corresponding `type` entry is 0 (the 'NONE' type).
 * This makes it possible to build union vectors with less overhead when
 * the `type` vector is already known. Use standand offset vector calls
 * prior to this call.
 */
flatcc_builder_ref_t flatcc_builder_end_offset_vector_for_unions(flatcc_builder_t *B,
        const flatcc_builder_utype_t *type);

/** Returns the number of elements currently on the stack. */
size_t flatcc_builder_offset_vector_count(flatcc_builder_t *B);

/**
 * Returns a pointer ot the first vector element on stack,
 * accessible up to the number of elements currently on stack.
 */
void *flatcc_builder_offset_vector_edit(flatcc_builder_t *B);

/**
 * Similar to `extend_vector` but returns a buffer indexable as
 * `flatcc_builder_ref_t` array. All elements must be set to a valid
 * unique non-null reference, but truncate and extend may be used to
 * perform edits. Unused references will leave garbage in the buffer.
 * References should not originate from any other buffer than the
 * current, including parents and nested buffers.  It is valid to reuse
 * references in DAG form when contained in the sammer, excluding any
 * nested, sibling or parent buffers.
 */
flatcc_builder_ref_t *flatcc_builder_extend_offset_vector(flatcc_builder_t *B, size_t count);

/** Similar to truncate_vector. */
int flatcc_builder_truncate_offset_vector(flatcc_builder_t *B, size_t count);

/**
 * A specialized extend that pushes a single element.
 *
 * Returns the buffer holding a modifiable copy of the added content,
 * or null on error.
 */
flatcc_builder_ref_t *flatcc_builder_offset_vector_push(flatcc_builder_t *B,
        flatcc_builder_ref_t ref);

/**
 * Takes an array of refs as argument to do a multi push operation.
 *
 * Returns the buffer holding a modifiable copy of the added content,
 * or null on error.
 */
flatcc_builder_ref_t *flatcc_builder_append_offset_vector(flatcc_builder_t *B,
        const flatcc_builder_ref_t *refs, size_t count);

/**
 * All union vector operations are like offset vector operations,
 * except they take a struct with a type and a reference rather than
 * just a reference. The finished union vector is returned as a struct
 * of two references, one for the type vector and one for the table offset
 * vector. Each reference goes to a separate table field where the type
 * offset vector id must be one larger than the type vector.
 */

/**
 * Creates a union vector which is in reality two vectors, a type vector
 * and an offset vector. Both vectors references are returned.
 */
flatcc_builder_union_vec_ref_t flatcc_builder_create_union_vector(flatcc_builder_t *B,
        const flatcc_builder_union_ref_t *urefs, size_t count);

/*
 * NOTE: this call takes non-const source array of references
 * and destroys the content. The type array remains intact.
 *
 * This is a faster version of `create_union_vector` where the source
 * references are destroyed and where the types are given in a separate
 * array. In return the vector can be emitted directly without passing
 * over the stack.
 *
 * Unlike `create_offset_vector` we do allow null references but only if
 * the union type is NONE (0).
 */
flatcc_builder_union_vec_ref_t flatcc_builder_create_union_vector_direct(flatcc_builder_t *B,
        const flatcc_builder_utype_t *types, flatcc_builder_ref_t *data, size_t count);

/*
 * Creates just the type vector part of a union vector. This is
 * similar to a normal `create_vector` call except that the size
 * and alignment are given implicitly. Can be used during
 * cloning or similar operations where the types are all given
 * but the values must be handled one by one as prescribed by
 * the type. The values can be added separately as an offset vector.
 */
flatcc_builder_ref_t flatcc_builder_create_type_vector(flatcc_builder_t *B,
        const flatcc_builder_utype_t *types, size_t count);

/**
 * Starts a vector holding types and offsets to tables or strings. Before
 * completion it will hold `flatcc_builder_union_ref_t` references because the
 * offset is not known until the vector start location is known, which
 * depends to the final size, which for parsers is generally unknown,
 * and also because the union type must be separated out into a separate
 * vector. It would not be practicaly to push on two different vectors
 * during construction.
 */
int flatcc_builder_start_union_vector(flatcc_builder_t *B);

/**
 * Similar to `end_vector` but updates all stored references so they
 * become offsets to the vector start and splits the union references
 * into a type vector and an offset vector.
 */
flatcc_builder_union_vec_ref_t flatcc_builder_end_union_vector(flatcc_builder_t *B);

/** Returns the number of elements currently on the stack. */
size_t flatcc_builder_union_vector_count(flatcc_builder_t *B);

/**
 * Returns a pointer ot the first vector element on stack,
 * accessible up to the number of elements currently on stack.
 */
void *flatcc_builder_union_vector_edit(flatcc_builder_t *B);

/**
 * Similar to `extend_offset_vector` but returns a buffer indexable as a
 * `flatcc_builder_union_ref_t` array. All elements must be set to a valid
 * unique non-null reference with a valid union type to match, or it
 * must be null with a zero union type.
 */
flatcc_builder_union_ref_t *flatcc_builder_extend_union_vector(flatcc_builder_t *B, size_t count);

/** Similar to truncate_vector. */
int flatcc_builder_truncate_union_vector(flatcc_builder_t *B, size_t count);

/**
 * A specialized extend that pushes a single element.
 *
 * Returns the buffer holding a modifiable copy of the added content,
 * or null on error.
 */
flatcc_builder_union_ref_t *flatcc_builder_union_vector_push(flatcc_builder_t *B,
        flatcc_builder_union_ref_t uref);

/**
 * Takes an array of union_refs as argument to do a multi push operation.
 *
 * Returns the buffer holding a modifiable copy of the added content,
 * or null on error.
 */
flatcc_builder_union_ref_t *flatcc_builder_append_union_vector(flatcc_builder_t *B,
        const flatcc_builder_union_ref_t *urefs, size_t count);

/**
 * Faster string operation that avoids temporary stack storage. The
 * string is not required to be zero-terminated, but is expected
 * (unchecked) to be utf-8. Embedded zeroes would be allowed but
 * ubyte vectors should be used for that. The resulting string will
 * have a zero termination added, not included in length.
 */
flatcc_builder_ref_t flatcc_builder_create_string(flatcc_builder_t *B,
        const char *s, size_t len);

/** `create_string` up to zero termination of source. */
flatcc_builder_ref_t flatcc_builder_create_string_str(flatcc_builder_t *B,
        const char *s);

/**
 * `create_string` up to zero termination or at most max_len of source.
 *
 * Note that like `strncpy` it will include `max_len` characters if
 * the source is longer than `max_len`, but unlike `strncpy` it will
 * always add zero termination.
 */
flatcc_builder_ref_t flatcc_builder_create_string_strn(flatcc_builder_t *B, const char *s, size_t max_len);

/**
 * Starts an empty string that can be extended subsequently.
 *
 * While the string is being created, it is guaranteed that there is
 * always a null character after the end of the current string length.
 * This also holds after `extend` and `append` operations. It is not
 * allowed to modify the null character.
 *
 * Returns 0 on success.
 */
int flatcc_builder_start_string(flatcc_builder_t *B);

/**
 * Similar to `extend_vector` except for the buffer return type and a
 * slight speed advantage. Strings are expected to contain utf-8 content
 * but this isn't verified, and null characters would be accepted. The
 * length is given in bytes.
 *
 * Appending too much, then truncating can be used to trim string
 * escapes during parsing, or convert between unicode formats etc.
 */
char *flatcc_builder_extend_string(flatcc_builder_t *B, size_t len);

/**
 * Concatenes a length of string. If the string contains zeroes (which
 * it formally shouldn't), they will be copied in.
 *
 * Returns the buffer holding a modifiable copy of the added content,
 * or null on error.
 */
char *flatcc_builder_append_string(flatcc_builder_t *B, const char *s, size_t len);

/** `append_string` up to zero termination of source. */
char *flatcc_builder_append_string_str(flatcc_builder_t *B, const char *s);

/** `append_string` up zero termination or at most max_len of source. */
char *flatcc_builder_append_string_strn(flatcc_builder_t *B, const char *s, size_t max_len);

/**
 * Similar to `truncate_vector` available for consistency and a slight
 * speed advantage. Reduces string by `len` bytes - it does not set
 * the length. The resulting length must not become negative. Zero
 * termination is not counted.
 *
 * Returns -1 of the length becomes negative, 0 on success.
 */
int flatcc_builder_truncate_string(flatcc_builder_t *B, size_t len);

/**
 * Similar to `end_vector` but adds a trailing zero not included
 * in the length. The trailing zero is added regardless of whatever
 * zero content may exist in the provided string (although it
 * formally should not contain any).
 */
flatcc_builder_ref_t flatcc_builder_end_string(flatcc_builder_t *B);

/** Returns the length of string currently on the stack. */
size_t flatcc_builder_string_len(flatcc_builder_t *B);

/**
 * Returns a ponter to the start of the string
 * accessible up the length of string currently on the stack.
 */
char *flatcc_builder_string_edit(flatcc_builder_t *B);


/*
 * Only for use with the default emitter.
 *
 * Fast acces to small buffers from default emitter.
 *
 * Only valid for default emitters before `flatcc_builder_clear`. The
 * return buffer is not valid after a call to `flatcc_builder_reset` or
 * `flatcc_builder_clear`.
 *
 * Returns null if the buffer size is too large to a have a linear
 * memory representation or if the emitter is not the default. A valid
 * size is between half and a full emitter page size depending on vtable
 * content.
 *
 * Non-default emitters must be accessed by means specific to the
 * particular emitter.
 *
 * If `size_out` is not null, it is set to the buffer size, or 0 if
 * operation failed.
 *
 * The returned buffer should NOT be deallocated explicitly.
 *
 * The buffer size is the size reported by `flatcc_builder_get_buffer_size`.
 */
void *flatcc_builder_get_direct_buffer(flatcc_builder_t *B, size_t *size_out);

/*
 * Only for use with the default emitter.
 *
 * Default finalizer that allocates a buffer from the default emitter.
 *
 * Returns null if memory could not be allocated or if the emitter is
 * not the default. This is just a convenience method - there are many
 * other possible ways to extract the result of the emitter depending on
 * use case.
 *
 * If `size_out` is not null, it is set to the buffer size, or 0 if
 * operation failed.
 *
 * The allocated buffer is aligned according to malloc which may not be
 * sufficient in advanced cases - for that purpose
 * `flatcc_builder_finalize_aligned_buffer` may be used.
 *
 * It may be worth calling `flatcc_builder_get_direct_buffer` first to see
 * if the buffer is small enough to avoid copying.
 *
 * The returned buffer must be deallocated using `free`.
 */
void *flatcc_builder_finalize_buffer(flatcc_builder_t *B, size_t *size_out);

/*
 * Only for use with the default emitter.
 *
 * Similar to `flatcc_builder_finalize_buffer` but ensures the returned
 * memory is aligned to the overall alignment required for the buffer.
 * Often it is not necessary unless special operations rely on larger
 * alignments than the stored scalars.
 *
 * If `size_out` is not null, it is set to the buffer size, or 0 if
 * operation failed.
 *
 * The returned buffer must be deallocated using `aligned_free` which is
 * implemented via `flatcc_flatbuffers.h`. `free` will usually work but
 * is not portable to platforms without posix_memalign or C11
 * aligned_alloc support.
 *
 * NOTE: if a library might be compiled with a version of aligned_free
 * that differs from the application using it, use
 * `flatcc_builder_aligned_free` to make sure the correct deallocation
 * function is used.
 */
void *flatcc_builder_finalize_aligned_buffer(flatcc_builder_t *B, size_t *size_out);

/*
 * A stable implementation of `aligned_alloc` that is not sensitive
 * to the applications compile time flags.
 */
void *flatcc_builder_aligned_alloc(size_t alignment, size_t size);

/*
 * A stable implementation of `aligned_free` that is not sensitive
 * to the applications compile time flags.
 */
void flatcc_builder_aligned_free(void *p);

/*
 * Same allocation as `flatcc_builder_finalize_buffer` returnes. Usually
 * same as `malloc` but can redefined via macros.
 */
void *flatcc_builder_alloc(size_t size);

/*
 * A stable implementation of `free` when the default allocation
 * methods have been redefined.
 *
 * Deallocates memory returned from `flatcc_builder_finalize_buffer`.
 */
void flatcc_builder_free(void *p);

/*
 * Only for use with the default emitter.
 *
 * Convenience method to copy buffer from default emitter. Forwards
 * call to default emitter and returns input pointer, or null if
 * the emitter is not default or of the given size is smaller than
 * the buffer size.
 *
 * Note: the `size` argument is the target buffers capacity, not the
 * flatcc_builders buffer size.
 *
 * Other emitters have custom interfaces for reaching their content.
 */
void *flatcc_builder_copy_buffer(flatcc_builder_t *B, void *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_BUILDER_H */
