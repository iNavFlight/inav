//
//  pathfinder.c
//  RTH
//
//  Created by Alberto García Hierro on 16/12/2017.
//

#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pathfinder.h"

#ifndef MAX
#define MAX(x,y) ({ \
    __typeof__(x) x1 = x; \
    __typeof__(y) y1 = y; \
    x1 > y1 ? x1 : y1; \
})
#endif

#define RAD(x) (((x)/1.0e7) * (M_PI/180.0f))
#define SQ(x) ({ \
    __typeof__(x) x1 = x; \
    x1 * x1; \
})

#define PATHFINDER_PTR_NONE 0
#define PATHFINDER_PTR_TO(ii) (ii+1)
#define PATHFINDER_DEREF(ii) (ii-1)

#define PATHFINDER_DEFAULT_INITIAL_EPSILON  50
#define PATHFINDER_DEFAULT_EPSILON_FACTOR   1.2f

// Maximum ratio between return path and RTH in straight line. If path
// is too long, a straight line RTH will be performed instead.
#define PATHFINDER_MAX_PATH_RATIO(dist) (1+10/SQ(dist / 1000))
// Mininum return path distance to use PATHFINDER_MAX_PATH_RATIO,
// in meters.
#define PATHFINDER_MIN_RATIO_DISTANCE 100
// Maximum (path / straight RTH) ratio allowed to go join the safe path
// instead of returning in straight line
#define PATHFINDER_MAX_JOIN_RATIO (1/1)

static float pathfinderPointDistance(const pathfinderPoint_t *p1, const pathfinderPoint_t *p2)
{
    const float R = 6371e3;

    float dlng = RAD(p1->lng - p2->lng);
    float lat1 = RAD(p1->lat);
    float lat2 = RAD(p2->lat);

    float dz = sinf(lat1) - sinf(lat2);
    float dx = cosf(dlng) * cosf(lat1) - cosf(lat2);
    float dy = sinf(dlng) * cosf(dlng);
    float gd = asinf(sqrtf(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
    return gd + abs(p1->alt - p2->alt) / 100.0f;
}

static pathfinderPoint_t *pathfinderPrev(pathfinder_t *pf, const pathfinderPoint_t *p)
{
    return &pf->points[PATHFINDER_DEREF(p->prev)];
}

static pathfinderPoint_t *pathfinderNext(pathfinder_t *pf, const pathfinderPoint_t *p)
{
    return &pf->points[PATHFINDER_DEREF(p->next)];
}

static void pathfinderPointCopy(pathfinderPoint_t *dst, const pathfinderPoint_t *src)
{
    dst->lat = src->lat;
    dst->lng = src->lng;
    dst->alt = src->alt;
}

static void pathfinderUpdateDistance(pathfinder_t *pf, pathfinderPoint_t *c)
{
    // We calculate the distance by doing a 3D triangle between the 3 points
    // and calculating an estimated upper bound for its height.
    // For the estimation, we calculate each side squared, then apply Heron's
    // formula to get an upper bound for the area. Finally, we apply the
    // A = 1/2ba area formula to estimate the distance from c to the line between
    // p and n.
    pathfinderPoint_t *p = pathfinderPrev(pf, c);
    pathfinderPoint_t *n = pathfinderNext(pf, c);
    float cp_dist = pathfinderPointDistance(c, p);
    float cn_dist = pathfinderPointDistance(c, n);
    float pn_dist = pathfinderPointDistance(p, n);
    if (pn_dist == 0) {
        // P and N are very close, use cp_dist as distance
        c->distance = SQ(cp_dist);
    } else {
        float s = (cp_dist + cn_dist + pn_dist) / 2;
        c->distance = MAX(s * (s - cp_dist) * (s - cn_dist) * (s - pn_dist), 0);
    }
}

static bool pathfinderShouldRemovePoint(pathfinder_t *pf, pathfinderPoint_t *p)
{
    return p->distance < pf->currentEpsilon;
}

static void pathfinderPrune(pathfinder_t *pf)
{
    if (pf->count <= 2) {
        return;
    }
    int count = pf->count;
    while (count == pf->count) {
        // Increase epsilon and drop all points that fall below the new epsilon
        pf->currentEpsilon *= pf->epsilonFactor;
        pathfinderPoint_t *cur = &pf->points[PATHFINDER_DEREF(pf->points[0].next)];
        pathfinderPoint_t *last = &pf->points[PATHFINDER_DEREF(pf->last)];
        pathfinderPoint_t *end = last;
        while(cur != end) {
            pathfinderPoint_t *next = &pf->points[PATHFINDER_DEREF(cur->next)];
            if (pathfinderShouldRemovePoint(pf, cur)) {
                pathfinderPoint_t *prev = pathfinderPrev(pf, cur);
                last->next = prev->next;
                prev->next = cur->next;
                next->prev = cur->prev;
                cur->prev = pathfinderPrev(pf, last)->next;
                cur->next = PATHFINDER_PTR_NONE;
                pf->count--;
                last = cur;
                pathfinderUpdateDistance(pf, prev);
                pathfinderUpdateDistance(pf, next);
            }
            cur = next;
        }
    }
}

// Returs true iff (num / denom) is in the [0, 1] interval
// without doing a division
static inline bool div_in_0_1(float num, float denom)
{
    return (num >= 0 && num <= denom) || (num <= 0 && num >= denom);
}

// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines
// intersect the intersection point may be stored in the floats i_x and i_y.
static char get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
                           float p2_x, float p2_y, float p3_x, float p3_y,
                           float *i_x, float *i_y)
{
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    float snum = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y));
    float sdenom = (-s2_x * s1_y + s1_x * s2_y);
    float tnum = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x));
    float tdenom = (-s2_x * s1_y + s1_x * s2_y);

    if (div_in_0_1(snum, sdenom) && div_in_0_1(tnum, tdenom))
    {
        // Collision detected
        float t = tnum / tdenom;
        if (i_x != NULL)
            *i_x = p0_x + (t * s1_x);
        if (i_y != NULL)
            *i_y = p0_y + (t * s1_y);
        return 1;
    }

    return 0; // No collision
}

