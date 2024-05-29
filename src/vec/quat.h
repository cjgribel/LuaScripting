
/*
 * Tau3D Dynamics
 * Carl Johan Gribel (c) 2011, cjgribel@gmail.com
 *
 */

#pragma once
#ifndef QUAT_H
#define QUAT_H

//#include <cstdio>
#include <ostream>
#include <cmath>
#include <cassert>

namespace linalg {

template <class T> class vec3;
template <class T> class mat3;
template <class T> class mat4;

//
// ------------------------------ Quternion --------------------------------
//

template<class T> class quat
{
public:
    T qw;				// Angular (real) component
    T qx, qy, qz;		// Axis (imaginary) components
    
    //
    // --------------------------- Constructors ----------------------------
    //
    
    //
    // Copy
    //
    quat(const quat<T>& q)
    : qw(q.qw), qx(q.qx), qy(q.qy), qz(q.qz)
    { }
    
    //
    // From elements
    //
    quat(const T &qw, const T &qx, const T &qy, const T &qz)
    : qw(qw), qx(qx), qy(qy), qz(qz)
    { }
    
    //
    // Identity
    //
    quat()
    {
        qw = 1.0;
        qx = qy = qz = 0.0;
    }
    
    //
    // Rotation quaternion from roation matrix
    //
    quat(const mat3<T> &R);
    
    //
    // ----------------------------- Algebra -------------------------------
    //
    
    inline T dot(const quat<T>& q) const
    {
        return qw*q.qw + qx*q.qx + qy*q.qy + qz*q.qz;
    }
    
    inline T length() const
    {
        return sqrt(qw*qw + qx*qx + qy*qy + qz*qz);
    }
    
    inline void normalize()
    {
        T f = 1.0/sqrt(qw*qw + qx*qx + qy*qy + qz*qz);
        qw *= f;
        qx *= f;
        qy *= f;
        qz *= f;
    }
    
    //
    // Euler angle & axis from rotation quaternion
    //
    // http://en.wikipedia.org/wiki/Rotation_representation#Euler_axis.2Fangle_.E2.86.94_quaternion
    //
    T getEulerAngle() const;
    
    vec3<T> getEulerAxis(const T &theta) const;
    
    //
    // Time-derivative dQ/dt of Q (this instance)
    // and a provided velocity vector w
    //
    // dQ/dt = 0.5 * [0, w]' * Q (quaternion mult)
    //
    // http://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Quaternion_.E2.86.94_angular_velocities
    //
    quat<T> getQdot(const vec3<T> &w) const;
    
    //
    // (...the other way around)
    // Velocity vector w of time-derivative dQ/dt (this instance)
    // and a provided Q
    //
    // w = 2 * qQ/dt * Q^-1
    //
    // http://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Quaternion_.E2.86.94_angular_velocities
    //
    vec3<T> get_W_from_Qdot(const quat<T> &Q) const;
    
    //
    // Conjugate
    //
    // http://se.mathworks.com/help/aeroblks/quaternionconjugate.html
    //
    quat<T> conjugate() const
    {
        return quat<T>(qw, -qx, -qy, -qz);
    }
    
    //
    // Inverse
    //
    // http://se.mathworks.com/help/aeroblks/quaternioninverse.html
    //
    quat<T> inverse() const
    {
        return conjugate() * (1.0/(qw*qw + qx*qx + qy*qy + qz*qz));
    }
    
    //
    // Decompose/factorize rotation quaternion wrt to a vector u into
    // twist: rotation around u
    // swing: rotation around vector perpendicular to u
    //
    // The original rotation can be recovered via q = swing * twist
    //
    // References
    // http://stackoverflow.com/questions/3684269/component-of-a-quaternion-rotation-around-an-axis
    // http://www.euclideanspace.com/maths/geometry/rotations/for/decomposition/
    //
    void twistswing_decomposition(const vec3<T> &u, quat<T> &twist, quat<T> &swing) const;
    
    //
    // ------------------------- Transformations --------------------------
    //
    
    //
    // Quaternion for rotation of θ radians around (normalized) vector u
    //
    // Game Physics Pearls, page 14: Q = (cos(θ/2), sin(θ/2)u).
    // https://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Euler_axis%E2%80%93angle_%E2%86%94_quaternion
    //
    static quat<T> rotation(const T& theta, const vec3<T>& u)
    {
        quat<T> q;
        const T theta_half = theta * 0.5;
        const T s = std::sin(theta_half);
        q.qw = std::cos(theta_half);
        q.qx = u.x * s;
        q.qy = u.y * s;
        q.qz = u.z * s;
        
        return q;
    }
    
    //
    // Quaternion for rotation from Euler angles
    //
    // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Source_code
    //
    static quat<T> rotation(const T& roll, const T& pitch, const T& yaw)
    {
        T cr = cos(roll * 0.5);
        T sr = sin(roll * 0.5);
        T cp = cos(pitch * 0.5);
        T sp = sin(pitch * 0.5);
        T cy = cos(yaw * 0.5);
        T sy = sin(yaw * 0.5);

        quat<T> q;
        q.qw = cr * cp * cy + sr * sp * sy;
        q.qx = sr * cp * cy - cr * sp * sy;
        q.qy = cr * sp * cy + sr * cp * sy;
        q.qz = cr * cp * sy - sr * sp * cy;
        return q;
    }
    
    //
    // ------------------------- Operator overloads -----------------------
    //
    
    //
    // Addition
    //
    // http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
    //
    inline quat<T> operator +(const quat<T> &q) const
    {
        return quat<T>(qw + q.qw,
                       qx + q.qx,
                       qy + q.qy,
                       qz + q.qz);
    }
    
    //
    // Quaternion multiplication
    //
    // http://se.mathworks.com/help/aeroblks/quaternionmultiplication.html
    //
    inline quat<T> operator *(const quat<T> &q) const
    {
        return quat<T>(qw*q.qw - qx*q.qx - qy*q.qy - qz*q.qz,
                       qx*q.qw + qw*q.qx - qz*q.qy + qy*q.qz,
                       qy*q.qw + qz*q.qx + qw*q.qy - qx*q.qz,
                       qz*q.qw - qy*q.qx + qx*q.qy + qw*q.qz);
    }
    
    inline quat<T>& operator +=(const quat<T> &q)
    {
        qw += q.qw;
        qx += q.qx;
        qy += q.qy;
        qz += q.qz;
        return *this;
    }
    
    inline quat<T> operator *(const T &s) const
    {
        quat<T> q = *this;
        q.qw *= s;
        q.qx *= s;
        q.qy *= s;
        q.qz *= s;
        return q;
    }
    
    inline quat<T>& operator *=(const T &s)
    {
        qw *= s;
        qx *= s;
        qy *= s;
        qz *= s;
        return *this;
    }
    
    //
    // ----------------------------- Auxiliary -----------------------------
    //
    
    void debugPrint()
    {
        printf("[%1.2f %1.2f %1.2f %1.2f]", qw, qx, qy, qz);
    }
    
};

//
// ------------------------- Dual Quternion --------------------------------
//

template<class T> class dualquat
{
public:
    quat<T> real; // typically a rotation quaternion
    quat<T> dual; // typically a translation quaternion
    
    //
    // --------------------------- Constructors ----------------------------
    //
    
    //
    // Trivial
    //
    dualquat()
    : real(), dual()
    {
    }
    
    //
    // Copy
    //
    dualquat(const dualquat<T>& dq)
    : real(dq.real), dual(dq.dual)
    {
    }
    
    //
    // From rotation and translation quaternions
    //
    dualquat(const quat<T>& real, const quat<T>& dual)
    : real(real), dual(dual)
    {
    }
    
    //
    // From rotation matrix and translation vector
    //
    dualquat(const mat3<T>& R, const vec3<T>& t)
    {
        real = quat<T>(R);
        
        quat<T> tq = quat<T>(0, t.x, t.y, t.z);
        dual = tq*real*0.5f;
    }
    
    //
    // ----------------------------- Algebra -------------------------------
    //
    
    inline T length() const
    {
        /*
         void dq_op_norm2( double *real, double *dual, const dq_t Q )
         {
         *real =     Q[0]*Q[0] + Q[1]*Q[1] + Q[2]*Q[2] + Q[3]*Q[3];
         *dual = 2.*(Q[0]*Q[7] + Q[1]*Q[4] + Q[2]*Q[5] + Q[3]*Q[6]);
         }
         */
        
        return real.length();
        //            return sqrt(qw*qw + qx*qx + qy*qy + qz*qz);
    }
    
    inline void normalize()
    {
        *this *= (1.0f/length());
        
        //real *= 1;
        
        //            T f = 1.0/sqrt(qw*qw + qx*qx + qy*qy + qz*qz);
        //            qw *= f;
        //            qx *= f;
        //            qy *= f;
        //            qz *= f;
    }
    
    
    
    //
    // ------------------------- Transformations --------------------------
    //
    
    
    //
    // ------------------------- Operator overloads -----------------------
    //
    
    inline dualquat<T> operator +(const dualquat<T> &dq) const
    {
        return dualquat<T>(real+dq.real, dual+dq.dual);
    }
    
    inline dualquat<T> operator *(const T &s) const
    {
        return dualquat<T>(real*s, dual*s);
    }
    
    inline dualquat<T>& operator *=(const T &s)
    {
        real *= s;
        dual *= s;
        return *this;
    }
    
    //
    // ----------------------------- Auxiliary -----------------------------
    //
    
    //
    // Extracting translation & rotation
    //
    // For a dual quaternion dq = (real, dual),
    // we can extract a rotation r and a translation d,
    // both in quaternion form:
    //      r = real
    //      d = 2 dual * real*,
    // where real* means conjugate.
    // This is derived from
    //      real = r
    //      dual = 0.5 d r
    //
    
    vec3<T> get_translation()
    {
        // Translation quaternion
        quat<T> d = dual*real.conjugate()*static_cast<T>(2);
        
        return {d.qx, d.qy, d.qz};
        
        /*
         float tx = 2.0 * (-wT * real.qx + xT * real.qw - yT * real.qz + zT * real.qy);
         float ty = 2.0 * (-wT * real.qy + xT * real.qz + yT * real.qw - zT * real.qx);
         float tz = 2.0 * (-wT * real.qz - xT * real.qy + yT * real.qx + zT * real.qw);
         */
    }
    
    mat3<T> get_rotation()
    {
        return mat3<T>(real);
        
        // Construct R (3x3) (uses r)
        // Same as constructing matrix from quaternion: m4f(DQr_avg).get_3x3()
        // https://github.com/bobbens/libdq/blob/master/dq.c
        //            mat3<T> R;
        //            R.mat[0][0] = real.qw*real.qw + real.qx*real.qx - real.qy*real.qy - real.qz*real.qz;
        //            R.mat[1][0] = 2.*real.qx*real.qy - 2.*real.qw*real.qz;
        //            R.mat[2][0] = 2.*real.qx*real.qz + 2.*real.qw*real.qy;
        //            R.mat[0][1] = 2.*real.qx*real.qy + 2.*real.qw*real.qz;
        //            R.mat[1][1] = real.qw*real.qw - real.qx*real.qx + real.qy*real.qy - real.qz*real.qz;
        //            R.mat[2][1] = 2.*real.qy*real.qz - 2.*real.qw*real.qx;
        //            R.mat[0][2] = 2.*real.qx*real.qz - 2.*real.qw*real.qy;
        //            R.mat[1][2] = 2.*real.qy*real.qz + 2.*real.qw*real.qx;
        //            R.mat[2][2] = real.qw*real.qw - real.qx*real.qx - real.qy*real.qy + real.qz*real.qz;
    }
    
    mat4<T> get_homogeneous_matrix()
    {
        //return mat3<T>(real);
        
        mat4<T> TR = mat4<T>(get_rotation());
        TR.col[3] = get_translation().xyz1();
        return TR;
        
        // Construct T*R (4x4) (uses r and d)
        // Same as constructing matrix from quaternion:
        // T(tx, ty, tz) * m4f(DQr_avg).get_3x3()
        // http://www.chinedufn.com/dual-quaternion-shader-explained/
        //            mat4<T> M;
        //            M.mat[0][0] = 1.0 - (2.0 * real.qy * real.qy) - (2.0 * real.qz * real.qz);
        //            M.mat[0][1] = (2.0 * real.qx * real.qy) + (2.0 * real.qw * real.qz);
        //            M.mat[0][2] = (2.0 * real.qx * real.qz) - (2.0 * real.qw * real.qy);
        //            M.mat[0][3] = 0;
        //            M.mat[1][0] = (2.0 * real.qx * real.qy) - (2.0 * real.qw * real.qz);
        //            M.mat[1][1] = 1.0 - (2.0 * real.qx * real.qx) - (2.0 * real.qz * real.qz);
        //            M.mat[1][2] = (2.0 * real.qy * real.qz) + (2.0 * real.qw * real.qx);
        //            M.mat[1][3] = 0;
        //            M.mat[2][0] = (2.0 * real.qx * real.qz) + (2.0 * real.qw * real.qy);
        //            M.mat[2][1] = (2.0 * real.qy * real.qz) - (2.0 * real.qw * real.qx);
        //            M.mat[2][2] = 1.0 - (2.0 * real.qx * real.qx) - (2.0 * real.qy * real.qy);
        //            M.mat[2][3] = 0;
        //            M.mat[3][0] = tx;
        //            M.mat[3][1] = ty;
        //            M.mat[3][2] = tz;
        //            M.mat[3][3] = 1;
        //            return M;
    }
    
    // FUNCTION: mat4f GET HOMOG
    // dq_cr_rotation_matrix
    // https://github.com/bobbens/libdq/blob/master/dq.c
    
    // FUNCTION_ Transforming the Vertex with Dual Quaternions (avoiding matrix form - e.g. skinning)
    // http://donw.io/post/dual-quaternion-skinning/
    // http://dev.theomader.com/dual-quaternion-skinning/
    // V = 2Qd * Qr'
    
    
    //        void debugPrint()
    //        {
    //            printf("[%1.2f %1.2f %1.2f %1.2f]", qw, qx, qy, qz);
    //        }
    
};

// Euler angle extraction
// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Source_code_2

// A.k.a. roll
template<class T>
T extract_Euler_angle_x(const quat<T>& q)
{
    T sinr_cosp = 2.0f * (q.qw * q.qx + q.qy * q.qz);
    T cosr_cosp = 1.0f - 2.0f * (q.qx * q.qx + q.qy * q.qy);
    return std::atan2(sinr_cosp, cosr_cosp);
}

// A.k.a. pitch
template<class T>
T extract_Euler_angle_y(const quat<T>& q)
{
    double sinp = std::sqrt(1.0 + 2.0 * (q.qw * q.qy - q.qx * q.qz));

    //    double cosp = std::sqrt(1.0 - 2.0 * (q.qw * q.qy - q.qx * q.qz));
    // Hack to avoid singularity for gimbal lock (pitch is +/- 90 degrees)
    double cosp = std::sqrt(std::max(0.0, 1.0 - 2.0 * (q.qw * q.qy - q.qx * q.qz)));
    
    return 2.0 * std::atan2(sinp, cosp) - M_PI / 2.0;
    
//    T sinp = 2.0 * (q.qw * q.qy - q.qz * q.qx);
//    if (std::abs(sinp) >= 1.0)
//        return std::copysign(M_PI / 2.0, sinp); // use 90 degrees if out of range
//    return std::asin(sinp);
}

// A.k.a. yaw
template<class T>
T extract_Euler_angle_z(const quat<T>& q)
{
    T siny_cosp = 2 * (q.qw * q.qz + q.qx * q.qy);
    T cosy_cosp = 1 - 2 * (q.qy * q.qy + q.qz * q.qz);
    return std::atan2(siny_cosp, cosy_cosp);
}

template<class T>
vec3<T> extract_Euler_angles(const quat<T>& q)
{
//    return quaternionToEulerAngles(q);
    return vec3<T> {
        extract_Euler_angle_x(q),
        extract_Euler_angle_y(q),
        extract_Euler_angle_z(q)
    };
}

#if 0
// ChatGPT solution to gmbal lock poblem
template<class T>
vec3<T> quaternionToEulerAngles(const quat<T>& q)
{
    double roll, pitch, yaw;
    
    // Extract Euler angles (ZYX convention)
    double sinr_cosp = 2.0 * (q.qw * q.qx + q.qy * q.qz);
    double cosr_cosp = 1.0 - 2.0 * (q.qx * q.qx + q.qy * q.qy);
    roll = std::atan2(sinr_cosp, cosr_cosp);

    double sinp = 2.0 * (q.qw * q.qy - q.qz * q.qx);
    if (std::abs(sinp) >= 1.0)
        pitch = std::copysign(M_PI / 2.0, sinp); // use 90 degrees if out of range
    else
        pitch = std::asin(sinp);

    double siny_cosp = 2.0 * (q.qw * q.qz + q.qx * q.qy);
    double cosy_cosp = 1.0 - 2.0 * (q.qy * q.qy + q.qz * q.qz);
    yaw = std::atan2(siny_cosp, cosy_cosp);

    // Handle gimbal lock
    if (std::abs(pitch - M_PI / 2.0) < 1e-6) {
        // Gimbal lock at +90 degrees
        roll = 0.0;
        yaw = std::atan2(2.0 * (q.qy * q.qz + q.qw * q.qx), 1.0 - 2.0 * (q.qx * q.qx + q.qy * q.qy));
    } else if (std::abs(pitch + M_PI / 2.0) < 1e-6) {
        // Gimbal lock at -90 degrees
        roll = 0.0;
        yaw = -std::atan2(2.0 * (q.qy * q.qz + q.qw * q.qx), 1.0 - 2.0 * (q.qx * q.qx + q.qy * q.qy));
    }
    
    return vec3<T> { roll, pitch, yaw };
}
#endif

// Linear rigid-body interpolation of homogeneous transformations using
// dual quaternions.
// Used for e.g. skinning and parmetric animation blending.
//
// Note: Ignores scaling.
// Todo: Polish the code
//
template<class T>
inline mat4<T> rigidbody_lerp(const mat4<T>& M0, const mat4<T>& M1, float w)
{
    float x = w, y = 1.0f-w;
    dualquat<T> dq0, dq1, dqblend;
    dq0 = dualquat<T>(M0.get_3x3(), M0.column(3).xyz());
    dq1 = dualquat<T>(M1.get_3x3(), M1.column(3).xyz());
    float c0 = x;
    float c1 = (dq0.real.dot(dq1.real) < 0)? -y : y;
    dqblend = dq0*c0 + dq1*c1;
    dqblend.normalize();
    return dqblend.get_homogeneous_matrix();
}

template<class T>
inline mat4<T> rigidbody_quadlerp(const mat4<T>& M0, const mat4<T>& M1, float w)
{
    assert(0);
    return mat4<T>();
}

template<class T>
inline std::ostream& operator<< (std::ostream &out, const quat<T> &q)
{
    return out << "(" << q.qw << " { " << q.qx << ", " << q.qy << ", " << q.qz << "})";
}

typedef quat<float> quatf;
typedef quat<double> quatd;
typedef dualquat<float> dualquatf;
typedef dualquat<double> dualquatd;

}

#endif /* QUAT_H */
