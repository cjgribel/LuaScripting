local node = {
    MIN_BOUND = -5.0,
    MAX_BOUND = 5.0,
    projectile_pool = nil
}

function node:init()
	print('player_behavior [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)
	local transform = self.owner:get(self.id(), Transform)
    local quad = self.owner:get(self.id(), QuadComponent)
    local radius = quad.w / 2

    -- Apply input to transform
    transform.x = transform.x + input.x * dt * 10.0
    transform.y = transform.y - input.y * dt * 10.0

    -- Clamp considering the radius
    transform.x = math.max(self.MIN_BOUND + radius, math.min(transform.x, self.MAX_BOUND - radius))
    transform.y = math.max(self.MIN_BOUND + radius, math.min(transform.y, self.MAX_BOUND - radius))

    if input.button_pressed then
        --print('button pressed')

        local transform = self.owner:get(self.id(), Transform)
        self.projectile_pool:fire(transform.x, transform.y, -1.0 + 2.0*math.random(), -1.0 + 2.0*math.random())
    end
end

-- (nx, ny) points away from this entity
function node:on_collision(x, y, nx, ny, entity)

end

function node:destroy()
    print('player_behavior [#' .. self.id() .. '] destroy()', self)
end

return node
