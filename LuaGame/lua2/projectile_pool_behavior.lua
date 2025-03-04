
--local prefabloaders = require("prefabs")
local spawn_projectile = require("projectile_spawner")

-- Define the ProjectilePool
ProjectilePool = {
    game = {},
    pool = {},
    entityToIndex = {},
    activeCount = 0,
    poolSize = 8
}

-- Initialize the pool
function ProjectilePool:init()
    print('ProjectilePool:init()', self)
end

function ProjectilePool:run()
    
    --print('ProjectilePool [#' .. self.id() .. '] run ()', self)
    
    -- Fetch game behavior
    self.game = engine.get_script_by_entity_name("game_behavior", "Game")

    -- Create projectiles
    for i = 1, self.poolSize do
        
        -- local entity = prefabloaders.projectile("game_chunk", 0.2, self)
        --local entity = prefabloaders.projectile("projectile_chunk", 0.2, self)
        local entity = spawn_projectile("projectile_chunk", 0.2, self)

        table.insert(self.pool, entity)
        self.entityToIndex[entity] = i

    end
    --engine.log("Pooled " .. self.poolSize .. " projectiles")

    --[[
    engine.log(string.format(
        'ProjectilePool:run() entity = %s, pool size = %s',
        self.id(),
        tostring(self.poolSize)
    ))
    ]]
    engine.log(string.format(
        'ProjectilePool:run() entity = %s, game = %s',
        self.id(),
        tostring(self.game)
    ))
end

-- ASSERT STOPPING NOT DONE MORE THAN ONE TIME CONSECUTIVELY

function ProjectilePool:stop()
	print('ProjectilePool [#' .. self.id() .. '] stop ()', self)

    -- Either destroy projectile entities one by one or destroy the chunk
    --[[
    engine.unload_chunk("projectile_chunk")
    ]]
    for _, entity in ipairs(self.pool) do
        engine.destroy_entity(entity)
    end

    self.pool = {}
    self.entityToIndex = {}
    self.activeCount = 0
end

-- Update function for projectiles
function ProjectilePool:update(dt)
    --print("ProjectilePool:update()", self)

    -- Projectiles have their own update()
    --[[
    for i = 1, self.activeCount do
        local projectile = self.pool[i]
        --print('ProjectilePool:update', projectile)
        -- Update projectile logic here
    end
    ]]

    --self:fire(-5.0 + 10.0*math.random(), -5.0 + 10.0*math.random(), 0.0, 0.0)
end

-- (nx, ny) points away from this entity
function ProjectilePool:on_collision(x, y, nx, ny, collider_index, entity)
    -- not used
end

function ProjectilePool:destroy()

    print("ProjectilePool:destroy() " .. self.id());
    --print('projectile_pool_behavior [#' .. self.id() .. '] destroy()', self)

    --[[
    for _, entity in ipairs(self.pool) do

        -- Flag for destruction
        engine.destroy_entity(entity)

    end
    ]]

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

    --local projectileBehavior = engine.get_script_by_entity(self.owner, "projectile_behavior", entity)
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
        --local velocity = engine.registry:get(entity, "Velocity")
        transform.x, transform.y = x, y
        --velocity.dx, velocity.dy = dx, dy

        -- Set velocity too
        local projectileBehavior = engine.get_script_by_entity(self.owner, "projectile_behavior", entity)
        if projectileBehavior then
            projectileBehavior.velocity.x, projectileBehavior.velocity.y = dx, dy
            --projectileBehavior.is_active = true;
            -- Interact with the scoreBehavior script
            --print('Other entity has bounce_behavior:', self.velocity.x, bounceBehavior.velocity.x)
        end

        -- Sound
        engine.audio:playEffect(self.game.config.sounds.projectile_fire1, 0)
    end
end

ProjectilePool:init()
return ProjectilePool