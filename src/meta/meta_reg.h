//
//  meta_aux.h
//
//  Created by Carl Johan Gribel on 2024-08-08.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef meta_reg_h
#define meta_reg_h

#include <sol/forward.hpp>
// #include <entt/entt.hpp>
// #include <tuple>
// #include <utility>
// #include <cassert>

namespace internal {



} // internal

/// @brief 
/// @tparam Type 
/// @param lua 
template<class Type>
void register_meta(sol::state& lua)
{
    assert(0 && "Specialization not available for type");
}

#endif /* meta_reg */
