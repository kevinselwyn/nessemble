function custom(...)
    local arg={...}
    local product = 0

    if arg ~= nil and #arg > 0
    then
        for i=1,#arg
        do
            if i == 1
            then
                product = tonumber(arg[i])
            else
                product = product * tonumber(arg[i])
            end
        end
    end

    return string.char(product % 256)
end
