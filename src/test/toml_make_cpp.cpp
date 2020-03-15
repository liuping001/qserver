//
// Created by lp on 2019/12/7.
//

#include "third/toml/cpptoml.h"
#include <iostream>

std::ostringstream header_struct;
std::ostringstream body;

void Print(std::shared_ptr<cpptoml::base> ptr, const std::string &key, std::ostringstream &ss) {

    if (ptr->is_array()) {
    } else if (ptr->is_table()) {
        ss << "struct " << key << " {\n";
        for (auto &item : *ptr->as_table()) {
            Print(item.second, item.first, ss);
        }
        ss << "};\n\n";
    } else if (ptr->is_table_array()) {
        ss << "struct " << key << " {\n";
        for (auto &item : **ptr->as_table_array()->begin()) {
            Print(item.second, item.first, ss);
        }
        ss << "};\n\n";
    } else {
    }

    if (ptr->is_array()) {
        ss << "std::vector<std::string> " << key << ";\n";
    } else if (ptr->is_table()) {
        ss << key <<" "<< key <<"; \n";
    } else if (ptr->is_table_array()) {
        ss << "std::vector<" << key <<"> " << key << "\n";
    } else {
        ss <<"std::string "<< key <<";\n";
    }

}

int main(int argc, char *argv[]) {
    auto root = cpptoml::parse_file("test.toml");
    Print(root, "root", body);
    std::cout << body.str() <<"\n";
}

