#pragma once
/*
 * Handler idioms for generated MSP wire structs.
 *
 * Two operations, defined once so no handler can get them subtly wrong:
 *
 *   mspReadRequest()  gate on length, copy into the struct, advance the buffer
 *   mspWriteReply()   emit a filled struct
 *
 * The advance matters. sbufReadDataSafe() does NOT move the read pointer, unlike
 * sbufReadU8()/U16()/U32(). Every hand-written call has to remember sbufAdvance()
 * afterwards, and forgetting it silently re-reads from offset 0 -- exactly the
 * bug found in MSP_SET_RC_TUNING's optional 11th byte. Encapsulating the pair
 * makes that class of bug unrepresentable rather than merely fixed.
 *
 * Length policy is deliberately lenient in one direction only:
 *   longer  than the struct -> accept, ignore the trailing bytes
 *   shorter than the struct -> reject
 * MSP payloads only ever gain fields at the end, so a newer sender must still be
 * understood by older firmware.
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "common/streambuf.h"

/* Read a fixed-layout request into *pkt. Returns false if the payload is short. */
static inline bool mspReadRequestBytes(sbuf_t *src, void *pkt, size_t size, int dataSize)
{
    if ((size_t)dataSize < size) {
        return false;
    }
    if (!sbufReadDataSafe(src, pkt, (int)size)) {
        return false;
    }
    sbufAdvance(src, (int)size);
    return true;
}

/* Read one appended optional field. Returns false when it is simply absent. */
static inline bool mspReadOptionalBytes(sbuf_t *src, void *field, size_t size)
{
    if ((size_t)sbufBytesRemaining(src) < size) {
        return false;
    }
    if (!sbufReadDataSafe(src, field, (int)size)) {
        return false;
    }
    sbufAdvance(src, (int)size);
    return true;
}

#define mspReadRequest(src, pkt, dataSize) \
    mspReadRequestBytes((src), (pkt), sizeof(*(pkt)), (dataSize))

#define mspReadOptional(src, field) \
    mspReadOptionalBytes((src), (field), sizeof(*(field)))

#define mspWriteReply(dst, pkt) \
    sbufWriteData((dst), (pkt), (int)sizeof(*(pkt)))

/* Emit the fixed head of a variable-length reply, records follow separately. */
#define mspWriteReplyBytes(dst, pkt, size) \
    sbufWriteData((dst), (pkt), (int)(size))
