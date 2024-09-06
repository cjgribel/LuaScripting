
-- Adjust the package path to include the "../../LuaGame/lua" directory
package.path = package.path .. ";../../LuaGame/lua/?.lua"

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

local function create_projectile_pool_entity(parent_entity)

    local entity = registry:create()
    --attach_entity_to_scenegraph(entity, "ProjectilePool", "root");

    -- SG
    scenegraph:add_entity(entity, parent_entity)

    -- Header
    registry:emplace(entity, HeaderComponent("ProjectilePool"))

    -- Behavior
    add_script(registry, entity, dofile("../../LuaGame/lua/projectile_pool_behavior.lua"), "projectile_pool_behavior")
    return entity
end

local function create_player_entity(size, color, projectile_pool, parent_entity)
    
    local entity = registry:create()

    -- SG
    scenegraph:add_entity(entity, parent_entity)

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

-- NOT USED YET
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

local function create_phasemanager_entity(parent_entity)

    local entity = registry:create()
    
    -- SG
    scenegraph:add_entity(entity, parent_entity)

    -- Header
    registry:emplace(entity, HeaderComponent("PhaseManager"))

    add_script(registry, entity, dofile("../../LuaGame/lua/phasemanager_behavior.lua"), "phasemanager_behavior")
    
    return entity
end

print('Lua init script...')

math.randomseed(os.time())

log("Creating config...")
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

-- Music and effects
-- TODO: Do load & unload per phase.
log("Loading music & effects...")
audio_manager:registerMusic(config.sounds.music_title, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Title Screen.wav")
audio_manager:registerMusic(config.sounds.music_end, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Ending.wav")
audio_manager:registerMusic(config.sounds.music_lvl1, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 1.wav")
audio_manager:registerMusic(config.sounds.music_lvl2, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 2.wav")
audio_manager:registerMusic(config.sounds.music_lvl3, "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Level 3.wav")

audio_manager:registerEffect(config.sounds.projectile_fire1, "../../assets/sounds/Misc Lasers/Fire 1.mp3")
audio_manager:registerEffect(config.sounds.player_death, "../../assets/sounds/Misc Lasers/Fire 2.mp3")
audio_manager:registerEffect(config.sounds.element_explode, "../../assets/sounds/Misc Lasers/Fire 2.mp3")
-- Volume
audio_manager:setMasterVolume(128)  -- Set master volume to 50%
audio_manager:setMusicVolume("music1", 64)  -- Set music volume to 25%
audio_manager:setEffectVolume(config.sounds.projectile_fire1, 32)
audio_manager:setEffectVolume(config.sounds.element_explode, 32)

-- Game root entity
-- TODO: init() should be a behavior, so this and other 'global' entities can be removed from the SG in destroy()
local game_entity = registry:create()
-- Header
registry:emplace(game_entity, HeaderComponent("GameRoot"))
-- SG
scenegraph:add_entity_as_root(game_entity)

-- Projectile pool
log("Creating projectile pool...")
local projectilepool_entity = create_projectile_pool_entity(game_entity)
local projectilepool_table = get_script(registry, projectilepool_entity, "projectile_pool_behavior")

-- Create player(s)
log("Creating player...")
local player_entity = create_player_entity(0.5, 0xffffffff, projectilepool_table, game_entity)

log("Creating phases...")
local phasemanager_entity = create_phasemanager_entity(game_entity)

log("Lua init done")
print('Lua init script done')

-- REMOVE FROM SG TEST
--print('Erasing player entity node from SG')
--scenegraph:erase_entity(player_entity)
-- -> will erase root & all remaining nodes
--print('Erasing game root entity node from SG')
--scenegraph:erase_entity(game_entity)