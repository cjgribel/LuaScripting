//
//  DebugClass.h
//  cppmisc
//
//  Created by Carl Johan Gribel on 2024-08-17.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef DebugClass_h
#define DebugClass_h

//#define JSON

#include <iostream>
#include <sstream>
#include <cassert>
#include <entt/entt.hpp>
#ifdef JSON
#include <nlohmann/json.hpp>
#endif
#include "meta_literals.h"

// !! sol in included earlier ...

// GET ME HOME
struct HeaderComponent
{
    std::string name;
};

bool HeaderComponent_inspect(void* ptr, Editor::InspectorState& inspector)
{
    return false;
}

inline void HeaderComponent_metaregister(sol::state& lua)
{
    // Register to entt::meta

    entt::meta<HeaderComponent>()
        .type("HeaderComponent"_hs).prop(display_name_hs, "HeaderComponent")
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

        lua.new_usertype<HeaderComponent>("HeaderComponent",
            "type_id", &entt::type_hash<HeaderComponent>::value,

            sol::call_constructor,
            sol::factories([](const std::string& name) {
                return HeaderComponent{
                    .name = name
                };
                }),
            "name", &HeaderComponent::name

            //sol::meta_function::to_string, &Transform::to_string
            );
}



struct debugvec3
{
    float x = 1, y = 2, z = 3;

    std::string to_string() const {
        std::ostringstream oss;
        oss << "debugvec3(" << x << ", " << y << ", " << z << ")";
        return oss.str();
    }

