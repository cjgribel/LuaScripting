local node = {
    --MIN_BOUND = -5.0,
    --MAX_BOUND = 5.0,
    VELOCITY_MIN = -5.0,
    VELOCITY_MAX = 5.0
}

function node:init()
    self.velocity = {
        x = math.random() * (self.VELOCITY_MAX - self.VELOCITY_MIN) + self.VELOCITY_MIN,
        y = math.random() * (self.VELOCITY_MAX - self.VELOCITY_MIN) + self.VELOCITY_MIN
    }

	print('bounce_behavior [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)
	local transform = self.owner:get(self.id(), Transform)
    local quad = self.owner:get(self.id(), QuadComponent)
    local radius = quad.w / 2

    -- Apply velocity
    transform.x = transform.x + self.velocity.x * dt
    transform.y = transform.y + self.velocity.y * dt

    -- Bounce at bounds
    if transform.x - radius <= config.bounds.left or transform.x + radius >= config.bounds.right then
        self.velocity.x = -self.velocity.x
    end
    if transform.y - radius <= config.bounds.bottom or transform.y + radius >= config.bounds.top then
        self.velocity.y = -self.velocity.y
    end

    -- Clamp to bounds
    transform.x = math.max(config.bounds.left + radius, math.min(transform.x, config.bounds.right - radius))
    transform.y = math.max(config.bounds.bottom + radius, math.min(transform.y, config.bounds.top - radius))
end

-- (nx, ny) points away from this entity
function node:on_collision(x, y, nx, ny, entity)
    local quad = self.owner:get(self.id(), QuadComponent)

    --local vel_length = math.sqrt(self.velocity.x * self.velocity.x + self.velocity.y * self.velocity.y)
    --emit_particle(x, y, nx * vel_length, ny * vel_length, quad_color)

    -- Check script in the other entity
    --local scriptComponent = self.owner:get(self.id(), ScriptedBehaviorComponent)
    --local bounceBehavior = scriptComponent:get_script_by_id("bounce_behavior")
    local bounceBehavior = get_script(self.owner, entity, "bounce_behavior")
    if bounceBehavior then
        -- Interact with the scoreBehavior script
        --print('Other entity has bounce_behavior:', self.velocity.x, bounceBehavior.velocity.x)
    end

    local projectileBehavior = get_script(self.owner, entity, "projectile_behavior")
    if projectileBehavior then
        local transform = self.owner:get(self.id(), Transform)
        -- Explosion
        emit_explosion(transform.x, transform.y, self.velocity.x, self.velocity.y, quad.color)
        
        -- Reset object
        --transform.x = -4.5
        --transform.y = 4.5
        transform.x = math.random() * (config.bounds.right - config.bounds.left) + config.bounds.left
        transform.y = math.random() * (config.bounds.top - config.bounds.bottom) + config.bounds.bottom
        self.velocity.x = math.random() * (self.VELOCITY_MAX - self.VELOCITY_MIN) + self.VELOCITY_MIN
        self.velocity.y = math.random() * (self.VELOCITY_MAX - self.VELOCITY_MIN) + self.VELOCITY_MIN
        quad.color = random_color()
    end
end

function node:destroy()
	print('bye, bye! from: node #' .. self.id())
end

return node
