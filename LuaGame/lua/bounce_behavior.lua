local node = {
    --MIN_BOUND = -5.0,
    --MAX_BOUND = 5.0,
    VELOCITY_MIN = -5.0,
    VELOCITY_MAX = 5.0,
    STRING = "STRING",
    BOOL = true,
    NUMBER = 123.123,

    velocity = {
        x = 0.0, 
        y = 0.0, 
        angle = 0.0,

        meta = {
            x = {inspectable = true, serializable = true}, 
            y = {inspectable = true, serializable = true}, 
            angle = {inspectable = true, serializable = true}
        }
    },

    -- usertype tests: these types are defined as usertypes via sol
    HEADER = HeaderComponent("Hello123"),
    TRANSFORM = Transform(1.0, 2.0, 3.0),
    --QUADGRID = QuadGridComponent(1,1,false),

    meta = {
        VELOCITY_MIN = {inspectable = true, serializable = true},
        VELOCITY_MAX = {inspectable = true},

        velocity = {inspectable = true, serializable = true},

        HEADER = {inspectable = true, serializable = true}
    }
}

function node:init()

    -- Inspect an instance of usertype HeaderComponent
    --local component = HeaderComponent("test_component")
    --print("Inspecting HeaderComponent metatable:")
    --for k, v in pairs(getmetatable(component)) do
    --    print("Key:", k, "Type:", type(v))
    --end

    self.velocity.x = math.random() * (self.VELOCITY_MAX - self.VELOCITY_MIN) + self.VELOCITY_MIN
    self.velocity.y = math.random() * (self.VELOCITY_MAX - self.VELOCITY_MIN) + self.VELOCITY_MIN
    self.velocity.yangle = -math.pi * 0.5 + math.random() * math.pi

	print('bounce_behavior [#' .. self.id() .. '] init ()', self)
end

function node:update(dt)

    local collidergrid = self.owner:get(self.id(), CircleColliderGridComponent)
    if not collidergrid.is_active then
        return
    end

	local transform = self.owner:get(self.id(), Transform)
    --local quad = self.owner:get(self.id(), QuadComponent)
    local radius = 0.5 --quad.w / 2

    -- Apply velocity
    transform.x = transform.x + self.velocity.x * dt
    transform.y = transform.y + self.velocity.y * dt
    transform.angle = transform.angle + self.velocity.angle * dt

    -- Bounce at bounds
    if transform.x - radius <= game.config.bounds.left or transform.x + radius >= game.config.bounds.right then
        self.velocity.x = -self.velocity.x
    end
    if transform.y - radius <= game.config.bounds.bottom or transform.y + radius >= game.config.bounds.top then
        self.velocity.y = -self.velocity.y
    end

    -- Clamp to bounds
    transform.x = math.max(game.config.bounds.left + radius, math.min(transform.x, game.config.bounds.right - radius))
    transform.y = math.max(game.config.bounds.bottom + radius, math.min(transform.y, game.config.bounds.top - radius))

    -- Destroy detected islands
    -- Edit: Break one at a time with some frequency, rather than all at once
    local islandFinder = self.owner:get(self.id(), IslandFinderComponent)
    local nbr_islands = islandFinder:get_nbr_islands()
    if nbr_islands > 0 and math.random() < 0.5 then 
        for i = 0, nbr_islands-1 do
            local island_index = islandFinder:get_island_index_at(i)
            self:hit_element(island_index, 0.0, 0.0)
            break
        end
    end
    self:check_if_destroyed()
    --print(nbr_islands)
end

function node:destroy()

	print('bounce_behavior:destroy() #' .. self.id())

end

