
function random_color()
    local alpha = 0x80
    local red = math.random(0, 255)
    local green = math.random(0, 255)
    local blue = math.random(0, 255)

    local color = (alpha << 24) | (red << 16) | (green << 8) | blue
    return color
end

-- Function to create and setup an entity
local function create_entity_with_components(index)
    local entity = registry:create()
    print("Created entity ID:", entity)
    
    -- Attach Transform, using the index in some way
    registry:emplace(entity, Transform(0.0, 0.0))
    
    -- Attach Quad
    registry:emplace(entity, QuadComponent(0.1 + math.random() * 0.5, random_color()))
    
    -- Attach behavior
    add_script(registry, entity, dofile("lua/bounce_behavior.lua"))
end

print('Lua init script...')

math.randomseed(os.time())

-- Create 5 entities
for i = 1, 25 do
    create_entity_with_components(i)
end

print('Lua init script done')
