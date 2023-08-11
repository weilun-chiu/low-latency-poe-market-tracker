#include <iostream>
#include <string>
#include "items.h"

extern unsigned char items_json[];
extern unsigned int items_json_len;

int main() {
    std::string content(reinterpret_cast<char*>(items_json), items_json_len);
    std::cout << content << std::endl;
    return 0;
}