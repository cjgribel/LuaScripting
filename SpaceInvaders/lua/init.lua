
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
    local size = 0.5 + math.random() * 0.0
    registry:emplace(entity, QuadComponent(size, random_color(), true))
    registry:emplace(entity, CircleColliderComponent(size * 0.5, true))
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
    registry:emplace(entity, QuadComponent(size, color, true))
--    registry:emplace(entity, CircleColliderComponent(size/2))

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
    enemy_kill_count = 0
}

-- Projectile entity
local projectile_pool_entity = create_projectile_pool_entity()
-- Fire a few test projectiles
local projectileBehavior = get_script(registry, projectile_pool_entity, "projectile_pool_behavior")
--projectileBehavior:fire(0.0, 0.0, 0.0, 1.0)

-- Create player(s)
local player_entity = create_player_entity(0.5, 0xffffffff, projectileBehavior)

-- Create 5 bouncing entities
for i = 1, 5 do
    create_bouncy_entity(i)
end

print('Lua init script done')
