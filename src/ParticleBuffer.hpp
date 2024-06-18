//
//  Created by Carl Johan Gribel on 2024-06-18.
//

#ifndef ParticleRenderer_h
#define ParticleRenderer_h

#include "ShapeRenderer.hpp"
#include "vec.h"

#define ParticlesCapacity 512
using ShapeRendererPtr = std::shared_ptr<Renderer::ImPrimitiveRenderer>;

class ParticleBuffer
{
    using PointType = Renderer::PointVertex;

    PointType   points[ParticlesCapacity];
    linalg::v3f point_vels[ParticlesCapacity];
    float       point_ages[ParticlesCapacity];
    int nbr_points = 0;

public:
    void push_point(const v3f& p, const v3f& v, uint color);

    int size();

    int capacity();

    // push_burst

    void update(float dt);

    void render(ShapeRendererPtr renderer);
};

#endif
