//
//  Created by Carl Johan Gribel on 2024-06-18.
//

//#include <stdio.h>
#include "ParticleBuffer.hpp"

using uchar = unsigned char;

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
        point_vels[i] += (v3f(0.0f, -9.82f, 0.0f) * 0.25 * dt);
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
        2);
}