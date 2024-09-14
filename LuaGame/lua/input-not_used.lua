
-- input.lua
input = {
    axis_left_x = 0.0,
    axis_left_y = 0.0,
    axis_right_x = 0.0,
    axis_right_y = 0.0,
    button_a = false,
    button_b = false,
    button_x = false,
    button_y = false
}

function update_input(
    axis_left_x, 
    axis_left_y,
    axis_right_x,
    axis_right_y,
    button_a,
    button_b,
    button_x,
    button_y
)
    input.axis_left_x = axis_left_x
    input.axis_left_y = axis_left_y
    input.axis_right_x = axis_right_x
    input.axis_right_y = axis_right_y
    input.button_a = button_a
    input.button_b = button_b
    input.button_x = button_x
    input.button_y = button_y
end
