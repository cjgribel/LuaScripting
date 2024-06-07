--local input = require("input")
nodes = nodes or {}

local node = {}

function node:init()
    self.index = #nodes + 1
    nodes[self.index] = self

	print('node [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)
	local transform = self.owner:get(self.id(), Transform)
	transform.x = transform.x + 1
	print('node [#' .. self.id() .. '] update()', transform)

    -- Input test
    if input.button_pressed then
        print('Transform.x before:', transform.x)
        transform.x = transform.x + input.x * dt
        print('Transform.x after:', transform.x)
        transform.y = transform.y + input.y * dt
    end
    print('Input type:', type(input.x), type(input.y))

    -- Example: Access another enemy instance
    if self.index > 1 then
        local other_node = nodes[self.index - 1]
        print(' Interacting with node [#' .. other_node.id() .. ']')
    end

    -- Interact with all other nodes
    for i, node in ipairs(nodes) do
        if i ~= self.index then
            -- Example interaction: print each node's ID
            print('  Interacting with node [#' .. node.id() .. ']')
            -- Example interaction: Adjust their Transform
            local other_transform = self.owner:get(node.id(), Transform)
            other_transform.x = other_transform.x - 1
        end
    end
end

function node:destroy()
	print('bye, bye! from: node #' .. self.id())

    nodes[self.index] = nil  -- Remove from the global table
end

return node