
local prefabloaders = require("prefabs")

-- Define the ProjectilePool
ProjectilePool = {
    pool = {},
    entityToIndex = {},
    activeCount = 0,
    poolSize = 128
}

-- Initialize the pool
function ProjectilePool:init()

    -- Create projectiles
    for i = 1, self.poolSize do

        local entity = prefabloaders.projectile(0.2, self)

        -- SG
        scenegraph:add_entity(entity, self.id())

        table.insert(self.pool, entity)
        self.entityToIndex[entity] = i

    end

    log("Pooled " .. self.poolSize .. " projectiles")
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
function ProjectilePool:on_collision(x, y, nx, ny, collider_index, entity)
    -- not used
end

function ProjectilePool:destroy()

    print('projectile_pool_behavior [#' .. self.id() .. '] destroy()', self)

    --for _, entity in ipairs(self.entities) do

        -- Remove from SG
        --scenegraph:remove_entity(entity)

        -- Flag for destruction
        --flag_entity_for_destruction(entity)

    --end

end

-- Get an inactive projectile from the pool
function ProjectilePool:get()

    --print(self.activeCount)

    if self.activeCount < self.poolSize then
        self.activeCount = self.activeCount + 1
        local projectile_entity = self.pool[self.activeCount]
        
        --projectile.active = true

        self:activate_entity(projectile_entity, true)
        --local circle_collider = self.owner:get(projectile.entity, CircleColliderComponent)
        --local quad = self.owner:get(projectile.entity, QuadComponent)
        --circle_collider.is_active, quad.is_visible = true, true

        --print(self.activeCount)
        return projectile_entity
    end
    return nil -- Pool exhausted
end

function ProjectilePool:is_active(entity)

    local index = self.entityToIndex[entity]
    --return self.pool[index].active
    return index <= self.activeCount

end

function ProjectilePool:activate_entity(entity, is_active)

    self.owner:get(entity, CircleColliderGridComponent).is_active = is_active
    self.owner:get(entity, QuadGridComponent).is_active = is_active

--    self.owner:get(entity, CircleColliderGridComponent):set_active_flag_all(is_active)
--    self.owner:get(entity, QuadGridComponent):set_active_flag_all(is_active)
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

    local projectile_entity = self.pool[index]
    --projectile.active = false
    
    if index ~= self.activeCount then
        -- Swap the inactive entity with the last active entity
        local lastActiveProjectile = self.pool[self.activeCount]
        
        -- Swap entities in the pool
        self.pool[index], self.pool[self.activeCount] = self.pool[self.activeCount], self.pool[index]
        
        -- Update the entityToIndex mapping
        self.entityToIndex[lastActiveProjectile] = index
        self.entityToIndex[projectile_entity] = self.activeCount
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

        -- Sound
        audio_manager:playEffect(config.sounds.projectile_fire1, 0)
    end
end

return ProjectilePool