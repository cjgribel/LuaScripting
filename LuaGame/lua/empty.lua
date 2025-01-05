
local testscript = {}

function testscript:init()
    engine.log("testscript:init()")
end

function testscript:update(dt)
end

function testscript:on_collision(x, y, nx, ny, element_index, entity)
end

function testscript:destroy()
    engine.log("testscript:destroy()")
end

return testscript
