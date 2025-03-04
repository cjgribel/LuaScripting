local node = {
    velocity = {x = 1.0, y = 0.0},
    --is_active = false,
    projectile_pool = nil
}

function node:init()
	print('projectile_behavior [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)

    if not self.projectile_pool:is_active(self.id()) then
        return
    end

    --print('projectile_behavior update')
	local transform = self.owner:get(self.id(), Transform)

    -- Update transform
    transform.x = transform.x + self.velocity.x * dt
    transform.y = transform.y + self.velocity.y * dt

    -- Despawn if out of bounds
    --if transform.x < game.config.bounds.left or transform.x > game.config.bounds.right or transform.y < game.config.bounds.bottom or transform.y > game.config.bounds.top then
    --    self.projectile_pool:release(self.id())
    --end
    if game.config:is_out_of_bounds(transform.x, transform.y) then
        self.projectile_pool:release(self.id())
    end

    -- Clamp considering the radius
    --transform.x = math.max(self.MIN_BOUND + radius, math.min(transform.x, self.MAX_BOUND - radius))
    --transform.y = math.max(self.MIN_BOUND + radius, math.min(transform.y, self.MAX_BOUND - radius))
end

-- (nx, ny) points away from this entity
function node:on_collision(x, y, nx, ny, collider_index, entity)

    if not self.projectile_pool:is_active(self.id()) then
        return
    end
    
    -- Deactivate if collided with bouncy entity
    local bounceBehavior = engine.get_script_by_entity(self.owner, "bounce_behavior", entity)
    if bounceBehavior then
        self.projectile_pool:release(self.id())

    end
    --end
end

function node:destroy()
	print('projectile:destroy() #' .. self.id())
end

return node
