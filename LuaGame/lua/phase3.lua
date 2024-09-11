
local prefabloaders = require("prefabs")

local phase3 = {
    name = 'Phase 3',
    entities = {},
    timer = 0.0,
    nbr_enemies = 50,
    start_nbr_killed
}

function phase3:init()

    for i = 1, self.nbr_enemies do
        table.insert(self.entities, prefabloaders.bouncing_enemy_block(1, 1, 0.25, 1.0, 0xffff8080))
    end

    self.start_nbr_killed = game.config.enemy_kill_count

    -- Music
    audio_manager:fadeInMusic(game.config.sounds.music_lvl3, 1, 4000)

    -- Log
    log("phase3: loaded " .. #self.entities .. " entities")
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

    return (game.config.enemy_kill_count - self.start_nbr_killed ) >= self.nbr_enemies

end

function phase3:destroy()

    log("phase3 ends")

    for _, entity in ipairs(self.entities) do
        --registry:destroy(entity, registry:valid(entity))
        engine.destroy_entity(entity)
    end

    -- Clear the entities list after destroying them
    self.entities = {}
    
    audio_manager:fadeOutMusic(0)
end

return phase3
