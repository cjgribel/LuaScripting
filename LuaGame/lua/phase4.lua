
local prefabloaders = require("prefabs")

local phase4 = {
    name = 'Phase 4',
    entities = {},
    timer = 0.0,
    nbr_enemies = 30,
    start_nbr_killed
}

function phase4:init()

    --for i = 1, self.nbr_enemies do
    --    table.insert(self.entities, prefabloaders.bouncing_enemy_block(1, 1, 0.25, 1.0, 0xffff8080))
    --end

    self.start_nbr_killed = config.enemy_kill_count

    -- Music
    audio_manager:fadeInMusic(config.sounds.music_end, 2, 4000)

    -- Log
    log("phase4: loaded " .. #self.entities .. " entities")
end

function phase4:update(dt)

    self.timer = self.timer + dt

    ImGui_SetNextWindowWorldPos(5, 6.5)
    ImGui_Begin("Phase4Text")
    ImGui_Text("You won!")
    ImGui_End()
end

function phase4:has_finished()

    return false

end

function phase4:destroy()

    log("phase4 ends, flagging " .. #self.entities .. " entities for destruction")

    for _, entity in ipairs(self.entities) do
        flag_entity_for_destruction(entity)
    end

    -- Clear the entities list after destroying them
    self.entities = {}
    
    audio_manager:fadeOutMusic(0)
end

return phase4
