
PlayerCollisionBit = 0x1
EnemyCollisionBit = 0x2
ProjectileCollisionBit = 0x4

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
    
    registry:emplace(entity, Transform(0.0, 0.0))

    --local size = 0.5 + math.random() * 0.0
    --registry:emplace(entity, QuadComponent(size, random_color(), true))
    
    --registry:emplace(entity, CircleColliderComponent(size * 0.5, true))
    
    -- QuadSetComponent
    local qsc = QuadSetComponent(true)
    --qsc:add_quad(0.25, -0.25, 0.5, 0xffff00ff, true)
    --qsc:add_quad(0.25, 0.25, 0.5, 0xffff00ff, true)
    --qsc:add_quad(-0.25, 0.25, 0.5, 0xffff00ff, true)
    --qsc:add_quad(-0.25, -0.25, 0.5, 0xffff00ff, true)
    local N = 6
    local W = 2.5
    local size = W / N
    for i = 0, N-1 do
        for j = 0, N-1 do
            -- Calculate the position of each quad
            local x = (i * size) + (size / 2) - (W / 2)
            local y = (j * size) + (size / 2) - (W / 2)
            qsc:add_quad(x, y, size, 0xffff00ff, true)
        end
    end
    registry:emplace(entity, qsc)

    -- CircleColliderSetComponent
    local ccs = CircleColliderSetComponent(true, EnemyCollisionBit, PlayerCollisionBit | ProjectileCollisionBit)
    --ccs:add_circle(0.25, -0.25, 0.25, true)
    --ccs:add_circle(0.25, 0.25, 0.25, true)
    --ccs:add_circle(-0.25, 0.25, 0.25, true)
    --ccs:add_circle(-0.25, -0.25, 0.25, true)
    for i = 0, N-1 do
        for j = 0, N-1 do
            -- Calculate the position of each quad
            local x = (i * size) + (size / 2) - (W / 2)
            local y = (j * size) + (size / 2) - (W / 2)
            ccs:add_circle(x, y, size, true)
        end
    end
    registry:emplace(entity, ccs)

    -- Bounce behavior
    add_script(registry, entity, dofile("../../SpaceInvaders/lua/bounce_behavior.lua"), "bounce_behavior")
end

local function create_projectile_pool_entity()
    local entity = registry:create()

    -- Behavior
    add_script(registry, entity, dofile("../../SpaceInvaders/lua/projectile_pool_behavior.lua"), "projectile_pool_behavior")
    return entity
end

local function create_player_entity(size, color, projectile_pool)
    local entity = registry:create()

    registry:emplace(entity, Transform(0.0, 0.0))

    --registry:emplace(entity, QuadComponent(size, color, true))
    --registry:emplace(entity, CircleColliderComponent(size * 0.5, true))

    -- QuadSetComponent
    local qsc = QuadSetComponent(true)
    qsc:add_quad(0.0, 0.0, size, 0xffffffff, true)
    registry:emplace(entity, qsc)

    -- CircleColliderSetComponent
    local ccs = CircleColliderSetComponent(true, PlayerCollisionBit, EnemyCollisionBit)
    ccs:add_circle(0.0, 0.0, size * 0.5, true)
    registry:emplace(entity, ccs)

    -- Behavior
    local player_table = add_script(registry, entity, dofile("../../SpaceInvaders/lua/player_behavior.lua"), "player_behavior")
    player_table.projectile_pool = projectile_pool

    return entity
end

print('Lua init script...')

math.randomseed(os.time())

config = {
    bounds = { left = -5, right = 10, bottom = -5, top = 5 },
    is_out_of_bounds = function(self, x, y)
        return x < self.bounds.left or x > self.bounds.right or y < self.bounds.bottom or y > self.bounds.top
    end,
    enemy_kill_count = 0,
    player_deaths = 0
}

-- Projectile entity
local projectile_pool_entity = create_projectile_pool_entity()
-- Fire a few test projectiles
local projectileBehavior = get_script(registry, projectile_pool_entity, "projectile_pool_behavior")
--projectileBehavior:fire(0.0, 0.0, 0.0, 1.0)

-- Create player(s)
local player_entity = create_player_entity(0.5, 0xffffffff, projectileBehavior)

-- Create 5 bouncing entities
for i = 1, 3 do
    create_bouncy_entity(i)
end

print('Lua init script done')
