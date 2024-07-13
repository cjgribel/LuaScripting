
local prefabloaders = require("prefabs")

local phase1 = {
    name = 'Phase 1',
    entities = {},
    timer = 0.0,
    dur = 15.0,
    nbr_enemies = 3,
    start_nbr_killed
}

function phase1:init()
    
    --print("phase1:init() called")
    
    --table.insert(self.entities, create_bouncy_entity())
    --table.insert(self.entities, create_bouncy_entity())

    --local ent1 = prefabloaders:bouncing_enemy_block()

    for i = 1, self.nbr_enemies do
        table.insert(self.entities, prefabloaders.bouncing_enemy_cross(0xffffffff))
    end

    self.start_nbr_killed = config.enemy_kill_count

    log("phase1 begins, " .. #self.entities .. " entities created")
end

function phase1:update(dt)

    self.timer = self.timer + dt

    ImGui_SetNextWindowWorldPos(5, 6.5)
    ImGui_Begin("Phase1Text")
    --ImGui_Text('Time ' .. tostring(self.timer) .. "/" .. tostring(self.dur))
    ImGui_Text("Kill all enemies")
    ImGui_End()
end

function phase1:has_finished()

    return (config.enemy_kill_count - self.start_nbr_killed ) >= self.nbr_enemies

    --if self.timer > self.dur then
    --    return true
    --else
    --    return false
    --end
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
