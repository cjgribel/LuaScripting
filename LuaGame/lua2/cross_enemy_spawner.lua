
function bouncing_enemy_cross(chunk_tag, color)

    local entity = engine.create_entity(chunk_tag, "CrossEnemy", engine.entity_null)
    print("bouncing_enemy_cross(): created entity ID:", entity)
    
    -- Header
    --engine.registry:emplace(entity, HeaderComponent("CrossEnemy"))

    -- Transform
    engine.registry:emplace(entity, Transform(0.0, 0.0, 0.0))

    --local size = 0.5 + math.random() * 0.0
    --engine.registry:emplace(entity, QuadComponent(size, random_color(), true))
    
    --engine.registry:emplace(entity, CircleColliderComponent(size * 0.5, true))
    
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
    local datagrid = DataGridComponent(W, H)

    local center_x = math.floor(W / 2)
    local center_y = math.floor(H / 2)
    
    for i = 0, W - 1 do
        for j = 0, H - 1 do
            local x = (i - (W - 1) / 2) * D
            local y = (j - (H - 1) / 2) * D
            local index = j * W + i
            if i == center_x or j == center_y then
                if i == core_x and j == core_y then
                    quadgrid:set_quad_at(index, x, y, D, 0xff0000ff, visible)
                    datagrid:set_slot1_at(index, 1.0)
                else
                    quadgrid:set_quad_at(index, x, y, D, color, visible)
                    datagrid:set_slot1_at(index, 2.0)
                end
                collidergrid:set_circle_at(index, x, y, D * 0.5, visible)
            end
        end
    end
    
    engine.registry:emplace(entity, quadgrid)
    engine.registry:emplace(entity, collidergrid)
    engine.registry:emplace(entity, datagrid)

    -- Island finder component
    engine.registry:emplace(entity, IslandFinderComponent(core_x, core_y))

    -- Bounce behavior
    engine.add_script(engine.registry, entity, "bounce_behavior")

    return entity

end

local entity = bouncing_enemy_cross("level1", 0xffffffff)

return entity