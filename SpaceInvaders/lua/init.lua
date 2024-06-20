
function random_color()
    local alpha = 0x80
    local red = math.random(0, 255)
    local green = math.random(0, 255)
    local blue = math.random(0, 255)

    local color = (alpha << 24) | (red << 16) | (green << 8) | blue
    return color
end

local function create_bouncy_entity(index)
    local entity = registry:create()
    print("Created entity ID:", entity)
    
    -- Attach Transform
    registry:emplace(entity, Transform(0.0, 0.0))
    
    -- Attach Quad
    local size = 0.1 + math.random() * 0.5
    registry:emplace(entity, QuadComponent(size, random_color()))
    
    -- Attach Circle collider
    registry:emplace(entity, CircleColliderComponent(size/2))

    -- Attach behavior
    add_script(registry, entity, dofile("lua/bounce_behavior.lua"), "bounce_behavior")
end

--local function create_projectile_pool__entity()
--    local entity = registry:create()
--
    -- Behavior
--    add_script(registry, entity, dofile("lua/projectile_behavior.lua"), "projectile_behavior")
--    return entity
--end

--local function create_player_entity(size, color, projectile_pool_entity)
--    local entity = registry:create()
--    registry:emplace(entity, Transform(0.0, 0.0))
--    registry:emplace(entity, QuadComponent(size, color))
--    registry:emplace(entity, CircleColliderComponent(size/2))

    -- Behavior
--    local player_table = add_script(registry, entity, dofile("lua/player_behavior.lua"), "player_behavior")
--    player_table.projectile_pool_entity = projectile_pool_entity
--end

print('Lua init script...')

math.randomseed(os.time())

-- Projectile entity
--local projectile_entity = create_projectile_pool__entity()

-- Create player(s)
--local player_entity = create_player_entity(0.5, 0xffffffff, projectile_entity)

-- Create 5 bouncing entities
for i = 1, 15 do
    create_bouncy_entity(i)
end

print('Lua init script done')
