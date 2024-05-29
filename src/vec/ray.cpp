//
//  ray.cpp
//  xiengine
//
//  Created by Carl Johan Gribel on 2021-08-14.
//  Copyright © 2021 Carl Johan Gribel. All rights reserved.
//

#include <stdio.h>
#include "config.h"
#include "ray.h"

/**
 Möller-Trumbore ray-triangle intersection
 \link http://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
*/
#define RayTriEps 0.000001
bool RayTriangleIntersection(Ray& ray,
                             const v3f& v0,
                             const v3f& v1,
                             const v3f& v2)
{
    v3f e1 = v1-v0;
    v3f e2 = v2-v0;
    
    v3f P = ray.dir % e2;
    float det = e1.dot(P);
    if (fabs(det) < RayTriEps) return false;
    
    float inv_det = 1.0f/det;
    v3f T = ray.origin - v0;
    float u = T.dot(P) * inv_det;
    if (u < 0.0f || u > 1.0f) return false;
    
    v3f Q = T % e1;
    float v = ray.dir.dot(Q) * inv_det;
    if (v < 0.0f || u+v > 1.0f) return false;
    
    float t = e2.dot(Q) * inv_det;
    if (t > RayTriEps && t < ray.z_near)
    {
        ray.z_near = t;
        ray.n_near = normalize(cross(v1 - v0, v2 - v0));
        return true;
    }
    return false;
}


// Integer representation of a floating-point value.
#define IR(x)    ((udword&)x)
#define RayAABBEps 0.00001f
typedef unsigned int udword;

bool RayAABBIntersection(const Ray& ray,
                         const AABB3d& aabb,
                         float& t_min)
{
    const v3f& origin = ray.origin;
    const v3f& dir = ray.dir;
    v3f coord;
    
    bool Inside = true;
    const v3f MinB = aabb.min; // aabb.mCenter - aabb.mExtents;
    const v3f MaxB = aabb.max; // aabb.mCenter + aabb.mExtents;
    v3f MaxT { -1.0f, -1.0f, -1.0f};

//    Point MinB = aabb.mCenter - aabb.mExtents;
//    Point MaxB = aabb.mCenter + aabb.mExtents;
//    Point MaxT;
//    MaxT.x=MaxT.y=MaxT.z=-1.0f;
    
    // Find candidate planes.
    for(udword i=0;i<3;i++)
    {
        if(origin.vec[i] < MinB.vec[i])
        {
            coord.vec[i] = MinB.vec[i];
            Inside = false;
            
            // Calculate T distances to candidate planes
            if(IR(dir.vec[i]))
                MaxT.vec[i] = (MinB.vec[i] - origin.vec[i]) / dir.vec[i];
        }
        else if(origin.vec[i] > MaxB.vec[i])
        {
            coord.vec[i] = MaxB.vec[i];
            Inside = false;
            
            // Calculate T distances to candidate planes
            if(IR(dir.vec[i]))
                MaxT.vec[i] = (MaxB.vec[i] - origin.vec[i]) / dir.vec[i];
        }
    }
    
    // Ray origin inside bounding box
    if(Inside)
    {
        coord = origin;
        return true;
    }
    
    // Get largest of the maxT's for final choice of intersection
    udword WhichPlane = 0;
    if(MaxT.vec[1] > MaxT.vec[WhichPlane])    WhichPlane = 1;
    if(MaxT.vec[2] > MaxT.vec[WhichPlane])    WhichPlane = 2;
    
    // Check final candidate actually inside box (MaxT is > 0)
    if(IR(MaxT.vec[WhichPlane]) & 0x80000000) return false;
    
    for(udword i = 0; i < 3; i++)
    {
        if(i != WhichPlane)
        {
            coord.vec[i] = origin.vec[i] + MaxT.vec[WhichPlane] * dir.vec[i];
#ifdef RayAABBEps
            if(coord.vec[i] < MinB.vec[i] - RayAABBEps || coord.vec[i] > MaxB.vec[i] + RayAABBEps)    return false;
#else
            if(coord.vec[i] < MinB.vec[i] || coord.vec[i] > MaxB.vec[i])    return false;
#endif
        }
    }
    
    t_min = MaxT.vec[WhichPlane];
    return true;    // ray hits box
}

bool Ray3dPlane3dIntersection(const Ray& ray,
                              const v3f& plane_n,
                              const v3f& plane_p,
                              float& t)
{
    float denom = dot(plane_n, ray.dir);
    if (fabsf(denom) > EENG_FEPSILON)
    {
        t = dot(plane_n, plane_p - ray.origin) / denom;
        return true;
    }
    return false;
};

bool Ray3dPlane3dIntersection(Ray& ray,
                              const v3f& plane_n,
                              const v3f& plane_p)
{
    float t;
    if (Ray3dPlane3dIntersection(ray, plane_n, plane_p, t))
        if (t > 0.0f && t < ray.z_near)
        {
            ray.z_near = t;
            ray.n_near = plane_n;
            return true;
        }
    return false;
}

bool Ray2dPlane2dIntersection(const Ray2d& ray,
                              const v2f& plane_n,
                              const v2f& plane_p,
                              float& t)
{
    float denom = dot(plane_n, ray.dir);
    if (fabsf(denom) > EENG_FEPSILON)
    {
        t = dot(plane_n, plane_p - ray.origin) / denom;
        return true;
    }
    return false;
};

bool Ray2dPlane2dIntersection(Ray2d& ray,
                              const v2f& plane_n,
                              const v2f& plane_p)
{
    float t;
    if (Ray2dPlane2dIntersection(ray, plane_n, plane_p, t))
        if (t > 0.0f && t < ray.z_near)
        {
            ray.z_near = t;
            ray.n_near = plane_n;
            return true;
        }
    return false;
}
