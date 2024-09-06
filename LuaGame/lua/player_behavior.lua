local node = {
    fire_cooldown = 0.1,
    fire_delay = 0.0,

    projectile_pool = nil,
    projectiles_fired = 0,

    time = 0.0,
    death_cooldown_time = 1.0,
    last_death_time = 0.0,
    invincible = true,

    blink = {
        delay = 0.1,
        next_blink_time = 0.0,
        flag = false
    }
}

function node:update_blink(time)

    if time > self.blink.next_blink_time then
        self.blink.flag = not self.blink.flag
        self.blink.next_blink_time = time + self.blink.delay
    end
end

function node:init()
	print('player_behavior [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)

	local transform = self.owner:get(self.id(), Transform)
    local quad = self.owner:get(self.id(), QuadGridComponent)
    local quad_color = quad:get_color_at(0) -- self.owner:get(self.id(), QuadGridComponent):get_color_at(0)
    local quad_r = 0.25 --TODO getter

    -- Apply input to transform
    transform.x = transform.x + input.axis_left_x * dt * config.player_speed
    transform.y = transform.y - input.axis_left_y * dt * config.player_speed

    -- Clamp to bounds
    transform.x = math.max(config.bounds.left + quad_r, math.min(transform.x, config.bounds.right - quad_r))
    transform.y = math.max(config.bounds.bottom + quad_r, math.min(transform.y, config.bounds.top - quad_r))

    --if input.button_x then
        --print('button pressed')

    --    local transform = self.owner:get(self.id(), Transform)
    --    self.projectile_pool:fire(transform.x, transform.y, -1.0 + 2.0*math.random(), -1.0 + 2.0*math.random())
    --end

    -- Firing
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

    -- Trail particles
    local axis_left_len = math.sqrt(input.axis_left_x * input.axis_left_x + input.axis_left_y * input.axis_left_y)
    if axis_left_len > 0.1 then
        emit_trail(transform.x, transform.y, -input.axis_left_x * 5.0, input.axis_left_y * 5.0, 2, quad_color)
    end

    -- Update time and invincibility
    self.time = self.time + dt
    self.invincible = (self.time < self.last_death_time + self.death_cooldown_time)
    
    -- Blink effect
    if self.invincible then
        self:update_blink(self.time)
        quad.is_active = self.blink.flag
    else
        quad.is_active = true
    end

    --ImGui_SetNextWindowWorldPos(-5, 5)
    ImGui_SetNextWindowWorldPos(-5, 7)
    ImGui_Begin("ProjectileCount2")
    ImGui_Text('Projectiles fired ' .. tostring(self.projectiles_fired))
    ImGui_Text('Targets destroyed ' .. tostring(config.enemy_kill_count))
    ImGui_Text('Deaths ' .. tostring(config.player_deaths))
    ImGui_End()
end

-- (nx, ny) points away from this entity
function node:on_collision(x, y, nx, ny, collider_index, entity)
    
    if self.invincible then 
        return 
    end

    -- Death
    local bounceBehavior = get_script(self.owner, entity, "bounce_behavior")
    if bounceBehavior then

        local transform = self.owner:get(self.id(), Transform)
        
        -- Particles
        emit_explosion(transform.x, transform.y, 0.0, 0.0, 80, 0xff0000ff)
        
        -- Reset
        --transform.x, transform.y = config.bounds.right, config.bounds.bottom
        
        -- Sound (death)
        --audio_manager:playEffect(config.sounds.player_death, 0)

        self.last_death_time = self.time
        config.player_deaths = config.player_deaths + 1

    end

end

function node:destroy()
    print('player_behavior [#' .. self.id() .. '] destroy()', self)
end

return node
