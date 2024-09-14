--local input = require("input")
nodes = nodes or {}

local node = {}

function node:init()
    self.index = #nodes + 1
    nodes[self.index] = self

    -- Stuff that might be done by an init script
    -- Add QuadComponent
    --self.owner:emplace(self.id(), QuadComponent(1.0))

    -- Test, add script to this entity
    --local script_table = dofile("lua/behavior.lua")
    --engine.add_script(self.owner, self.id(), script_table)

	print('node [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)
	local transform = self.owner:get(self.id(), Transform)
	--transform.x = transform.x + 1
	--print('node [#' .. self.id() .. '] update()', transform)

    -- Apply input to transform
    transform.x = transform.x + input.axis_left_x * dt * 10.0
    transform.y = transform.y - input.axis_left_y * dt * 10.0
    -- Clamp
    transform.x = math.max(-5.0, math.min(transform.x, 5.0))
    transform.y = math.max(-5.0, math.min(transform.y, 5.0))

    -- Input test
    if input.button_pressed then
        --transform.x = transform.x + input.x * dt
        --transform.y = transform.y + input.y * dt
    end
    --print('Input type:', type(input.x), type(input.y))

    -- Alter transform
    local theta = math.pi * dt
    local cos_theta = math.cos(theta)
    --local sin_theta = math.sin(theta)
    --transform.x = cos_theta * transform.x - sin_theta * transform.y
    --transform.y = sin_theta * transform.x + cos_theta * transform.y
    --transform.x = cos_theta
    --transform.y = cos_theta
    --print('dt ', dt)

    -- Example: Access another enemy instance
    if self.index > 1 then
        local other_node = nodes[self.index - 1]
        --print(' Interacting with node [#' .. other_node.id() .. ']')
    end

    -- Interact with all other nodes
    --for i, node in ipairs(nodes) do
    --    if i ~= self.index then
    --        -- Example interaction: print each node's ID
    --        --print('  Interacting with node [#' .. node.id() .. ']')
    --        -- Example interaction: Adjust their Transform
    --        local other_transform = self.owner:get(node.id(), Transform)
    --        --other_transform.x = other_transform.x - 1
    --    end
    --end
end

function node:on_collision(x, y, nx, ny, entity)
    print(x, y, nx, ny, entity)
end

function node:destroy()
	print('bye, bye! from: node #' .. self.id())

    nodes[self.index] = nil  -- Remove from the global table
end

return node