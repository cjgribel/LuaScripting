//
//  MetaClone.hpp
//  engine_core_2024
//
//  Created by Carl Johan Gribel on 2024-08-08.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef MetaClone_hpp
#define MetaClone_hpp

#include <entt/entt.hpp>

namespace Editor {
    
    /// @brief 
    /// @param registry 
    /// @param src_entity 
    /// @param dst_entity 
    void clone_entity(entt::registry& registry, entt::entity src_entity, entt::entity dst_entity);

}

#endif /* MetaSerialize_hpp */
