
local function create_bouncy_entity()

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
                quadset:set_quad(i, j, x, y, D, 0xff0000ff, visible)
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

    return entity
end

local phase1 = {
    name = 'Phase 1',
    entities = {},
    timer = 0.0
}

function phase1:init()
    
    print("phase1:init() called")
    
    table.insert(self.entities, create_bouncy_entity())
    table.insert(self.entities, create_bouncy_entity())
    
end

function phase1:update(dt)
    --print("phase1:update() called with dt:", dt)

    self.timer = self.timer + dt
end

function phase1:has_finished()
    --print("phase1:has_finished() called")
    
    if self.timer > 2.0 then
        return true
    else
        return false
    end
end

function phase1:destroy()
    print("phase1:destroy() called")
    for _, entity in ipairs(self.entities) do
        registry:destroy(entity)
    end
    -- Clear the entities list after destroying them
    self.entities = {}
end

return phase1
