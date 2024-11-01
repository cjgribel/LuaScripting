
#include <entt/entt.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "imgui.h" // If GUI code is here

#include "meta_literals.h" // for entt literals
#include "meta_aux.h"

#include "CoreComponents.hpp"
#include "InspectType.hpp"

//#include <iostream>

// === Transform ==============================================================

std::string Transform::to_string() const
{
    std::stringstream ss;
    ss << "Transform { x = " << std::to_string(x)
        << ", y = " << std::to_string(y)
        << ", angle = " << std::to_string(angle) << " }";
    return ss.str();
}

// + const (e.g. when used as key) ?
bool inspect_Transform(void* ptr, Editor::InspectorState& inspector)
{
    Transform* t = static_cast<Transform*>(ptr);
    bool mod = false;

    inspector.begin_leaf("x");
    mod |= Editor::inspect_type(t->x, inspector);
    inspector.end_leaf();

    inspector.begin_leaf("y");
    mod |= Editor::inspect_type(t->y, inspector);
    inspector.end_leaf();

    inspector.begin_leaf("angle");
    mod |= Editor::inspect_type(t->angle, inspector);
    inspector.end_leaf();

    return mod;
}

template<>
void register_meta<Transform>(std::shared_ptr<sol::state>& lua)
{
    // Note: appends meta asssigned to type by register_meta_component() in bond.hpp
    entt::meta<Transform>()
        .type("Transform"_hs).prop(display_name_hs, "Transform")

        .data<&Transform::x>("x"_hs).prop(display_name_hs, "x")
        .data<&Transform::y>("y"_hs).prop(display_name_hs, "y")
        .data<&Transform::angle>("angle"_hs).prop(display_name_hs, "angle")
        .data<&Transform::x_global>("x_global"_hs).prop(display_name_hs, "x_global").prop(readonly_hs, true)
        .data<&Transform::y_global>("y_global"_hs).prop(display_name_hs, "y_global").prop(readonly_hs, true)
        .data<&Transform::angle_global>("angle_global"_hs).prop(display_name_hs, "angle_global").prop(readonly_hs, true)

        // Inspection function (optional)
        // Sign: bool(void* ptr, Editor::InspectorState& inspector)
        // .func<&...>(inspect_hs)

        // Clone (optional)
        // Sign: Type(void* src)
        // .func<&...>(clone_hs)

        //.func<&vec3_to_json>(to_json_hs)
        //.func < [](nlohmann::json& j, const void* ptr) { to_json(j, *static_cast<const vec3*>(ptr)); }, entt::as_void_t > (to_json_hs)
        //.func < [](const nlohmann::json& j, void* ptr) { from_json(j, *static_cast<vec3*>(ptr)); }, entt::as_void_t > (from_json_hs)
        //        .func<&vec3::to_string>(to_string_hs)
        //.func<&vec3_to_string>(to_string_hs)
        ;

    lua->new_usertype<Transform>("Transform",

        sol::call_constructor,
        sol::factories([](float x, float y, float angle) {
            return Transform{
                .x = x, .y = y, .angle = angle
            };
            }),

        // type_id is required for component types, copying and inspection
        "type_id", &entt::type_hash<Transform>::value,

        // Default construction
        // Needed to copy userdata
        "construct",
        []() { return Transform{}; },

        "x", &Transform::x,
        "y", &Transform::y,
        "angle", &Transform::angle,
        "x_global", &Transform::x_global,
        "y_global", &Transform::y_global,
        "angle_global", &Transform::angle_global,

        sol::meta_function::to_string, &Transform::to_string
    );
}

// === HeaderComponent ========================================================

std::string HeaderComponent::to_string() const
{
    std::stringstream ss;
    ss << "HeaderComponent { name = " << name << " }";
    return  ss.str();
}

namespace {
    bool HeaderComponent_inspect(void* ptr, Editor::InspectorState& inspector)
    {
        return false;
    }
}

