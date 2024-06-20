-- Define the ProjectilePool
ProjectilePool = {
    pool = {},
    activeCount = 0,
    poolSize = 100
}

-- Initialize the pool
function ProjectilePool:init()
    for i = 1, self.poolSize do
        local entity = registry:create()
        registry:emplace(entity, Transform(0.0, 0.0))
        --registry:emplace(entity, "Velocity", { dx = 0, dy = 0 })
        table.insert(self.pool, { entity = entity, active = false })
    end
end

-- Update function for projectiles
function ProjectilePool:update(dt)
    for i = 1, ProjectilePool.activeCount do
        local projectile = ProjectilePool.pool[i]
        -- Update projectile logic here
        -- e.g., move the projectile, check for collisions, emit particles
    end
end

-- (nx, ny) points away from this entity
function ProjectilePool:on_collision(x, y, nx, ny, entity)

end

function ProjectilePool:destroy()

end

-- Get an inactive projectile from the pool
function ProjectilePool:get()
    if self.activeCount < self.poolSize then
        local projectile = self.pool[self.activeCount + 1]
        projectile.active = true
        self.activeCount = self.activeCount + 1
        return projectile.entity
    end
    return nil -- Pool exhausted
end

-- Return a projectile to the pool using back-swapping
function ProjectilePool:release(entity)
    for i = 1, self.activeCount do
        if self.pool[i].entity == entity then
            self.pool[i].active = false
            if i ~= self.activeCount then
                self.pool[i], self.pool[self.activeCount] = self.pool[self.activeCount], self.pool[i]
            end
            self.activeCount = self.activeCount - 1
            break
        end
    end
end

-- Fire a projectile from a given position with a given velocity
function ProjectilePool:fire(x, y, dx, dy)
    local entity = self:get()
    if entity then
        local transform = registry:get(entity, "Transform")
        --local velocity = registry:get(entity, "Velocity")
        transform.x, transform.y = x, y
        --velocity.dx, velocity.dy = dx, dy
    end
end

return ProjectilePool