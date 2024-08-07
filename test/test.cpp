#include <Geode/Result.hpp>

#include <iostream>

using namespace geode;

Result<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    } else {
        return Ok(a / b);
    }
}

Result<void, std::string> print(int a) {
    if (a < 0) {
        return Err("Negative number");
    }
    std::cout << a << std::endl;
    return Ok();
}

class MyClass {
public:
    MyClass() = delete;
    MyClass(int a) {}
    MyClass(const MyClass&) = delete;
    MyClass(MyClass&&) = default;
};

Result<MyClass, void> createMyClass(int a) {
    if (a < 0) {
        return Err();
    }
    return Ok(MyClass(a));
}

Result<void, void> tryVoid(int b) {
    if (b < 0) {
        return Err();
    }
    return Ok();
}

Result<MyClass&, std::string> tryRef(MyClass* obj) {
    if (obj == nullptr) {
        return Err("Null pointer");
    }
    return Ok(*obj);
}

int main() {
    MyClass obj(5);
    auto res = tryRef(&obj);
    if (res.isOk()) {
        MyClass& obj = res.unwrap();
        std::cout << "Object is not null" << std::endl;
    } else {
        std::string err = res.unwrapErr();
        std::cout << "Object is null" << std::endl;
    }
}