template<>
void register_meta<HeaderComponent>(std::shared_ptr<sol::state>& lua)
{
    // Register to entt::meta

    entt::meta<HeaderComponent>()
        .type("HeaderComponent"_hs).prop(display_name_hs, "Header")
        .data<&HeaderComponent::name>("name"_hs).prop(display_name_hs, "name")

        // Optional meta functions

        // to_string, member version
            //.func<&DebugClass::to_string>(to_string_hs)
        // to_string, lambda version
        .func < [](const void* ptr) {
        return static_cast<const HeaderComponent*>(ptr)->name;
        } > (to_string_hs)
            // inspect
                // .func<&inspect_Transform>(inspect_hs)
            // clone
                //.func<&cloneDebugClass>(clone_hs)
            ;

        // Register to sol

        lua->new_usertype<HeaderComponent>("HeaderComponent",
            //sol::constructors<HeaderComponent(), HeaderComponent(const std::string&)>(),

            // If the type has defined ctors
            //sol::constructors<HeaderComponent(), HeaderComponent(const std::string&)>(),

            sol::call_constructor,
            sol::factories(
                []() -> HeaderComponent {  // Default constructor
                    return HeaderComponent{};
                },
                [](const std::string& name) {
                    return HeaderComponent{
                        .name = name
                    };
                }),

            "type_id", &entt::type_hash<HeaderComponent>::value,

            "name", &HeaderComponent::name,
            // "name2", &HeaderComponent::name2,
            // "name3", &HeaderComponent::name3,

            // clone
            "copy",
            [](sol::userdata userdata)
            {
                // TODO: check fields
                return HeaderComponent{ userdata.get<std::string>("name") };
            },

            // Needed for value-copying
            "construct",
            []() { return HeaderComponent{}; },

            sol::meta_function::to_string, &HeaderComponent::to_string
        );

        // TEST
        // struct ABC { int x; };

        // lua.new_usertype<ABC>("ABC",
        //     sol::meta_function::construct,
        //     sol::factories([] { return ABC{}; }),

        //     "x", &ABC::x
        // );
}

// === CircleColliderGridComponent ============================================

std::string CircleColliderGridComponent::to_string() const
{
    return "CircleColliderGridComponent { ... }";
    //     std::stringstream ss;
    //     ss << "{ radii = ";
    //     for (int i = 0; i < count; i++) ss << std::to_string(radii[i]) << ", ";
    //     ss << "{ is_active_flags = ";
    //     for (int i = 0; i < count; i++) ss << std::to_string(is_active_flags[i]) << ", ";
    //     return ss.str();
}

template<>
void register_meta<CircleColliderGridComponent>(std::shared_ptr<sol::state>& lua)
{
    entt::meta<CircleColliderGridComponent>()
        .type("CircleColliderGridComponent"_hs).prop(display_name_hs, "CircleColliderGrid")

        .data<&CircleColliderGridComponent::is_active>("is_active"_hs).prop(display_name_hs, "is_active")
        .data<&CircleColliderGridComponent::element_count>("element_count"_hs).prop(display_name_hs, "element_count").prop(readonly_hs, true)
        .data<&CircleColliderGridComponent::layer_bit>("layer_bit"_hs).prop(display_name_hs, "layer_bit")
        .data<&CircleColliderGridComponent::layer_mask>("layer_mask"_hs).prop(display_name_hs, "layer_mask")
        .data<&CircleColliderGridComponent::circles>("circles"_hs).prop(display_name_hs, "circles")

        // Optional meta functions

        // to_string, member version
            //.func<&DebugClass::to_string>(to_string_hs)
        // to_string, lambda version
        // .func < [](const void* ptr) {
        // return static_cast<const HeaderComponent*>(ptr)->name;
        // } > (to_string_hs)
            // inspect
                // .func<&inspect_Transform>(inspect_hs)
            // clone
                //.func<&cloneDebugClass>(clone_hs)
        ;

    entt::meta<CircleColliderGridComponent::Circle>()
        .type("Circle"_hs).prop(display_name_hs, "Circle")

        .data<&CircleColliderGridComponent::Circle::pos>("pos"_hs).prop(display_name_hs, "position")
        .data<&CircleColliderGridComponent::Circle::radius>("radius"_hs).prop(display_name_hs, "radius")
        ;

    entt::meta<linalg::v2f>()
        .type("v2f"_hs).prop(display_name_hs, "Vector2D")

        .data<&linalg::v2f::x>("x"_hs).prop(display_name_hs, "x")
        .data<&linalg::v2f::y>("y"_hs).prop(display_name_hs, "y")
        ;

    lua->new_usertype<CircleColliderGridComponent>("CircleColliderGridComponent",
        "type_id",
        &entt::type_hash<CircleColliderGridComponent>::value,

        sol::call_constructor,
        sol::factories([](
            int width,
            int height,
            bool is_active,
            unsigned char layer_bit,
            unsigned char layer_mask)
            {
                assert(width * height <= GridSize);
                auto c = CircleColliderGridComponent{
                    .element_count = width * height,
                    .width = width,
                    .is_active = is_active,
                    .layer_bit = layer_bit,
                    .layer_mask = layer_mask };
                //for (int i = 0; i < width * height; i++)
                //    c.active_indices.add(i);
                return c;
            }),

        // "add_circle",
        // [](CircleColliderGridComponent& c, float x, float y, float radius, bool is_active)
        // {
        //     if (c.count >= GridSize) throw std::out_of_range("Index out of range");
        //     c.pos[c.count].x = x;
        //     c.pos[c.count].y = y;
        //     c.radii[c.count] = radius;
        //     c.is_active_flags[c.count] = is_active;
        //     c.count++;
        // },
        "set_circle_at",
        [](CircleColliderGridComponent& c,
            int index,
            float x,
            float y,
            float radius,
            bool is_active)
        {
            assert(index >= 0 && index < c.element_count);
            // c.pos[index].x = x;
            // c.pos[index].y = y;
            // c.radii[index] = radius;
            // c.is_active_flags[index] = is_active;

            auto& circle = c.circles[index];
            circle.pos = v2f{ x, y };
            circle.radius = radius;

            if (is_active) c.active_indices.add(index);
            else c.active_indices.remove(index);
        },
        "set_active_flag_all", [](CircleColliderGridComponent& c, bool is_active)
        {
            for (int i = 0; i < c.element_count; i++)
            {
                // c.is_active_flags[i] = is_active;
                if (is_active) c.active_indices.add(i);
                else c.active_indices.remove(i);
            }
            c.is_active = is_active;
        },
        // "get_radius", [](CircleColliderGridComponent& ccsc, int index) -> float {
        //     if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
        //     return ccsc.radii[index];
        // },
        // "set_radius", [](CircleColliderGridComponent& ccsc, int index, float value) {
        //     if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
        //     ccsc.radii[index] = value;
        // },
        // "get_is_active_flag", [](CircleColliderGridComponent& ccsc, int index) -> float {
        //     if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
        //     return ccsc.is_active_flags[index];
        // },
        "set_active_flag_at", [](CircleColliderGridComponent& c, int index, bool is_active)
        {
            assert(index >= 0 && index < c.element_count);
            //if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
            //std::cout << index << std::endl;
            // c.is_active_flags[index] = is_active;
            if (is_active) c.active_indices.add(index);
            else c.active_indices.remove(index);
        },
        // TODO
        "is_any_active", [](CircleColliderGridComponent& c) -> bool
        {
            return c.active_indices.get_dense_count() > 0;
            // for (int i = 0; i < c.count; i++)
            // {
            //     if (c.is_active_flags[i]) return true;
            // }
            // return false;
        },
        "get_element_count", [](CircleColliderGridComponent& c)
        {
            return c.element_count;
        },
        "is_active",
        &CircleColliderGridComponent::is_active,

        sol::meta_function::to_string,
        &CircleColliderGridComponent::to_string
    );
}

