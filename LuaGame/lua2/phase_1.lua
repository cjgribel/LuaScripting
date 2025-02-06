phase_1 = {

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

function phase_1:run()

    -- TODO create phase entities

    --[[
        engine.log(string.format(
            'player_behavior:run() entity = %s, projectile_pool_behavior = %s, game_behavior = %s',
            self.id(),
            tostring(self.projectile_pool),
            tostring(self.game)
            ))
            ]]
    engine.log('phase_1:run() #' .. self.id())
end

function phase_1:stop()
    engine.log('phase_1:stop() #' .. self.id())

    -- TODO destroy phase entities
end

function phase_1:update(dt)


end

function phase_1:has_finished()

    return false
    --return (game.config.enemy_kill_count - self.start_nbr_killed ) >= self.nbr_enemies

end

return phase_1