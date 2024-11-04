#include <Geode/Result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using namespace geode;

class IntWrapper {
public:
    IntWrapper(int value) : value(value) {}

    IntWrapper(IntWrapper const& other) = delete;
    IntWrapper(IntWrapper&& other) = default;
    IntWrapper& operator=(IntWrapper const& other) = delete;
    IntWrapper& operator=(IntWrapper&& other) = default;

    int value;
};

Result<IntWrapper, std::string> divideWrapper(int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    }
    else {
        return Ok(IntWrapper(a / b));
    }
}

TEST_CASE("MoveOnly") {
    SECTION("Ok") {
        auto res = divideWrapper(32, 2);
        REQUIRE(res.isOk());
        REQUIRE(res.unwrap().value == 16);
    }

    SECTION("Err") {
        auto res = divideWrapper(32, 0);
        REQUIRE(res.isErr());
        REQUIRE(res.unwrapErr() == "Division by zero");
    }

    SECTION("ok") {
        auto res = divideWrapper(32, 2);
        REQUIRE(res.ok().value().value == 16);
    }

    SECTION("err") {
        auto res = divideWrapper(32, 0);
        REQUIRE(res.err().value() == "Division by zero");
    }

    SECTION("asRef") {
        auto res = divideWrapper(32, 2);
        auto ref = res.asRef();
        REQUIRE((res.isOk() && ref.isOk()));
        REQUIRE(ref.unwrap().value == res.unwrap().value);
    }

    SECTION("asConst") {
        auto res = divideWrapper(32, 2);
        auto ref = res.asConst();
        REQUIRE((res.isOk() && ref.isOk()));
        REQUIRE(ref.unwrap().value == res.unwrap().value);
    }

    SECTION("unwrapOr") {
        auto res = divideWrapper(32, 2);
        REQUIRE(res.unwrapOr(IntWrapper(-64)).value == 16);

        auto res2 = divideWrapper(32, 0);
        REQUIRE(res2.unwrapOr(IntWrapper(-64)).value == -64);
    }

    SECTION("unwrapOrElse") {
        auto res = divideWrapper(32, 2);
        REQUIRE(
            res.unwrapOrElse([]() {
                   return IntWrapper(-64);
               }
            ).value == 16
        );

        auto res2 = divideWrapper(32, 0);
        REQUIRE(
            res2.unwrapOrElse([]() {
                    return IntWrapper(-64);
                }
            ).value == -64
        );
    }

    SECTION("map") {
        auto res = divideWrapper(32, 2);
        auto res2 = res.map([](IntWrapper a) {
            return IntWrapper(a.value / 2);
        });
        REQUIRE(res2.unwrap().value == 8);
    }

    SECTION("mapOr") {
        auto res = divideWrapper(32, 2);
        auto res2 = res.mapOr(IntWrapper(0), [](IntWrapper a) {
            return IntWrapper(a.value / 2);
        });
        REQUIRE(res2.value == 8);

        auto res3 = divideWrapper(32, 0);
        auto res4 = res3.mapOr(IntWrapper(0), [](IntWrapper a) {
            return IntWrapper(a.value / 2);
        });
        REQUIRE(res4.value == 0);
    }

    SECTION("mapOrElse") {
        auto res = divideWrapper(32, 2);
        auto res2 = res.mapOrElse(
            []() {
                return IntWrapper(0);
            },
            [](IntWrapper a) {
                return IntWrapper(a.value / 2);
            }
        );
        REQUIRE(res2.value == 8);

        auto res3 = divideWrapper(32, 0);
        auto res4 = res3.mapOrElse(
            []() {
                return IntWrapper(0);
            },
            [](IntWrapper a) {
                return IntWrapper(a.value / 2);
            }
        );
        REQUIRE(res4.value == 0);
    }

    SECTION("mapErr") {
        auto res = divideWrapper(32, 2);
        auto res2 = res.mapErr([](std::string s) {
            return s + " mapped";
        });
        REQUIRE(res2.unwrap().value == 16);

        auto res3 = divideWrapper(32, 0);
        auto res4 = res3.mapErr([](std::string s) {
            return s + " mapped";
        });
        REQUIRE(res4.unwrapErr() == "Division by zero mapped");
    }

    SECTION("inspect") {
        auto res = divideWrapper(32, 2);
        res.inspect([](IntWrapper const& a) {
            REQUIRE(a.value == 16);
        });
    }

    SECTION("inspectErr") {
        auto res = divideWrapper(32, 0);
        res.inspectErr([](std::string const& s) {
            REQUIRE(s == "Division by zero");
        });
    }

    SECTION("and_") {
        auto res = divideWrapper(32, 2);
        auto res2 = divideWrapper(32, 2);
        auto res3 = res.and_(std::move(res2));
        REQUIRE(res3.unwrap().value == 16);

        auto res4 = divideWrapper(32, 0);
        auto res5 = divideWrapper(32, 2);
        auto res6 = res4.and_(std::move(res5));
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("andThen") {
        auto res = divideWrapper(32, 2);
        auto res2 = res.andThen([](IntWrapper a) {
            return divideWrapper(a.value, 2);
        });
        REQUIRE(res2.unwrap().value == 8);

        auto res3 = divideWrapper(32, 0);
        auto res4 = res3.andThen([](IntWrapper a) {
            return divideWrapper(a.value, 2);
        });
        REQUIRE(res4.unwrapErr() == "Division by zero");

        auto res5 = divideWrapper(32, 2);
        auto res6 = res5.andThen([](IntWrapper a) {
            return divideWrapper(a.value, 0);
        });
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("or_") {
        auto res = divideWrapper(32, 2);
        auto res2 = divideWrapper(32, 0);
        auto res3 = res.or_(std::move(res2));
        REQUIRE(res3.unwrap().value == 16);

        auto res4 = divideWrapper(32, 0);
        auto res5 = divideWrapper(32, 0);
        auto res6 = res4.or_(std::move(res5));
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("orElse") {
        auto res = divideWrapper(32, 2);
        auto res2 = res.orElse([](std::string error) {
            return divideWrapper(32, 0);
        });
        REQUIRE(res2.unwrap().value == 16);

        auto res3 = divideWrapper(32, 0);
        auto res4 = res3.orElse([](std::string error) {
            return divideWrapper(32, 0);
        });
        REQUIRE(res4.unwrapErr() == "Division by zero");

        auto res5 = divideWrapper(32, 0);
        auto res6 = res5.orElse([](std::string error) {
            return divideWrapper(32, 2);
        });
        REQUIRE(res6.unwrap().value == 16);
    }
}