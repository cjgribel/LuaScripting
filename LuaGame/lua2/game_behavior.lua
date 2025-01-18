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
        enemy_kill_count = 0,
        player_deaths = 0,

        meta = {
            player_speed = {inspectable = true, serializable = true}
        }
    },

    meta = {
        config = {inspectable = true, serializable = true}
    }
}

function game:init()
    engine.log("game:init()")
end

function game:destroy()
    engine.log('game:destroy() #' .. self.id())
end

function game:run(dt)
    engine.log('game:run() #' .. self.id())
--[[
    engine.log(string.format(
        'player_behavior:run() entity = %s, projectile_pool_behavior = %s, game_behavior = %s',
        self.id(),
        tostring(self.projectile_pool),
        tostring(self.game)
    ))
    ]]
end

function game:stop(dt)
    engine.log('game:stop() #' .. self.id())
end

function game:update(dt)
end

game:init()
return game