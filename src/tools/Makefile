.PHONY: toml_cpp_src

tomlcpp:
    $(shell git clone https://github.com/liuping001/toml_cpp.git toml_cpp_src)
    $(shell cd toml_cpp_src && mkdir build && cd build && cmake .. && make)
    $(shell cp toml_cpp_src/toml_cpp toml_cpp)
    $(shell rm -rf toml_cpp_src)