// === IslandFinderComponent ==================================================

std::string IslandFinderComponent::to_string() const
{
    return "IslandFinderComponent { ... }";
}

template<>
void register_meta<IslandFinderComponent>(std::shared_ptr<sol::state>& lua)
{
    entt::meta<IslandFinderComponent>()
        .type("IslandFinderComponent"_hs).prop(display_name_hs, "IslandFinder")

        .data<&IslandFinderComponent::core_x>("core_x"_hs).prop(display_name_hs, "core_x")
        .data<&IslandFinderComponent::core_y>("core_y"_hs).prop(display_name_hs, "core_y")//.prop(readonly_hs, true)
        .data<&IslandFinderComponent::islands>("islands"_hs).prop(display_name_hs, "islands") // ???
        ;

    lua->new_usertype<IslandFinderComponent>("IslandFinderComponent",
        "type_id",
        &entt::type_hash<IslandFinderComponent>::value,
        sol::call_constructor,
        sol::factories([](int core_x, int core_y) {
            return IslandFinderComponent{
                .core_x = core_x,
                .core_y = core_y
            };
            }),
        "get_nbr_islands", [](IslandFinderComponent& c) {
            return c.islands.size();
        },
        "get_island_index_at", [](IslandFinderComponent& c, int index) {
            assert(index < c.islands.size());
            return c.islands[index];
        },

        sol::meta_function::to_string,
        &IslandFinderComponent::to_string
    );
}

// === QuadGridComponent ======================================================

std::string QuadGridComponent::to_string() const
{
    return "QuadGridComponent { ... }";
    // std::stringstream ss;
    // ss << "{ size = ";
    // for (int i = 0; i < count; i++) ss << std::to_string(sizes[i]) << ", ";
    // ss << "{ colors = ";
    // for (int i = 0; i < count; i++) ss << std::to_string(colors[i]) << ", ";
    // ss << "{ is_active_flags = ";
    // for (int i = 0; i < count; i++) ss << std::to_string(is_active_flags[i]) << ", ";
    // return ss.str();
}

