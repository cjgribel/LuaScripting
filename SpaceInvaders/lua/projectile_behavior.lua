local node = {
    velocity = {x = 1.0, y = 0.0},
    is_active = false,
    projectile_pool = nil
}

function node:init()
	print('projectile_behavior [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)
    --print('projectile_behavior update')
	local transform = self.owner:get(self.id(), Transform)

    -- Apply input to transform
    transform.x = transform.x + self.velocity.x * dt
    transform.y = transform.y + self.velocity.y * dt

    -- Clamp considering the radius
    --transform.x = math.max(self.MIN_BOUND + radius, math.min(transform.x, self.MAX_BOUND - radius))
    --transform.y = math.max(self.MIN_BOUND + radius, math.min(transform.y, self.MAX_BOUND - radius))
end

-- (nx, ny) points away from this entity
function node:on_collision(x, y, nx, ny, entity)
    -- Deactivate if hitting target, going outside of bounds etc
    
    -- Deactivate if collided with bouny entity
    --if self.is_active then
    local bounceBehavior = get_script(self.owner, entity, "bounce_behavior")
    if bounceBehavior then
        self.projectile_pool:release(self.id())

    end
    --end
end

function node:destroy()
	print('bye, bye! from: node #' .. self.id())
end

return node
