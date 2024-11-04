#include <Geode/Result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using namespace geode;

constexpr Result<int, int> divideConstexpr(int a, int b) {
    if (b == 0) {
        return Err(-1); // Using -1 as an error code for division by zero
    }
    else {
        return Ok(a / b);
    }
}

Result<int const&, std::string> divideConstRef(int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    }
    else {
        static int res = a / b;
        return Ok(res);
    }
}

Result<int, std::string const&> divideErrRef(int a, int b) {
    if (b == 0) {
        static std::string err = "Division by zero";
        return Err(err);
    }
    else {
        return Ok(a / b);
    }
}

Result<int const&, std::string const&> divideConstRefErrRef(int a, int b) {
    if (b == 0) {
        static std::string err = "Division by zero";
        return Err(err);
    }
    else {
        static int res = a / b;
        return Ok(res);
    }
}

Result<int, void> divideVoidErr(int a, int b) {
    if (b == 0) {
        return Err();
    }
    else {
        return Ok(a / b);
    }
}

Result<void, void> divideVoidOkVoidErr(int a, int b) {
    if (b == 0) {
        return Err();
    }
    else {
        return Ok();
    }
}

// Test the special GEODE_UNWRAP macro on void unwrap
Result<int, int> unwrapVoidOk() {
    auto lambda = []() -> Result<void, int> {
        return Ok();
    };
    GEODE_UNWRAP(lambda());
    return Ok(10);
}

TEST_CASE("Misc") {
    SECTION("Void Err") {
        SECTION("Ok") {
            auto res = divideVoidErr(32, 2);
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16);
        }

        SECTION("Err") {
            auto res = divideVoidErr(32, 0);
            REQUIRE(res.isErr());
        }
    }

    SECTION("Void Ok Void Err") {
        SECTION("Ok") {
            auto res = divideVoidOkVoidErr(32, 2);
            REQUIRE(res.isOk());
        }

        SECTION("Err") {
            auto res = divideVoidOkVoidErr(32, 0);
            REQUIRE(res.isErr());
        }
    }

    SECTION("Const Ref") {
        SECTION("Ok") {
            auto res = divideConstRef(32, 2);
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16);
        }

        SECTION("Err") {
            auto res = divideConstRef(32, 0);
            REQUIRE(res.isErr());
            REQUIRE(res.unwrapErr() == "Division by zero");
        }
    }

    SECTION("Err Ref") {
        SECTION("Ok") {
            auto res = divideErrRef(32, 2);
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16);
        }

        SECTION("Err") {
            auto res = divideErrRef(32, 0);
            REQUIRE(res.isErr());
            REQUIRE(res.unwrapErr() == "Division by zero");
        }
    }

    SECTION("Const Ref Err Ref") {
        SECTION("Ok") {
            auto res = divideConstRefErrRef(32, 2);
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16);
        }

        SECTION("Err") {
            auto res = divideConstRefErrRef(32, 0);
            REQUIRE(res.isErr());
            REQUIRE(res.unwrapErr() == "Division by zero");
        }
    }

    SECTION("Constexpr") {
        SECTION("Ok") {
            static_assert(divideConstexpr(32, 2).isOk(), "Expected Ok result");
            static_assert(divideConstexpr(32, 2).unwrap() == 16, "Expected result to be 16");
        }

        SECTION("Err") {
            static_assert(divideConstexpr(32, 0).isErr(), "Expected Err result");
            static_assert(divideConstexpr(32, 0).unwrapErr() == -1, "Expected error code to be -1");
        }
    }

    SECTION("Exceptions") {
        SECTION("Ok") {
            auto res = divideConstexpr(32, 2);
            REQUIRE_NOTHROW(res.unwrap());
            try {
                res.unwrapErr();
                FAIL("Expected UnwrapException to be thrown");
            }
            catch (UnwrapException const& e) {
                REQUIRE(e.what() == std::string("Called unwrapErr on an Ok Result: 16"));
            }
        }

        SECTION("Err") {
            auto res = divideConstexpr(32, 0);
            REQUIRE_NOTHROW(res.unwrapErr());
            try {
                res.unwrap();
                FAIL("Expected UnwrapException to be thrown");
            }
            catch (UnwrapException const& e) {
                REQUIRE(e.what() == std::string("Called unwrap on an Err Result: -1"));
            }
        }
    }
}