template<>
void register_meta<QuadGridComponent>(std::shared_ptr<sol::state>& lua)
{
    entt::meta<QuadGridComponent>()
        .type("QuadGridComponent"_hs).prop(display_name_hs, "QuadGrid")

        .data<&QuadGridComponent::count>("count"_hs).prop(display_name_hs, "count").prop(readonly_hs, true)
        .data<&QuadGridComponent::width>("width"_hs).prop(display_name_hs, "width").prop(readonly_hs, true)
        .data<&QuadGridComponent::is_active>("is_active"_hs).prop(display_name_hs, "is_active")

        // Todo: custom inspect for this type to visualize arrays more efficently
        .data<&QuadGridComponent::positions>("positions"_hs).prop(display_name_hs, "positions")
        .data<&QuadGridComponent::sizes>("sizes"_hs).prop(display_name_hs, "sizes")
        .data<&QuadGridComponent::colors>("colors"_hs).prop(display_name_hs, "colors")
        .data<&QuadGridComponent::is_active_flags>("is_active_flags"_hs).prop(display_name_hs, "is_active_flags")
        ;

    lua->new_usertype<QuadGridComponent>("QuadGridComponent",

        "type_id",
        &entt::type_hash<QuadGridComponent>::value,

        sol::call_constructor,
        sol::factories([](
            int width,
            int height,
            bool is_active)
            {
                assert(width * height <= GridSize);
                return QuadGridComponent{
                    .count = width * height,
                    .width = width,
                    .is_active = is_active
                };
            }),

        // Needed for value-copying
        // Skip - has containers I cannot copy
        // "construct",
        // []() { return QuadGridComponent{}; },

        "count", &QuadGridComponent::count,
        "width", &QuadGridComponent::width,
        "is_active", &QuadGridComponent::is_active,

        // Containers become userdata - not sure how to clone/inspect/serialize when type-erased
        // "positions", &QuadGridComponent::positions,
        // "sizes", &QuadGridComponent::sizes,
        // "colors", &QuadGridComponent::colors,
        // "is_active_flags", &QuadGridComponent::is_active_flags,

        sol::meta_function::to_string,
        &QuadGridComponent::to_string,

        "set_quad_at",
        [](QuadGridComponent& c,
            int index,
            float x,
            float y,
            float size,
            uint32_t color,
            bool is_active)
        {
            assert(index >= 0 && index < c.count);
            c.positions[index].x = x;
            c.positions[index].y = y;
            c.sizes[index] = size;
            c.colors[index] = color;
            // if (is_active && !c.is_active_flags[index]) c.active_indices[c.nbr_active++] = index;
            c.is_active_flags[index] = is_active;
        },

        "set_active_flag_all", [](QuadGridComponent& c, bool is_active) {
            for (int i = 0; i < c.count; i++)
                c.is_active_flags[i] = is_active;
            c.is_active = is_active;
        },

        "get_pos_at", [](QuadGridComponent& c, int index) {
            //if (index < 0 || index >= c.count) throw std::out_of_range("Index out of range");
            assert(index >= 0 && index < c.count);
            return std::make_tuple(c.positions[index].x, c.positions[index].y);
        },
        // "set_pos", [](QuadGridComponent& c, int index, float x, float y) {
        //     if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
        //     c.pos[index].x = x;
        //     c.pos[index].y = y;
        // },

        "get_size_at", [](QuadGridComponent& c, int index) -> float {
            //if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
            assert(index >= 0 && index < c.count);
            return c.sizes[index];
        },
        // "set_size", [](QuadGridComponent& c, int index, float value) {
        //     if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
        //     c.sizes[index] = value;
        // },

        "get_color_at", [](QuadGridComponent& c, int index) -> uint32_t {
            // if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
            assert(index >= 0 && index < c.count);
            return c.colors[index];
        },

        "set_color_at", [](QuadGridComponent& c, int index, uint32_t color) {
            // if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
            assert(index >= 0 && index < c.count);
            c.colors[index] = color;
        },

        "set_color_all", [](QuadGridComponent& c, uint32_t color) {
            for (int i = 0; i < c.count; i++)
                c.colors[i] = color;
        },

        // "get_is_active_flag", [](QuadGridComponent& c, int index) -> float {
        //     if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
        //     return c.is_active_flags[index];
        // },
        "set_active_flag_at", [](QuadGridComponent& c, int index, bool is_active) {
            // if (index < 0 || index >= GridSize) throw std::out_of_range("Index out of range");
            assert(index >= 0 && index < c.count);
            c.is_active_flags[index] = is_active;
        },

        "get_element_count", [](QuadGridComponent& c) {
            return c.count;
        }
    );
}

