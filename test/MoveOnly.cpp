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
        REQUIRE(std::move(res).unwrap().value == 16);
    }

    SECTION("Err") {
        auto res = divideWrapper(32, 0);
        REQUIRE(res.isErr());
        REQUIRE(std::move(res).unwrapErr() == "Division by zero");
    }

    SECTION("ok") {
        auto res = divideWrapper(32, 2);
        REQUIRE(std::move(res).ok().value().value == 16);
    }

    SECTION("err") {
        auto res = divideWrapper(32, 0);
        REQUIRE(std::move(res).err().value() == "Division by zero");
    }

    SECTION("asRef") {
        auto res = divideWrapper(32, 2);
        auto ref = res.asRef();
        REQUIRE((res.isOk() && ref.isOk()));
        REQUIRE(std::move(ref).unwrap().value == std::move(res).unwrap().value);
    }

    SECTION("asConst") {
        auto res = divideWrapper(32, 2);
        auto ref = res.asConst();
        REQUIRE((res.isOk() && ref.isOk()));
        REQUIRE(std::move(ref).unwrap().value == std::move(res).unwrap().value);
    }

    SECTION("unwrapOr") {
        auto res = divideWrapper(32, 2);
        REQUIRE(std::move(res).unwrapOr(IntWrapper(-64)).value == 16);

        auto res2 = divideWrapper(32, 0);
        REQUIRE(std::move(res2).unwrapOr(IntWrapper(-64)).value == -64);
    }

    SECTION("unwrapOrElse") {
        auto res = divideWrapper(32, 2);
        REQUIRE(
            std::move(res)
                .unwrapOrElse([]() {
                    return IntWrapper(-64);
                })
                .value == 16
        );

        auto res2 = divideWrapper(32, 0);
        REQUIRE(
            std::move(res2)
                .unwrapOrElse([]() {
                    return IntWrapper(-64);
                })
                .value == -64
        );
    }

    SECTION("map") {
        auto res = divideWrapper(32, 2);
        auto res2 = std::move(res).map([](IntWrapper a) {
            return IntWrapper(a.value / 2);
        });
        REQUIRE(std::move(res2).unwrap().value == 8);
    }

    SECTION("mapOr") {
        auto res = divideWrapper(32, 2);
        auto res2 = std::move(res).mapOr(IntWrapper(0), [](IntWrapper a) {
            return IntWrapper(a.value / 2);
        });
        REQUIRE(std::move(res2).value == 8);

        auto res3 = divideWrapper(32, 0);
        auto res4 = std::move(res3).mapOr(IntWrapper(0), [](IntWrapper a) {
            return IntWrapper(a.value / 2);
        });
        REQUIRE(std::move(res4).value == 0);
    }

    SECTION("mapOrElse") {
        auto res = divideWrapper(32, 2);
        auto res2 = std::move(res).mapOrElse(
            []() {
                return IntWrapper(0);
            },
            [](IntWrapper a) {
                return IntWrapper(a.value / 2);
            }
        );
        REQUIRE(std::move(res2).value == 8);

        auto res3 = divideWrapper(32, 0);
        auto res4 = std::move(res3).mapOrElse(
            []() {
                return IntWrapper(0);
            },
            [](IntWrapper a) {
                return IntWrapper(a.value / 2);
            }
        );
        REQUIRE(std::move(res4).value == 0);
    }

    SECTION("mapErr") {
        auto res = divideWrapper(32, 2);
        auto res2 = std::move(res).mapErr([](std::string s) {
            return s + " mapped";
        });
        REQUIRE(std::move(res2).unwrap().value == 16);

        auto res3 = divideWrapper(32, 0);
        auto res4 = std::move(res3).mapErr([](std::string s) {
            return s + " mapped";
        });
        REQUIRE(std::move(res4).unwrapErr() == "Division by zero mapped");
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
        auto res3 = std::move(res).and_(std::move(res2));
        REQUIRE(std::move(res3).unwrap().value == 16);

        auto res4 = divideWrapper(32, 0);
        auto res5 = divideWrapper(32, 2);
        auto res6 = std::move(res4).and_(std::move(res5));
        REQUIRE(std::move(res6).unwrapErr() == "Division by zero");
    }

    SECTION("andThen") {
        auto res = divideWrapper(32, 2);
        auto res2 = std::move(res).andThen([](IntWrapper a) {
            return divideWrapper(a.value, 2);
        });
        REQUIRE(std::move(res2).unwrap().value == 8);

        auto res3 = divideWrapper(32, 0);
        auto res4 = std::move(res3).andThen([](IntWrapper a) {
            return divideWrapper(a.value, 2);
        });
        REQUIRE(std::move(res4).unwrapErr() == "Division by zero");

        auto res5 = divideWrapper(32, 2);
        auto res6 = std::move(res5).andThen([](IntWrapper a) {
            return divideWrapper(a.value, 0);
        });
        REQUIRE(std::move(res6).unwrapErr() == "Division by zero");
    }

    SECTION("or_") {
        auto res = divideWrapper(32, 2);
        auto res2 = divideWrapper(32, 0);
        auto res3 = std::move(res).or_(std::move(res2));
        REQUIRE(std::move(res3).unwrap().value == 16);

        auto res4 = divideWrapper(32, 0);
        auto res5 = divideWrapper(32, 0);
        auto res6 = std::move(res4).or_(std::move(res5));
        REQUIRE(std::move(res6).unwrapErr() == "Division by zero");
    }

    SECTION("orElse") {
        auto res = divideWrapper(32, 2);
        auto res2 = std::move(res).orElse([](std::string error) {
            return divideWrapper(32, 0);
        });
        REQUIRE(std::move(res2).unwrap().value == 16);

        auto res3 = divideWrapper(32, 0);
        auto res4 = std::move(res3).orElse([](std::string error) {
            return divideWrapper(32, 0);
        });
        REQUIRE(std::move(res4).unwrapErr() == "Division by zero");

        auto res5 = divideWrapper(32, 0);
        auto res6 = std::move(res5).orElse([](std::string error) {
            return divideWrapper(32, 2);
        });
        REQUIRE(std::move(res6).unwrap().value == 16);
    }
}