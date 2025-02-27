phase_2 = {

    assets = {"sound1.wav", "model1.fbx", "texture1.png"},

    meta = {
        --config = {inspectable = true, serializable = true}
    }
}

--function game:init()
--    engine.log("game:init()")
--end

--function game:destroy()
--    engine.log('game:destroy() #' .. self.id())
--end

function phase_2:run()

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
    engine.log("phase_2:run()")
end

function phase_2:stop()
    engine.log("phase_2:stop()")
end

function phase_2:update(dt)


end

function phase_2:has_finished()

    return false
    --return (game.config.enemy_kill_count - self.start_nbr_killed ) >= self.nbr_enemies

end

return phase_2