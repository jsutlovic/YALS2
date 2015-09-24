#ifndef GEOM_H
#define GEOM_H

#include "linmath.h"

typedef struct {
    vec3 point;
    vec3 vector;
} Ray;

typedef struct {
    vec3 point;
    vec3 normal;
} Plane;

static inline void ray_intersection_point(vec3 *result, Ray ray, Plane plane) {
    vec3 ray_to_plane_vector;
    vec3_sub(ray_to_plane_vector, plane.point, ray.point);

    float scale_factor = vec3_mul_inner(ray_to_plane_vector, plane.normal) /
        vec3_mul_inner(ray.vector, plane.normal);

    vec3 intersection_point;
    vec3 scaled_ray_vector;
    vec3_scale(scaled_ray_vector, ray.vector, scale_factor);
    vec3_add(intersection_point, ray.point, scaled_ray_vector);
    memcpy(result, intersection_point, sizeof(intersection_point));
}

static inline void divide_by_w(vec4 vec) {
    vec[0] /= vec[3];
    vec[1] /= vec[3];
    vec[2] /= vec[3];
}

#endif
