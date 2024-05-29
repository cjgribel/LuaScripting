//
//  AABB.h
//
//  Created by Carl Johan Gribel on 2015-01-20.
//  Updated 2021
//

#ifndef tau3d_aabb_h
#define tau3d_aabb_h

#include "vec.h"
#include "mat.h"
#include "config.h"
#include <float.h>
#include <iostream>
//#define AABB_EPS FLT_EPSILON
#define AABB_EPS 0

using linalg::v2f;
using linalg::v3f;
using linalg::v4f;
using linalg::m3f;
using linalg::m4f;

class AABB2d;

struct AABB3d
{
    v3f min, max;
    
//    union
//    {
//        v3f vmin;
//        float min[3];   // skip and use vmin.vec[]
//    };
//    union
//    {
//        v3f vmax;
//        float max[3];
//    };
    
    AABB3d() { reset(); }
    
    AABB3d(const v3f& min,
           const v3f& max):
    min(min), max(max) { }
    
    AABB3d(float* min,
           float* max)
    {
        std::copy(this->min.vec, this->min.vec+3, min);
        std::copy(this->max.vec, this->max.vec+3, max);
    }
    
//    explicit AABB3d(const AABB2d& aabb2d);
    
    inline void reset()
    {
        min.x = EENG_FINF;
        min.y = EENG_FINF;
        min.z = EENG_FINF;
        max.x = EENG_FNINF;
        max.y = EENG_FNINF;
        max.z = EENG_FNINF;
        
//        min[0] = min[1] = min[2] = XI_FINF;
//        max[0] = max[1] = max[2] = XI_FNINF;
    }

    /// Reset x and y. Grow to infinity along z
//    inline void reset2d()
//    {
//        min[0] = min[1] = max[2] = XI_FINF;
//        max[0] = max[1] = min[2] = XI_FNINF;
//    }
    
    inline void grow(const AABB3d& aabb)
    {
        grow(aabb.min);
        grow(aabb.max);
    }
    
    inline void grow(const v3f &p)
    {
        grow(p.vec);
    }
    
    inline void grow(const float p[3])
    {
        min.x = std::fminf(min.x, p[0]);
        min.y = std::fminf(min.y, p[1]);
        min.z = std::fminf(min.z, p[2]);

        max.x = std::fmaxf(max.x, p[0]);
        max.y = std::fmaxf(max.y, p[1]);
        max.z = std::fmaxf(max.z, p[2]);
        
//        grow_x(p[0]);
//        grow_y(p[1]);
//        grow_z(p[2]);
        
//        for (int i=0; i<3; i++)
//        {
//            min[i] = std::fminf(min[i], p[i]);
//            max[i] = std::fmaxf(max[i], p[i]);
//        }
    }

//    inline void grow_x(const float p)
//    {
//        min[0] = std::fminf(min[0], p);
//        max[0] = std::fmaxf(max[0], p);
//    }
//
//    inline void grow_y(const float p)
//    {
//        min[1] = std::fminf(min[1], p);
//        max[1] = std::fmaxf(max[1], p);
//    }
//
//    inline void grow_z(const float p)
//    {
//        min[2] = std::fminf(min[2], p);
//        max[2] = std::fmaxf(max[2], p);
//    }
    
    inline v3f get_midpoint() const
    {
        return (min + max)*0.5f;
    }
    
    inline v4f get_boundingsphere() const
    {
        v3f mid = get_midpoint();
//        v4f bs;
//        bs.x = (min.x + max.x) * 0.5f;
//        bs.y = (min.y + max.y) * 0.5f;
//        bs.z = (min.z + max.z) * 0.5f;
        //for(int i=0; i<3; i++) bs.vec[i] = (min[i] + max[i])/2;
        
        v3f rv = linalg::max(mid - min, max - mid);
//        vmax.x = fmaxf(mid.x - min.x, max.x - mid.x);
//        vmax.y = fmaxf(mid.y - min.y, max.y - mid.y);
//        vmax.z = fmaxf(mid.z - min.z, max.z - mid.z);
//        for(int i=0; i<3; i++)
//            vmax.vec[i] = fmaxf(bs.vec[i]-min[i], max[i]-bs.vec[i]);
//        bs.w = vmax.norm2();
        
        return v4f { mid, rv.norm2() };
    }
    
