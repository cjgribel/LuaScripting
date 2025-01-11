

function create_player_entity(size, color, projectile_pool, parent_entity)
    
    local entity = engine.create_entity("game_chunk", "Player", parent_entity)

    -- Header
    --engine.registry:emplace(entity, HeaderComponent("Player"))

    -- Transform
    engine.registry:emplace(entity, Transform(0.0, 0.0, 0.0))

    -- QuadGridComponent
    local quadgrid = QuadGridComponent(1, 1, true)
    quadgrid:set_quad_at(0, 0.0, 0.0, size, 0xffffffff, true)
    engine.registry:emplace(entity, quadgrid)

    -- CircleColliderGridComponent
    local collidergrid = CircleColliderGridComponent(1, 1, true, PlayerCollisionBit, EnemyCollisionBit)
    collidergrid:set_circle_at(0, 0.0, 0.0, size * 0.5, true)
    engine.registry:emplace(entity, collidergrid)

    -- Behavior
    local player_table = engine.add_script(engine.registry, entity, "player_behavior")
    player_table.projectile_pool = projectile_pool

    return entity
end

engine.log("Running player_spawner.lua...")

local projectilepool_table = {} -- fetch in init
local game_entity = {}
local player_entity = create_player_entity(0.5, 0xffffffff, projectilepool_table, engine.entity_null)

engine.log("Created Player")