-- (nx, ny) points away from this entity
function node:on_collision(x, y, nx, ny, element_index, entity)

    -- Check script in the other entity
    --local bounceBehavior = engine.get_script(self.owner, entity, "bounce_behavior")
    --if bounceBehavior then
    --    -- Interact with the scoreBehavior script
    --    --print('Other entity has bounce_behavior:', self.velocity.x, bounceBehavior.velocity.x)
    --end

    -- Hit by projectile?
    local projectileBehavior = engine.get_script(self.owner, entity, "projectile_behavior")
    if projectileBehavior then

        -- Hit by projectile

        -- Obtain current armor
        --local datagrid = self.owner:get(self.id(), DataGridComponent)
        --local element_armor = datagrid:get_slot1_at(element_index)
        --element_armor = element_armor - 1.0
        --datagrid:set_slot1_at(element_index, element_armor)

        -- Alter element color
        --local quadgrid = self.owner:get(self.id(), QuadGridComponent)
        --local new_color = self:scale_alpha(quadgrid:get_color_at(element_index), element_armor, 2.0)
        --quadgrid:set_color_at(element_index, new_color)

        --if element_armor == 0 then
        self:hit_element(element_index, -projectileBehavior.velocity.x, -projectileBehavior.velocity.y)
        self:check_if_destroyed()
        --end
    end
end

function node:hit_element(element_index, vel_x, vel_y)

    local transform = self.owner:get(self.id(), Transform)
    local collider = self.owner:get(self.id(), CircleColliderGridComponent)
    local quadgrid = self.owner:get(self.id(), QuadGridComponent)
    
    -- Emit particles in the (vel_x, vel_y) direction
    local x, y = quadgrid:get_pos_at(element_index)
    x_angle, y_angle = rotate(x, y, transform.angle)
    engine.emit_explosion(
        transform.x + x_angle, 
        transform.y + y_angle, 
        vel_x, 
        vel_y, 
        20, 
        quadgrid:get_color_at(element_index))

    -- Reduce armor
    local datagrid = self.owner:get(self.id(), DataGridComponent)
    local element_armor = datagrid:get_slot1_at(element_index)
    element_armor = element_armor - 1.0
    datagrid:set_slot1_at(element_index, element_armor)

    -- Update element color based on current armor
    --local quadgrid = self.owner:get(self.id(), QuadGridComponent)
    local new_color = self:scale_alpha(quadgrid:get_color_at(element_index), element_armor, 2.0)
    quadgrid:set_color_at(element_index, new_color)

    -- Deactivate element if armor is exhausted
    if element_armor == 0 then
        collider:set_active_flag_at(element_index, false)
        quadgrid:set_active_flag_at(element_index, false)
    end

    -- Sound
    engine.audio:playEffect(game.config.sounds.element_explode, 1)
end

function node:check_if_destroyed()

    local collider = self.owner:get(self.id(), CircleColliderGridComponent)
    if (not collider:is_any_active()) then

        -- Object is destroyed (has no active element left)
        
        --local transform = self.owner:get(self.id(), Transform)
        local quadgrid = self.owner:get(self.id(), QuadGridComponent)
        local collidergrid = self.owner:get(self.id(), CircleColliderGridComponent)

        -- TODO set is_active = false for quadgrid & collidergrid
        quadgrid.is_active = false
        collidergrid.is_active = false

        -- Activate all quads and colliders
    --    collider:set_active_flag_all(true)
    --    quad:set_active_flag_all(true)
        -- Random transform
    --    transform.x = math.random() * (game.config.bounds.right - game.config.bounds.left) + game.config.bounds.left
    --    transform.y = math.random() * (game.config.bounds.top - game.config.bounds.bottom) + game.config.bounds.bottom
        -- Random velocity
    --    self.velocity.x = math.random() * (self.VELOCITY_MAX - self.VELOCITY_MIN) + self.VELOCITY_MIN
    --    self.velocity.y = math.random() * (self.VELOCITY_MAX - self.VELOCITY_MIN) + self.VELOCITY_MIN
        -- Random color
        --quad:set_color_all(random_color())

        -- Kill counter
        game.config.enemy_kill_count = game.config.enemy_kill_count + 1

    end

end

function node:scale_alpha(color, f, fmax)
    -- Extract the RGBA components from the color
    local a = (color >> 24) & 0xFF
    local r = (color >> 16) & 0xFF
    local g = (color >> 8) & 0xFF
    local b = color & 0xFF

    -- Calculate the scaling factor
    local scale = f / fmax

    -- Scale the alpha component
    local a_scaled = math.floor(a * scale)

    -- Recompose the scaled alpha with the unchanged RGB components into a single color value
    local scaled_color = (a_scaled << 24) | (r << 16) | (g << 8) | b

    return scaled_color
end

return node
