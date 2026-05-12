#include "input.h"

#include "util/bitwise.h"

void Input::button_pressed(GbButton button) {
    set_button(button, true);
}

void Input::button_released(GbButton button) {
    set_button(button, false);
}

void Input::set_button(GbButton button, bool set) {
    if (button == GbButton::Up) { up = set; }
    if (button == GbButton::Down) { down = set; }
    if (button == GbButton::Left) { left = set; }
    if (button == GbButton::Right) { right = set; }
    if (button == GbButton::A) { a = set; }
    if (button == GbButton::B) { b = set; }
    if (button == GbButton::Select) { select = set; }
    if (button == GbButton::Start) { start = set; }
}

void Input::write(u8 set) {
    using bitwise::check_bit;

    direction_switch = !check_bit(set, 4);
    button_switch = !check_bit(set, 5);
}

auto Input::get_input() const -> u8 {
    using bitwise::bit_set_to;

    u8 buttons = 0b1111;

    if (direction_switch) {
        buttons = bit_set_to(buttons, 0, !right);
        buttons = bit_set_to(buttons, 1, !left);
        buttons = bit_set_to(buttons, 2, !up);
        buttons = bit_set_to(buttons, 3, !down);
    }

    if (button_switch) {
        buttons = bit_set_to(buttons, 0, !a);
        buttons = bit_set_to(buttons, 1, !b);
        buttons = bit_set_to(buttons, 2, !select);
        buttons = bit_set_to(buttons, 3, !start);
    }

    buttons = bit_set_to(buttons, 4, !direction_switch);
    buttons = bit_set_to(buttons, 5, !button_switch);

    return buttons;
}