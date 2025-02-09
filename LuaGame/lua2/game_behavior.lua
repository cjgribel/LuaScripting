
-- Lua caches that returned value so any subsequent calls to require(...) will reuse the same result and won’t re-run the file.
local phase_load = require("phase_load")
local phase_1 = require("phase_1")
local phase_2 = require("phase_2")

game = {
    --game_entity = {},
    config = {
        player_speed = 10.0,
        bounds = { left = -5, right = 10, bottom = -5, top = 5 },
        is_out_of_bounds = function(self, x, y)
            return x < self.bounds.left or x > self.bounds.right or y < self.bounds.bottom or y > self.bounds.top
        end,
        sounds = {
            music_title = "music_title",
            music_end = "music_end",
            music_lvl1 = "music_lvl1",
            music_lvl2 = "music_lvl2",
            music_lvl3 = "music_lvl3",
    
            projectile_fire1 = "fire1",
            player_death = "player_death",
            element_explode = "player_death"
        }, 
        enemy_kill_count = 0, -- move 
        player_deaths = 0, -- move
        
        meta = {
            player_speed = {inspectable = true, serializable = true}
        }
    },

    batch_id = nil, -- -> TEST

    phases = {phase_1, phase_2},
    current_phase_index = 0, -- 0 = not started yet
    --next_phase_index = 1,
    current_phase = nil,
    --phase_is_loading = false, -- ???

    meta = {
        config = {inspectable = true, serializable = true}
    }
}

--[[
function game:switch_phase(phase)

    if current_phase then
        print("game:switch_phase error: current_phase is not nil")
        return
    end
    if !phase then
        print("game:switch_phase error: phase is nil")
        return
    end

    -- Stop current phase
    --if self.current_phase then
    --    self.current_phase:stop() -- destroys phase entities
    --end
    
    -- Run new phase
    self.current_phase = phase
    if self.current_phase then
        self.current_phase:run() -- creates phase entities
    end

end
]]

function game:stop_current_phase()

    --print("game:stop_current_phase() > " .. tostring(self.current_phase))

    if self.current_phase then
        self.current_phase:stop()
    end
    --current_phase_index = 0
    self.current_phase = nil    

    print("game:stop_current_phase()")
end

function game:reset()
    print("game:reset() >")
    -- Reset game state
    self.config.enemy_kill_count = 0
    self.config.player_deaths = 0
    
    self:stop_current_phase()
    print("game:reset()")
    self.current_phase_index = 0
end

function game:load_next_phase()

    -- Advance phase index
    self.current_phase_index = self.current_phase_index + 1

    if self.phases[self.current_phase_index] then
        -- Start loading the next phase
        phase_load:run(self.phases[self.current_phase_index])

        -- Switch to next phase
        --self:switch_phase(self.phases[self.current_phase_index])
    else
        print('All phases completed.')
    end

end

function game:init()
    
    engine.log("game:init()")

end

function game:destroy()
    engine.log('game:destroy() #' .. self.id())
end

function game:run()

    self:reset()
    self:load_next_phase()



    -- TEST
    -- Queue a batch of assets
    --self.batch_id = engine.queue_assets({"sound1.wav", "model1.fbx", "texture1.png"})

    --[[
        engine.log(string.format(
            'player_behavior:run() entity = %s, projectile_pool_behavior = %s, game_behavior = %s',
            self.id(),
            tostring(self.projectile_pool),
            tostring(self.game)
            ))
            ]]
    engine.log('game:run() #' .. self.id())
end

-- Called when play state is stopped
function game:stop()

    engine.log('game:stop() #' .. self.id())
    self:reset()

end

function game:update(dt)

    if not self.current_phase then
        if phase_load:is_done() then
            --print(".")
            if phase_load:did_loading_succeed() then
                --print("All assets in batch " .. self.batch_id .. " loaded successfully!")
                --print("All assets loaded successfully!")
                --self:phase_is_loading = false
                
                self.current_phase = self.phases[self.current_phase_index]
                self.current_phase:run()
            else
                --print("Some assets in batch " .. self.batch_id .. " failed to load.")
            end
        else
            --local progress = engine.get_loading_progress(self.batch_id)
            --print(string.format("Loading progress: %.2f%%", progress * 100))
        end
    end
    --print(",")
    
    
    if self.current_phase then
        self.current_phase:update(dt)

        if self.current_phase:has_finished() then
            self:stop_current_phase()
            self:load_next_phase()
        end
    else
        phase_load:update(dt)
    end
    



    --[[
    -- TEST
    if engine.is_loading_complete(self.batch_id) then
        -- Loading is complete
        if engine.did_loading_succeed(self.batch_id) then
            print("All assets in batch " .. self.batch_id .. " loaded successfully!")
        else
            print("Some assets in batch " .. self.batch_id .. " failed to load.")
        end
    else
        -- Loading is still in progress
        local progress = engine.get_loading_progress(self.batch_id)
        print(string.format("Loading progress: %.2f%%", progress * 100))
    end
    ]]

end

game:init()
return game