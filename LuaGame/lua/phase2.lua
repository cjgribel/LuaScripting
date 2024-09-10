
local prefabloaders = require("prefabs")

local phase2 = {
    name = 'Phase 2',
    entities = {},
    timer = 0.0,
    nbr_enemies = 3,
    start_nbr_killed
}

function phase2:init()

    for i = 1, self.nbr_enemies do
        table.insert(self.entities, prefabloaders.bouncing_enemy_block(7, 7, 0.25, 2.0, 0xffff8080))
    end

    self.start_nbr_killed = config.enemy_kill_count

    -- Music
    audio_manager:fadeInMusic(config.sounds.music_lvl2, 1, 4000)

    -- Log
    log("phase2: loaded " .. #self.entities .. " entities")
end

function phase2:update(dt)
    --print("phase2:update() called with dt:", dt)

    self.timer = self.timer + dt

    ImGui_SetNextWindowWorldPos(5, 6.5)
    ImGui_Begin("Phase2Text")
    --ImGui_Text('Time ' .. tostring(self.timer) .. "/" .. tostring(self.dur))
    ImGui_Text("Kill all enemies")
    ImGui_End()
end

function phase2:has_finished()

    return (config.enemy_kill_count - self.start_nbr_killed ) >= self.nbr_enemies

end

function phase2:destroy()

    log("phase2 ends")

    for _, entity in ipairs(self.entities) do
        --registry:destroy(entity, registry:valid(entity))
        engine.destroy_entity(entity)
    end

    -- Clear the entities list after destroying them
    self.entities = {}

    audio_manager:fadeOutMusic(0)
    
    --print("phase2:destroy() called")
end

return phase2