// === DataGridComponent ======================================================

std::string DataGridComponent::to_string() const
{
    return "DataGridComponent { ... }";
}

template<>
void register_meta<DataGridComponent>(std::shared_ptr<sol::state>& lua)
{
    entt::meta<DataGridComponent>()
        .type("DataGridComponent"_hs).prop(display_name_hs, "DataGrid")

        .data<&DataGridComponent::count>("count"_hs).prop(display_name_hs, "count").prop(readonly_hs, true)
        .data<&DataGridComponent::width>("width"_hs).prop(display_name_hs, "width").prop(readonly_hs, true)

        // Todo: custom inspect for this type to visualize arrays more efficently
        .data<&DataGridComponent::slot1>("slot1"_hs).prop(display_name_hs, "slot1")
        .data<&DataGridComponent::slot2>("slot2"_hs).prop(display_name_hs, "slot2")
        ;

    lua->new_usertype<DataGridComponent>("DataGridComponent",
        "type_id",
        &entt::type_hash<DataGridComponent>::value,

        sol::call_constructor,
        sol::factories([](
            int width,
            int height)
            {
                assert(width * height <= GridSize);
                return DataGridComponent{
                    .count = width * height,
                    .width = width };
            }),
        "set_slot1_at",
        [](DataGridComponent& c,
            int index,
            float value)
        {
            assert(index >= 0 && index < c.count);
            c.slot1[index] = value;
        },
        "set_slot2_at",
        [](DataGridComponent& c,
            int index,
            float value)
        {
            assert(index >= 0 && index < c.count);
            c.slot2[index] = value;
        },
        "get_slot1_at",
        [](DataGridComponent& c, int index)
        {
            assert(index >= 0 && index < c.count);
            return c.slot1[index];
        },
        "get_slot2_at",
        [](DataGridComponent& c, int index)
        {
            assert(index >= 0 && index < c.count);
            return c.slot2[index];
        },

        sol::meta_function::to_string,
        &DataGridComponent::to_string
    );
}

// === ScriptedBehaviorComponent ==============================================

std::string ScriptedBehaviorComponent::to_string() const {
    std::stringstream ss;
    ss << "{ scripts = ";
    for (auto& script : scripts) ss << script.identifier << " ";
    ss << "}";
    return ss.str();
}

namespace
{
    std::string sol_object_to_string(std::shared_ptr<const sol::state> lua, const sol::object object)
    {
        return lua->operator[]("tostring")(object).get<std::string>();
    }

    std::string get_lua_type_name(std::shared_ptr<const sol::state> lua, const sol::object object)
    {
        return lua_typename(lua->lua_state(), static_cast<int>(object.get_type()));
    }
}

// sol inspection
namespace Editor {

    /// Inspect sol::function
    template<>
    bool inspect_type<sol::function>(sol::function& t, InspectorState& inspector)
    {
        ImGui::TextDisabled("%s", sol_object_to_string(inspector.context.lua, t).c_str());
        return false;
    }

    /// Inspect sol::table
    template<>
    bool inspect_type<sol::table>(sol::table&, InspectorState&);

