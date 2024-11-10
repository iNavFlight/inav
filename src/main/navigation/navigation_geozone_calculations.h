/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <float.h>

#include "common/vector.h" 
#include "navigation/navigation_private.h"

#define K_EPSILON 1e-8f

static bool isPointInCircle(const fpVector2_t *point, const fpVector2_t *center, const float radius)
{
    return calculateDistance2(point, center) < radius;
}

static bool isPointInPloygon(const fpVector2_t *point, fpVector2_t* vertices, const uint8_t verticesNum)
{
	bool result = false;
	fpVector2_t *p1, *p2;
	fpVector2_t* prev = &vertices[verticesNum - 1];
	fpVector2_t *current;
	for (uint8_t i = 0; i < verticesNum; i++)
	{
		current = &vertices[i];

		if (current->x > prev->x) {
			p1 = prev;
			p2 = current;
		} else {
			p1 = current;
			p2 = prev;
		}

		if ((current->x < point->x) == (point->x <= prev->x)
			&& (point->y - p1->y) * (p2->x - p1->x) < (p2->y - p1->y) * (point->x - p1->x))
		{
			result = !result;
		}
		prev = current;
	}
	return result;
}

static float angelFromSideLength(const float a, const float b, const float c)
{
	return acos_approx((sq(b) + sq(c) - sq(a)) / (2 * b * c));
}

static bool isPointRightFromLine(const fpVector2_t *lineStart, const fpVector2_t *lineEnd, const fpVector2_t *point) {
	return (lineEnd->x - lineStart->x) * (point->y - lineStart->y) - (lineEnd->y - lineStart->y) * (point->x - lineStart->x) > 0;
}

static float calcAngelFrom3Points(const fpVector2_t* a, const fpVector2_t* b, const fpVector2_t* c)
{
	float ab = calculateDistance2(a, b);
	float ac = calculateDistance2(a, c);
	float bc = calculateDistance2(b, c);
	
	return RADIANS_TO_DEGREES(angelFromSideLength(ab, ac, bc));
}

static void calcPointOnLine(fpVector2_t *result, const fpVector2_t *start, fpVector2_t *end, float distance) 
{
	fpVector2_t dir, a;
	float fac;
	vector2Sub(&dir, end, start);
	fac = distance / fast_fsqrtf(vector2NormSquared(&dir));
	vector2Scale(&a, &dir, fac);
	vector2Add(result, start, &a);
}

// Intersection of two lines https://en.wikipedia.org/wiki/Line-line_intersection
bool calcLineLineIntersection(fpVector2_t* intersection, const fpVector2_t* line1Start, const fpVector2_t* line1End, const fpVector2_t* line2Start, const fpVector2_t* line2End, bool isSegment)
{
	intersection->x = intersection->y = 0;

    // Double precision is needed here
	double s1 = (double)(line1End->x - line1Start->x);
	double t1 = (double)(-(line2End->x - line2Start->x));
	double r1 = (double)(line2Start->x - line1Start->x);

	double s2 = (double)(line1End->y - line1Start->y);
	double t2 = (double)(-(line2End->y - line2Start->y));
	double r2 = (double)(line2Start->y - line1Start->y);

	// Use Cramer's rule for the solution of the system of linear equations
	double determ = s1 * t2 - t1 * s2;
	if (determ == 0) { // No solution
		return false;
	}

	double s0 = (r1 * t2 - t1 * r2) / determ;
	double t0 = (s1 * r2 - r1 * s2) / determ;

	if (s0 == 0 && t0 == 0) {
		return false;
	}

	// No intersection
	if (isSegment && (s0 <= 0 || s0 >= 1 || t0 <= 0 || t0 >= 1)) {
		return false;
	}

	intersection->x = (line1Start->x + (float)s0 * (line1End->x - line1Start->x));
	intersection->y = (line1Start->y + (float)s0 * (line1End->y - line1Start->y));

	return true;
}