static void pathfinderIntersect(pathfinder_t *pf)
{
    // Based on https://stackoverflow.com/a/1968345
    // Grab the segment defined by the last two points, then check it against
    // all previous segments in the path.
    pathfinderPoint_t *end = &pf->points[PATHFINDER_DEREF(pf->last)];
    pathfinderPoint_t *last = pathfinderPrev(pf, end);
    if (last->prev == PATHFINDER_PTR_NONE) {
        return;
    }
    pathfinderPoint_t *prev_last = pathfinderPrev(pf, last);
    if (prev_last->prev == PATHFINDER_PTR_NONE) {
        return;
    }
    // Start 3 points away, otherwise we always get a collision between the last
    // two segments because they share a point.
    pathfinderPoint_t *cur = pathfinderPrev(pf, prev_last);
    int removed_count = 2;
    while(cur->prev != PATHFINDER_PTR_NONE) {
        pathfinderPoint_t *cur_prev = pathfinderPrev(pf, cur);
        float i_x, i_y;
        if (get_line_intersection(last->lat, last->lng, prev_last->lat, prev_last->lng,
                                  cur->lat, cur->lng, cur_prev->lat, cur_prev->lng,
                                  &i_x, &i_y)) {
            cur->lat = i_x;
            cur->lng = i_y;
            cur->alt = cur_prev->alt;
            pathfinderUpdateDistance(pf, cur_prev);
            pf->last = cur->next;
            pf->count -= removed_count;
            pf->currentEpsilon = pf->initialEpsilon;
            break;
        }
        removed_count++;
        cur = cur_prev;
    }
}

void pathfinderInit(pathfinder_t *pf, pathfinderPoint_t *storage, pathfinder_index_t capacity)
{
    pf->points = storage;
    pf->capacity = capacity;
    pathfinderReset(pf);
}

void pathfinderReset(pathfinder_t *pf)
{
    pf->last = PATHFINDER_PTR_TO(0);
    pf->count = 0;
    pf->initialEpsilon = PATHFINDER_DEFAULT_INITIAL_EPSILON;
    pf->currentEpsilon = pf->initialEpsilon;
    pf->epsilonFactor = PATHFINDER_DEFAULT_EPSILON_FACTOR;
    memset(pf->points, 0, sizeof(pf->points[0]) * pf->capacity);
    for (unsigned ii = 0; ii < pf->capacity; ii++) {
        pf->points[ii].prev = PATHFINDER_PTR_TO(ii - 1);
        pf->points[ii].next = PATHFINDER_PTR_TO(ii + 1);
    }
    pf->points[0].prev = PATHFINDER_PTR_NONE;
    pf->points[pf->capacity - 1].next = PATHFINDER_PTR_NONE;
}

