
local prefabloaders = require("prefabs")

local phase1 = {
    name = 'Phase 1',
    entities = {},
    timer = 0.0,
    dur = 5.0
}

function phase1:init()
    
    print("phase1:init() called")
    
    --table.insert(self.entities, create_bouncy_entity())
    --table.insert(self.entities, create_bouncy_entity())

    --local ent1 = prefabloaders:bouncing_enemy_block()

    for i = 1, 10 do
        table.insert(self.entities, prefabloaders:bouncing_enemy_cross())
    end

    log("phase1 begins, " .. #self.entities .. " entities created")
end

function phase1:update(dt)

    self.timer = self.timer + dt

    ImGui_SetNextWindowWorldPos(5, 6)
    ImGui_Begin("Phase1Text")
    ImGui_Text('Time ' .. tostring(self.timer) .. "/" .. tostring(self.dur))
    ImGui_End()
end

function phase1:has_finished()

    if self.timer > self.dur then
        return true
    else
        return false
    end
end

function phase1:destroy()

    log("phase1 ends, flagging " .. #self.entities .. " entities for destruction")

    for _, entity in ipairs(self.entities) do
        --registry:destroy(entity)
        flag_entity_for_destruction(entity)
    end
    -- Clear the entities list after destroying them
    self.entities = {}
    
    --print("phase1:destroy() called")
end

return phase1
