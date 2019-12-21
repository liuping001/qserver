.PHONY : clean
toml_cpp :
	@echo $(shell git clone https://github.com/liuping001/toml_cpp.git toml_cpp_src)
	@echo $(shell cd toml_cpp_src && mkdir build && cd build && cmake .. && make)
	@echo $(shell cp toml_cpp_src/toml_cpp toml_cpp)
	@echo $(shell rm -rf toml_cpp_src)

clean :
	@echo $(shell rm -rf toml_cpp_src)
