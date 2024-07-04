local view = registry:runtime_view(Transform)
assert(view:size_hint() > 0)

local koopa = registry:create()
registry:emplace(koopa, Transform(100, -200))
transform = registry:get(koopa, Transform)
print('Koopa position = ' .. tostring(transform))

assert(view:size_hint() == 3)

view:each(function(entity)
  print('Remove Transform from entity: ' .. entity)
  registry:remove(entity, Transform)
end)

-- size_hint might not be zero just because Transform was removed
--assert(view:size_hint() == 0)
