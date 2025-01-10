#ifndef Observer_hpp
#define Observer_hpp

#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <iostream>

namespace {
    template <typename T>
    struct get_signature;

    template <typename R, typename Arg>
    struct get_signature<std::function<R(Arg)>>
    {
        using FunctionType = R(Arg);
        using ReturnType = R;
        using ArgType = Arg;
    };
}

// Observer class
class ConditionalObserver
{
    using CallbackType = std::function<void(const std::any&)>;

    std::unordered_map<std::type_index, std::vector<CallbackType>> callback_map;
    std::vector<std::any> events;

    void dispatch_event(const std::any& eventData) const
    {
        std::type_index type = std::type_index(eventData.type());
        auto it = callback_map.find(type);
        if (it != callback_map.end())
        {
            for (const auto& callback : it->second)
            {
                callback(eventData);
            }
        }
    }

public:

    /// Enqueues an event.
    template <typename EventType>
    void enqueue_event(const EventType& event)
    {
        events.push_back(std::any(event));
    }

    /// Dispatches an event immediately.
    template<class EventType>
    void dispatch(const EventType& event)
    {
        dispatch_event(std::any(event));
    }

    /// Dispatches stored events of a given type.
    template<class EventType>
    void dispatch_event_type()
    {
        std::type_index type = typeid(EventType);
        for (const auto& event : events)
        {
            if (event.type() == type)
            {
                dispatch_event(event);
            }
        }
    }

    /// Dispatches all stored events, then clears events.
    void dispatch_all_events()
    {
        for (const auto& event : events)
        {
            dispatch_event(event);
        }
        events.clear();
    }

    /// 
    bool has_pending_events()
    {
        return events.size();
    }

    /// Clears all stored events.
    void clear()
    {
        events.clear();
    }

    /// Registers a callback for a given event type
    template<typename Callable>
    void register_callback(Callable&& callable)
    {
        using EventType = std::decay_t<typename get_signature<decltype(std::function{ callable }) > ::ArgType > ;

        std::function<void(const EventType&)> callback{ std::forward<Callable>(callable) };

        auto wrapped_callback = [callback](const std::any& eventData) {
            if (auto castedEvent = std::any_cast<EventType>(&eventData))
            {
                callback(*castedEvent);
            }
            else
            {
                std::cerr << "Error: Mismatched event type detected." << std::endl;
            }
            };

        callback_map[typeid(EventType)].push_back(wrapped_callback);
    }
};

#endif