#include "ChunkRegistry.hpp"

// namespace eeng {

// ChunkIterator

ChunkRegistry::ChunkIterator::ChunkIterator(MapType& registry)
    : registry(registry)
{
}

ChunkRegistry::ChunkIterator::iterator ChunkRegistry::ChunkIterator::begin()
{
    return registry.begin();
}

ChunkRegistry::ChunkIterator::iterator ChunkRegistry::ChunkIterator::end()
{
    return registry.end();
}

// EntityIterator

ChunkRegistry::EntityIterator::EntityIterator(EntityVector& entities)
    : entities(entities)
{
}

ChunkRegistry::EntityIterator::iterator ChunkRegistry::EntityIterator::begin()
{
    return entities.begin();
}

ChunkRegistry::EntityIterator::iterator ChunkRegistry::EntityIterator::end()
{
    return entities.end();
}

// Member functions

bool ChunkRegistry::chunk_exists(const std::string& chunk_id) const
{
    return registry.find(chunk_id) != registry.end();
}

ChunkRegistry::EntityIterator ChunkRegistry::chunk(const std::string& chunk_id)
{
    assert(chunk_exists(chunk_id) && "Chunk does not exist!");
    return EntityIterator(registry.at(chunk_id));
}

ChunkRegistry::ChunkIterator ChunkRegistry::chunks()
{
    return ChunkIterator(registry);
}

void ChunkRegistry::addEntity(const std::string& chunk_id, entt::entity entity)
{
    assert(!entity_exists(entity) && "Entity already exists in a chunk!");

    registry[chunk_id].push_back(entity);
}

void ChunkRegistry::removeEntity(const std::string& chunk_id, entt::entity entity)
{
    assert(chunk_exists(chunk_id) && "Chunk does not exist!");

    auto& entities = registry.at(chunk_id);
    auto it = std::find(entities.begin(), entities.end(), entity);
    if (it != entities.end())
    {
        entities.erase(it);
    }
}

void ChunkRegistry::removeEntity(entt::entity entity)
{
    for (auto& [chunk_id, entities] : registry)
    {
        auto it = std::find(entities.begin(), entities.end(), entity);
        if (it != entities.end())
        {
            entities.erase(it);
            return;
        }
    }
}

bool ChunkRegistry::entity_exists_in_chunk(const std::string& chunk_id, entt::entity entity) const
{
    assert(chunk_exists(chunk_id) && "Chunk does not exist!");
    const auto& entities = registry.at(chunk_id);
    return std::find(entities.begin(), entities.end(), entity) != entities.end();
}

bool ChunkRegistry::entity_exists(entt::entity entity) const
{
    for (const auto& [chunk_id, entities] : registry)
    {
        if (std::find(entities.begin(), entities.end(), entity) != entities.end())
        {
            return true;
        }
    }
    return false;
}

void ChunkRegistry::create_chunk(const std::string& chunk_id)
{
    assert(!chunk_exists(chunk_id) && "Chunk already exists!");
    registry[chunk_id] = {};
}

void ChunkRegistry::clear()
{
    registry.clear();
}

// } // namespace eeng
