
-- Adjust the package path to include the "../../LuaGame/lua" directory
package.path = package.path .. ";../../LuaGame/lua/?.lua"

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

function game:init()

    print("game:init() called")

    math.randomseed(os.time())

    -- Game root entity
    -- TODO: init() should be a behavior, so this and other 'global' entities can be removed from the SG in destroy()
    self.game_entity = engine.create_entity(engine.entity_null) -- registry:create() -- global for now so it's reachable to phases
    -- Header
    registry:emplace(self.game_entity, HeaderComponent("GameRoot"))

    -- Projectile pool
    log("Creating projectile pool...")
    self.projectilepool_entity = self:create_projectile_pool_entity(self.game_entity)
    local projectilepool_table = get_script(registry, self.projectilepool_entity, "projectile_pool_behavior")

    -- Create player(s)
    log("Creating player...")
    self.player_entity = self:create_player_entity(0.5, 0xffffffff, projectilepool_table, self.game_entity)

    log("Creating phases...")
    self.phasemanager_entity = self:create_phasemanager_entity(self.game_entity)

    -- Music and effects (for all phases)
    log("Loading music & effects...")
    local sounds = game.config.sounds
    audio_manager:registerMusic(sounds.music_title, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Title Screen.wav")
    audio_manager:registerMusic(sounds.music_end, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Ending.wav")
    audio_manager:registerMusic(sounds.music_lvl1, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 1.wav")
    audio_manager:registerMusic(sounds.music_lvl2, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 2.wav")
    audio_manager:registerMusic(sounds.music_lvl3, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 3.wav")

    audio_manager:registerEffect(sounds.projectile_fire1, "../../assets/sounds/Misc Lasers/Fire 1.mp3")
    audio_manager:registerEffect(sounds.player_death, "../../assets/sounds/Misc Lasers/Fire 2.mp3")
    audio_manager:registerEffect(sounds.element_explode, "../../assets/sounds/Misc Lasers/Fire 2.mp3")
    -- Volume
    audio_manager:setMasterVolume(128)  -- Set master volume to 50%
    --audio_manager:setMusicVolume("music1", 64)  -- Set music volume to 25%
    audio_manager:setEffectVolume(sounds.projectile_fire1, 32)
    audio_manager:setEffectVolume(sounds.element_explode, 32)

end

function game:destroy()

    print("game:destroy() called")

    -- Destroy entities
    engine.destroy_entity(self.phasemanager_entity)
    engine.destroy_entity(self.player_entity)
    engine.destroy_entity(self.projectilepool_entity)
    engine.destroy_entity(self.game_entity)

end

-- remove local here and make it global to entire lua state?
local prefabloaders = require("prefabs")

PlayerCollisionBit = 0x1
EnemyCollisionBit = 0x2
ProjectileCollisionBit = 0x4

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

function game:create_projectile_pool_entity(parent_entity)

    --local entity = registry:create()
    local entity = engine.create_entity(parent_entity)

    -- Header
    registry:emplace(entity, HeaderComponent("ProjectilePool"))

    -- Behavior
    add_script(registry, entity, dofile("../../LuaGame/lua/projectile_pool_behavior.lua"), "projectile_pool_behavior")
    return entity
end

function game:create_player_entity(size, color, projectile_pool, parent_entity)
    
    local entity = engine.create_entity(parent_entity)

    -- Header
    registry:emplace(entity, HeaderComponent("Player"))

    -- Transform
    registry:emplace(entity, Transform(0.0, 0.0, 0.0))

    -- QuadGridComponent
    local quadgrid = QuadGridComponent(1, 1, true)
    quadgrid:set_quad_at(0, 0.0, 0.0, size, 0xffffffff, true)
    registry:emplace(entity, quadgrid)

    -- CircleColliderGridComponent
    local collidergrid = CircleColliderGridComponent(1, 1, true, PlayerCollisionBit, EnemyCollisionBit)
    collidergrid:set_circle_at(0, 0.0, 0.0, size * 0.5, true)
    registry:emplace(entity, collidergrid)

    -- Behavior
    local player_table = add_script(registry, entity, dofile("../../LuaGame/lua/player_behavior.lua"), "player_behavior")
    player_table.projectile_pool = projectile_pool

    return entity
end

-- NOT USED
local function create_background_entity(size, color)

    local entity = registry:create()
    --attach_entity_to_scenegraph(entity, "Background", "root");

    -- Transform
    -- TODO: non-uniform size
    registry:emplace(entity, Transform(0.0, 0.0, 0.0))

    -- QuadGridComponent
    local quadgrid = QuadGridComponent(1, 1, true)
    quadgrid:set_quad_at(0, 0.0, 0.0, size, 0x40ffffff, true)
    registry:emplace(entity, quadgrid)

    return entity
end

function game:create_phasemanager_entity(parent_entity)

    local entity = engine.create_entity(parent_entity)

    -- Header
    registry:emplace(entity, HeaderComponent("PhaseManager"))

    -- Behavior
    add_script(registry, entity, dofile("../../LuaGame/lua/phasemanager_behavior.lua"), "phasemanager_behavior")
    
    return entity
end
