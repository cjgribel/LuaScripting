#ifndef CHUNKREGISTRY_HPP
#define CHUNKREGISTRY_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cassert>
#include <entt/entt.hpp> // Ensure this is included for entt::entity

// namespace eeng {

class ChunkRegistry
{
public:
    using EntityVector = std::vector<entt::entity>;

    // Adapter for chunks
    class ChunkIterator
    {
    public:
        using MapType = std::unordered_map<std::string, EntityVector>;
        using iterator = MapType::iterator;

        ChunkIterator(MapType& registry);

        iterator begin();
        iterator end();

    private:
        MapType& registry;
    };

    // Adapter for entities in a chunk
    class EntityIterator
    {
    public:
        using iterator = EntityVector::iterator;

        EntityIterator(EntityVector& entities);

        iterator begin();
        iterator end();

    private:
        EntityVector& entities;
    };

    bool chunk_exists(const std::string& chunk_id) const;

    EntityIterator chunk(const std::string& chunk_id);

    ChunkIterator chunks();

    void addEntity(const std::string& chunk_id, entt::entity entity);

    void removeEntity(const std::string& chunk_id, entt::entity entity);

    void removeEntity(entt::entity entity);

    bool entity_exists_in_chunk(const std::string& chunk_id, entt::entity entity) const;

    bool entity_exists(entt::entity entity) const;

    void create_chunk(const std::string& chunk_id);

    void clear();

private:
    std::unordered_map<std::string, EntityVector> registry;
};

// } // namespace eeng

#endif // CHUNKREGISTRY_HPP
