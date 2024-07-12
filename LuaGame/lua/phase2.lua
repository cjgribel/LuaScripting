
local prefabloaders = require("prefabs")

local phase2 = {
    name = 'Phase 2',
    entities = {},
    timer = 0.0
}

function phase2:init()
    print("phase2:init() called")

    --table.insert(self.entities, create_bouncy_entity())
    table.insert(self.entities, prefabloaders.bouncing_enemy_block(0xffff8080))

    log("phase2 begins, " .. #self.entities .. " entities created")
end

function phase2:update(dt)
    --print("phase2:update() called with dt:", dt)

    self.timer = self.timer + dt
end

function phase2:has_finished()
    --print("phase2:has_finished() called")

    if self.timer > 10.0 then
        return true
    else
        return false
    end
end

function phase2:destroy()

    log("phase2 ends, flagging " .. #self.entities .. " entities for destruction")

    for _, entity in ipairs(self.entities) do
        --registry:destroy(entity, registry:valid(entity))
        flag_entity_for_destruction(entity)
    end
    -- Clear the entities list after destroying them
    self.entities = {}
    
    --print("phase2:destroy() called")
end

return phase2
