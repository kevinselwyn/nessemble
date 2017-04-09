def custom(*args):
    output = 0

    for arg in args:
        output += int(arg)

    return chr(output % 256)
