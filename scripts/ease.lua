--| String variables

local strings = {}
local string_index = 0

--| `add_string` function

function add_string(str)
    strings[string_index] = string.gsub(str, "\"", "")
    string_index = string_index + 1
end

--| Easing functions

local easing_funcs = {}

easing_funcs["easeInQuad"] = function (t, b, c, d)
    local new_t = t / d

    return c * new_t * new_t + b
end

easing_funcs["easeOutQuad"] = function (t, b, c, d)
    local new_t = t / d

    return -c * new_t * (new_t - 2) + b
end

easing_funcs["easeInOutQuad"] = function (t, b, c, d)
    local new_t = t / (d / 2)
    local lower_t = 0

    if new_t < 1
    then
        return c / 2 * new_t * new_t + b
    end

    lower_t = new_t - 1

    return -c / 2 * (lower_t * (lower_t - 2) - 1) + b
end

easing_funcs["easeInCubic"] = function (t, b, c, d)
    local new_t = t / d

    return c * new_t * new_t * new_t + b
end

easing_funcs["easeOutCubic"] = function (t, b, c, d)
    local new_t = (t / d) - 1

    return c * (new_t * new_t * new_t + 1) + b
end

easing_funcs["easeInOutCubic"] = function (t, b, c, d)
    local new_t = t / (d / 2)
    local lower_t = 0

    if new_t < 1
    then
        return c / 2 * new_t * new_t * new_t + b
    end

    lower_t = new_t - 2

    return c / 2 * (lower_t * lower_t * lower_t + 2) + b
end

easing_funcs["easeInQuint"] = function (t, b, c, d)
    local new_t = t / d

    return c * new_t * new_t * new_t * new_t * new_t + b
end

easing_funcs["easeOutQuint"] = function (t, b, c, d)
    local new_t = (t / d) - 1

    return c * (new_t * new_t * new_t * new_t * new_t + 1) + b
end

easing_funcs["easeInOutQuint"] = function (t, b, c, d)
    local new_t = (t / (d / 2))
    local lower_t = 0

    if new_t < 1
    then
        return c / 2 * new_t * new_t * new_t * new_t * new_t + b
    end

    lower_t = new_t - 2

    return c / 2 * (lower_t * lower_t * lower_t * lower_t * lower_t + 2) + b
end

easing_funcs["easeInBounce"] = function (t, b, c, d)
    return c - easing_funcs["easeOutBounce"](d - t, 0, c, d) + b
end

easing_funcs["easeOutBounce"] = function (t, b, c, d)
    local new_t = t / d
    local lower_t = 0
    local magic = 7.5625

    if new_t < (1 / 2.75)
    then
        return c * (magic * new_t * new_t) + b
    elseif new_t < (2 / 2.75)
    then
        lower_t = new_t - (1.5 / 2.75)
        return c * (magic * lower_t * lower_t + 0.75) + b
    elseif new_t < (2.5 / 2.75)
    then
        lower_t = new_t - (2.25 / 2.75)
        return c * (magic * lower_t * lower_t + 0.9375) + b
    else
        lower_t = new_t - (2.625 / 2.75)
        return c * (magic * lower_t * lower_t + 0.984375) + b
    end
end

easing_funcs["easeInOutBounce"] = function (t, b, c, d)
    if t < d / 2
    then
        return easing_funcs["easeInBounce"](t * 2, 0, c, d) * 0.5 + b
    end

    return easing_funcs["easeOutBounce"](t * 2 - d, 0, c, d) * 0.5 + c * 0.5 + b
end

--| Custom function

function custom(...)
    local arg={...}
    local arg_start, arg_end, arg_steps = 0, 16, 16
    local arg_ease, arg_func = strings[0], nil
    local arg_result = ""

    if arg_ease == nil
    then
        error("Missing easing type")
    end

    arg_func = easing_funcs[arg_ease]

    if arg_func == nil
    then
        error("Invalid easing type `" .. arg_ease .. "`")
    end

    if arg[0] ~= nil
    then
        arg_start = arg[0]
    end

    if arg[1] ~= nil
    then
        arg_end = arg[1]
    end

    if arg[2] ~= nil
    then
        arg_steps = arg[2]
    end

    if arg_steps > 255
    then
        arg_steps = 255
    end

    for i=0, arg_steps-1
    do
        value = 0

        if arg_end > arg_start
        then
            value = arg_func(i / (arg_steps - 1), 0, 1, 1) * (arg_end - arg_start)
            value = math.floor(value + arg_start)
        else
            value = arg_func((arg_steps - i - 1) / (arg_steps - 1), 0, 1, 1) * (arg_start - arg_end)
            value = math.floor(value + arg_end)
        end

        arg_result = arg_result .. string.char(value % 256)
    end

    return arg_result
end
