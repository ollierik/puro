import os

def get_include_from_line(line):
    try:
        include = line.split(" ")[1]
        return include.strip()
    except:
        return None

def is_local_include(include):
    return (include[0] == '\"')

def strip_pragma(line):
    if line.strip() == '#pragma once':
        return None
    return line

def include_file_contents(path, output):
    f = open(path, "r")
    lines = f.readlines()
    for line in lines:
        line = strip_pragma(line)
        if line is not None:
            output.append(line)
    f.close()

def add_to_library_includes(lib_includes, include):
    if include not in lib_includes:
        lib_includes.append(include)

def create_single_file_header(header_file_path, output_path):

    dirname = os.path.dirname(header_file_path)
    filename = os.path.basename(header_file_path)

    os.chdir(dirname)

    header_file = open(filename, "r")
    lines = header_file.readlines()

    lib_includes = []
    output = []

    for line in lines:
        line = strip_pragma(line)

        if line is None:
            continue

        include = get_include_from_line(line)

        if include is None:
            continue  # skip empty lines

        if is_local_include(include):
            path = include.replace('\"', '')
            include_file_contents(path, output)
        else:
            lib_includes.append(include)

    header_file.close()

    # construct output file
    output_file = open(output_path, 'w')

    output_file.write('#pragma once\n')
    for include in lib_includes:
        output_file.write('#include ' + include + '\n')

    for line in output:
        output_file.write(line)

    output_file.close()

if __name__ == "__main__":
    create_single_file_header("../src/PuroHeader.h", "../src/puro.hpp")