    /// Inspect sol::userdata
    template<>
    bool inspect_type<sol::userdata>(sol::userdata& userdata, InspectorState& inspector)
    {
        auto& lua = inspector.context.lua;
        bool mod = false;

        // Fetch type id of the inspected usertype
        auto type_id = userdata["type_id"];

#if 0
        // Inspect the raw metatable directly
        sol::table metatable = userdata[sol::metatable_key];
        mod |= inspect_type(metatable, inspector);
        return mod;
#endif

        // Non-component types typically don't have a "type_id" field
        if (!type_id.valid())
        {
#if 1
            // No inspection
            ImGui::TextDisabled("[Unvailable]");
            return mod;
#else
            // Check if the userdata has an `__index` metamethod that acts like a table
            sol::optional<sol::table> metatable = userdata[sol::metatable_key];
            if (metatable && metatable->get<sol::object>("__index").is<sol::table>())
            {
                sol::table index_table = metatable->get<sol::table>("__index");
                mod |= Editor::inspect_type(index_table, inspector);
            }
            else
                ImGui::TextDisabled("[Unvailable]");
            return mod;
#endif
        }

        assert(type_id.get_type() == sol::type::function);
        entt::id_type id = type_id.call();

        // Get entt meta type for this type id
        auto meta_type = entt::resolve(id);
        assert(meta_type);

        // List type fields via entt::meta
        for (auto&& [id, meta_data] : meta_type.data())
        {
            // entt field name
            std::string key_name = meta_data_name(id, meta_data);
            const auto key_name_cstr = key_name.c_str();

            bool readonly = get_meta_data_prop<bool, ReadonlyDefault>(meta_data, readonly_hs);
            if (readonly) inspector.begin_disabled();

            // Fetch usertype field with this field name
            // Note: requires fields to be registered with the EXACT same name ...
            sol::object value = userdata[key_name_cstr];

            if (!value)
            {
                // entt meta data was not found in sol usertype
                inspector.begin_leaf(key_name_cstr);
                ImGui::TextDisabled("[Not in usertype]");
                inspector.end_leaf();
            }
            // userdata, table ???
            else if (value.get_type() == sol::type::string)
            {
                std::string str = value.as<std::string>();
                inspector.begin_leaf(key_name_cstr);
                if (inspect_type(str, inspector)) { userdata[key_name_cstr] = str; mod = true; }
                inspector.end_leaf();
            }
            else if (value.get_type() == sol::type::number)
            {
                double nbr = value.as<double>();
                inspector.begin_leaf(key_name_cstr);
                if (inspect_type(nbr, inspector)) { userdata[key_name_cstr] = nbr; mod = true; }
                inspector.end_leaf();
            }
            else if (value.get_type() == sol::type::boolean)
            {
                bool bl = value.as<bool>();
                inspector.begin_leaf(key_name_cstr);
                if (inspect_type(bl, inspector)) { userdata[key_name_cstr] = bl; mod = true; }
                inspector.end_leaf();
            }
            else
            {
                inspector.begin_leaf(key_name_cstr);
                ImGui::TextDisabled("[to_string] %s", sol_object_to_string(lua, value).c_str());
                inspector.end_leaf();
            }
            if (readonly) inspector.end_disabled();
        }
        return mod;
    }

    template<>
    bool inspect_type<sol::table>(sol::table& tbl, InspectorState& inspector)
    {
        // std::cout << "inspecting sol::table" << std::endl;
        auto& lua = inspector.context.lua;
        bool mod = false;

        if (!tbl.valid()) return mod;

        for (auto& [key, value] : tbl)
        {
            std::string key_str = sol_object_to_string(lua, key) + " [" + get_lua_type_name(lua, value) + "]";
            std::string key_str_label = "##" + key_str;

            // Note: value.is<sol::table>() is true also for sol::type::userdata and possibly other lua types
            // This form,
            //      value.get_type() == sol::type::table
            // seems more precise when detecting types, even though it isn't type-safe
            if (value.get_type() == sol::type::table)
            {
                if (inspector.begin_node(key_str.c_str()))
                {
                    sol::table tbl_nested = value.as<sol::table>();

                    mod |= Editor::inspect_type(tbl_nested, inspector);
                    inspector.end_node();
                }
            }
            else if (value.get_type() == sol::type::userdata)
            {
                if (inspector.begin_node(key_str.c_str()))
                {
                    sol::userdata userdata = value.as<sol::userdata>();
                    mod |= inspect_type(userdata, inspector);
                    inspector.end_node();
                }
            }
            else
            {
                inspector.begin_leaf(key_str.c_str());

                if (value.get_type() == sol::type::function)
                {
                    sol::function func = value.as<sol::function>();
                    mod |= Editor::inspect_type(func, inspector);
                }
                else if (value.get_type() == sol::type::number)
                {
                    double dbl = value.as<double>();
                    if (ImGui::InputDouble(key_str_label.c_str(), &dbl, 0.1, 0.5))
                    {
                        // Commit modified value to Lua
                        tbl[key] = dbl;
                        mod = true;
                    }
                }
                else if (value.get_type() == sol::type::boolean)
                {
                    bool b = value.as<bool>();
                    if (ImGui::Checkbox(key_str_label.c_str(), &b))
                    {
                        // Commit modified value to Lua
                        tbl[key] = b;
                        mod = true;
                    }
                }
                else if (value.get_type() == sol::type::string)
                {
                    std::string str = value.as<std::string>();
                    if (inspect_type(str, inspector))
                    {
                        // Commit modified value to Lua
                        tbl[key] = str;
                        mod = true;
                    }
                }
                else
                {
                    // Fallback: display object as a string
                    // Applies to
                    // sol::type::lightuserdata
                    // sol::type::lua_nil
                    // sol::type::none
                    // sol::type::poly
                    // sol::type::thread

                    ImGui::TextDisabled("%s", sol_object_to_string(lua, value).c_str());
                }
                inspector.end_leaf();
            }
        }

        return mod;
    }

}

// sol copying
namespace {

    sol::table deep_copy_table(sol::state_view lua, const sol::table& original);

