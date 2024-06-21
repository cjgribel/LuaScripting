-- Define the ProjectilePool
ProjectilePool = {
    pool = {},
    entityToIndex = {},
    activeCount = 0,
    poolSize = 100
}

-- Initialize the pool
function ProjectilePool:init()
    print('ProjectilePool:init() start')
    for i = 1, self.poolSize do
        local entity = registry:create()
        --print('Entity created:', entity)

        registry:emplace(entity, Transform(0.0, 0.0))
        --print('Transform component added to entity:', entity)
        --registry:emplace(entity, "Velocity", { dx = 0, dy = 0 })

        local size = 0.2
        registry:emplace(entity, QuadComponent(size, 0xffffffff, false))
        registry:emplace(entity, CircleColliderComponent(size * 0.5, false))
        local projectile_behavior = add_script(registry, entity, dofile("lua/projectile_behavior.lua"), "projectile_behavior")
        --projectile_behavior.projectile_pool = self

        table.insert(self.pool, { entity = entity, active = false })
        self.entityToIndex[entity] = i
    end
    print('ProjectilePool:init() ended')
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
        self.activeCount = self.activeCount + 1
        local projectile = self.pool[self.activeCount]
        
        projectile.active = true

        local circle_collider = self.owner:get(projectile.entity, CircleColliderComponent)
        local quad = self.owner:get(projectile.entity, QuadComponent)
        circle_collider.is_active, quad.is_visible = true, true

        return projectile.entity
    end
    return nil -- Pool exhausted
end

-- Return a projectile to the pool
function ProjectilePool:release(entity)
    local circle_collider = self.owner:get(entity, CircleColliderComponent)
    local quad = self.owner:get(entity, QuadComponent)
    circle_collider.is_active, quad.is_visible = false, false

    local index = self.entityToIndex[entity]
    local projectile = self.pool[index]
    
    projectile.active = false
    
    if index ~= self.activeCount then
        -- Swap the inactive entity with the last active entity
        local lastActiveProjectile = self.pool[self.activeCount]
        
        -- Swap entities in the pool
        self.pool[index], self.pool[self.activeCount] = self.pool[self.activeCount], self.pool[index]
        
        -- Update the entityToIndex mapping
        self.entityToIndex[lastActiveProjectile.entity] = index
        self.entityToIndex[projectile.entity] = self.activeCount
    end
    
    self.activeCount = self.activeCount - 1
end

-- Fire a projectile from a given position with a given velocity
function ProjectilePool:fire(x, y, dx, dy)
    local entity = self:get()
    if entity then
        local transform = self.owner:get(self.id(), Transform)
        --local velocity = registry:get(entity, "Velocity")
        transform.x, transform.y = x, y
        --velocity.dx, velocity.dy = dx, dy
    end
end

return ProjectilePool