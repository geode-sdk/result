#include <Geode/Result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using namespace geode;

Result<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    }
    else {
        return Ok(a / b);
    }
}

Result<float, std::string> divideFloat(float a, float b) {
    GEODE_UNWRAP_INTO(auto res, divideFloat(a, b));
    return Ok(res);
}

Result<std::optional<int>, std::string> divideOpt(int a, int b) {
    if (a == 0) {
        return Ok(std::nullopt);
    }
    else if (b == 0) {
        return Err("Division by zero");
    }
    else {
        return Ok(std::optional<int>(a / b));
    }
}

Result<Result<int, std::string>, std::string> divideAndDivide(int a, int b) {
    auto res = divide(a, b);
    if (res.isOk()) {
        return Ok(divide(res.unwrap(), b));
    }
    return Err(res.unwrapErr());
}

TEST_CASE("Normal") {
    SECTION("Ok") {
        auto res = divide(32, 2);
        REQUIRE(res.isOk());
        REQUIRE(res.unwrap() == 16);
    }

    SECTION("Err") {
        auto res = divide(32, 0);
        REQUIRE(res.isErr());
        REQUIRE(res.unwrapErr() == "Division by zero");
    }

    SECTION("ok") {
        auto res = divide(32, 2);
        REQUIRE(res.ok().value() == 16);
    }

    SECTION("err") {
        auto res = divide(32, 0);
        REQUIRE(res.err().value() == "Division by zero");
    }

    SECTION("asRef") {
        auto res = divide(32, 2);
        auto ref = res.asRef();
        REQUIRE((res.isOk() && ref.isOk()));
        REQUIRE(ref.unwrap() == res.unwrap());
    }

    SECTION("asConst") {
        auto res = divide(32, 2);
        auto ref = res.asConst();
        REQUIRE((res.isOk() && ref.isOk()));
        REQUIRE(ref.unwrap() == res.unwrap());
    }

    SECTION("unwrapOrDefault") {
        auto res = divide(32, 2);
        REQUIRE(res.unwrapOrDefault() == 16);

        auto res2 = divide(32, 0);
        REQUIRE(res2.unwrapOrDefault() == 0);
    }

    SECTION("unwrapOr") {
        auto res = divide(32, 2);
        REQUIRE(res.unwrapOr(-64) == 16);

        auto res2 = divide(32, 0);
        REQUIRE(res2.unwrapOr(-64) == -64);
    }

    SECTION("unwrapOrElse") {
        auto res = divide(32, 2);
        REQUIRE(res.unwrapOrElse([]() {
            return -64;
        }) == 16);

        auto res2 = divide(32, 0);
        REQUIRE(res2.unwrapOrElse([]() {
            return -64;
        }) == -64);
    }

    SECTION("map") {
        auto res = divide(32, 2);
        auto res2 = res.map([](int a) {
            return a / 2;
        });
        REQUIRE(res2.unwrap() == 8);
    }

    SECTION("mapOr") {
        auto res = divide(32, 2);
        auto res2 = res.mapOr(0, [](int a) {
            return a / 2;
        });
        REQUIRE(res2 == 8);

        auto res3 = divide(32, 0);
        auto res4 = res3.mapOr(0, [](int a) {
            return a / 2;
        });
        REQUIRE(res4 == 0);
    }

    SECTION("mapOrElse") {
        auto res = divide(32, 2);
        auto res2 = res.mapOrElse(
            []() {
                return 0;
            },
            [](int a) {
                return a / 2;
            }
        );
        REQUIRE(res2 == 8);

        auto res3 = divide(32, 0);
        auto res4 = res3.mapOrElse(
            []() {
                return 0;
            },
            [](int a) {
                return a / 2;
            }
        );
        REQUIRE(res4 == 0);
    }

    SECTION("mapErr") {
        auto res = divide(32, 2);
        auto res2 = res.mapErr([](std::string s) {
            return s + " mapped";
        });
        REQUIRE(res2.unwrap() == 16);

        auto res3 = divide(32, 0);
        auto res4 = res3.mapErr([](std::string s) {
            return s + " mapped";
        });
        REQUIRE(res4.unwrapErr() == "Division by zero mapped");
    }

    SECTION("inspect") {
        auto res = divide(32, 2);
        res.inspect([](int const& a) {
            REQUIRE(a == 16);
        });
    }

    SECTION("inspectErr") {
        auto res = divide(32, 0);
        res.inspectErr([](std::string const& s) {
            REQUIRE(s == "Division by zero");
        });
    }

    SECTION("and_") {
        auto res = divide(32, 2);
        auto res2 = divide(32, 2);
        auto res3 = res.and_(std::move(res2));
        REQUIRE(res3.unwrap() == 16);

        auto res4 = divide(32, 0);
        auto res5 = divide(32, 2);
        auto res6 = res4.and_(std::move(res5));
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("andThen") {
        auto res = divide(32, 2);
        auto res2 = res.andThen([](int a) {
            return divide(a, 2);
        });
        REQUIRE(res2.unwrap() == 8);

        auto res3 = divide(32, 0);
        auto res4 = res3.andThen([](int a) {
            return divide(a, 2);
        });
        REQUIRE(res4.unwrapErr() == "Division by zero");

        auto res5 = divide(32, 2);
        auto res6 = res5.andThen([](int a) {
            return divide(a, 0);
        });
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("or_") {
        auto res = divide(32, 2);
        auto res2 = divide(32, 0);
        auto res3 = res.or_(std::move(res2));
        REQUIRE(res3.unwrap() == 16);

        auto res4 = divide(32, 0);
        auto res5 = divide(32, 0);
        auto res6 = res4.or_(std::move(res5));
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("orElse") {
        auto res = divide(32, 2);
        auto res2 = res.orElse([](std::string error) {
            return divide(32, 0);
        });
        REQUIRE(res2.unwrap() == 16);

        auto res3 = divide(32, 0);
        auto res4 = res3.orElse([](std::string error) {
            return divide(32, 0);
        });
        REQUIRE(res4.unwrapErr() == "Division by zero");

        auto res5 = divide(32, 0);
        auto res6 = res5.orElse([](std::string error) {
            return divide(32, 2);
        });
        REQUIRE(res6.unwrap() == 16);
    }

    SECTION("transpose") {
        auto res = divideOpt(32, 2);
        auto res2 = res.transpose();
        REQUIRE(res2.has_value());
        REQUIRE(res2.value().isOk());
        REQUIRE(res2.value().unwrap() == 16);

        auto res3 = divideOpt(32, 0);
        auto res4 = res3.transpose();
        REQUIRE(res4.has_value());
        REQUIRE(res4.value().isErr());
        REQUIRE(res4.value().unwrapErr() == "Division by zero");

        auto res5 = divideOpt(0, 2);
        auto res6 = res5.transpose();
        REQUIRE(res6.has_value() == false);
    }

    SECTION("flatten") {
        auto res = divideAndDivide(32, 2);
        auto res2 = res.flatten();
        REQUIRE(res2.unwrap() == 8);

        auto res3 = divideAndDivide(32, 0);
        auto res4 = res3.flatten();
        REQUIRE(res4.unwrapErr() == "Division by zero");
    }

    SECTION("equality") {
        Result<float, std::string> res1 = Ok(32.f);
        Result<float, std::string> res2 = Ok(16.f);
        Result<float, std::string> res3 = Err("Division by zero");

        REQUIRE(res1 == res1);
        REQUIRE(res1 != res2);
        REQUIRE(res1 != res3);
        REQUIRE(res1 == Ok(32.f));
        REQUIRE(res1 != Ok(16.f));
        REQUIRE(res1 != Err("Division by zero"));
        REQUIRE(res3 == Err("Division by zero"));
        // REQUIRE(res3 != Err(32.f)); // compilation error
    }
}