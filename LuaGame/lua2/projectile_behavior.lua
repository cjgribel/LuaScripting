local projectile_behavior = {
    velocity = {x = 1.0, y = 0.0},
    --is_active = false,
    projectile_pool = nil
}

function projectile_behavior:init()
    print("projectile_behavior:init ()", self)
	--print('projectile_behavior [#' .. self.id() .. '] init ()', self)
end

function projectile_behavior:destroy()
	print('projectile_behavior:destroy() #' .. self.id())
end

function projectile_behavior:run()
	print('projectile_behavior [#' .. self.id() .. '] run ()', self)

    -- fetch projectile_pool
    self.projectile_pool = engine.get_script_by_entity_name("projectile_pool_behavior", "ProjectilePool")
    if self.projectile_pool then
        engine.log("projectile_behavior detected ProjectilePool, poolSize = " .. self.projectile_pool.poolSize)
    else
        engine.log("projectile_behavior did not detect ProjectilePool")
    end
end

function projectile_behavior:stop()
	print('projectile_behavior [#' .. self.id() .. '] stop ()', self)

    -- ...
end

function projectile_behavior:update(dt)

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
function projectile_behavior:on_collision(x, y, nx, ny, collider_index, entity)

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

projectile_behavior:init()
return projectile_behavior