float calculateDistance3(const fpVector3_t* startPos, const fpVector3_t* destinationPos) 
{
   return fast_fsqrtf(sq(destinationPos->x - startPos->x) + sq(destinationPos->y - startPos->y) + sq(destinationPos->z - startPos->z));
}

static fpVector3_t calcPlaneNormalFromPoints(const fpVector3_t* p1, const fpVector3_t* p2, const fpVector3_t* p3)
{
	fpVector3_t result, a, b;
	vectorSub(&a, p2, p1);
	vectorSub(&b, p3, p1);
	vectorCrossProduct(&result, &a, &b);
	return result;
}

static fpVector3_t calcDirVectorFromPoints(const fpVector3_t* p1, const fpVector3_t* p2)
{
	fpVector3_t result;
	vectorSub(&result, p1, p2);
	return result;
}

static void generatePolygonFromCircle(fpVector2_t* polygon, const fpVector2_t* center, const float radius, const uint8_t sides)
{
	for (uint8_t i = 0, j = sides -1; i < sides; i++, j--) {
		polygon[i].x = radius * cos_approx(2 * M_PIf * j / sides) + center->x;
		polygon[i].y = radius * sin_approx(2 * M_PIf * j / sides) + center->y;
	}
}

// TRUE if point is in the same direction from pos as ref
static bool isInFront(const fpVector3_t *pos, const fpVector3_t *point, const fpVector3_t *ref)
{
    fpVector3_t refDir = calcDirVectorFromPoints(pos, ref);
    fpVector3_t dir = calcDirVectorFromPoints(point, pos);
    return vectorDotProduct(&refDir, &dir) < 0.0f;
}

static fpVector2_t calcNearestPointOnLine(const fpVector2_t *lineStart, const fpVector2_t *lineEnd, const fpVector2_t *point)
{
    fpVector2_t ap, ab, prod2, result;
	float distance, magAb, prod;
	vector2Sub(&ap, point, lineStart);
	vector2Sub(&ab, lineEnd, lineStart);
	magAb = vector2NormSquared(&ab);
	prod = vector2DotProduct(&ap, &ab);
	distance = prod / magAb;
	if (distance < 0) {
		return *lineStart;
	} else if (distance > 1) {
		return *lineEnd;
	}
	vector2Scale(&prod2, &ab, distance);
	vector2Add(&result, lineStart, &prod2);
	return result;
}

static bool isPointOnLine2(const fpVector2_t *lineStart, const fpVector2_t *lineEnd, const fpVector2_t *linepoint) 
{
	float a = roundf(calculateDistance2(linepoint, lineStart));
	float b = roundf(calculateDistance2(linepoint, lineEnd));
	float c = roundf(calculateDistance2(lineStart, lineEnd));
	return ABS(a + b - c) <= 1;
}

static bool isPointOnLine3(const fpVector3_t *lineStart, const fpVector3_t *lineEnd, const fpVector3_t *linepoint) 
{
	float a = roundf(calculateDistance3(linepoint, lineStart));
	float b = roundf(calculateDistance3(linepoint, lineEnd));
	float c = roundf(calculateDistance3(lineStart, lineEnd));
	return ABS(a + b - c) <= 1;
}

static bool calcIntersectionLinePlane(fpVector3_t* result, const fpVector3_t* lineVector, const fpVector3_t* linePoint, const fpVector3_t* planeNormal, const fpVector3_t* planePoint)
{
	if (vectorDotProduct(linePoint, planeNormal) == 0) {
		return false;
	}
	fpVector3_t diff, p1, p4;
	float p2 = 0, p3 = 0;
	vectorSub(&diff, linePoint, planePoint);
	vectorAdd(&p1, &diff, planePoint);
	p2 = vectorDotProduct(&diff, planeNormal);
	p3 = vectorDotProduct(lineVector, planeNormal);
	vectorScale(&p4, lineVector, -p2 / p3);
	vectorAdd(result, &p1, &p4);
	return true;
}