    bool operator==(const debugvec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

// Free function test
inline std::string debugvec3_to_string(const void* ptr)
{
    return static_cast<const debugvec3*>(ptr)->to_string();
}

#ifdef JSON
// Makes debugvec3 work in nlohmann expressions
// Resource ref??? void to_json(nlohmann::json& j, const ResourceWrapper<const GLuint>& resource) {}
inline void to_json(nlohmann::json& j, const debugvec3& value)
{
    j =
    {
        {"x", value.x},
        {"y", value.y},
        {"z", value.z}
    };

    //    std::cout << "to_json debugvec3 " << value.x << ", " << value.y << ", " << value.z << std::endl;

    std::cout << "to_json: " << j.dump() << " <- " << value.to_string() << std::endl;
}

inline void from_json(const nlohmann::json& j, debugvec3& value)
{
    value.x = j["x"];
    value.y = j["y"];
    value.z = j["z"];

    //    std::cout << "from_json debugvec3 " << value.x << ", " << value.y << ", " << value.z << std::endl;
    std::cout << "from_json: " << j.dump() << " -> " << value.to_string() << std::endl;
}
#endif

//void debugvec3_to_json(nlohmann::json& j, const void* ptr)
//{
//    std::cout << "[debugvec3_to_json]" << std::endl;
//
//    to_json(j, *static_cast<const debugvec3*>(ptr));
//
//    std::cout << "dump sent " << j.dump() << std::endl;
//}

struct ElementType
{
    float m;

    std::string to_string() const {
        std::ostringstream oss;
        oss << "ElementType(" << m << ")";
        return oss.str();
    }

    bool operator==(const ElementType& other) const {
        return m == other.m;
    }
};

// CORE COMPONENT TYPES ========================================================

enum class AnEnum : int { Hello = 5, Bye = 6, Hola = 8 };

struct DebugClass
{
    int a = 1, b = 2, c = 3;
    bool flag = true;
    debugvec3 position;
    std::string somestring = "Hello";
    std::vector<int> vector1 = { 1, 2, 3 };
    std::vector<ElementType> vector2 = { {4.0f}, {5.0f}, {6.0f} };
    std::map<int, float> map1 = { {7, 7.5f}, {8, 8.5f} };
    std::map<int, ElementType> map2 = { {9, {9.5f}}, {10, {10.5f}} };
    std::set<int> set1 = { 11, 12 };
    //    enum class AnEnum: int { A = 1, B = 2 } anEnum;
    AnEnum anEnum = AnEnum::Hello;

    std::string to_string() const {
        std::ostringstream oss;
        oss << "DebugClass {\n"
            << "  a: " << a << "\n"
            << "  b: " << b << "\n"
            << "  c: " << c << "\n"
            << "  flag: " << std::boolalpha << flag << "\n"
            << "  position: " << position.to_string() << "\n"
            << "  somestring: \"" << somestring << "\"\n"
            << "  vector1: [";
        for (size_t i = 0; i < vector1.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << vector1[i];
        }
        oss << "]\n"
            << "  vector2: [";
        for (size_t i = 0; i < vector2.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << vector2[i].to_string();
        }
        oss << "]\n"
            << "  map1: {\n";
        for (auto it = map1.begin(); it != map1.end(); ++it) {
            oss << "    " << it->first << ": " << it->second << "\n";
        }
        oss << "  }\n"
            << "  map2: {\n";
        for (auto it = map2.begin(); it != map2.end(); ++it) {
            oss << "    " << it->first << ": " << it->second.to_string() << "\n";
        }
        oss << "  }\n"
            << "  set1: {";
        for (auto it = set1.begin(); it != set1.end(); ++it) {
            if (it != set1.begin()) oss << ", ";
            oss << *it;
        }
        oss << "}\n}";
        return oss.str();
    }

    bool operator==(const DebugClass& rhs) const
    {
        return a == rhs.a &&
            b == rhs.b &&
            c == rhs.c &&
            flag == rhs.flag &&
            position == rhs.position &&
            somestring == rhs.somestring &&
            vector1 == rhs.vector1 &&
            vector2 == rhs.vector2 &&
            map1 == rhs.map1 &&
            map2 == rhs.map2 &&
            set1 == rhs.set1;
    }
};

inline std::string to_string_DebugClass(const void* ptr)
{
    return static_cast<const DebugClass*>(ptr)->to_string();
}

// Clone function for Transform component
inline DebugClass cloneDebugClass(void* src)
{
    const DebugClass* src_class = static_cast<const DebugClass*>(src);
    return *src_class; // DebugClass{ src_class->x, src_class->y, src_class->z };
}

inline void registerDebugClass()
{
    entt::meta<debugvec3>()
        .type("debugvec3"_hs).prop(display_name_hs, "debugvec3")
        .data<&debugvec3::x>("x"_hs).prop(display_name_hs, "x")
        .data<&debugvec3::y>("y"_hs).prop(display_name_hs, "y")
        .data<&debugvec3::z>("z"_hs).prop(display_name_hs, "z")
#ifdef JSON
        //.func<&debugvec3_to_json>(to_json_hs)
        .func < [](nlohmann::json& j, const void* ptr) { to_json(j, *static_cast<const debugvec3*>(ptr)); }, entt::as_void_t > (to_json_hs)
        .func < [](const nlohmann::json& j, void* ptr) { from_json(j, *static_cast<debugvec3*>(ptr)); }, entt::as_void_t > (from_json_hs)
#endif
        //        .func<&debugvec3::to_string>(to_string_hs)
        .func<&debugvec3_to_string>(to_string_hs)
        ;

    entt::meta<ElementType>()
        .type("ElementType"_hs).prop(display_name_hs, "ElementType")
        .data<&ElementType::m>("m"_hs).prop(display_name_hs, "m")
        //        .func<&debugvec3::to_string>(to_string_hs)
        ;

    entt::meta<DebugClass>()
        .type("DebugClass"_hs).prop(display_name_hs, "DebugClass")
        .data<&DebugClass::flag>("flag"_hs).prop(display_name_hs, "flag")
        .data<&DebugClass::a>("a"_hs).prop(display_name_hs, "a")
        .data<&DebugClass::b>("b"_hs).prop(display_name_hs, "b")
        .data<&DebugClass::c>("c"_hs).prop(display_name_hs, "c")
        .data<&DebugClass::position>("position"_hs).prop(display_name_hs, "position")
        .data<&DebugClass::somestring>("somestring"_hs).prop(display_name_hs, "somestring")
        .data<&DebugClass::vector1>("vector1"_hs).prop(display_name_hs, "vector1")
        .data<&DebugClass::vector2>("vector2"_hs).prop(display_name_hs, "vector2")
        .data<&DebugClass::map1>("map1"_hs).prop(display_name_hs, "map1")
        .data<&DebugClass::map2>("map2"_hs).prop(display_name_hs, "map2")
        .data<&DebugClass::set1>("set1"_hs).prop(display_name_hs, "set1")
        .data<&DebugClass::anEnum>("anEnum"_hs).prop(display_name_hs, "anEnum")
        // to_string, member version
            //.func<&DebugClass::to_string>(to_string_hs)
        // to_string, free function version
            //.func<&to_string_DebugClass>(to_string_hs)
        // clone
            //.func<&cloneDebugClass>(clone_hs)
        ;

    entt::meta<AnEnum>()
        .type("AnEnum"_hs)
        .prop(display_name_hs, "AnEnum")
        .prop("underlying_meta_type"_hs, entt::resolve<std::underlying_type_t<AnEnum>>())  // Store underlying type
        //        .prop("name_map"_hs, std::map<int, const char*> {{5,"A"}, {6,"B"}})

        .data<AnEnum::Hello>("Hello"_hs).prop(display_name_hs, "Hello")
        .data<AnEnum::Bye>("Bye"_hs).prop(display_name_hs, "Bye")
        .data<AnEnum::Hola>("Hola"_hs).prop(display_name_hs, "Hola")

        // Not used
        .func < [](const void* value) {
        return *static_cast<const std::underlying_type_t<AnEnum>*>(value);
        } > ("cast_to_underlying"_hs)

            // Not used
            .func < [](const void* value) -> const char* {
            static const std::map<int, const char*> enum_map = {
                { static_cast<int>(AnEnum::Hello), "Hello" },
                { static_cast<int>(AnEnum::Bye), "Bye" }
            };

            auto value_ = static_cast<const std::underlying_type_t<AnEnum>*>(value);
            auto it = enum_map.find(*value_);
            if (it != enum_map.end()) {
                return it->second;  // Return the name if found
            }
            return "Unknown";  // Return a default name if the value is not found
            } > ("name_from_value"_hs)

                //        .func<[](const entt::meta_any& value) -> entt::meta_any {
                //            // Retrieve the underlying type of the enum
                //            using underlying_type = std::underlying_type_t<AnEnum>;
                //
                //            // Attempt to cast the meta_any to the underlying type
                //            return value.allow_cast<underlying_type>();
                //            //if (auto cast_value = value.allow_cast<underlying_type>()) {
                //            //    return *cast_value;  // Return the cast value
                //            //}
                //            //return {};  // Return an empty meta_any if the cast fails
                //        }>("cast_to_underlying"_hs)

                    // to_string, member version
                        //.func<&DebugClass::to_string>(to_string_hs)
                    // to_string, free function version
                        //.func<&to_string_DebugClass>(to_string_hs)
                    // clone
                        //.func<&cloneDebugClass>(clone_hs)
                ;
}

#endif /* DebugClass_h */