    sol::userdata deep_copy_userdata(sol::state_view lua, const sol::userdata& userdata)
    {
        sol::function construct = userdata["construct"];
        if (!construct.valid())  return userdata; // Copy by reference
        // Let userdata decide how to copy
        sol::userdata userdata_cpy = construct();

        // Fetch type id
        auto type_id = userdata["type_id"];
        if (type_id.get_type() != sol::type::function) return userdata;

        // Fetch entt meta type, iterate its data and copy to userdata
        entt::id_type id = type_id.call();
        entt::meta_type meta_type = entt::resolve(id);
        assert(meta_type);
        // entt::meta_any meta_any = meta_type.construct(); // cannot go any -> userdata
        for (auto&& [id, meta_data] : meta_type.data())
        {
            // entt field name
            std::string key_name = meta_data_name(id, meta_data); // Don't use displayname
            const auto key_name_cstr = key_name.c_str();

            sol::object value = userdata[key_name_cstr];

            if (!value.valid())
            {
                std::cout << "Copy userdata warning: meta data '" << key_name
                    << "' was not found in userdata" << std::endl;
                assert(0);
                continue;
            }
            else if (value.get_type() == sol::type::table)
            {
                userdata_cpy[key_name_cstr] = deep_copy_table(lua, value.as<sol::table>());
            }
            else if (value.get_type() == sol::type::userdata)
            {
                userdata_cpy[key_name_cstr] = deep_copy_userdata(lua, value.as<sol::userdata>());
            }
            else
            {
                userdata_cpy[key_name_cstr] = value;
            }
        }

        return userdata_cpy;
    }

    sol::table deep_copy_table(sol::state_view lua, const sol::table& original)
    {
        sol::table copy = lua.create_table();

        for (const auto& [key, value] : original)
        {
            if (value.get_type() == sol::type::table)
            {
                copy[key] = deep_copy_table(lua, value.as<sol::table>());
            }
            else if (value.get_type() == sol::type::userdata)
            {
                copy[key] = deep_copy_userdata(lua, value.as<sol::userdata>());
            }
            else
            {
                copy[key] = value;
            }
        }
        return copy;
    }

    ScriptedBehaviorComponent copy_ScriptedBehaviorComponent(void* ptr, entt::entity dst_entity)
    {
        auto comp_ptr = static_cast<ScriptedBehaviorComponent*>(ptr);
        std::cout << "COPY ScriptedBehaviorComponent" << std::endl;

        // Will suffice if ScriptedBehaviorComponent has a copy constructor
        // (Except updating the id() function)
        // Can entity be sent to script's update() ??
        ScriptedBehaviorComponent cpy = *comp_ptr;

        // Deep copy self
        // functions?
        // std::cout << "cpy.scripts.size() " << cpy.scripts.size() << ", " << entt::to_integral(dst_entity) << std::endl;
        // std::cout << "comp_ptr->scripts.size() " << comp_ptr->scripts.size() << ", " << entt::to_integral(dst_entity) << std::endl;

        // std::cout << "comp_ptr " << comp_ptr << std::endl;
        // if (comp_ptr->scripts.size() == 0) 
        // {
        //     std::cout << "== 0! comp_ptr->scripts.size() " << comp_ptr->scripts.size() << ", " << entt::to_integral(dst_entity) << std::endl;
        // }
        // else
        // {
        //     std::cout << "!= 0! comp_ptr->scripts.size() " << comp_ptr->scripts.size() << ", " << entt::to_integral(dst_entity) << std::endl;
        // }

        for (size_t i = 0; i < comp_ptr->scripts.size(); i++)
        {
            // std::cout << "cpy.scripts.size() " << cpy.scripts.size() << ", " << entt::to_integral(dst_entity) << std::endl;
            // std::cout << "comp_ptr->scripts.size() " << comp_ptr->scripts.size() << ", " << entt::to_integral(dst_entity) << std::endl;
            //     std::cout << i << std::endl << std::flush;

                // Shallow (already done ^)
                ///cpy.scripts[i].self = comp_ptr->scripts[i].self;
                // Deep
            cpy.scripts[i].self = deep_copy_table(comp_ptr->scripts[i].self.lua_state(), comp_ptr->scripts[i].self);

            // std::cout << i << std::endl << std::flush;

            // Not needed probably functions are references inside table
            cpy.scripts[i].update = cpy.scripts[i].self["update"];
            cpy.scripts[i].on_collision = cpy.scripts[i].self["on_collision"];

            // std::cout << i << std::endl << std::flush;

            // -> entityID?
            cpy.scripts[i].self["id"] = sol::readonly_property([dst_entity] { return dst_entity; });
            // -> registry?
            // cpy.scripts[i].self["owner"] = comp_ptr->scripts[i].self["owner"]; // std::ref(registry);

            // Changes copy AND original - must be some kind of reference
            // cpy.scripts[i].self["HEADER"]["name"] = "BYE";

            // std::cout << i << std::endl << std::flush;
        }

        std::cout << "DONE COPY ScriptedBehaviorComponent" << std::endl << std::flush;
        return cpy;
    };

