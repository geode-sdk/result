#include <Geode/Result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using namespace geode;

Result<void, std::string> divideVoidOk(int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    } else {
        return Ok();
    }
}

TEST_CASE("Void") {
    SECTION("Ok") {
        auto res = divideVoidOk(32, 2);
        REQUIRE(res.isOk());
    }

    SECTION("Err") {
        auto res = divideVoidOk(32, 0);
        REQUIRE(res.isErr());
        REQUIRE(res.unwrapErr() == "Division by zero");
    }

    SECTION("map") {
        auto res = divideVoidOk(32, 2);
        auto res2 = res.map([]() { return 16; });
        REQUIRE(res2.isOk());
        REQUIRE(res2.unwrap() == 16);
    }

    SECTION("mapOr") {
        auto res = divideVoidOk(32, 2);
        auto res2 = res.mapOr(0, []() { return 16; });
        REQUIRE(res2 == 16);

        auto res3 = divideVoidOk(32, 0);
        auto res4 = res3.mapOr(0, []() { return 16; });
        REQUIRE(res4 == 0);
    }

    SECTION("mapOrElse") {
        auto res = divideVoidOk(32, 2);
        auto res2 = res.mapOrElse([]() { return 0; }, []() { return 16; });
        REQUIRE(res2 == 16);

        auto res3 = divideVoidOk(32, 0);
        auto res4 = res3.mapOrElse([]() { return 0; }, []() { return 16; });
        REQUIRE(res4 == 0);
    }

    SECTION("mapErr") {
        auto res = divideVoidOk(32, 0);
        auto res2 = res.mapErr([]() -> std::string { return "Division by zero mapped"; });
        REQUIRE(res2.isErr());
        REQUIRE(res2.unwrapErr() == "Division by zero mapped");
    }

    SECTION("and_") {
        auto res = divideVoidOk(32, 2);
        auto res2 = divideVoidOk(32, 2);
        auto res3 = res.and_(std::move(res2));
        REQUIRE(res3.isOk());

        auto res4 = divideVoidOk(32, 0);
        auto res5 = divideVoidOk(32, 2);
        auto res6 = res4.and_(std::move(res5));
        REQUIRE(res6.isErr());
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("andThen") {
        auto res = divideVoidOk(32, 2);
        auto res2 = res.andThen([]() { return divideVoidOk(16, 2); });
        REQUIRE(res2.isOk());

        auto res3 = divideVoidOk(32, 0);
        auto res4 = res3.andThen([]() { return divideVoidOk(16, 2); });
        REQUIRE(res4.isErr());
        REQUIRE(res4.unwrapErr() == "Division by zero");

        auto res5 = divideVoidOk(32, 2);
        auto res6 = res5.andThen([]() { return divideVoidOk(16, 0); });
        REQUIRE(res6.isErr());
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("or_") {
        auto res = divideVoidOk(32, 2);
        auto res2 = divideVoidOk(32, 0);
        auto res3 = res.or_(std::move(res2));
        REQUIRE(res3.isOk());

        auto res4 = divideVoidOk(32, 0);
        auto res5 = divideVoidOk(32, 0);
        auto res6 = res4.or_(std::move(res5));
        REQUIRE(res6.isErr());
        REQUIRE(res6.unwrapErr() == "Division by zero");
    }

    SECTION("orElse") {
        auto res = divideVoidOk(32, 2);
        auto res2 = res.orElse([]() { return divideVoidOk(32, 0); });
        REQUIRE(res2.isOk());

        auto res3 = divideVoidOk(32, 0);
        auto res4 = res3.orElse([]() { return divideVoidOk(32, 0); });
        REQUIRE(res4.isErr());
        REQUIRE(res4.unwrapErr() == "Division by zero");

        auto res5 = divideVoidOk(32, 0);
        auto res6 = res5.orElse([]() { return divideVoidOk(32, 2); });
        REQUIRE(res6.isOk());
    }
}