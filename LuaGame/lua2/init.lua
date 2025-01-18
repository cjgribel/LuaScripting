
-- Adjust the package path to include the "../../LuaGame/lua" directory
package.path = package.path .. ";../../LuaGame/lua2/?.lua"
-- remove local here and make it global to entire lua state?
--local prefabloaders = require("prefabs")
scripts_path = "../../LuaGame/lua2/"
assets_path = "../../assets/"

PlayerCollisionBit = 0x1
EnemyCollisionBit = 0x2
ProjectileCollisionBit = 0x4

math.randomseed(os.time())

function rotate(x, y, theta)

    local cos_theta = math.cos(theta)
    local sin_theta = math.sin(theta)
    return x * cos_theta - y * sin_theta, x * sin_theta + y * cos_theta

end

function random_color()
    local alpha = 0xff
    local red = math.random(0, 255)
    local green = math.random(0, 255)
    local blue = math.random(0, 255)

    local color = (alpha << 24) | (red << 16) | (green << 8) | blue
    return color
end

game = {
    game_entity = {},
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
        player_deaths = 0
    }
}

-- Music and effects (for all phases)
engine.log("Loading music & effects...")
local sounds = game.config.sounds
-- Music
engine.audio:registerMusic(sounds.music_title, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Title Screen.wav")
engine.audio:registerMusic(sounds.music_end, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Ending.wav")
engine.audio:registerMusic(sounds.music_lvl1, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 1.wav")
engine.audio:registerMusic(sounds.music_lvl2, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 2.wav")
engine.audio:registerMusic(sounds.music_lvl3, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 3.wav")
-- Effects
engine.audio:registerEffect(sounds.projectile_fire1, "../../assets/sounds/Misc Lasers/Fire 1.mp3")
engine.audio:registerEffect(sounds.player_death, "../../assets/sounds/Misc Lasers/Fire 2.mp3")
engine.audio:registerEffect(sounds.element_explode, "../../assets/sounds/Misc Lasers/Fire 2.mp3")
-- Volume
engine.audio:setMasterVolume(128)  -- Set master volume to 50%
--engine.audio:setMusicVolume("music1", 64)  -- Set music volume to 25%
engine.audio:setEffectVolume(sounds.projectile_fire1, 32)
engine.audio:setEffectVolume(sounds.element_explode, 32)

--[[
--> Spawners
]]

--[[
Debug entities
]]
--[[
local node1 = engine.create_entity("test_chunk", "Node1", engine.entity_null)
local node2 = engine.create_entity("test_chunk", "Node2", node1)
local node3 = engine.create_entity("test_chunk", "Node3", node2)
--
engine.registry:emplace(node1, Transform(0.0, 0.0, 0.0))
engine.registry:emplace(node2, Transform(1.0, 0.0, 0.0))
engine.registry:emplace(node3, Transform(1.0, 0.0, 0.0))
--
local quadgrid1 = QuadGridComponent(1, 1, true)
quadgrid1:set_quad_at(0, 0.0, 0.0, 0.7, 0xffffff00, true)
engine.registry:emplace(node1, quadgrid1)
local quadgrid2 = QuadGridComponent(1, 1, true)
quadgrid2:set_quad_at(0, 0.0, 0.0, 0.5, 0xffff00ff, true)
engine.registry:emplace(node2, quadgrid2)
local quadgrid3 = QuadGridComponent(1, 1, true)
quadgrid3:set_quad_at(0, 0.0, 0.0, 0.3, 0xff00ffff, true)
engine.registry:emplace(node3, quadgrid3)
]]

--[[
-- Game root entity
-- TODO: init() should be a behavior, so this and other 'global' entities can be removed from the SG in destroy()
self.game_entity = engine.create_entity("game_chunk", "GameRoot", engine.entity_null) -- engine.registry:create() -- global for now so it's reachable to phases
-- Header
--engine.registry:emplace(self.game_entity, HeaderComponent("GameRoot"))
-- Transform
engine.registry:emplace(self.game_entity, Transform(0.0, 0.0, 0.0))
]]

--[[
-- Projectile pool
engine.log("Creating projectile pool...")
self.projectilepool_entity = self:create_projectile_pool_entity(self.game_entity)
local projectilepool_table = engine.get_script_by_entity(engine.registry, "projectile_pool_behavior", self.projectilepool_entity)
]]

--[[
-- Create player(s)
engine.log("Creating player...")
self.player_entity = self:create_player_entity(0.5, 0xffffffff, projectilepool_table, self.game_entity)
]]

--[[
engine.log("Creating phases...")
self.phasemanager_entity = self:create_phasemanager_entity(self.game_entity)
]]

--[[
(game_init.lua)
Game entity
]]
local game_entity = engine.create_entity("game_chunk", "Game", engine.entity_null)
local game_behavior = engine.add_script(engine.registry, game_entity, "game_behavior")

--[[
(game_init.lua)
-- or import_chunk("game_chunk"), if these entities were edited & saved in the UI
]]
dofile(scripts_path .. "player_spawner.lua")
dofile(scripts_path .. "projectilepool_spawner.lua")

engine.log("Lua init")