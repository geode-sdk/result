# Result

A result class for C++ implemented mainly for Geode.

The purpose of the class is wrapping the return value of a function which may fail, thus passing the error as part of the return type.

The design of this library is heavily inspired by [rust's Result type](https://doc.rust-lang.org/std/result/)

## Example

```cpp
Result<int> integerDivision(int a, int b) {
    if (b == 0)
        return Err("Division by zero");
    return Ok(a / b);
}

int value = integerDivision(3, 2).unwrap(); // 1
value = integerDivision(3, 0).unwrap(); // Throws a runtime error
value = integerDivision(3, 0).unwrapOr(0); // 0
```
