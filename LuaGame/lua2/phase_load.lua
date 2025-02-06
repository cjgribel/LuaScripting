phase_load = {

    batch_id = nil,
}

--function phase_load:init()
--    engine.log("phase_load:init()")
--end

--function phase_load:destroy()
--    engine.log('phase_load:destroy() #' .. self.id())
--end

function phase_load:run(next_phase)

    -- -> PHASE
    -- Queue a batch of assets
    self.batch_id = engine.queue_assets({"sound1.wav", "model1.fbx", "texture1.png"})


    --[[
        engine.log(string.format(
            'player_behavior:run() entity = %s, projectile_pool_behavior = %s, game_behavior = %s',
            self.id(),
            tostring(self.projectile_pool),
            tostring(self.game)
            ))
            ]]
    engine.log('phase_load:run() #' .. self.id())
end

function phase_load:stop()
    engine.log('phase_load:stop() #' .. self.id())
end

function phase_load:is_done()
    return engine.is_loading_complete(self.batch_id)
end

function phase_load:did_loading_succeed(next_phase)
    return engine.did_loading_succeed(self.batch_id)
end

function phase_load:update(dt)

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

    ImGui_SetNextWindowWorldPos(2, 6.5)
    ImGui_Begin("Phase1Text")
    --ImGui_Text('Time ' .. tostring(self.timer) .. "/" .. tostring(self.dur))
    ImGui_Text("Kill all enemies")
    ImGui_End()
end

return phase_load