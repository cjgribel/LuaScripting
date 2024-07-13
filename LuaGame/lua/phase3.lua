
local prefabloaders = require("prefabs")

local phase3 = {
    name = 'Phase 3',
    entities = {},
    timer = 0.0,
    nbr_enemies = 30,
    start_nbr_killed
}

function phase3:init()

    for i = 1, self.nbr_enemies do
        table.insert(self.entities, prefabloaders.bouncing_enemy_block(1, 1, 0.25, 1.0, 0xffff8080))
    end

    self.start_nbr_killed = config.enemy_kill_count

    log("phase3 begins, " .. #self.entities .. " entities created")
end

function phase3:update(dt)

    self.timer = self.timer + dt

    ImGui_SetNextWindowWorldPos(5, 6.5)
    ImGui_Begin("Phase3Text")
    --ImGui_Text('Time ' .. tostring(self.timer) .. "/" .. tostring(self.dur))
    ImGui_Text("Kill all enemies")
    ImGui_End()
end

function phase3:has_finished()

    return (config.enemy_kill_count - self.start_nbr_killed ) >= self.nbr_enemies

    --if self.timer > 10.0 then
    --    return true
    --else
    --    return false
    --end
end

function phase3:destroy()

    log("phase3 ends, flagging " .. #self.entities .. " entities for destruction")

    for _, entity in ipairs(self.entities) do
        --registry:destroy(entity, registry:valid(entity))
        flag_entity_for_destruction(entity)
    end
    -- Clear the entities list after destroying them
    self.entities = {}
    
    --print("phase3:destroy() called")
end

return phase3
