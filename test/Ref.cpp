#include <Geode/Result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using namespace geode;

Result<int&, std::string> divideRef(int& res, int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    } else {
        res = a / b;
        return Ok(res);
    }
}

TEST_CASE("Ref") {
    SECTION("Ok") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        REQUIRE(res.isOk());
        REQUIRE(res.unwrap() == 16);
        REQUIRE(val == 16);
    }

    SECTION("Err") {
        int val = -256;
        auto res = divideRef(val, 32, 0);
        REQUIRE(res.isErr());
        REQUIRE(res.unwrapErr() == "Division by zero");
        REQUIRE(val == -256);
    }

    SECTION("err") {
        int val = -256;
        auto res = divideRef(val, 32, 0);
        REQUIRE(res.err().value() == "Division by zero");
        REQUIRE(val == -256);
    }

    SECTION("asRef") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto ref = res.asRef();
        REQUIRE((res.isOk() && ref.isOk()));
        REQUIRE(ref.unwrap() == res.unwrap());
        REQUIRE(val == 16);
    }

    SECTION("asConst") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto ref = res.asConst();
        REQUIRE((res.isOk() && ref.isOk()));
        REQUIRE(ref.unwrap() == res.unwrap());
        REQUIRE(val == 16);
    }

    SECTION("unwrapOr") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        int orVal = -64;
        REQUIRE(res.unwrapOr(orVal) == 16);
        REQUIRE(val == 16);

        int val2 = -256;
        auto res2 = divideRef(val2, 32, 0);
        int orVal2 = -64;
        REQUIRE(res2.unwrapOr(orVal2) == -64);
        REQUIRE(val2 == -256);
    }

    SECTION("unwrapOrElse") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        int orVal = -64;
        REQUIRE(res.unwrapOrElse([&]() -> int& { return orVal; }) == 16);
        REQUIRE(val == 16);

        int val2 = -256;
        auto res2 = divideRef(val2, 32, 0);
        int orVal2 = -64;
        REQUIRE(res2.unwrapOrElse([&]() -> int& { return orVal2; }) == -64);
        REQUIRE(val2 == -256);
    }

    SECTION("copied") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = res.copied();
        REQUIRE(res2.unwrap() == 16);
        REQUIRE(val == 16);
    }

    SECTION("map") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = res.map([](int a) { return a / 2; });
        REQUIRE(res2.unwrap() == 8);
        REQUIRE(val == 16);
    }

    SECTION("mapOr") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = res.mapOr(0, [](int a) { return a / 2; });
        REQUIRE(res2 == 8);
        REQUIRE(val == 16);

        int val2 = -256;
        auto res3 = divideRef(val2, 32, 0);
        auto res4 = res3.mapOr(0, [](int a) { return a / 2; });
        REQUIRE(res4 == 0);
        REQUIRE(val2 == -256);
    }

    SECTION("mapOrElse") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = res.mapOrElse([]() { return 0; }, [](int a) { return a / 2; });
        REQUIRE(res2 == 8);
        REQUIRE(val == 16);

        int val2 = -256;
        auto res3 = divideRef(val2, 32, 0);
        auto res4 = res3.mapOrElse([]() { return 0; }, [](int a) { return a / 2; });
        REQUIRE(res4 == 0);
        REQUIRE(val2 == -256);
    }

    SECTION("mapErr") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = res.mapErr([](std::string s) { return s + " mapped"; });
        REQUIRE(res2.unwrap() == 16);
        REQUIRE(val == 16);

        int val2 = -256;
        auto res3 = divideRef(val2, 32, 0);
        auto res4 = res3.mapErr([](std::string s) { return s + " mapped"; });
        REQUIRE(res4.unwrapErr() == "Division by zero mapped");
        REQUIRE(val2 == -256);
    }

    SECTION("inspect") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        res.inspect([](int const& a) { REQUIRE(a == 16); });
        REQUIRE(val == 16);
    }

    SECTION("inspectErr") {
        int val = -256;
        auto res = divideRef(val, 32, 0);
        res.inspectErr([](std::string const& s) { REQUIRE(s == "Division by zero"); });
        REQUIRE(val == -256);
    }

    SECTION("and_") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = divideRef(val, 32, 2);
        auto res3 = res.and_(std::move(res2));
        REQUIRE(res3.unwrap() == 16);
        REQUIRE(val == 16);

        int val2 = -256;
        auto res4 = divideRef(val2, 32, 0);
        auto res5 = divideRef(val2, 32, 2);
        auto res6 = res4.and_(std::move(res5));
        REQUIRE(res6.unwrapErr() == "Division by zero");
        REQUIRE(val2 == 16);
    }

    SECTION("andThen") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = res.andThen([&](int a) { return divideRef(val, a, 2); });
        REQUIRE(res2.unwrap() == 8);
        REQUIRE(val == 8);

        int val2 = -256;
        auto res3 = divideRef(val2, 32, 0);
        auto res4 = res3.andThen([&](int a) { return divideRef(val2, a, 2); });
        REQUIRE(res4.unwrapErr() == "Division by zero");
        REQUIRE(val2 == -256);

        int val3 = -256;
        auto res5 = divideRef(val3, 32, 2);
        auto res6 = res5.andThen([&](int a) { return divideRef(val3, a, 0); });
        REQUIRE(res6.unwrapErr() == "Division by zero");
        REQUIRE(val3 == 16);
    }

    SECTION("or_") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = divideRef(val, 32, 0);
        auto res3 = res.or_(std::move(res2));
        REQUIRE(res3.unwrap() == 16);
        REQUIRE(val == 16);

        int val2 = -256;
        auto res4 = divideRef(val2, 32, 0);
        auto res5 = divideRef(val2, 32, 0);
        auto res6 = res4.or_(std::move(res5));
        REQUIRE(res6.unwrapErr() == "Division by zero");
        REQUIRE(val2 == -256);
    }

    SECTION("orElse") {
        int val = -256;
        auto res = divideRef(val, 32, 2);
        auto res2 = res.orElse([&](std::string error) { return divideRef(val, 32, 0); });
        REQUIRE(res2.unwrap() == 16);
        REQUIRE(val == 16);

        int val2 = -256;
        auto res3 = divideRef(val2, 32, 0);
        auto res4 = res3.orElse([&](std::string error) { return divideRef(val2, 32, 0); });
        REQUIRE(res4.unwrapErr() == "Division by zero");
        REQUIRE(val2 == -256);

        int val3 = -256;
        auto res5 = divideRef(val3, 32, 0);
        auto res6 = res5.orElse([&](std::string error) { return divideRef(val3, 32, 2); });
        REQUIRE(res6.unwrap() == 16);
        REQUIRE(val3 == 16);
    }
} 