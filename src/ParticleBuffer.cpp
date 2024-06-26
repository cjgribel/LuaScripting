//
//  Created by Carl Johan Gribel on 2024-06-18.
//

//#include <stdio.h>
#include "ParticleBuffer.hpp"
#include "interp.h"

using uchar = unsigned char;

namespace {
    inline vec3f color_spark(float x)
    {
        // white (1,1,1) -> yellow (1,1,0) -> orange (1,0.5,0) -> black (0,0,0)
        // Transition
        // # 0 1 2   3
        // R 1 1 1   0 : step_down(2)
        // G 1 1 0.5 0 : step_down(1) + 0.5*pulse(1,2)
        // B 1 0 0   0 : step_down(1)

        const int nbrStrata = 3;
        float xf, xs, x0, x1;
        xs = x * nbrStrata;
        xf = modff(x * nbrStrata, &x0);
        x1 = x0 + 1;

        const float r0 = step_down(2, x0);
        const float r1 = step_down(2, x1);
        const float g0 = (float)(x0 < 1) + 0.5 * step_updown(1, 2, x0); // ! = 1.5 at 1
        const float g1 = (float)(x1 < 1) + 0.5 * step_updown(1, 2, x1);
        const float b0 = step_down(1, x0);
        const float b1 = step_down(1, x1);

        return
        {
            lerp<float>(r0, r1, xf),
            lerp<float>(g0, g1, xf),
            lerp<float>(b0, b1, xf)
        };
    }

    inline vec3f color_heatmap_(float x)
    {
        const int nbrStrata = 4;
        float xf, xs, x0, x1;
        xs = x * nbrStrata;
        xf = modff(x * nbrStrata, &x0);
        x1 = x0 + 1;

        float r0 = step_up(3, x0);
        float r1 = step_up(3, x1);
        float g0 = step_updown(1, 3, x0);
        float g1 = step_updown(1, 3, x1);
        float b0 = step_down(1, x0);
        float b1 = step_down(1, x1);

        //        float r = lerp<float>(step_up(3, x0), step_up(3, x1), xf);

        return
        {
            lerp<float>(r0, r1, xf),
            lerp<float>(g0, g1, xf),
            lerp<float>(b0, b1, xf)
        };
    }
}

void ParticleBuffer::push_point(const v3f& p, const v3f& v, uint color)
{
    if (nbr_points < ParticlesCapacity)
    {
        points[nbr_points] = PointType{ p, color };
        point_vels[nbr_points] = v;
        point_ages[nbr_points] = 0.0f;
        ++nbr_points;
    }
}

void ParticleBuffer::push_explosion(const v3f& p, const v3f& v, uint color)
{
    const float vel_len = length(v);
    const float vel_max = 5.0f;
    const float theta_min = 20.0f * fTO_RAD, theta_max = fPI;
    float vel_factor = clamp(vel_len / vel_max, 0.0f, 1.0f);
    float theta_spread = lerp(theta_max, theta_min, vel_factor);

    //std::cout << v << ", " << vel_factor << ", " << theta_spread << std::endl;

    v3f vn;
    if (vel_len < 0.001f)
        vn = v3f{ 1.0f, 0.0f, 0.0f };
    else
        vn = v3f{ v.x / vel_len, v.y / vel_len, 0.0f };

    for (int i = 0; i < 40; i++)
    {
        const float theta = rnd(-theta_spread, theta_spread);
        const float sin_theta = std::sin(theta);
        const float cos_theta = std::cos(theta);
        const v3f pvn = v3f{
            vn.x * cos_theta - vn.y * sin_theta,
            vn.x * sin_theta + vn.y * cos_theta,
            0.0f
        };
        const float speed = rnd(1.0f, 7.5f);
        push_point(p, pvn * speed, color);
    }
}

int ParticleBuffer::size()
{
    return nbr_points;
}

int ParticleBuffer::capacity()
{
    return ParticlesCapacity;
}

void ParticleBuffer::update(float dt)
{
    const float max_age = 0.75f;

    for (int i = 0; i < nbr_points; i++)
    {
        // Gravity
        //point_vels[i] += (v3f(0.0f, -9.82f, 0.0f) * 0.25 * dt);
        // Damping
        point_vels[i] += (point_vels[i] * -5.0f * dt);

        points[i].p += (point_vels[i] * dt);
        point_ages[i] += dt;

        // Current point has expired: shift last point to current pos
        if (point_ages[i] > max_age)
        {
            points[i] = points[nbr_points - 1];
            point_vels[i] = point_vels[nbr_points - 1];
            point_ages[i] = point_ages[nbr_points - 1];
            --nbr_points;
            --i;
        }
        else
        {
            uchar alpha = (uchar)((1.0f - point_ages[i] / max_age) * 255);
            points[i].color = ((points[i].color & 0x00ffffff) | ((uint)alpha << 24));
        }
    }
}

void ParticleBuffer::update_explosion(float dt)
{
    const float max_age = 0.75f;

    for (int i = 0; i < nbr_points; i++)
    {
        // Gravity
        //point_vels[i] += (v3f(0.0f, -9.82f, 0.0f) * 0.25 * dt);
        // Damping
        point_vels[i] += (point_vels[i] * -5.0f * dt);

        points[i].p += (point_vels[i] * dt);
        point_ages[i] += dt;

        // Current point has expired: shift last point to current pos
        if (point_ages[i] > max_age)
        {
            points[i] = points[nbr_points - 1];
            point_vels[i] = point_vels[nbr_points - 1];
            point_ages[i] = point_ages[nbr_points - 1];
            --nbr_points;
            --i;
        }
        else
        {
            uchar alpha = (uchar)((1.0f - point_ages[i] / max_age) * 255);
            points[i].color = ((points[i].color & 0x00ffffff) | ((uint)alpha << 24));
        }
    }
}

void ParticleBuffer::render(ShapeRendererPtr renderer)
{
    renderer->push_points(points,
        nbr_points,
        4);
}