float pathfinderGetInitialEpsilon(const pathfinder_t *pf)
{
    return pf->initialEpsilon;
}

void pathfinderSetInitialEpsilon(pathfinder_t *pf, float epsilon)
{
    assert(epsilon > 0);
    if (pf->currentEpsilon == pf->initialEpsilon) {
        pf->currentEpsilon = epsilon;
    }
    pf->initialEpsilon = epsilon;
}

float pathfinderGetEpsilonFactor(const pathfinder_t *pf)
{
    return pf->epsilonFactor;
}

void pathfinderSetEpsilonFactor(pathfinder_t *pf, float factor)
{
    assert(factor > 1);
    pf->epsilonFactor = factor;
}

void pathFinderForEach(pathfinder_t *pf, pathfinderPoint_f f, void *userData)
{
    assert(f);
    pathfinderPoint_t *cur = &pf->points[0];
    pathfinderPoint_t *last = &pf->points[PATHFINDER_DEREF(pf->last)];
    int pos = 0;

    while (cur != last) {
        f(pf, pos++, cur, userData);
        cur = pathfinderNext(pf, cur);
    }
}

void pathfinderAdd(pathfinder_t *pf, int32_t lat, int32_t lng, pathfinder_alt_t alt)
{
    pathfinderPoint_t *last = &pf->points[PATHFINDER_DEREF(pf->last)];
    if (last->next == PATHFINDER_PTR_NONE) {
        pathfinderPrune(pf);
        assert(last->next != PATHFINDER_PTR_NONE);
    }
    last->lat = lat;
    last->lng = lng;
    last->alt = alt;
    if (last->prev != PATHFINDER_PTR_NONE && last->prev != PATHFINDER_PTR_TO(0)) {
        // Update distance
        pathfinderPoint_t *prev = &pf->points[PATHFINDER_DEREF(last->prev)];
        pathfinderUpdateDistance(pf, prev);
        if (pathfinderShouldRemovePoint(pf, prev)) {
            // Adding this new point allows deleting the previous last one.
            // We don't need to update pf->first nor pf->last, just overwrite
            pathfinderPointCopy(prev, last);
            if (prev->prev != PATHFINDER_PTR_NONE) {
                pathfinderUpdateDistance(pf, pathfinderPrev(pf, prev));
            }
            pathfinderIntersect(pf);
            return;
        }
    }
    pf->last = last->next;
    pf->count++;
    pathfinderIntersect(pf);
}

