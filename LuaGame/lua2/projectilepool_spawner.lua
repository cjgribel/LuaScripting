

local entity = engine.create_entity("game_chunk", "ProjectilePool", engine.entity_null)

local projectilepool_behavior = engine.add_script(engine.registry, entity, "projectile_pool_behavior")

return entity

-- To fetch this table and use it to fire projectiles
--engine.log("Creating projectile pool...")
--self.projectilepool_entity = self:create_projectile_pool_entity(self.game_entity)
--local projectilepool_table = engine.get_script(engine.registry, self.projectilepool_entity, --"projectile_pool_behavior")