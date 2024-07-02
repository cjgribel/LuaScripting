
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
    
    local W = 8 -- Number of columns
    local H = 8 -- Number of rows
    local D = 0.2 -- Size of each quad (width/height)
    local color = 0xffff00ff -- Color of the quads
    local visible = true -- Visibility flag
    local core_x = math.floor(W / 2)
    local core_y = math.floor(H / 2)
    
    -- QuadSetComponent
    local qsc = QuadSetComponent(W, H, true)
    for i = 0, W - 1 do
        for j = 0, H - 1 do
            local x = (i - (W - 1) / 2) * D
            local y = (j - (H - 1) / 2) * D
            if i == core_x and j == core_y then
                --qsc:add_quad(x, y, D, 0xff0000ff, visible)
                qsc:set_quad(i, j, x, y, D, 0xff0000ff, visible)
            else
                --qsc:add_quad(x, y, D, color, visible)
                qsc:set_quad(i, j, x, y, D, color, visible)
            end
        end
    end
    registry:emplace(entity, qsc)

    -- CircleColliderSetComponent
    local ccs = CircleColliderSetComponent(W, H, true, EnemyCollisionBit, PlayerCollisionBit | ProjectileCollisionBit)
    --ccs:add_circle(0.25, -0.25, 0.25, true)
    --ccs:add_circle(0.25, 0.25, 0.25, true)
    --ccs:add_circle(-0.25, 0.25, 0.25, true)
    --ccs:add_circle(-0.25, -0.25, 0.25, true)
    for i = 0, W - 1 do
        for j = 0, H - 1 do
            local x = (i - (W - 1) / 2) * D
            local y = (j - (H - 1) / 2) * D
            --ccs:add_circle(x, y, D, visible)
            ccs:set_circle(i, j, x, y, D, visible)
        end
    end
    registry:emplace(entity, ccs)

    -- Island finder component
    registry:emplace(entity, IslandFinderComponent(core_x, core_y))

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
    local qsc = QuadSetComponent(1, 1, true)
    --qsc:add_quad(0.0, 0.0, size, 0xffffffff, true)
    qsc:set_quad(0, 0, 0.0, 0.0, size, 0xffffffff, true)
    registry:emplace(entity, qsc)

    -- CircleColliderSetComponent
    local ccs = CircleColliderSetComponent(1, 1, true, PlayerCollisionBit, EnemyCollisionBit)
    --ccs:add_circle(0.0, 0.0, size * 0.5, true)
    ccs:set_circle(0, 0, 0.0, 0.0, size * 0.5, true)
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