// Möller–Trumbore intersection algorithm
static bool calcLineTriangleIntersection(fpVector3_t *intersection, const fpVector3_t* org, const fpVector3_t* dir, const fpVector3_t* v0, const fpVector3_t* v1, const fpVector3_t* v2)
{
	fpVector3_t v0v1, v0v2, pvec, tvec, quvec, prod;
	float det, invDet, t, u, v;

	vectorSub(&v0v1, v1, v0);
	vectorSub(&v0v2, v2, v0);
	vectorCrossProduct(&pvec, dir, &v0v2);
	det = vectorDotProduct(&v0v1, &pvec);
	if (fabsf(det) < K_EPSILON) {
		return false;
	}
	invDet = 1.f / det;
	vectorSub(&tvec, org, v0);
	u = vectorDotProduct(&tvec, &pvec) * invDet;
	if (u < 0 || u > 1) {
		return false;
	}
	vectorCrossProduct(&quvec, &tvec, &v0v1);
	v = vectorDotProduct(dir, &quvec) * invDet;
	if (v < 0 || u + v > 1) {
		return false;
	}
	t = vectorDotProduct(&v0v2, &quvec) * invDet;
	vectorScale(&prod, dir, t);
	vectorAdd(intersection, &prod, org);
	return true;
}


static bool calcLinePolygonIntersection(fpVector3_t *intersect, const fpVector3_t *pos, const fpVector3_t *pos2, const float height, fpVector2_t* vertices, const uint8_t verticesNum) 
{
	if (verticesNum < 3) {
		return false;
	}
	
	fpVector3_t p1 = { .x = vertices[0].x, .y = vertices[0].y, .z = height };
	fpVector3_t p2 = { .x = vertices[1].x, .y = vertices[1].y, .z = height };
	fpVector3_t p3 = { .x = vertices[2].x, .y = vertices[2].y, .z = height };
	fpVector3_t normale = calcPlaneNormalFromPoints(&p1, &p2, &p3);
	fpVector3_t dir = calcDirVectorFromPoints(pos, pos2);

	fpVector3_t tmp;	
	if (calcIntersectionLinePlane(&tmp, &dir, pos2, &normale, &p1)) {
		if (isPointInPloygon((fpVector2_t*)&tmp, vertices, verticesNum)) {
			memcpy(intersect, &tmp, sizeof(fpVector3_t));
			return true;
		}
	}
	return false;
}

static bool calcLineCircleIntersection(fpVector2_t *intersection1, fpVector2_t *intersection2, const fpVector2_t *startPos, const fpVector2_t *endPos, const fpVector2_t *circleCenter, const float radius)
{
    // Unfortunately, we need double precision here
	double slope = (double)((endPos->y - startPos->y) / (endPos->x - startPos->x));
	double yIntercept = (double)startPos->y - slope * (double)startPos->x;

	double a = (double)1.0 + sq(slope);
	double b = (double)-2.0 * (double)circleCenter->x + (double)2.0 * slope * (yIntercept - (double)circleCenter->y);
	double c = sq((double)circleCenter->x) + (yIntercept - (double)circleCenter->y) * (yIntercept - (double)circleCenter->y) - sq((double)radius);

	double discr = sq(b) - (double)4.0 * a * c;
	if (discr > 0)
	{
        double x1 = (-b + (double)fast_fsqrtf(discr)) / ((double)2.0 * a);
        double y1 = slope * x1 + yIntercept;
        double x2 = (-b - (double)fast_fsqrtf(discr)) / ((double)2.0 * a);
        double y2 = slope * x2 + yIntercept;
        
        intersection1->x = (float)x1;
		intersection1->y = (float)y1;
		intersection2->x = (float)x2;
		intersection2->y = (float)y2;
		return true;
	}
	return false;
}

