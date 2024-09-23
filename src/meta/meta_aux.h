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
