
local chunk_tag = "projectile_chunk"
local size = 0.2

function prefabloaders.projectile(chunk_tag, size, projectile_pool)
    
    local entity = engine.create_entity(chunk_tag, "Projectile", projectile_pool.id())

    -- Header
    --engine.registry:emplace(entity, HeaderComponent("Projectile"))

    -- Transform
    engine.registry:emplace(entity, Transform(0.0, 0.0, 0.0))
    
    -- QuadGridComponent
    local qsc = QuadGridComponent(1, 1, false)
    qsc:set_quad_at(0, 0.0, 0.0, size, 0xff00ffff, true)
    engine.registry:emplace(entity, qsc)

    -- CircleColliderGridComponent
    local ccs = CircleColliderGridComponent(1, 1, false, ProjectileCollisionBit, EnemyCollisionBit)
    ccs:set_circle_at(0, 0.0, 0.0, size * 0.75, true)
    engine.registry:emplace(entity, ccs)

    -- Projectile_behavior
    local projectile_behavior = engine.add_script(engine.registry, entity, "projectile_behavior")
    projectile_behavior.projectile_pool = projectile_pool

    print("prefabloaders:projectile(size, projectile_pool)")

    return entity
end