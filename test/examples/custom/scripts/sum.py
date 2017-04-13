import sys

def main(argc, argv):
    output = 0

    for i in range(1, argc):
        output += int(argv[i])

    sys.stdout.write(chr(output % 256))

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
