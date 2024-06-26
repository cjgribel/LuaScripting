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
        registry:emplace(entity, QuadComponent(size, 0xff00ffff, false))
        registry:emplace(entity, CircleColliderComponent(size * 0.75, false))

        print('Adding projectile_behavior to: ', entity)
        local projectile_behavior = add_script(registry, entity, dofile("../../SpaceInvaders/lua/projectile_behavior.lua"), "projectile_behavior")
        projectile_behavior.projectile_pool = self

        table.insert(self.pool, { entity = entity })
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

    --self:fire(-5.0 + 10.0*math.random(), -5.0 + 10.0*math.random(), 0.0, 0.0)
end

-- (nx, ny) points away from this entity
function ProjectilePool:on_collision(x, y, nx, ny, entity)

end

function ProjectilePool:destroy()

end

-- Get an inactive projectile from the pool
function ProjectilePool:get()

    --print(self.activeCount)

    if self.activeCount < self.poolSize then
        self.activeCount = self.activeCount + 1
        local projectile = self.pool[self.activeCount]
        
        --projectile.active = true

        self:activate_entity(projectile.entity, true)
        local circle_collider = self.owner:get(projectile.entity, CircleColliderComponent)
        local quad = self.owner:get(projectile.entity, QuadComponent)
        circle_collider.is_active, quad.is_visible = true, true

        --print(self.activeCount)
        return projectile.entity
    end
    return nil -- Pool exhausted
end

function ProjectilePool:is_active(entity)
    local index = self.entityToIndex[entity]
    --return self.pool[index].active
    return index <= self.activeCount
end

function ProjectilePool:activate_entity(entity, is_active)
    self.owner:get(entity, CircleColliderComponent).is_active = is_active
    self.owner:get(entity, QuadComponent).is_visible = is_active
end

-- Return a projectile to the pool
function ProjectilePool:release(entity)

    --local circle_collider = self.owner:get(entity, CircleColliderComponent)
    --local quad = self.owner:get(entity, QuadComponent)
    --circle_collider.is_active, quad.is_visible = false, false

    --local projectileBehavior = get_script(self.owner, entity, "projectile_behavior")
    --if projectileBehavior then
        --projectileBehavior.velocity.x, projectileBehavior.velocity.y = dx, dy
    --    projectileBehavior.is_active = false;
        -- Interact with the scoreBehavior script
        --print('Other entity has bounce_behavior:', self.velocity.x, bounceBehavior.velocity.x)
    --end

    local index = self.entityToIndex[entity]
   
    -- Check if entity is already inactive
    if index > self.activeCount then
        return
    end

    local projectile = self.pool[index]
    --projectile.active = false
    
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
    self:activate_entity(entity, false)

    --print(self.activeCount)
    --print('ProjectilePool:release', self.activeCount)
end

-- Fire a projectile from a given position with a given velocity
function ProjectilePool:fire(x, y, dx, dy)

    --print('hello')

    local entity = self:get()
    if entity then
        local transform = self.owner:get(entity, Transform)
        --local velocity = registry:get(entity, "Velocity")
        transform.x, transform.y = x, y
        --velocity.dx, velocity.dy = dx, dy

        -- Set velocity too
        local projectileBehavior = get_script(self.owner, entity, "projectile_behavior")
        if projectileBehavior then
            projectileBehavior.velocity.x, projectileBehavior.velocity.y = dx, dy
            --projectileBehavior.is_active = true;
            -- Interact with the scoreBehavior script
            --print('Other entity has bounce_behavior:', self.velocity.x, bounceBehavior.velocity.x)
        end
    end
end

return ProjectilePool