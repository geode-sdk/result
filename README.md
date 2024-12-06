# Result

A result class for C++ implemented mainly for Geode.

The purpose of the class is wrapping the return value of a function which may fail, thus passing the error as part of the return type.

The design of this library is heavily inspired by [rust's Result type](https://doc.rust-lang.org/std/result/)

## Example

```cpp
Result<int> integerDivision(int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    }
    return Ok(a / b);
}

int main() {
    int value = integerDivision(3, 2).unwrapOrDefault();
    assert(value == 1);

    value = integerDivision(3, 0).unwrapOr(0);
    assert(value == 0);
}
```

## Extracting the values
There are tons of conveniency tools to error check and extract the value from a Result. You are encouraged to use them.

### Returning early
Here are the convenience utils for returning early from a Result returning function: 
```cpp
Result<int> myFunction() {
    // !! Only on Clang
    // Returns Err early from the function if the result is an error,
    // otherwise passes the value
    int p1 = GEODE_UNWRAP(integerDivision(3, 2));
    assert(p1 == 1);

    // You can use this macro on MSVC as well if you don't need the value
    // it will return Err early if the result is an error
    GEODE_UNWRAP(integerDivision(3, 2));

    // Returns Err early from the function if the result is an error,
    // otherwise sets the value into the variable
    GEODE_UNWRAP_INTO(int p2, integerDivision(3, 2));
    assert(p2 == 1);

    return Ok(0);
}
```

### Entering into an if block
Here are the convenience utils for entering into an if block with the underlying value:
```cpp
int main() {
    // Only enters the block if the result is ok,
    // setting the value into the variable
    if (GEODE_UNWRAP_IF_OK(p3, integerDivision(3, 2))) {
        assert(p3 == 1);
    }

    // Only enters the block if the result is an error,
    // setting the value into the variable
    if (GEODE_UNWRAP_IF_ERR(e1, integerDivision(3, 0))) {
        assert(e1 == "Division by zero");
    }
}
```

### Setting the value inline
Here are the convenience utils for setting a value inline with manually handling the error:
```cpp
int main() {
    // Enters the trailing block if the result is an error,
    // otherwise sets the value into the variable
    GEODE_UNWRAP_OR_ELSE(p4, err, integerDivision(3, 2)) {
        return -1;
    }
    assert(p4 == 1);

    GEODE_UNWRAP_OR_ELSE(p5, err, integerDivision(3, 0)) {
        p5 = -1;
    }
    assert(p5 == -1);
}
```

### Builtin functions
And here are the functions built into the Result to extract the value:
```cpp
int main() {
    // Returns the value if the result is ok,
    // otherwise returns the default value for type
    int p6 = integerDivision(3, 0).unwrapOrDefault();
    assert(p6 == 0);

    // Returns the value if the result is ok,
    // otherwise returns the passed value
    int p7 = integerDivision(3, 0).unwrapOr(-1);
    assert(p7 == -1);

    // Returns the value if the result is ok,
    // otherwise returns the result of the operation
    int p8 = integerDivision(3, 0).unwrapOrElse([](){
        return -1;
    });
    assert(p7 == -1);

    // NOT RECOMMENDED!!!
    // Returns the value if the result is ok,
    // otherwise **throws an exception**
    int p9 = integerDivision(3, 2).unwrap();
    std::string e2 = integerDivision(3, 0).unwrapErr();
}
```

## Manipulating the values
There are lots of ways to manipulate a Result to better fit the required use case. Feel free to use as you please.

### Chaining results
Here are the convenience utils for chaining results:
```cpp
int main() {
    // Returns the passed result if the result is ok,
    // otherwise returns the result of itself
    int v1 = integerDivision(3, 2).and_(integerDivision(5, 2)).unwrapOrDefault();
    assert(v1 == 2);

    // Returns the passed result if the result is err,
    // otherwise returns the result of itself
    int v2 = integerDivision(3, 2).or_(integerDivision(5, 2)).unwrapOrDefault();
    assert(v2 == 1);

    // Returns the result of the operation if the result is ok,
    // otherwise returns the result of itself
    int v3 = integerDivision(3, 2).andThen([](){
        return Ok(10);
    }).unwrapOrDefault();
    assert(v3 == 10);

    // Returns the result of the operation if the result is err,
    // otherwise returns the result of itself
    int v4 = integerDivision(3, 0).orElse([](){
        return Ok(-10);
    }).unwrapOrDefault();
    assert(v4 == 10);
}
```

### Mapping values
Here are the convenience utils for mapping values:
```cpp
int main() {
    // Maps the value if the result is ok,
    // otherwise returns the result of itself
    if (GEODE_UNWRAP_IF_OK(v5, integerDivision(3, 2).map([](int v){
        return v * 2;
    }))) {
        assert(v5 == 2);
    }

    // Maps the error if the result is err,
    // otherwise returns the result of itself
    if (GEODE_UNWRAP_IF_ERR(e1, integerDivision(3, 0).mapErr([](auto const& e){
        return e + "!";
    }))) {
        assert(e1 == "Division by zero!");
    }

    // Maps the value if the result is ok,
    // otherwise returns the result of the operation
    int v6 = integerDivision(3, 2).mapOrElse([](){
        return -1;
    }, [](int v){
        return v * 2;
    });
    assert(v6 == 2);

    // Maps the value if the result is ok,
    // otherwise returns the given value
    int v7 = integerDivision(3, 2).mapOr(-1, [](int v){
        return v * 2;
    });
    assert(v7 == 2);
}
```