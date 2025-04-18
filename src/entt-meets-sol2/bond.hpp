#ifndef bond_hpp
#define bond_hpp
#pragma once

#include "entt/entity/registry.hpp"
#include "entt/entity/runtime_view.hpp"
#include "meta_helper.hpp"
#include <set>

template <typename Component>
auto is_valid(const entt::registry* registry, entt::entity entity)
{
    assert(registry);
    return registry->valid(entity);
}

template <typename Component>
auto emplace_or_replace_component(entt::registry& registry, entt::entity entity) 
{
    return registry.emplace_or_replace<Component>(entity);
}

template <typename Component>
auto get_or_emplace_component(entt::registry& registry, entt::entity entity) 
{
    return registry.get_or_emplace<Component>(entity);
}

template <typename Component>
auto lua_emplace_or_replace_component(entt::registry* registry, entt::entity entity,
    const sol::table& instance, sol::this_state s) {
    assert(registry);
    auto& comp = registry->emplace_or_replace<Component>(
        entity, instance.valid() ? instance.as<Component>() : Component{});
    return sol::make_reference(s, std::ref(comp));
}

template <typename Component>
auto lua_get_or_emplace_component(entt::registry* registry, entt::entity entity,
    sol::this_state s) {
    assert(registry);
    auto& comp = registry->get_or_emplace<Component>(entity);
    return sol::make_reference(s, std::ref(comp));
}

template <typename Component>
bool has_component(entt::registry* registry, entt::entity entity) {
    assert(registry);
    return registry->any_of<Component>(entity);
}

template <typename Component>
auto remove_component(entt::registry* registry, entt::entity entity) {
    assert(registry);
    return registry->remove<Component>(entity);
}

template <typename Component> void clear_component(entt::registry* registry) {
    assert(registry);
    registry->clear<Component>();
}

template <typename Component> void register_meta_component()
{
    using namespace entt::literals;

    entt::meta<Component>()

        .prop("is_component"_hs, true)

        .template func<&is_valid<Component>>("valid_component"_hs)
        .template func<&clear_component<Component>>("clear_component"_hs)
        .template func<&has_component<Component>>("has_component"_hs)
        .template func<&remove_component<Component>>("remove_component"_hs)

        // C++ specific
        .template func<&emplace_or_replace_component<Component>>("emplace_component"_hs)
        .template func<&get_or_emplace_component<Component>>("get_component"_hs)

        // Lua specific
        .template func<&lua_emplace_or_replace_component<Component>>("lua_emplace"_hs)
        .template func<&lua_get_or_emplace_component<Component>>("lua_get"_hs)
        ;
}

auto collect_types(const sol::variadic_args& va) {
    std::set<entt::id_type> types;
    std::transform(va.cbegin(), va.cend(), std::inserter(types, types.begin()),
        [](const auto& obj) { return deduce_type(obj); });
    return types;
}

sol::table open_registry(sol::this_state s)
{
    // To create a registry inside a script: entt.registry.new()

    sol::state_view lua{ s };
    auto entt_module = lua["entt"].get_or_create<sol::table>();

    // clang-format off
    entt_module.new_usertype<entt::runtime_view>("runtime_view",
        sol::no_constructor,

        "size_hint", &entt::runtime_view::size_hint,
        "contains", &entt::runtime_view::contains,
        "each",
        [](const entt::runtime_view& self, const sol::function& callback) {
            if (callback.valid()) {
                for (auto entity : self) callback(entity);
            }
        }
    );

    using namespace entt::literals;

    entt_module.new_usertype<entt::registry>("registry",

        sol::meta_function::construct,
        sol::factories([] { return entt::registry{}; }),

        "size", [](const entt::registry& self) {
            return self.storage<entt::entity>()->size();
        },

        "alive", [](const entt::registry& self) {
            return self.storage<entt::entity>()->in_use();
        },

        "valid", &entt::registry::valid,

        "current", &entt::registry::current,

        "create", [](entt::registry& self) { return self.create(); },

        "destroy",
        [](entt::registry& self, entt::entity entity) {
            return self.destroy(entity);
        },

        "emplace",
        [](entt::registry& self, entt::entity entity, const sol::table& comp,
            sol::this_state s) -> sol::object {
                if (!comp.valid()) return sol::lua_nil_t{};
                const auto maybe_any = invoke_meta_func(get_type_id(comp), "lua_emplace"_hs,
                    &self, entity, comp, s);
                return maybe_any ? maybe_any.cast<sol::reference>() : sol::lua_nil_t{};
        },

        "remove",
        [](entt::registry& self, entt::entity entity, const sol::object& type_or_id) {
            const auto maybe_any =
                invoke_meta_func(deduce_type(type_or_id), "remove_component"_hs, &self, entity);
            return maybe_any ? maybe_any.cast<size_t>() : 0;
        },

        "has",
        [](entt::registry& self, entt::entity entity, const sol::object& type_or_id) {
            const auto maybe_any =
                invoke_meta_func(deduce_type(type_or_id), "has_component"_hs, &self, entity);
            return maybe_any ? maybe_any.cast<bool>() : false;
        },

        "any_of",
        [](const sol::table& self,
            entt::entity entity,
            const sol::variadic_args& va)
        {
            const auto types = collect_types(va);
            const auto has = self["has"].get<sol::function>();
            return std::any_of(types.cbegin(), types.cend(),
                [&](auto type_id) { return has(self, entity, type_id).template get<bool>(); }
            );
        },

        "get",
        [](entt::registry& self,
            entt::entity entity,
            const sol::object& type_or_id,
            sol::this_state s)
        {
            const auto maybe_any = invoke_meta_func(
                deduce_type(type_or_id),
                "lua_get"_hs,
                &self, entity,
                s);
            return maybe_any ? maybe_any.cast<sol::reference>() : sol::lua_nil_t{};
        },

        "clear",
        sol::overload(
            &entt::registry::clear<>,
            [](entt::registry& self,
                sol::object type_or_id)
            {
                invoke_meta_func(deduce_type(type_or_id), "clear_component"_hs, &self);
            }
        ),

        "orphan", &entt::registry::orphan,

        "runtime_view",
        [](entt::registry& self, const sol::variadic_args& va) {
            const auto types = collect_types(va);

            auto view = entt::runtime_view{};
            for (auto&& [componentId, storage] : self.storage()) {
                if (types.find(componentId) != types.cend()) {
                    view.iterate(storage);
                }
            }
            return view;
        }

        // "rebind",
        // [](entt::registry& self, sol::table new_table) {
        //     new_table["owner"] = std::ref(self); // <- TODO: try to not use key here
        //     std::cout << "copy rebind" << std::endl; // 
        // }
    );

    return entt_module;
}

#endif