//
//  ray.h
//
//  Created by Carl Johan Gribel on 2021-07-29.
//
//

#ifndef tau3d_ray_h
#define tau3d_ray_h

#include "vec.h"
#include "AABB.h"

using namespace linalg;

struct Ray
{
    v3f origin, dir;
    v3f n_near = v3f_000;
    float z_near = std::numeric_limits<float>::max();

    Ray() { }
    
    Ray(const v3f& origin,
        const v3f& dir)
    : origin(origin), dir(dir) { }
    
    inline v3f point_of_contact() const { return origin + dir * z_near; }
    
//    explicit operator bool() const { return body != NULL; }
};

class Ray2d
{
public:
    
    v2f origin, dir;
    v2f n_near = v2f_00;
    float z_near = std::numeric_limits<float>::max();

    Ray2d() { }
    
    Ray2d(const v2f& origin,
          const v2f& dir)
    : origin(origin), dir(dir) { }
    
    inline v2f point_of_contact() const { return origin + dir * z_near; }
    
//    explicit operator bool() const { return body != NULL; }
};


/// Möller-Trumbore ray-triangle intersection
/// \link http://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
///
bool RayTriangleIntersection(Ray& ray,
                             const v3f& v0,
                             const v3f& v1,
                             const v3f& v2);

///  Woo-Terdiman ray-AABB intersection
/// \link http://www.codercorner.com/RayAABB.cpp
///
bool RayAABBIntersection(const Ray& ray,
                         const AABB3d& aabb,
                         float& t_min);

/// Ray intersections
/// Ericson, page 151, 175
///
bool Ray3dPlane3dIntersection(const Ray& ray,
                              const v3f& plane_n,
                              const v3f& plane_p,
                              float& t);

bool Ray3dPlane3dIntersection(Ray& ray,
                              const v3f& plane_n,
                              const v3f& plane_p);

bool Ray2dPlane2dIntersection(const Ray2d& ray,
                              const v2f& plane_n,
                              const v2f& plane_p,
                              float& t);

bool Ray2dPlane2dIntersection(Ray2d& ray,
                              const v2f& plane_n,
                              const v2f& plane_p);

//bool RayAABBIntersection(const Ray& ray,
//                         const AABB3d& aabb,
//                         v3f& coord);

//bool Ray2dSegmentIntersection(const v3f& p0,
//                             const v3f& p1,
//                             Ray& ray);

#endif
