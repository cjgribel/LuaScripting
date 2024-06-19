local node = {
    MIN_BOUND = -5.0,
    MAX_BOUND = 5.0,
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

    -- Apply input to transform
    transform.x = transform.x + self.velocity.x * dt
    transform.y = transform.y + self.velocity.y * dt

    -- Bounce off walls at MIN_BOUND and MAX_BOUND considering the radius
    if transform.x - radius <= self.MIN_BOUND or transform.x + radius >= self.MAX_BOUND then
        self.velocity.x = -self.velocity.x
    end

    if transform.y - radius <= self.MIN_BOUND or transform.y + radius >= self.MAX_BOUND then
        self.velocity.y = -self.velocity.y
    end

    -- Clamp considering the radius
    transform.x = math.max(self.MIN_BOUND + radius, math.min(transform.x, self.MAX_BOUND - radius))
    transform.y = math.max(self.MIN_BOUND + radius, math.min(transform.y, self.MAX_BOUND - radius))
end

function node:on_collision(x, y, nx, ny, entity)
    local quad = self.owner:get(self.id(), QuadComponent)
    local quad_color = quad.color

    local vel_length = math.sqrt(self.velocity.x * self.velocity.x + self.velocity.y * self.velocity.y)
    emit_particle(x, y, -ny * vel_length, nx * vel_length, quad_color)
    --print(x, y)
end

function node:destroy()
	print('bye, bye! from: node #' .. self.id())
end

return node
