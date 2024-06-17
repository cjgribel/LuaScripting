
-- Function to create and setup an entity
local function create_entity_with_components(index)
    local entity = registry:create()
    print("Created entity ID:", entity)
    
    -- Attach Transform, using the index in some way
    registry:emplace(entity, Transform(index, index))
    
    -- Attach Quad
    registry:emplace(entity, QuadComponent(0.5))
    
    -- Attach behavior
    add_script(registry, entity, dofile("lua/bounce_behavior.lua"))
end

print('Lua init script...')

-- Create 5 entities
for i = 1, 25 do
    create_entity_with_components(i)
end

print('Lua init script done')
