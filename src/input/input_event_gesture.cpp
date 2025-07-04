#include "input_event_gesture.h"

void sunaba::input::bindInputEventGesture(sol::state_view& lua) {
    lua.new_usertype<InputEventGesture>("InputEventGesture",
        "new", sol::factories(
            []() { return new InputEventGesture(); }
        ),
        sol::base_classes, sol::bases<sunaba::core::BaseObject, sunaba::core::Resource, sunaba::input::InputEvent>(),
        "position", sol::property(&InputEventGesture::getPosition, &InputEventGesture::setPosition),
        "cast", [](sunaba::core::Resource* instance) {
            return new InputEventGesture(godot::Object::cast_to<GodotInputEventGesture>(instance->getResource()));
        }
    );
}