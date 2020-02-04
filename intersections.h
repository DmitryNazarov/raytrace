#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <transform.h>
#include <primitives.h>
#include <algorithm>


bool intersection_sphere(const Sphere& s, const Ray& r, float& dist);
bool intersection_triangle_BROKEN(const Triangle& tri, const Ray& r, float& dist);
bool intersection_triangle(const Triangle& tri, const Ray& r, float& dist);
bool intersection_triangle(const TriangleNormals& tri, const Ray& r, float& dist);

#endif INTERSECTION_H
