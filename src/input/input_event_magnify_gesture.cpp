#include "input_event_magnify_gesture.h"

void sunaba::input::bindInputEventMagnifyGesture(sol::state_view& lua) {
    lua.new_usertype<InputEventMagnifyGesture>("InputEventMagnifyGesture",
        "new", sol::factories(
            []() { return new InputEventMagnifyGesture(); }
        ),
        sol::base_classes, sol::bases<sunaba::core::BaseObject, sunaba::core::Resource, sunaba::input::InputEvent, sunaba::input::InputEventGesture>(),
        "factor", sol::property(&InputEventMagnifyGesture::getFactor, &InputEventMagnifyGesture::setFactor),
        "cast", [](sunaba::core::Resource* instance) {
            return new InputEventMagnifyGesture(godot::Object::cast_to<GodotInputEventMagnifyGesture>(instance->getResource()));
        }
    );
}