    inline AABB3d operator+ (const v3f& v)
    {
        AABB3d aabb = *this;
        aabb.min +=  v;
        aabb.max +=  v;
        
        return aabb;
    }
    
    //
    // AABB resulting from this AABB being rotated and translated.
    //
    // Adopted for column-order from
    // Ericsson, Real-time Collision Detection, page 86.
    // -However had to change index order (i<->j) in the matrix element extraction,
    // perhaps because Ericson assumes row-odered matrix elements in his code.
    // With this change the "axes" in the outer loop correspond to columns of
    // the rotation matrix, as intended per the decription in the book.
    //
    
    inline AABB3d post_transform(const m4f& M) const
    {
        //        return post_transform(extract_translation(M), M.get_3x3());
        AABB3d aabb;
        
        // For all three axes
        for (int i = 0; i < 3; i++) {
            // Start by adding in translation
            aabb.min.vec[i] = aabb.max.vec[i] = M.mat[3][i];
            // Form extent by summing smaller and larger terms respectively
            for (int j = 0; j < 3; j++) {
                float e = M.mat[j][i] * min.vec[j];
                float f = M.mat[j][i] * max.vec[j];
                if (e < f) {
                    aabb.min.vec[i] += e;
                    aabb.max.vec[i] += f;
                } else {
                    aabb.min.vec[i] += f;
                    aabb.max.vec[i] += e;
                }
            }
        }
        return aabb;
    }
    
//    inline AABB_t post_transform(const v3f& T, const m3f& R) const
//    {
//        AABB_t aabb;
//
//        // For all three axes
//        for (int i = 0; i < 3; i++) {
//            // Start by adding in translation
//            aabb.min.vec[i] = aabb.max.vec[i] = T.vec[i];
//            // Form extent by summing smaller and larger terms respectively
//            for (int j = 0; j < 3; j++) {
//                float e = R.mat[j][i] * min.vec[j];
//                float f = R.mat[j][i] * max.vec[j];
//                //            float e = R.mat[i][j] * aabb.min[j];
//                //            float f = R.mat[i][j] * aabb.max[j];
//                if (e < f) {
//                    aabb.min.vec[i] += e;
//                    aabb.max.vec[i] += f;
//                } else {
//                    aabb.min.vec[i] += f;
//                    aabb.max.vec[i] += e;
//                }
//            }
//        }
//        return aabb;
//    }
    
    operator bool ()
    {
        return max.x >= min.x && max.y >= min.y && max.z >= min.z;
    }
    
    // Can it be assumed that M can be factorized into R and T this way?
    //    inline AABB_t post_transform(const linalg::mat4f& M)
    //    {
    //        return post_transform_AABB(M.col[3].xyz(), M.get_3x3());
    //    }
        
    inline bool intersect(const AABB3d &aabb) const
    {
        if (max.x < aabb.min.x - AABB_EPS || min.x > aabb.max.x + AABB_EPS) return false;
        if (max.y < aabb.min.y - AABB_EPS || min.y > aabb.max.y + AABB_EPS) return false;
        if (max.z < aabb.min.z - AABB_EPS || min.z > aabb.max.z + AABB_EPS) return false;
        return true;
    }
    
    inline AABB3d intersection(const AABB3d &aabb) const
    {
        return AABB3d {
            linalg::max(min, aabb.min),
            linalg::min(max, aabb.max)
//            v3f {
//                fmaxf(min.x, aabb.min.x),
//                fmaxf(min.y, aabb.min.y),
//                fmaxf(min.z, aabb.min.z)
//            },
//            v3f {
//                fminf(min.x, aabb.min.x),
//                fminf(min.y, aabb.min.y),
//                fminf(min.z, aabb.min.z)
//            }
        };
    }
    
    //    bool intersect_sphere(const linalg::v3f& p, float r)
    //    {
    //        return true;
    //    }
    
    // see RayAABB.cpp by Terdiman
    //
    //    bool intersect(const ray_t& ray) const
    //    {
    //        return false;
    //    }
    
private:
    
