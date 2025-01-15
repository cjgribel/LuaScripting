
local function spawn_projectile(chunk_tag, size, projectile_pool)
    
    -- local entity = engine.create_entity(chunk_tag, "Projectile", projectile_pool.id())
    -- Don't parent to the projectile pool unless they belong to the same chunk
    local entity = engine.create_entity(chunk_tag, "Projectile", engine.entity_null)

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

    --print("prefabloaders:projectile(size, projectile_pool)")

    return entity
end

--return spawn_projectile(chunk_tag, 0.2, nil)

-- Check if this script is being run directly (main chunk) or required by another script
if ... == nil then
    -- Running directly, return an entity
    return spawn_projectile("", 0.2, nil)
else
    -- When required, return the function so it can be called with arguments
    return spawn_projectile
end