#!/usr/bin/env lua

function main(args)
    local product = 0

    for i=1,#args
    do
        if i == 1
        then
            product = tonumber(args[i])
        else
            product = product * tonumber(args[i])
        end
    end

    io.write(string.char(product % 256))
end

main(arg)
