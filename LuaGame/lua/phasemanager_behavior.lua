-- Adjust the package path to include the "../../LuaGame/lua" directory
package.path = package.path .. ";../../LuaGame/lua/?.lua"

-- Require the phase modules
local phase1 = require("phase1")
local phase2 = require("phase2")

local behavior = {
    current_phase = nil,
    phases = {phase1, phase2}, -- Add your phases here
    current_phase_index = 1,
}

function behavior:init()
    print('phase_behavior [#' .. self.id() .. '] init ()', self)
    self:load_phase(self.phases[self.current_phase_index])
end

function behavior:load_phase(phase)
    if self.current_phase then
        self.current_phase:destroy()
    end
    self.current_phase = phase
    if self.current_phase then
        self.current_phase:init()
    end
end

function behavior:update(dt)
    if self.current_phase then
        self.current_phase:update(dt)
        if self.current_phase:has_finished() then
            self.current_phase_index = self.current_phase_index + 1
            if self.phases[self.current_phase_index] then
                self:load_phase(self.phases[self.current_phase_index])
            else
                print('All phases completed.')
                self.current_phase:destroy()
                self.current_phase = nil
            end
        end
    end

    ImGui_SetNextWindowWorldPos(5, 7)
    ImGui_Begin("PhaseName")
    if self.current_phase then
        ImGui_Text(self.current_phase.name)
    else
        ImGui_Text('All phases done')
    end
    ImGui_End()
end

function behavior:on_collision(x, y, nx, ny, collider_index, entity)
    -- Delegate collision handling to the current phase if needed
end

function behavior:destroy()
    if self.current_phase then
        --self.current_phase:destroy()
    end
end

return behavior