static void generateOffsetPolygon(fpVector2_t* verticesNew, fpVector2_t* verticesOld, const uint8_t numVertices, const float offset)
{
	fpVector2_t* prev = &verticesOld[numVertices - 1];
	fpVector2_t* current;
	fpVector2_t* next;
	for (uint8_t i = 0; i < numVertices; i++) {
		current = &verticesOld[i];
		if (i + 1 < numVertices) {
			next = &verticesOld[i + 1];
		}
		else {
			next = &verticesOld[0];
		}

		fpVector2_t v, vn, vs, pcp1, pcp2, pcn1, pcn2, intersect;
		vector2Sub(&v, current, prev);
		vector2Normalize(&vn, &v);
		vector2Scale(&vs, &vn, offset);
		pcp1.x = prev->x - vs.y;
		pcp1.y = prev->y + vs.x;
		pcp2.x = current->x - vs.y;
		pcp2.y = current->y + vs.x;

		vector2Sub(&v, next, current);
		vector2Normalize(&vn, &v);
		vector2Scale(&vs, &vn, offset);
		pcn1.x = current->x - vs.y;
		pcn1.y = current->y + vs.x;
		pcn2.x = next->x - vs.y;
		pcn2.y = next->y + vs.x;

		if (calcLineLineIntersection(&intersect, &pcp1, &pcp2, &pcn1, &pcn2, false)) {
			verticesNew[i].x = intersect.x;
			verticesNew[i].y = intersect.y;
		}
		prev = current;
	}
}

// Calculates the nearest intersection point
// Inspired by raytracing algortyhms
static bool calcLineCylinderIntersection(fpVector3_t* intersection, float* distance, const fpVector3_t* startPos, const fpVector3_t* endPos, const fpVector3_t* circleCenter, const float radius, const float height, const bool inside)
{
	float distToIntersection = FLT_MAX;
	fpVector3_t intersect;

	fpVector2_t i1, i2;
	if (!calcLineCircleIntersection(&i1, &i2, (fpVector2_t*)startPos, (fpVector2_t*)endPos, (fpVector2_t*)circleCenter, radius)) {
		return false;
	}

	if (calculateDistance2((fpVector2_t*)startPos, &i1) < calculateDistance2((fpVector2_t*)startPos, &i2)) {
		intersect.x = i1.x;
		intersect.y = i1.y;
	} else {
		intersect.x = i2.x;
		intersect.y = i2.y;
	}

	float dist1 = calculateDistance2((fpVector2_t*)endPos, (fpVector2_t*)&intersect);
	float dist2 = calculateDistance2((fpVector2_t*)startPos, (fpVector2_t*)endPos);
	fpVector2_t p4, p5, p6, p7;
	p4.x = 0;
	p4.y = endPos->z;
	p5.x = dist2;
	p5.y = startPos->z;
	p6.x = dist1;
	p6.y = circleCenter->z;
	p7.x = dist1;
	p7.y = circleCenter->z + height;

	fpVector2_t heightIntersection;
	if (calcLineLineIntersection(&heightIntersection, &p4, &p5, &p6, &p7, true)) {
		intersect.z = heightIntersection.y;
		if (isInFront(startPos, &intersect, endPos)) {
			distToIntersection = calculateDistance3(startPos, &intersect);
		}
	}

	fpVector3_t intersectCap;
	fpVector3_t dir = calcDirVectorFromPoints(startPos, endPos);
	if (startPos->z < circleCenter->z || (inside && circleCenter->z != 0)) {
		fpVector3_t p1 = *circleCenter;
		p1.x = circleCenter->x + radius;
		fpVector3_t p2 = *circleCenter;
		p2.y = circleCenter->y + radius;
		fpVector3_t normal = calcPlaneNormalFromPoints(circleCenter, &p1, &p2);

		if (calcIntersectionLinePlane(&intersectCap, &dir, endPos, &normal, circleCenter)
			&& isPointInCircle((fpVector2_t*)&intersectCap, (fpVector2_t*)circleCenter, radius)
			&& isInFront(startPos, &intersectCap, endPos)) {
			float distanceCap = calculateDistance3(startPos, &intersectCap);
			if (distanceCap < distToIntersection) {
				distToIntersection = distanceCap;
				intersect = intersectCap;
			}
		}
	}

	if (startPos->z > circleCenter->z + height || inside) {
		fpVector3_t p1 = { .x = circleCenter->x + radius, .y = circleCenter->y, .z = circleCenter->z + height };
		fpVector3_t p2 = { .x = circleCenter->x, .y = circleCenter->y + radius, .z = circleCenter->z + height };
		fpVector3_t p3 = *circleCenter;
		p3.z = circleCenter->z + height;
		fpVector3_t normal = calcPlaneNormalFromPoints(&p3, &p1, &p2);

		if (calcIntersectionLinePlane(&intersectCap, &dir, startPos, &normal, &p3)
			&& isPointInCircle((fpVector2_t*)&intersectCap, (fpVector2_t*)circleCenter, radius)
			&& isInFront(startPos, &intersectCap, endPos)) {
			float distanceCap = calculateDistance3(startPos, &intersectCap);
			if (distanceCap < distToIntersection) {
				distToIntersection = distanceCap;
				intersect = intersectCap;
			}
		}
	}

	if (distToIntersection < FLT_MAX) {
		*distance = distToIntersection;
		memcpy(intersection, &intersect, sizeof(fpVector3_t));
		return true;
	}
	return false;
}