    // v2
#if 0
    ScriptedBehaviorComponent copy_ScriptedBehaviorComponent_(void* ptr, entt::entity dst_entity)
    {
        // -- Need to either 1) send in a object to clone, or 2) use Command
        // registry is needed
        // lua is needed (can dig it out from self table in ptr)
        // In other cloning siutations, stuff like Scene, resources ... might be needed
        //      for complex objects such as meshes, node hierarchies ...

        // 1. Use script paths (ptr) to call add_script for dst_entity
        // 2. Copy "serializable fields" of sol::table from ptr to dst_entity

        // sol::table add_script(
        // entt::registry& registry,
        // entt::entity entity,
        // const sol::table& script_table,
        // const std::string& identifier,
        // const std::string& script_path)
    }
#endif
}

template<>
void register_meta<ScriptedBehaviorComponent>(std::shared_ptr<sol::state>& lua)
{
    // TODO: Where should meta for sol stuff be placed (table, function etc)?
    // sol::table
    entt::meta<sol::table>()
        .type("sol::table"_hs).prop(display_name_hs, "sol::table")

        // inspect
        //.func<&soltable_inspect>(inspect_hs)
        // inspect v2
        .func < [](void* ptr, Editor::InspectorState& inspector) {return Editor::inspect_type(*static_cast<sol::table*>(ptr), inspector); } > (inspect_hs)

        // clone
        // Note: Let ScriptedBehaviorComponent do the copying
        //.func <copy_table>(clone_hs)
        ;

    // sol::protected_function
    entt::meta<sol::protected_function>()
        .type("sol::protected_function"_hs).prop(display_name_hs, "sol::protected_function")
        // inspect
        //.func<&solfunction_inspect>(inspect_hs)
        // inspect v2 - 'widget not implemented' for BehaviorScript::update, on_collision
        .func < [](void* ptr, Editor::InspectorState& inspector) {return Editor::inspect_type(*static_cast<sol::protected_function*>(ptr), inspector); } > (inspect_hs)
        ;

    // ScriptedBehaviorComponent::BehaviorScript
    entt::meta<BehaviorScript>()
        .type("BehaviorScript"_hs).prop(display_name_hs, "BehaviorScript")

        .data<&BehaviorScript::identifier>("identifier"_hs)
        .prop(display_name_hs, "identifier")
        .prop(readonly_hs, true)
        .data<&BehaviorScript::path>("path"_hs)
        .prop(display_name_hs, "path")
        .prop(readonly_hs, true)

        // sol stuff
        .data<&BehaviorScript::self>("self"_hs).prop(display_name_hs, "self")
        .data<&BehaviorScript::update>("update"_hs).prop(display_name_hs, "update")
        .data<&BehaviorScript::on_collision>("on_collision"_hs).prop(display_name_hs, "on_collision")
        ;

    // ScriptedBehaviorComponent
    entt::meta<ScriptedBehaviorComponent>()
        .type("ScriptedBehaviorComponent"_hs).prop(display_name_hs, "ScriptedBehavior")
        .data<&ScriptedBehaviorComponent::scripts>("scripts"_hs).prop(display_name_hs, "scripts")

        // Optional meta functions

        // to_string, member version
//        .func<&DebugClass::to_string>(to_string_hs)
        // to_string, lambda version
//        .func < [](const void* ptr) { return static_cast<const HeaderComponent*>(ptr)->name; } > (to_string_hs)
            // inspect
                // .func<&inspect_Transform>(inspect_hs)

            // clone
        .func<&copy_ScriptedBehaviorComponent>(clone_hs)
        ;

    lua->new_usertype<ScriptedBehaviorComponent>("ScriptedBehaviorComponent",
        "type_id",
        &entt::type_hash<ScriptedBehaviorComponent>::value,
        sol::call_constructor,
        sol::factories([]() {
            return ScriptedBehaviorComponent{ };
            }),
        // "scripts",
        // &ScriptedBehaviorComponent::scripts,
        // "get_script_by_id",
        // &ScriptedBehaviorComponent::get_script_by_id,
        sol::meta_function::to_string,
        &ScriptedBehaviorComponent::to_string
    );
}