void pathfinderPrepareReturn(pathfinder_t *pf, pathfinder_alt_f alt, void *alt_data)
{
    const pathfinderPoint_t *last = &pf->points[PATHFINDER_DEREF(pf->last)];
    if (last->prev != PATHFINDER_PTR_NONE) {
        pathfinderPoint_t *begin = &pf->points[0];
        pathfinderPoint_t *end = pathfinderPrev(pf, last);
        float direct_distance = pathfinderPointDistance(begin, end);
        float path_distance = 0;
        float path_distance_at[pf->capacity];
        pathfinderPoint_t *cur;
        pathfinderPoint_t *prev;
        int ii;
        int count = 0;
        for (cur = end, ii = 0; cur != begin; cur = prev, count++) {
            prev = pathfinderPrev(pf, cur);
            float distance = pathfinderPointDistance(cur, prev);
            path_distance += distance;
            path_distance_at[ii++] = path_distance;
        }
        float max_distance_ratio = PATHFINDER_MAX_PATH_RATIO(path_distance);
        if (path_distance >= PATHFINDER_MIN_RATIO_DISTANCE &&
            (path_distance / direct_distance) > max_distance_ratio) {
            // Try to find a point along the path which ends up with an
            // acceptable total distance minimizing distance flown over
            // unsafe space.
            float min_acceptable_distance = direct_distance * (max_distance_ratio * PATHFINDER_MAX_JOIN_RATIO);
            float min_unsafe_distance = FLT_MAX;
            pathfinderPoint_t *path_join = NULL;
            pathfinderPoint_t *home_join = NULL;
            int removed = 0;
            int removed_at_cur = 0;
            // Skip testing "end", since it's the last node and it won't
            // reduce the total path length
            for (cur = pathfinderPrev(pf, end), ii = 1; cur != begin; cur = pathfinderPrev(pf, cur), removed_at_cur++, ii++) {
                // For each point test both CUR==straight=>PATH->HOME (path join)
                // and PATH->CUR==straight==>HOME (home join)
                float path_join_unsafe_distance = pathfinderPointDistance(end, cur);
                float path_join_total_distance = path_join_unsafe_distance + (path_distance - path_distance_at[ii]);
                if (path_join_total_distance <= min_acceptable_distance &&
                    path_join_unsafe_distance < min_unsafe_distance) {

                    path_join = cur;
                    home_join = NULL;
                    min_unsafe_distance = path_join_unsafe_distance;
                    removed = removed_at_cur;
                }
                float home_join_unsafe_distance = pathfinderPointDistance(cur, begin);
                float home_join_total_distance = home_join_unsafe_distance + path_distance_at[ii];
                if (home_join_total_distance <= min_acceptable_distance &&
                    home_join_unsafe_distance < min_unsafe_distance) {

                    path_join = NULL;
                    home_join = cur;
                    min_unsafe_distance = home_join_unsafe_distance;
                    removed = pf->count - removed_at_cur - 2;
                }
            }
            // TODO: If the craft can climb vertically, we want to add TWO middle
            // points with the RTH altitude between the join and home. Otherwise
            // we want to adjust the altitude of the join point AND home.
            pathfinderPoint_t *join;
            if (home_join) {
                // Follow the path until the join, then go straight home
                join = pathfinderPrev(pf, home_join);
                begin->next = join->next;
                home_join->prev = pathfinderNext(pf, begin)->prev;
                pf->count -= removed;
                // TODO: Altitude
            } else if (path_join) {
                // Go to the path first, the follow it to home
                join = pathfinderNext(pf, path_join);
                pathfinderPointCopy(join, end);
                pf->count -= removed;
                if (alt) {
                    join->alt = alt(join->lat, join->lng, alt_data);
                }
                pathfinderUpdateDistance(pf, join);
            } else {
                // If we have no joining point, go straight to home
                join = pathfinderNext(pf, begin);
                pf->count = 2;
                pathfinderPointCopy(join, end);
                join->distance = 0;
                if (alt) {
                    join->alt = alt(join->lat, join->lng, alt_data);
                }
            }
        }
    }
}

int pathfinderCount(const pathfinder_t *pf)
{
    return pf->count;
}

static bool pathfinderFullPointAt(const pathfinder_t *pf, pathfinder_index_t n, const pathfinderPoint_t **point)
{
    if (n >= pf->count) {
        return false;
    }

    // TODO: Fast path for pf->last. Don't add it yet since it might mask bugs

    const pathfinderPoint_t *p = &pf->points[0];
    for (unsigned ii = 0; ii < n; ii++) {
        if (p->next == PATHFINDER_PTR_NONE) {
            return false;
        }
        p = &pf->points[PATHFINDER_DEREF(p->next)];
    }
    // We're at the desired point
    *point = p;
    return true;
}

bool pathfinderPointAt(const pathfinder_t *pf, pathfinder_index_t n,
                       int32_t *lat, int32_t *lng, int32_t *alt)
{
    const pathfinderPoint_t *p;
    if (pathfinderFullPointAt(pf, n, &p)) {
        *lat = p->lat;
        *lng = p->lng;
        *alt = p->alt;
        return true;
    }
    return false;
}

bool pathfinderPop(pathfinder_t *pf, int32_t *lat, int32_t *lng, int32_t *alt)
{
    const pathfinderPoint_t *p;
    if (pathfinderFullPointAt(pf, pf->count - 1, &p)) {
        *lat = p->lat;
        *lng = p->lng;
        *alt = p->alt;

        // Remove point
        pathfinderPoint_t *prev = pathfinderPrev(pf, p);
        pf->last = prev->next;
        pf->count--;
        if (pf->count == 0) {
            // Do a full reset, so all slots are aligned again
            pathfinderReset(pf);
        }
        return true;
    }
    return false;
}