    //
    // Split AABB at frac ∈ [0,1] in the dim:th dimension
    //
    void split2(int dim, float frac, AABB3d& aabb_left, AABB3d& aabb_right)
    {
        aabb_left = *this;
        aabb_right = *this;
        float halfx = min.vec[dim] + (max.vec[dim]-min.vec[dim]) * frac;
        
        aabb_left.max.vec[dim] = halfx;
        aabb_right.min.vec[dim] = halfx;
    }
    
public:
    
    //
    // Split AABB into four in the xz-plane
    //
    void split4_xz(AABB3d aabb[4])
    {
        AABB3d l, r;
        split2(0, 0.5f, l, r);               // split in x
        l.split2(2, 0.5f,aabb[0], aabb[1]);  // split in z
        r.split2(2, 0.5f, aabb[2], aabb[3]); // split in z
    }
};

struct AABB2d
{
    v2f min, max;
    
    AABB2d(const v2f& min,
           const v2f& max):
    min(min), max(max)
    { }
    
    AABB2d(const AABB3d& aabb3d):
    min(aabb3d.min.xy()), max(aabb3d.max.xy())
    { }
    
//    AABB3d(float* min, float* max)
//    {
//        std::copy(this->min.vec, this->min.vec+3, min);
//        std::copy(this->max.vec, this->max.vec+3, max);
//    }
    
    AABB2d()
    {
        reset();
    }
    
    inline void reset()
    {
        min.x = EENG_FINF;
        min.y = EENG_FINF;
        max.x = EENG_FNINF;
        max.y = EENG_FNINF;
    }
    
    inline void grow(const AABB2d& aabb)
    {
        grow(aabb.min);
        grow(aabb.max);
    }
    
    inline void grow(const v2f &p)
    {
        grow(p.vec);
    }
    
    inline void grow(const float p[2])
    {
        min.x = std::fminf(min.x, p[0]);
        min.y = std::fminf(min.y, p[1]);

        max.x = std::fmaxf(max.x, p[0]);
        max.y = std::fmaxf(max.y, p[1]);
    }
    
    inline v2f get_midpoint() const
    {
        return (min + max)*0.5f;
    }
    
    inline AABB2d post_transform(const m3f& M) const
    {
        //        return post_transform(extract_translation(M), M.get_3x3());
        AABB2d aabb;
        
        // For all three axes
        for (int i = 0; i < 2; i++) {
            // Start by adding in translation
            aabb.min.vec[i] = aabb.max.vec[i] = M.mat[2][i];
            // Form extent by summing smaller and larger terms respectively
            for (int j = 0; j < 2; j++) {
                float e = M.mat[j][i] * min.vec[j];
                float f = M.mat[j][i] * max.vec[j];
                if (e < f) {
                    aabb.min.vec[i] += e;
                    aabb.max.vec[i] += f;
                } else {
                    aabb.min.vec[i] += f;
                    aabb.max.vec[i] += e;
                }
            }
        }
        return aabb;
    }
    
    operator bool ()
    {
        return max.x >= min.x && max.y >= min.y;
    }
    
    // Can it be assumed that M can be factorized into R and T this way?
    //    inline AABB_t post_transform(const linalg::mat4f& M)
    //    {
    //        return post_transform_AABB(M.col[3].xyz(), M.get_3x3());
    //    }
        
    inline bool intersect(const AABB2d &aabb) const
    {
        if (max.x < aabb.min.x - AABB_EPS || min.x > aabb.max.x + AABB_EPS) return false;
        if (max.y < aabb.min.y - AABB_EPS || min.y > aabb.max.y + AABB_EPS) return false;
        return true;
    }
    
    inline AABB2d intersection(const AABB2d &aabb) const
    {
        return AABB2d {
            linalg::max(min, aabb.min),
            linalg::min(max, aabb.max)
        };
    }
};

// TODO: FIXME

inline AABB3d toAABB3d(const AABB2d& aabb2d)
{
    return AABB3d { xy0(aabb2d.min), xy0(aabb2d.max) };
}

#endif
