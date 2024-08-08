#include <Geode/Result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using namespace geode;

Result<int const&, std::string> divideConstRef(int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    } else {
        static int res = a / b;
        return Ok(res);
    }
}

Result<int, std::string const&> divideErrRef(int a, int b) {
    if (b == 0) {
        static std::string err = "Division by zero";
        return Err(err);
    } else {
        return Ok(a / b);
    }
}

Result<int const&, std::string const&> divideConstRefErrRef(int a, int b) {
    if (b == 0) {
        static std::string err = "Division by zero";
        return Err(err);
    } else {
        static int res = a / b;
        return Ok(res);
    }
}

Result<int, void> divideVoidErr(int a, int b) {
    if (b == 0) {
        return Err();
    } else {
        return Ok(a / b);
    }
}

Result<void, void> divideVoidOkVoidErr(int a, int b) {
    if (b == 0) {
        return Err();
    } else {
        return Ok();
    }
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
}