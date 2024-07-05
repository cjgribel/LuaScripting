
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

local function create_bouncy_entity(index)
    local entity = registry:create()
    print("Created entity ID:", entity)
    
    registry:emplace(entity, Transform(0.0, 0.0, math.pi*0.5))

    --local size = 0.5 + math.random() * 0.0
    --registry:emplace(entity, QuadComponent(size, random_color(), true))
    
    --registry:emplace(entity, CircleColliderComponent(size * 0.5, true))
    
    local W = 7 -- Number of columns
    local H = 7 -- Number of rows
    local D = 0.25 -- Size of each quad (width/height)
    local color = 0xffff00ff -- Color of the quads
    local visible = true -- Visibility flag
    local core_x = math.floor(W / 2)
    local core_y = math.floor(H / 2)
    
    -- QuadSetComponent
    local quadset = QuadSetComponent(W, H, true)
    for i = 0, W - 1 do
        for j = 0, H - 1 do
            local x = (i - (W - 1) / 2) * D
            local y = (j - (H - 1) / 2) * D
            if i == core_x and j == core_y then
                quadset:set_quad(i, j, x, y, D, 0xff0000ff, visible)
            else
                quadset:set_quad(i, j, x, y, D, random_color(), visible)
            end
        end
    end
    registry:emplace(entity, quadset)

    -- CircleColliderSetComponent
    local circleset = CircleColliderSetComponent(W, H, true, EnemyCollisionBit, PlayerCollisionBit | ProjectileCollisionBit)
    for i = 0, W - 1 do
        for j = 0, H - 1 do
            local x = (i - (W - 1) / 2) * D
            local y = (j - (H - 1) / 2) * D
            circleset:set_circle(i, j, x, y, D * 0.5, visible)
        end
    end
    registry:emplace(entity, circleset)

    -- Island finder component
    registry:emplace(entity, IslandFinderComponent(core_x, core_y))

    -- Bounce behavior
    add_script(registry, entity, dofile("../../LuaGame/lua/bounce_behavior.lua"), "bounce_behavior")
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

    -- QuadSetComponent
    local quadset = QuadSetComponent(1, 1, true)
    quadset:set_quad(0, 0, 0.0, 0.0, size, 0xffffffff, true)
    registry:emplace(entity, quadset)

    -- CircleColliderSetComponent
    local circleset = CircleColliderSetComponent(1, 1, true, PlayerCollisionBit, EnemyCollisionBit)
    circleset:set_circle(0, 0, 0.0, 0.0, size * 0.5, true)
    registry:emplace(entity, circleset)

    -- Behavior
    local player_table = add_script(registry, entity, dofile("../../LuaGame/lua/player_behavior.lua"), "player_behavior")
    player_table.projectile_pool = projectile_pool

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

-- Projectile entity
local projectile_pool_entity = create_projectile_pool_entity()

-- Projectile pool
local projectilePool = get_script(registry, projectile_pool_entity, "projectile_pool_behavior")

-- Create player(s)
local player_entity = create_player_entity(0.5, 0xffffffff, projectilePool)

-- Create 5 bouncing entities
for i = 1, 3 do
    create_bouncy_entity(i)
end

print('Lua init script done')
