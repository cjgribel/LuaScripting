print('Lua init')

-- Function to create and setup an entity
local function create_entity_with_components(index)
    local entity = registry:create()
    print("Created entity ID:", entity)
    -- Attach Transform, using the index in some way
    registry:emplace(entity, Transform(index, index))
    -- Attach Quad
    registry:emplace(entity, QuadComponent(2.0))
    -- Attach behavior
    --local script_table = dofile("lua/behavior.lua")
    add_script(registry, entity, dofile("lua/behavior.lua"))
end

-- Loop to create 5 entities
for i = 1, 5 do
    create_entity_with_components(i)
end

print('Lua init done')
