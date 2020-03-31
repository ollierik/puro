# Used with Python 3.7
import os

lib_includes = []
output = []

def get_include_from_line(line):
    try:
        first = line.split(" ")[0].strip()
        if first == "#include":
            include = line.split(" ")[1]
            return include.strip()
        else:
            return None
    except:
        return None

def get_file_extension(path):
    return os.path.splitext(path)[1]

def is_local_include(include):
    return (include[0] == '\"')

def strip_pragma(line):
    if line.strip() == '#pragma once':
        return None
    return line


def add_to_library_includes(include):
    if include not in lib_includes:
        lib_includes.append(include)

def try_to_process_include(line):

    include = get_include_from_line(line)

    if include is None:
        return False

    if is_local_include(include):
        path = include.replace('\"', '')
        include_file_contents(path)
    else:
        lib_includes.append(include)
    return True

def process_line(line):

    line = strip_pragma(line)

    if line is None:
        return

    if not try_to_process_include(line):
        output.append(line)

def add_file_name_tag(path):
    output.append('\n///////////////////////////////////////////\n')
    output.append("// " + path + '\n')
    output.append('///////////////////////////////////////////\n')

def include_file_contents(path):
    print("Including", path)
    f = open(path, "r")
    add_file_name_tag(path)
    lines = f.readlines()
    for line in lines:
        process_line(line)
    f.close()


def create_single_file_header(input_file_path, output_path):

    dirname = os.path.dirname(input_file_path)
    filename = os.path.basename(input_file_path)

    os.chdir(dirname)

    include_file_contents(filename)

    if os.path.exists(output_path):
        print("Removing existing:", output_path)
        os.remove(output_path)

    output_file = open(output_path, 'w')

    # add pragma once if this is a header file
    if get_file_extension(output_path) in [".h", ".hpp"]:
        output_file.write('#pragma once\n')

    for include in lib_includes:
        output_file.write('#include ' + include + '\n')

    for line in output:
        output_file.write(line)

    output_file.close()
    print("Done, wrote to:", output_path)

if __name__ == "__main__":
    create_single_file_header("../src/main.cpp", "../src/export.cpp")
