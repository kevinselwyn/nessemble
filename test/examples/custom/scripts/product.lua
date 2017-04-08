function custom(...)
    local product = 0

    for i=1,#arg
    do
        if i == 1
        then
            product = tonumber(arg[i])
        else
            product = product * tonumber(arg[i])
        end
    end

    return string.char(product % 256)
end
