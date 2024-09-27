//
//  meta_aux.h
//
//  Created by Carl Johan Gribel on 2024-08-08.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef any_aux_h
#define any_aux_h

#include <entt/entt.hpp>
#include <tuple>
#include <utility>
#include <cassert>

namespace internal {

    template<typename T, typename Callable>
    bool try_apply(entt::meta_any& value, Callable callable)
    {
        if (T* ptr = value.try_cast<T>(); ptr) {
            callable(*ptr);
            return true;
        }
        if (const T* ptr = value.try_cast<const T>(); ptr) {
            callable(*ptr);
            return true;
        }
        return false;
    }

    template<typename Callable, typename... Types>
    bool any_apply_impl(entt::meta_any& value, Callable callable, std::tuple<Types...>)
    {
        bool result = false;
        (... || (result = try_apply<Types>(value, callable)));
        return result;
    }

    using TryCastTypes = std::tuple<
        bool,
        char, unsigned char,
        short, unsigned short,
        int, unsigned int,
        long, unsigned long,
        long long, unsigned long long,
        float, double, long double
    >;

} // internal

template<typename Callable>
bool try_apply(entt::meta_any& value, Callable callable)
{
    return internal::any_apply_impl(value, callable, internal::TryCastTypes{});
}

/// @brief Get inspector-friendly name of a meta_type
        /// @param meta_type 
        /// @return Name provided as a display name property, or default name
inline auto meta_type_name(const entt::meta_type meta_type)
{
    assert(meta_type);

    // Use display name if present, assumed to be in C-string format
    //if (auto meta_type = entt::resolve(any.type().id()); meta_type)
    if (auto name_prop = meta_type.prop(display_name_hs); name_prop)
        if (auto name = name_prop.value().cast<const char*>(); name)
            return std::string(name);

    // Use the name generated by entt
    return std::string(meta_type.info().name());
}

/// @brief Get name of a meta data field
/// @param id Data field id, used as fallback
/// @param data Meta data field
/// @return Name provided as a display name property, or string generated from id
inline auto meta_data_name(const entt::id_type& id, const entt::meta_data& data)
{
    if (auto display_name_prop = data.prop(display_name_hs); display_name_prop)
    {
        auto name_ptr = display_name_prop.value().try_cast<const char*>();
        assert(name_ptr);
        return std::string(*name_ptr);
    }
    return std::to_string(id);
}

/// @brief Get the value of a meta_data property
/// @tparam Type Non-class property type
/// @tparam Default Value to be used if property does ot exist or cast to Type fails
/// @param data 
/// @param id 
/// @return The property value or the default value
template<class Type, Type Default>
inline Type get_meta_data_prop(const entt::meta_data& data, const entt::id_type& id)
{
    if (auto prop = data.prop(id); prop)
        if (auto ptr = prop.value().try_cast<Type>(); ptr)
            return *ptr;
    return Default;
}

// Return a copy of an enum value, converted to the underlying enum type
// meta_type: enum type
// any: enum instance
// Note: if any is not const, allow_cast will convert in-place and
//      return a bool instead of a new any
inline auto cast_to_underlying_type(const entt::meta_type& meta_type, const entt::meta_any& enum_any)
{
    auto underlying_type = meta_type.prop("underlying_meta_type"_hs).value().cast<entt::meta_type>();
    return enum_any.allow_cast(underlying_type);
};

/// @brief 
/// @param enum_any 
/// @return 
inline auto gather_meta_enum_entries(const entt::meta_any enum_any)
{
    entt::meta_type meta_type = entt::resolve(enum_any.type().id());
    assert(meta_type);
    assert(meta_type.is_enum());

    std::vector<std::pair<const std::string, const entt::meta_any>> entries;

    for (auto&& [id, meta_data] : meta_type.data())
    {
        // Name of data member
        auto display_name = meta_data.prop(display_name_hs).value().cast<const char*>();

        // Value of data member
        auto data_any = meta_data.get(enum_any);

        entries.push_back({
            std::string(display_name),
            cast_to_underlying_type(meta_type, data_any)
            });
    }
    return entries;
}

#endif /* meta_aux */
