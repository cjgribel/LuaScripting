
local prefabloaders = require("prefabs")

local phase1 = {
    name = 'Phase 1',
    entities = {},
    timer = 0.0,
    dur = 15.0,
    nbr_enemies = 1,
    start_nbr_killed
}

function phase1:init()

    -- Add entities
    for i = 1, self.nbr_enemies do

        local entity = prefabloaders.bouncing_enemy_cross(0xffffffff)

        table.insert(self.entities, entity)
    end

    self.start_nbr_killed = game.config.enemy_kill_count

    -- Music
    --audio_manager:fadeInMusic(game.config.sounds.music_lvl1, 1, 4000)

    -- Log
    log("phase1: loaded " .. #self.entities .. " entities")
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

    return (game.config.enemy_kill_count - self.start_nbr_killed ) >= self.nbr_enemies

end

function phase1:destroy()

    log("phase1 ends")

    for _, entity in ipairs(self.entities) do

        -- Flag for destruction
        engine.destroy_entity(entity)

    end

    -- Clear the entities list after destroying them
    self.entities = {}
    
    audio_manager:fadeOutMusic(0)

end

return phase1
