local node = {
    --MIN_BOUND = -5.0,
    --MAX_BOUND = 5.0,
    fire_cooldown = 0.1,
    fire_delay = 0.0,
    projectile_pool = nil,
    projectiles_fired = 0
}

function node:init()
	print('player_behavior [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)
	local transform = self.owner:get(self.id(), Transform)
    local quad = self.owner:get(self.id(), QuadComponent)
    local radius = quad.w / 2

    -- Apply input to transform
    transform.x = transform.x + input.axis_left_x * dt * 10.0
    transform.y = transform.y - input.axis_left_y * dt * 10.0

    -- Clamp to bounds
    transform.x = math.max(config.bounds.left + radius, math.min(transform.x, config.bounds.right - radius))
    transform.y = math.max(config.bounds.bottom + radius, math.min(transform.y, config.bounds.top - radius))

    --if input.button_x then
        --print('button pressed')

    --    local transform = self.owner:get(self.id(), Transform)
    --    self.projectile_pool:fire(transform.x, transform.y, -1.0 + 2.0*math.random(), -1.0 + 2.0*math.random())
    --end

    if self.fire_delay > self.fire_cooldown then
        local axis_right_len = math.sqrt(input.axis_right_x * input.axis_right_x + input.axis_right_y * input.axis_right_y)
        if axis_right_len > 0.5 then
            local transform = self.owner:get(self.id(), Transform)
            local dir_x = input.axis_right_x/axis_right_len
            local dir_y = input.axis_right_y/axis_right_len
            self.projectile_pool:fire(transform.x, transform.y, dir_x * 12.0, dir_y * -12.0)

            self.projectiles_fired = self.projectiles_fired + 1
            self.fire_delay = 0.0
        end
    end
    self.fire_delay = self.fire_delay + dt
    --print(self.fire_delay, self.fire_cooldown)

    ImGui_SetNextWindowPos(500, 100)
    ImGui_Begin("ProjectileCount")
    ImGui_Text('Projectiles fired ' .. tostring(self.projectiles_fired))
    ImGui_End()

    ImGui_SetNextWindowPos(-5, 5)
    ImGui_Begin("ProjectileCount2")
    ImGui_Text('WORLD Projectiles fired ' .. tostring(self.projectiles_fired))
    ImGui_End()
end

-- (nx, ny) points away from this entity
function node:on_collision(x, y, nx, ny, entity)

end

function node:destroy()
    print('player_behavior [#' .. self.id() .. '] destroy()', self)
end

return node
