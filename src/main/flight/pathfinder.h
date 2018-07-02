//
//  pathfinder.h
//  RTH
//
//  Created by Alberto García Hierro on 16/12/2017.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t pathfinder_index_t;
typedef int32_t pathfinder_alt_t;

// TODO: Assert that pathfinder_index_t can address all points

typedef struct pathfinderPoint_s {
    int32_t lat; // deg * 1e7
    int32_t lng;
    pathfinder_alt_t alt; // cm
    float distance; // Hausdorff distance that removing this point would add to the curve
    pathfinder_index_t prev;
    pathfinder_index_t next;
} pathfinderPoint_t;

typedef struct pathfinder_s {
    pathfinder_index_t last;
    pathfinder_index_t count;
    float initialEpsilon;               // Initial threshold for removing insignificant points. Must be > 0.
    float currentEpsilon;               // Currently applied epsilon value. Will grow when the path becomes full and shrink when the path intesects with itself.
    float epsilonFactor;                // How much we grow epsilon when we need to prune. Must be > 1.
    pathfinderPoint_t *points;
    pathfinder_index_t capacity;
} pathfinder_t;

typedef pathfinder_alt_t (*pathfinder_alt_f)(int32_t lat, int32_t lng, void *userData);
typedef void (*pathfinderPoint_f)(pathfinder_t *pf, pathfinder_index_t pos, pathfinderPoint_t *p, void *userData);

void pathfinderInit(pathfinder_t *pf, pathfinderPoint_t *storage, pathfinder_index_t capacity);
void pathfinderReset(pathfinder_t *pf);

float pathfinderGetInitialEpsilon(const pathfinder_t *pf);
void pathfinderSetInitialEpsilon(pathfinder_t *pf, float epsilon);
float pathfinderGetEpsilonFactor(const pathfinder_t *pf);
void pathfinderSetEpsilonFactor(pathfinder_t *pf, float factor);

void pathFinderForEach(pathfinder_t *pf, pathfinderPoint_f f, void *userData);

void pathfinderAdd(pathfinder_t *pf, int32_t lat, int32_t lng, pathfinder_alt_t alt);
// alt is called to determine altitude for RTH path over unsafe space in case
// the return path is too long. It will be called once per point in the unsafe
// path (if any).
void pathfinderPrepareReturn(pathfinder_t *pf, pathfinder_alt_f alt, void *alt_data);
int pathfinderCount(const pathfinder_t *pf);
bool pathfinderPointAt(const pathfinder_t *pf, pathfinder_index_t n,
                       int32_t *lat, int32_t *lng, int32_t *alt);
bool pathfinderPop(pathfinder_t *pf, int32_t *lat, int32_t *lng, int32_t *alt);
