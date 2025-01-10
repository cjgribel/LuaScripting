/**
 * @file ChunkRegistry.hpp
 * @brief Provides functionality for managing entities within chunks.
 * @note Documentation may be AI-generated.
 */

#ifndef CHUNKREGISTRY_HPP
#define CHUNKREGISTRY_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cassert>
#include "Entity.hpp"

/**
 * @class ChunkRegistry
 * @brief Manages entities grouped by chunks, providing utility functions to manage and query these groups.
 * @note Documentation may be AI-generated.
 */
class ChunkRegistry
{
public:
    using EntityVector = std::vector<Entity>;

    /**
     * @class ChunkIterator
     * @brief Provides an iterator interface over chunks in the registry.
     * @note Documentation may be AI-generated.
     */
    class ChunkIterator
    {
    public:
        using MapType = std::unordered_map<std::string, EntityVector>;
        using iterator = MapType::iterator;

        /**
         * @brief Constructs a ChunkIterator.
         * @param registry Reference to the map of chunks.
         * @note Documentation may be AI-generated.
         */
        ChunkIterator(MapType& registry);

        /**
         * @brief Returns the beginning iterator of the chunks map.
         * @return Iterator to the first chunk.
         * @note Documentation may be AI-generated.
         */
        iterator begin();

        /**
         * @brief Returns the end iterator of the chunks map.
         * @return Iterator to the end of the chunks map.
         * @note Documentation may be AI-generated.
         */
        iterator end();

    private:
        MapType& registry; ///< Reference to the map of chunks.
    };

    /**
     * @class EntityIterator
     * @brief Provides an iterator interface over entities within a chunk.
     * @note Documentation may be AI-generated.
     */
    class EntityIterator
    {
    public:
        using iterator = EntityVector::iterator;

        /**
         * @brief Constructs an EntityIterator.
         * @param entities Reference to the vector of entities.
         * @note Documentation may be AI-generated.
         */
        EntityIterator(EntityVector& entities);

        /**
         * @brief Returns the beginning iterator of the entities vector.
         * @return Iterator to the first entity.
         * @note Documentation may be AI-generated.
         */
        iterator begin();

        /**
         * @brief Returns the end iterator of the entities vector.
         * @return Iterator to the end of the entities vector.
         * @note Documentation may be AI-generated.
         */
        iterator end();

    private:
        EntityVector& entities; ///< Reference to the vector of entities.
    };

    /**
     * @brief Checks if a chunk exists.
     * @param chunk_id ID of the chunk to check.
     * @return True if the chunk exists, false otherwise.
     * @note Documentation may be AI-generated.
     */
    bool chunk_exists(const std::string& chunk_id) const;

    /**
     * @brief Retrieves an EntityIterator for a given chunk.
     * @param chunk_id ID of the chunk.
     * @return EntityIterator for the specified chunk.
     * @note Documentation may be AI-generated.
     */
    EntityIterator chunk(const std::string& chunk_id);

    /**
     * @brief Retrieves a ChunkIterator for all chunks.
     * @return ChunkIterator for the registry.
     * @note Documentation may be AI-generated.
     */
    ChunkIterator chunks();

    /**
     * @brief Adds an entity to a chunk.
     * @param chunk_id ID of the chunk.
     * @param entity Entity to add.
     * @note Documentation may be AI-generated.
     */
    void add_entity(const std::string& chunk_id, const Entity& entity);

    /**
     * @brief Removes an entity from a specific chunk.
     * @param chunk_id ID of the chunk.
     * @param entity Entity to remove.
     * @note Documentation may be AI-generated.
     */
    void remove_entity(const std::string& chunk_id, const Entity& entity);

    /**
     * @brief Removes an entity from all chunks.
     * @param entity Entity to remove.
     * @note Documentation may be AI-generated.
     */
    void remove_entity(const Entity& entity);

    /**
     * @brief Reassigns an entity to a new chunk.
     * @param chunk_id ID of the new chunk.
     * @param entity Entity to reassign.
     * @note Documentation may be AI-generated.
     */
    void reassign_entity(const std::string& chunk_id, const Entity& entity);

    /**
     * @brief Finds the chunk containing a given entity.
     * @param entity Entity to search for.
     * @return Pair containing the chunk ID and a boolean indicating success.
     * @note Documentation may be AI-generated.
     */
    std::pair<std::string, bool> find_chunk(const Entity& entity) const;

    /**
     * @brief Checks if an entity exists in a specific chunk.
     * @param chunk_id ID of the chunk.
     * @param entity Entity to check.
     * @return True if the entity exists in the chunk, false otherwise.
     * @note Documentation may be AI-generated.
     */
    bool entity_exists_in_chunk(const std::string& chunk_id, const Entity& entity) const;

    /**
     * @brief Checks if an entity exists in any chunk.
     * @param entity Entity to check.
     * @return True if the entity exists, false otherwise.
     * @note Documentation may be AI-generated.
     */
    bool entity_exists(const Entity& entity) const;

    /**
     * @brief Creates a new chunk.
     * @param chunk_id ID of the chunk to create.
     * @note Documentation may be AI-generated.
     */
    void create_chunk(const std::string& chunk_id);

    /**
     * @brief Clears all chunks and entities.
     * @note Documentation may be AI-generated.
     */
    void clear();

private:
    std::unordered_map<std::string, EntityVector> registry; ///< Map of chunks to their entities.
};

#endif // CHUNKREGISTRY_HPP
