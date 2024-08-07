//
//  Created by Carl Johan Gribel on 2024-06-18.
//

#ifndef ParticleRenderer_h
#define ParticleRenderer_h

#include <memory>
#include "ShapeRenderer.hpp"
#include "vec.h"

#define ParticlesCapacity 1024
using ShapeRendererPtr = std::shared_ptr<Renderer::ImPrimitiveRenderer>;

class ParticleBuffer
{
    using uchar = unsigned char;
    using uint = unsigned int;
    using PointType = Renderer::PointVertex;

    PointType   points[ParticlesCapacity];
    linalg::v3f point_vels[ParticlesCapacity];
    float       point_ages[ParticlesCapacity];
    int nbr_points = 0;

public:
    void push_point(const v3f& p, const v3f& v, uint color);

    void push_trail(const v3f& p, const v3f& v, int nbr_particles, uint color);

    void push_explosion(const v3f& p, const v3f& v, int nbr_particle, uint color);

    int size();

    int capacity();

    // push_burst

    void update(float dt);

    void update_explosion(float dt);

    void render(ShapeRendererPtr renderer);
};

#endif
