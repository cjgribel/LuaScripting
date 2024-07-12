
-- Adjust the package path to include the "../../LuaGame/lua" directory
package.path = package.path .. ";../../LuaGame/lua/?.lua"

local prefabloaders = require("prefabs")

PlayerCollisionBit = 0x1
EnemyCollisionBit = 0x2
ProjectileCollisionBit = 0x4

function rotate(x, y, theta)

    local cos_theta = math.cos(theta)
    local sin_theta = math.sin(theta)
    return x * cos_theta - y * sin_theta, x * sin_theta + y * cos_theta

end

function random_color()
    local alpha = 0xff
    local red = math.random(0, 255)
    local green = math.random(0, 255)
    local blue = math.random(0, 255)

    local color = (alpha << 24) | (red << 16) | (green << 8) | blue
    return color
end

local function create_projectile_pool_entity()
    local entity = registry:create()

    -- Behavior
    add_script(registry, entity, dofile("../../LuaGame/lua/projectile_pool_behavior.lua"), "projectile_pool_behavior")
    return entity
end

local function create_player_entity(size, color, projectile_pool)
    local entity = registry:create()

    -- Transform
    registry:emplace(entity, Transform(0.0, 0.0, 0.0))

    -- QuadGridComponent
    local quadgrid = QuadGridComponent(1, 1, true)
    quadgrid:set_quad_at(0, 0.0, 0.0, size, 0xffffffff, true)
    registry:emplace(entity, quadgrid)

    -- CircleColliderGridComponent
    local collidergrid = CircleColliderGridComponent(1, 1, true, PlayerCollisionBit, EnemyCollisionBit)
    collidergrid:set_circle_at(0, 0.0, 0.0, size * 0.5, true)
    registry:emplace(entity, collidergrid)

    -- Behavior
    local player_table = add_script(registry, entity, dofile("../../LuaGame/lua/player_behavior.lua"), "player_behavior")
    player_table.projectile_pool = projectile_pool

    return entity
end

-- NOT USED YET
local function create_background_entity(size, color)
    local entity = registry:create()

    -- Transform
    -- TODO: non-uniform size
    registry:emplace(entity, Transform(0.0, 0.0, 0.0))

    -- QuadGridComponent
    local quadgrid = QuadGridComponent(1, 1, true)
    quadgrid:set_quad_at(0, 0.0, 0.0, size, 0x40ffffff, true)
    registry:emplace(entity, quadgrid)

    return entity
end

local function create_phasemanager_entity()

    local entity = registry:create()
    
    add_script(registry, entity, dofile("../../LuaGame/lua/phasemanager_behavior.lua"), "phasemanager_behavior")
    
    return entity
end

print('Lua init script...')

math.randomseed(os.time())

config = {
    player_speed = 10.0,
    bounds = { left = -5, right = 10, bottom = -5, top = 5 },
    is_out_of_bounds = function(self, x, y)
        return x < self.bounds.left or x > self.bounds.right or y < self.bounds.bottom or y > self.bounds.top
    end,
    enemy_kill_count = 0,
    player_deaths = 0
}

-- Projectile pool
local projectilepool_entity = create_projectile_pool_entity()
local projectilepool_table = get_script(registry, projectilepool_entity, "projectile_pool_behavior")

-- Create player(s)
local player_entity = create_player_entity(0.5, 0xffffffff, projectilepool_table)

-- Create bouncing entities
for i = 1, 1 do
    --create_bouncy_entity(i)
    prefabloaders.bouncing_enemy_block(0xffff00ff)
end

create_phasemanager_entity()

print('Lua init script done')