static bool calcLine3dPolygonIntersection(fpVector3_t *intersection, float *distance, const fpVector3_t *startPos, const fpVector3_t *endPos, fpVector2_t *vertices, const uint8_t numVertices, const float minHeight, const float maxHeight, const bool isInclusiveZone)
{
    float distToIntersection = FLT_MAX;
    fpVector3_t intersect;

    fpVector2_t* prev = &vertices[numVertices - 1];
	fpVector2_t* current;
	for (uint8_t i = 0; i < numVertices; i++) {
		current = &vertices[i];
		
		fpVector3_t p1 = { .x = prev->x, .y = prev->y, .z = minHeight };
		fpVector3_t p2 = { .x = prev->x, .y = prev->y, .z = maxHeight };
		fpVector3_t p3 = { .x = current->x, .y = current->y, .z = minHeight };
		fpVector3_t p4 = { .x = current->x, .y = current->y, .z = maxHeight};
	
		fpVector3_t dir = calcDirVectorFromPoints(startPos, endPos);
		fpVector3_t intersectCurrent;
		if ((calcLineTriangleIntersection(&intersectCurrent, startPos, &dir, &p1, &p2, &p3)
			|| calcLineTriangleIntersection(&intersectCurrent, startPos, &dir, &p3, &p4, &p2)) && isInFront(startPos, &intersectCurrent, endPos) ) {
                float distWall = calculateDistance3(startPos, &intersectCurrent);
                if (distWall < distToIntersection) {
                    distToIntersection = distWall;
					intersect = intersectCurrent;
                }
		}
		prev = current;
	}

    fpVector3_t intersectCap;
    if (startPos->z < minHeight || (isInclusiveZone && minHeight != 0)) {
		if (calcLinePolygonIntersection(&intersectCap, startPos, endPos, minHeight, vertices, numVertices) && isInFront(startPos, &intersectCap, endPos))
		{
			float distanceCap = calculateDistance3(startPos, &intersectCap);
            if (distanceCap < distToIntersection) {
                distToIntersection = distanceCap;
				intersect = intersectCap;
            }
		}
	}

    if (startPos->z > maxHeight || isInclusiveZone) {
		if (calcLinePolygonIntersection(&intersectCap, startPos, endPos, maxHeight, vertices, numVertices) && isInFront(startPos, &intersectCap, endPos))
		{
			float distanceCap = calculateDistance3(startPos, &intersectCap);
            if (distanceCap < distToIntersection) {
                distToIntersection = distanceCap;
				intersect = intersectCap;
            }
		}
	}

    if (distToIntersection < FLT_MAX) {
        *distance = distToIntersection;
        memcpy(intersection, &intersect, sizeof(fpVector3_t));
        return true;
    }
    return false;
}
