local prefabloaders = {}

function prefabloaders.projectile(size, projectile_pool)
    
        local entity = registry:create()

        -- Transform
        registry:emplace(entity, Transform(0.0, 0.0, 0.0))
        
        -- QuadGridComponent
        local qsc = QuadGridComponent(1, 1, false)
        qsc:set_quad_at(0, 0, 0.0, 0.0, size, 0xff00ffff, true)
        registry:emplace(entity, qsc)

        -- CircleColliderGridComponent
        local ccs = CircleColliderGridComponent(1, 1, false, ProjectileCollisionBit, EnemyCollisionBit)
        ccs:set_circle_at(0, 0, 0.0, 0.0, size * 0.75, true)
        registry:emplace(entity, ccs)

        -- Projectile_behavior
        local projectile_behavior = add_script(registry, entity, dofile("../../LuaGame/lua/projectile_behavior.lua"), "projectile_behavior")
        projectile_behavior.projectile_pool = projectile_pool

        print("prefabloaders:projectile(size, projectile_pool)")

        return entity
end

function prefabloaders.bouncing_enemy_block(color)

    local entity = registry:create()
    print("Created entity ID:", entity)
    
    registry:emplace(entity, Transform(0.0, 0.0, math.pi*0.5))

    --local size = 0.5 + math.random() * 0.0
    --registry:emplace(entity, QuadComponent(size, random_color(), true))
    
    --registry:emplace(entity, CircleColliderComponent(size * 0.5, true))
    
    local W = 7 -- Number of columns
    local H = 7 -- Number of rows
    local D = 0.25 -- Size of each quad (width/height)
    --local color = 0xffff00ff -- Color of the quads
    local visible = true -- Visibility flag
    local core_x = math.floor(W / 2)
    local core_y = math.floor(H / 2)
    
    -- QuadGridComponent & CircleColliderGridComponent
    local quadgrid = QuadGridComponent(W, H, true)
    local collidergrid = CircleColliderGridComponent(W, H, true, EnemyCollisionBit, PlayerCollisionBit | ProjectileCollisionBit)

    for i = 0, W - 1 do
        for j = 0, H - 1 do
            local x = (i - (W - 1) / 2) * D
            local y = (j - (H - 1) / 2) * D
            if i == core_x and j == core_y then
                quadgrid:set_quad_at(i, j, x, y, D, 0xff0000ff, visible)
            else
                quadgrid:set_quad_at(i, j, x, y, D, color, visible)
            end
            collidergrid:set_circle_at(i, j, x, y, D * 0.5, visible)
        end
    end

    registry:emplace(entity, quadgrid)
    registry:emplace(entity, collidergrid)

    -- Island finder component
    registry:emplace(entity, IslandFinderComponent(core_x, core_y))

    -- Bounce behavior
    add_script(registry, entity, dofile("../../LuaGame/lua/bounce_behavior.lua"), "bounce_behavior")

    return entity

end

function prefabloaders.bouncing_enemy_cross(color)

    local entity = registry:create()
    print("prefabloaders.bouncing_enemy_cross(): created entity ID:", entity)
    
    registry:emplace(entity, Transform(0.0, 0.0, 0.0))

    --local size = 0.5 + math.random() * 0.0
    --registry:emplace(entity, QuadComponent(size, random_color(), true))
    
    --registry:emplace(entity, CircleColliderComponent(size * 0.5, true))
    
    local W = 7 -- Number of columns
    local H = 7 -- Number of rows
    local D = 0.25 -- Size of each quad (width/height)
    --local color = 0xffff00ff -- Color of the quads
    local visible = true -- Visibility flag
    local core_x = math.floor(3)
    local core_y = math.floor(3)
    
    -- QuadGridComponent & CircleColliderGridComponent
    local quadgrid = QuadGridComponent(W, H, true)
    local collidergrid = CircleColliderGridComponent(W, H, true, EnemyCollisionBit, PlayerCollisionBit | ProjectileCollisionBit)

    local center_x = math.floor(W / 2)
    local center_y = math.floor(H / 2)
    
    for i = 0, W - 1 do
        for j = 0, H - 1 do
            local x = (i - (W - 1) / 2) * D
            local y = (j - (H - 1) / 2) * D
    
            if i == center_x or j == center_y then
                if i == core_x and j == core_y then
                    quadgrid:set_quad_at(i, j, x, y, D, 0xff0000ff, visible)
                else
                    quadgrid:set_quad_at(i, j, x, y, D, color, visible)
                end
                collidergrid:set_circle_at(i, j, x, y, D * 0.5, visible)
            end
        end
    end
    
    
    
    registry:emplace(entity, quadgrid)
    registry:emplace(entity, collidergrid)

    -- Island finder component
    registry:emplace(entity, IslandFinderComponent(core_x, core_y))

    -- Bounce behavior
    add_script(registry, entity, dofile("../../LuaGame/lua/bounce_behavior.lua"), "bounce_behavior")

    return entity

end

return prefabloaders