#include <iostream>
#include <string>
#include <fstream>

// Compile time file to string to save file I/O during runtime
template <typename T>
struct FileToString {
    static const T value;
};

template <typename T>
const T FileToString<T>::value = [] {
    std::ifstream file("items.json");
    if (file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return content;
    }
    return T();
}();

int main() {
    std::cout << FileToString<std::string>::value << std::endl;
    return 0;
}