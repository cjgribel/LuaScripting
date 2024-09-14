
/*
 * 2D & 3D vector
 * v 1.0
 *
 * Carl Johan Gribel (c) 2011, cjgribel@gmail.com
 *
 */

#pragma once
#ifndef VEC_H
#define VEC_H

#include <cmath>
#include <cstdio>
#include <ostream>
#include "quat.h"

namespace linalg {

template<class T>
class vec3;

template<class T> class vec2
{
public:
    union
    {
        T vec[2];
        struct { T x, y; };
    };
    
    vec2()
    {
        x = 0.0f;
        y = 0.0f;
    }
    
    vec2(const T& x, const T& y)
    {
        this->x = x;
        this->y = y;
    }
    
    explicit vec2(const vec3<T>& v3);
    
    void set(const T &x, const T &y)
    {
        this->x = x;
        this->y = y;
    }
    
    float dot(const vec2<T> &u) const
    {
        return x*u.x + y*u.y;
    }
    
    /**
     * 2-norm: |u| = sqrt(u.u)
     */
    float norm2() const
    {
        return sqrt(x*x + y*y);
    }
    
    /**
     * normalization: u/|u| = u/(u.u)
     */
    vec2<T>& normalize()
    {
        T normSquared = x*x + y*y;
        
        if( normSquared < 1e-8 )
            set(0.0, 0.0);
        else
        {
            T inormSquared = 1.0 / sqrt(normSquared);
            set(x * inormSquared, y * inormSquared);
        }
        return *this;
    }
    
    /**
     * project on v: v * u.v/v.v
     */
    vec2<T> project(vec2<T> &v) const
    {
        T vnormSquared = v.x*v.x + v.y*v.y;
        return v * (this->dot(v) / vnormSquared);
    }
    
    /**
     * angle to vector
     */
    float angle(vec2<T> &v)
    {
        vec2<T>	un = vec2f(*this).normalize(),
        vn = vec2f(v).normalize();
        return acos( un.dot(vn) );
    }
    
    //    vec2<T>& operator =(const vec2<T> &v)
    //    {
    //        x = v.x;
    //        y = v.y;
    //        return *this;
    //    }
    
    vec2<T>& operator +=(const vec2<T> &v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }
    
    vec2<T>& operator -=(const vec2<T> &v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }
    
    vec2<T>& operator *=(const T &s)
    {
        x *= s;
        y *= s;
        return *this;
    }
    
    vec2<T>& operator *=(const vec2<T> &v)
    {
        x *= v.x;
        y *= v.y;
        return *this;
    }
    
    vec2<T>& operator /=(const T &v)
    {
        x /= v;
        y /= v;
        return *this;
    }
    
    vec2<T> operator -() const
    {
        return vec2<T>(-x, -y);
    }
    
    vec2<T> operator *(const T &s) const
    {
        return vec2<T>(x * s, y * s);
    }
    
    vec2<T> operator *(const vec2<T> &v) const
    {
        return vec2<T>(x * v.x, y * v.y);
    }
    
    vec2<T> operator /(const T &v) const
    {
        T iv = 1.0 / v;
        return vec2(x * iv, y * iv);
    }
    
    vec2<T> operator +(const vec2<T> &v) const
    {
        return vec2<T>(x + v.x, y + v.y);
    }
    
    vec2<T> operator -(const vec2<T> &v) const
    {
        return vec2<T>(x - v.x, y - v.y);
    }
    
    //    T operator %(const vec2<T> &v) const
    //    {
    //        return x * v.y - y * v.x;
    //    }
    
};

template<class T>
inline vec2<T> normalize(const vec2<T>& u)
{
    T norm2 = u.x*u.x + u.y*u.y;
    
    if( norm2 < 1.0e-8 )
        return vec2<T>(0.0, 0.0);
    else
        return u * (1.0/sqrt(norm2));
}

template<class T>
inline T dot(const vec2<T>& u, const vec2<T>& v)
{
    return u.x * v.x + u.y * v.y;
}

// Interpretation: (0, 0, s) x (v.x, v.y, 0)
template<class T>
inline vec2<T> cross(T s, const vec2<T>& v)
{
    return vec2<T>(-s * v.y, s * v.x);
}

// Interpretation: z-component of (u.x, u.y, 1) x (v.x, v.y, 1)
template<class T>
inline T cross(const vec2<T>& u, const vec2<T>& v)
{
    return u.x * v.y - u.y * v.x;
}

template<class T>
inline std::ostream& operator<< (std::ostream &out, const vec2<T> &v)
{
    return out << "(" << v.x << ", " << v.y << ")";
}

template<class T> class vec4;
template<class T> class mat3;

/**
 * 3d-vector
 */
template<class T> class vec3
{
public:
    union
    {
        T vec[3];
        struct { T x, y, z; };
    };
    
    vec3()
    {
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }
    
    vec3(const T &x, const T &y, const T &z)
    : x(x), y(y), z(z)
    {
    }
    
    explicit vec3(const vec2<T>& v2)
    : x(v2.x), y(v2.y), z(0)
    {
    }
    
    vec2<T> xy() const
    {
        return vec2<T>(x, y);
    }
    
    vec4<T> xyz0() const;
    
    vec4<T> xyz1() const;
    
    void set(const T &x, const T &y, const T &z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    
    T dot(const vec3<T> &u) const
    {
        return x*u.x + y*u.y + z*u.z;
    }
    
    /**
     * 2-norm: |u| = sqrt(u.u)
     */
    T norm2() const
    {
        return sqrt(x*x + y*y + z*z);
    }
    
    T norm2squared() const
    {
        return x*x + y*y + z*z;
    }
    
    /**
     * normalization: u/|u| = u/(u.u)
     */
    vec3<T>& normalize()
    {
        T normSquared = x*x + y*y + z*z;
        
        if( normSquared < 1e-8 )
            set(0.0, 0.0, 0.0);
        else
        {
            float inormSquared = 1.0 / sqrt(normSquared);
            set(x*inormSquared, y*inormSquared, z*inormSquared);
        }
        return *this;
    }
    
    /**
     * project on v: v * u.v/v.v
     */
    vec3<T> project(const vec3<T> &v) const
    {
        T vnormSquared = v.x*v.x + v.y*v.y + v.z*v.z;
        return v * (this->dot(v) / vnormSquared);
    }
    
    /**
     * angle to vector
     */
    T angle(const vec3<T> &v) const
    {
        vec3<T>	un = vec3<T>(*this).normalize(),
        vn = vec3<T>(v).normalize();
        return acos( un.dot(vn) );
    }
    
    //    static angle_plane(const vec3<T> &u, const vec3<T> &v, const vec3<T> &axis)
    //    {
    //        // static public float CalcSignedCentralAngle(Vector3 dir1, Vector3 dir2, Vector3 normal)
    //        //=> Mathf.Atan2(Vector3.Dot(Vector3.Cross(dir1, dir2), normal), Vector3.Dot(dir1, dir2));
    //    }
    
    //	inline vec3<T>& operator =(const vec3<T> &v)
    //	{
    //		x = v.x;
    //		y = v.y;
    //		z = v.z;
    //		return *this;
    //	}
    
    vec3<T>& operator +=(const vec3<T> &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    
    vec3<T>& operator -=(const vec3<T> &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    
    vec3<T>& operator *=(const T &s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    
    vec3<T>& operator *=(const vec3<T> &v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    
    vec3<T>& operator /=(const T &v)
    {
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }
    
    vec3<T> operator -() const
    {
        return vec3<T>(-x, -y, -z);
    }
    
    vec3<T> operator *(const T& s) const
    {
        return vec3(x*s, y*s, z*s);
    }
    
    vec3<T> operator *(const vec3<T>& v) const
    {
        return vec3<T>(x*v.x, y*v.y, z*v.z);
    }
    
    vec3<T> operator /(const T& s) const
    {
        T is = 1.0 / s;
        return vec3<T>(x*is, y*is, z*is);
    }
    
    vec3<T> operator +(const vec3<T>& v) const
    {
        return vec3<T>(x+v.x, y+v.y, z+v.z);
    }
    
    vec3<T> operator -(const vec3<T>& v) const
    {
        return vec3<T>(x-v.x, y-v.y, z-v.z);
    }
    
    vec3<T> operator %(const vec3<T>& v) const
    {
        return vec3<T>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
    }
    
    vec3<T> operator *(const mat3<T>& m) const;
    
    bool operator == (const vec3<T>& rhs) const;
    //        {
    //            return x == rhs.x && y == rhs.y && z == rhs.z;
    //        }
    
    mat3<T> outer_product(const vec3<T>& v) const;
    
    void debugPrint() const
    {
        printf("(%f,%f,%f)\n", x, y, z);
    }
    
};

template<class T>
inline vec3<T> cross(const vec3<T>& u, const vec3<T>& v)
{
    return vec3<T>(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
}

template<class T>
inline std::ostream& operator<< (std::ostream &out, const vec3<T> &v)
{
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

/**
 * 4d-vector
 */
template<class T> class vec4
{
public:
    union
    {
        T vec[4];
        struct { T x, y, z, w; };
    };
    
    vec4()
    {
        x = y = z = w = 0;
    }
    
    vec4(const T &x, const T &y, const T &z, const T &w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    
    vec4(const vec3<T> &v, const T &w)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->w = w;
    }
    
    void set(const T &x, const T &y, const T &z, const T &w){
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    
    vec2<T> xy() const
    {
        return vec2<T>(x, y);
    }
    
    vec3<T> xyz() const
    {
        return vec3<T>(x, y, z);
    }
    
#if 0
    vec4<T>& normalize()
    {
        float normSquared = x*x + y*y + z*z + w*w;
        
        if( normSquared < 1e-8 )
            set(0.0, 0.0, 0.0, 0.0);
        else
        {
            float inormSquared = 1.0 / sqrt(normSquared);
            set(x*inormSquared, y*inormSquared, z*inormSquared, w*inormSquared);
        }
        return *this;
    }
#endif
    
    T dot(const vec4<T> &u) const
    {
        return x*u.x + y*u.y + z*u.z + w*u.w;
    }
    
    vec4<T> operator +(const vec4<T> &v) const
    {
        return vec4<T>(x+v.x, y+v.y, z+v.z, w+v.w);
    }
    
    vec4<T>& operator += (const vec4<T>& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        
        return *this;
    }
    
    vec4<T> operator -(const vec4<T> &v) const
    {
        return vec4<T>(x-v.x, y-v.y, z-v.z, w-v.w);
    }
    
    vec4<T> operator *(const T &s) const
    {
        return vec4<T>(x*s, y*s, z*s, w*s);
    }
    
    vec4<T> operator *(const vec4<T> &v) const
    {
        return vec4<T>(x*v.x, y*v.y, z*v.z, w*v.w);
    }
    
    bool operator == (const vec4<T>& rhs) const;
};

// --- 5d-vector ---------------------------------------------------------------

/**
 * 5d-vector
 */
template<class T> class vec5
{
public:
    union
    {
        T vec[5];
        struct { T x, y, z, w, q; };
    };
};

// --- Specializations ---------------------------------------------------------

template<>
inline bool vec4<unsigned>::operator == (const vec4<unsigned>& rhs) const
{
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

// --- Aux ---------------------------------------------------------------------

template<class T>
inline T dot(const vec3<T>& u, const vec3<T>& v)
{
    return u.x*v.x + u.y*v.y + u.z*v.z;
}

template<class T>
inline T dot(const vec4<T>& u, const vec4<T>& v)
{
    return u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w;
}

inline vec2<float> min(const vec2<float>& u, const vec2<float>& v)
{
    return vec2<float>
    {
        std::fminf(u.x, v.x),
        std::fminf(u.y, v.y)
    };
}

inline vec3<float> min(const vec3<float>& u, const vec3<float>& v)
{
    return vec3<float>
    {
        std::fminf(u.x, v.x),
        std::fminf(u.y, v.y),
        std::fminf(u.z, v.z)
    };
}

inline vec2<float> max(const vec2<float>& u, const vec2<float>& v)
{
    return vec2<float>
    {
        std::fmaxf(u.x, v.x),
        std::fmaxf(u.y, v.y)
    };
}

inline vec3<float> max(const vec3<float>& u, const vec3<float>& v)
{
    return vec3<float>
    {
        std::fmaxf(u.x, v.x),
        std::fmaxf(u.y, v.y),
        std::fmaxf(u.z, v.z)
    };
}

template<class T>
inline T length_squared(const vec2<T>& u)
{
    return u.x*u.x + u.y*u.y;
}

template<class T>
inline T length_squared(const vec3<T>& u)
{
    return u.x*u.x + u.y*u.y + u.z*u.z;
}

template<class T>
inline T length(const vec2<T>& u)
{
    return sqrt(length_squared(u));
}

template<class T>
inline T length(const vec3<T>& u)
{
    return sqrt(length_squared(u));
}

template<class T>
inline vec3<T> normalize(const vec3<T>& u)
{
    T norm2 = u.x*u.x + u.y*u.y + u.z*u.z;
    
    if( norm2 < 1.0e-8 )
        return vec3<T>(0.0, 0.0, 0.0);
    else
        return u * (1.0/sqrt(norm2));
}

template<class T>
inline vec4<T> normalize(const vec4<T>& u)
{
    T norm2 = u.x*u.x + u.y*u.y + u.z*u.z + u.w*u.w;
    
    if( norm2 < 1.0e-8 )
        return vec4<T>(0.0, 0.0, 0.0, 0.0);
    else
        return u * (1.0/sqrt(norm2));
}

template<class SrcVecType, class DestVecType>
DestVecType as(const SrcVecType& v)
{
    if constexpr (std::is_same<SrcVecType, vec2<float>>::value)
    {
        if constexpr (std::is_same<DestVecType, vec3<float>>::value)
            return vec3<float>(v.x, v.y, 0.0f);
    }
    else if constexpr (std::is_same<SrcVecType, vec3<float>>::value)
    {
        if constexpr (std::is_same<DestVecType, vec2<float>>::value)
            return v.xy();
    }
    return v;
}

template<class T>
inline std::ostream& operator << (std::ostream &out, const vec4<T> &v)
{
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

// --- Swizzles ----------------------------------------------------------------

template<class T> inline auto xy(const vec3<T>& v)      { return vec2<T> {v.x, v.y}; }
template<class T> inline auto xy(const vec4<T>& v)      { return vec2<T> {v.x, v.y}; }
template<class T> inline auto x00(const T& v)           { return vec3<T> {v, 0, 0}; }
template<class T> inline auto xy0(const vec2<T>& v)     { return vec3<T> {v.x, v.y, 0}; }
template<class T> inline auto xy0(const vec3<T>& v)     { return vec3<T> {v.x, v.y, 0}; }
template<class T> inline auto xy1(const vec2<T>& v)     { return vec3<T> {v.x, v.y, 1}; }
template<class T> inline auto xy1(const vec3<T>& v)     { return vec3<T> {v.x, v.y, 1}; }
template<class T> inline auto xyz(const vec4<T>& v)     { return vec3<T> {v.x, v.y, v.z}; }
template<class T> inline auto xyz0(const vec3<T>& v)    { return vec4<T> {v.x, v.y, v.z, 0}; }
template<class T> inline auto xyz1(const vec3<T>& v)    { return vec4<T> {v.x, v.y, v.z, 1}; }
template<class T> inline auto xy00(const vec2<T>& v)    { return vec4<T> {v.x, v.y, 0, 0}; }
template<class T> inline auto xy01(const vec2<T>& v)    { return vec4<T> {v.x, v.y, 0, 1}; }

// --- typedefs ----------------------------------------------------------------

typedef vec2<float> float2;
typedef vec3<float> float3;
typedef vec4<float> float4;
typedef vec5<float> float5;

typedef vec2<float> vec2f;
typedef vec3<float> vec3f;
typedef vec4<float> vec4f;
typedef vec5<float> vec5f;

typedef vec2<float> v2f;
typedef vec3<float> v3f;
typedef vec4<float> v4f;
typedef vec5<float> v5f;

typedef vec2<uint32_t> v2u;
typedef vec3<uint32_t> v3u;
typedef vec4<uint32_t> v4u;
typedef vec5<uint32_t> v5u;

typedef vec2<unsigned char> v2uc;
typedef vec3<unsigned char> v3uc;
typedef vec4<unsigned char> v4uc;
typedef vec5<unsigned char> v5uc;

typedef vec2<int32_t> int2;
typedef vec3<int32_t> int3;
typedef vec4<int32_t> int4;
typedef vec5<int32_t> int5;

typedef vec2<int32_t> vec2i;
typedef vec3<int32_t> vec3i;
typedef vec4<int32_t> vec4i;
typedef vec5<int32_t> vec5i;

typedef vec2<int32_t> v2i;
typedef vec3<int32_t> v3i;
typedef vec4<int32_t> v4i;
typedef vec4<uint32_t> v4ui;
typedef vec5<int32_t> v5i;

//typedef vec2<long> long2;
//typedef vec3<long> long3;
//typedef vec4<long> long4;

//typedef vec2<unsigned> unsigned2;
//typedef vec3<unsigned> unsigned3;
//typedef vec4<unsigned> unsigned4;
typedef vec2<uint32_t> vec2ui;
typedef vec3<uint32_t> vec3ui;
typedef vec4<uint32_t> vec4ui;


const v2i v2i_11 {1, 1};
const v2i v2i_00 {0, 0};
const v2i v2i_10 {1, 0};
const v2i v2i_01 {0, 1};

const v2f v2f_11 {1.0f, 1.0f};
const v2f v2f_00 {0.0f, 0.0f};
const v2f v2f_10 {1.0f, 0.0f};
const v2f v2f_01 {0.0f, 1.0f};

const v3f v3f_111 {1.0f, 1.0f, 1.0f};
const v3f v3f_000 {0.0f, 0.0f, 0.0f};
const v3f v3f_100 {1.0f, 0.0f, 0.0f};
const v3f v3f_010 {0.0f, 1.0f, 0.0f};
const v3f v3f_001 {0.0f, 0.0f, 1.0f};
const v3f v3f_110 {1.0f, 1.0f, 0.0f};
const v3f v3f_011 {0.0f, 1.0f, 1.0f};
const v3f v3f_101 {1.0f, 0.0f, 1.0f};

const v4f v4f_0000 {0.0f, 0.0f, 0.0f, 0.0f};

template<class T, int N> class vec
{
public:
    T vec[N];
};

typedef vec<float, 6> float6;

}

#endif /* VEC_H */
