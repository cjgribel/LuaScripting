-- input.lua
input = {
    x = 0.0,
    y = 0.0,
    button_pressed = false
}

function update_input(new_x, new_y, new_button_pressed)
    input.x = new_x
    input.y = new_y
    input.button_pressed = new_button_pressed
end
