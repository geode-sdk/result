#include <Geode/Result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

using namespace geode;

namespace {
struct CustomString {
    CustomString(std::string value) : m_value(std::move(value)) {}

    operator std::string() const {
        return m_value;
    }

private:
    std::string m_value;
};

struct Base {
    explicit Base(int value) : value(value) {}

    virtual ~Base() = default;

    int value;
};

struct Derived : Base {
    explicit Derived(int value) : Base(value) {}
};

struct ThrowingConversion {
    ThrowingConversion() = default;

    ThrowingConversion(ThrowingConversion const&) {}

    operator int() const {
        return 0;
    }
};

struct NothrowConversion {
    NothrowConversion() = default;

    NothrowConversion(NothrowConversion const&) noexcept {}

    operator int() const noexcept {
        return 0;
    }
};

Result<void, CustomString> returnsCustomString() {
    return Err(CustomString("Custom error message"));
}

Result<void, std::string> propagatesCustomString() {
    GEODE_UNWRAP(returnsCustomString());
    return Ok();
}

Result<void, std::string> forwardsCustomString() {
    return returnsCustomString();
}
}

TEST_CASE("Convert") {
    SECTION("Same Type") {
        Result<int, std::string> src = Ok(16);

        Result<int, std::string> copy = src;
        REQUIRE(copy.isOk());
        REQUIRE(copy.unwrap() == 16);
        REQUIRE(src.isOk());

        Result<int, std::string> moved = std::move(copy);
        REQUIRE(moved.isOk());
        REQUIRE(moved.unwrap() == 16);
    }

    SECTION("Value Types") {
        SECTION("Ok") {
            Result<int, std::string> src = Ok(16);

            Result<long, std::string> res = src;
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16L);
            REQUIRE(src.unwrap() == 16);
        }

        SECTION("Err") {
            Result<int, CustomString> src = Err(CustomString("oops"));

            Result<int, std::string> res = src;
            REQUIRE(res.isErr());
            REQUIRE(res.unwrapErr() == "oops");
        }

        SECTION("Ok and Err") {
            Result<CustomString, int> okSrc = Ok(CustomString("value"));
            Result<std::string, long> okRes = okSrc;
            REQUIRE(okRes.isOk());
            REQUIRE(static_cast<std::string>(okRes.unwrap()) == "value");

            Result<CustomString, int> errSrc = Err(32);
            Result<std::string, long> errRes = errSrc;
            REQUIRE(errRes.isErr());
            REQUIRE(errRes.unwrapErr() == 32L);
        }

        SECTION("Move") {
            Result<int, CustomString> src = Err(CustomString("oops"));

            Result<long, std::string> res = std::move(src);
            REQUIRE(res.isErr());
            REQUIRE(res.unwrapErr() == "oops");
        }
    }

    SECTION("Duplicate Ok and Err Types") {
        SECTION("Ok") {
            Result<int, int> src = Ok(16);

            Result<long, long> res = src;
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16L);
        }

        SECTION("Err") {
            Result<int, int> src = Err(32);

            Result<long, long> res = src;
            REQUIRE(res.isErr());
            REQUIRE(res.unwrapErr() == 32L);
        }

        SECTION("Same Type") {
            Result<int, int> src = Ok(16);

            Result<int, int> res = src;
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16);
        }

        SECTION("Converted To Same Type") {
            Result<CustomString, std::string> okSrc = Ok(CustomString("value"));
            Result<std::string, std::string> okRes = okSrc;
            REQUIRE(okRes.isOk());
            REQUIRE(static_cast<std::string>(okRes.unwrap()) == "value");

            Result<CustomString, std::string> errSrc = Err("oops");
            Result<std::string, std::string> errRes = errSrc;
            REQUIRE(errRes.isErr());
            REQUIRE(errRes.unwrapErr() == "oops");
        }
    }

    SECTION("Void Ok Type") {
        SECTION("Err") {
            auto src = returnsCustomString();

            Result<void, std::string> copy = src;
            REQUIRE(copy.isErr());
            REQUIRE(copy.unwrapErr() == "Custom error message");

            Result<void, std::string> moved = std::move(src);
            REQUIRE(moved.isErr());
            REQUIRE(moved.unwrapErr() == "Custom error message");
        }

        SECTION("Ok") {
            Result<void, CustomString> src = Ok();

            Result<void, std::string> res = src;
            REQUIRE(res.isOk());
        }
    }

    SECTION("Void Err Type") {
        Result<CustomString, void> src = Ok(CustomString("value"));

        Result<std::string, void> res = src;
        REQUIRE(res.isOk());
        REQUIRE(static_cast<std::string>(res.unwrap()) == "value");
    }

    SECTION("Void Ok and Err Types") {
        Result<void, void> src = Ok();

        Result<void, void> copy = src;
        REQUIRE(copy.isOk());

        Result<void, void> moved = std::move(src);
        REQUIRE(moved.isOk());
    }

    SECTION("Reference Types") {
        SECTION("Reference to Value") {
            int value = 16;
            Result<int&, std::string> src = Ok(value);

            Result<long, std::string> res = src;
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16L);
        }

        SECTION("Reference to Const Reference") {
            int value = 16;
            Result<int&, std::string> src = Ok(value);

            Result<int const&, std::string> res = src;
            REQUIRE(res.isOk());
            REQUIRE(&res.unwrap() == &value);
        }

        SECTION("Const Reference to Value") {
            int value = 16;
            Result<int const&, std::string> src = Ok(value);

            Result<long, std::string> res = src;
            REQUIRE(res.isOk());
            REQUIRE(res.unwrap() == 16L);
        }

        SECTION("Same Reference Type") {
            int value = 16;
            Result<int&, std::string> src = Ok(value);

            Result<int&, std::string> copy = src;
            REQUIRE(&copy.unwrap() == &value);

            Result<int&, std::string> moved = std::move(copy);
            REQUIRE(&moved.unwrap() == &value);
        }

        SECTION("Err Reference to Const Reference") {
            std::string message = "oops";
            Result<void, std::string&> src = Err(message);

            Result<void, std::string const&> res = src;
            REQUIRE(res.isErr());
            REQUIRE(&res.unwrapErr() == &message);
        }

        SECTION("Err Reference to Value") {
            std::string message = "oops";
            Result<void, std::string&> src = Err(message);

            Result<void, std::string> res = src;
            REQUIRE(res.isErr());
            REQUIRE(res.unwrapErr() == "oops");
        }
    }

    SECTION("Move Only Types") {
        Result<std::unique_ptr<Derived>, std::string> src = Ok(std::make_unique<Derived>(16));

        Result<std::unique_ptr<Base>, std::string> res = std::move(src);
        REQUIRE(res.isOk());
        REQUIRE(res.unwrap()->value == 16);
    }

    SECTION("Noexcept") {
        STATIC_REQUIRE(
            std::is_nothrow_constructible_v<Result<long, NothrowConversion>, Result<int, NothrowConversion> const&>
        );
        STATIC_REQUIRE(
            std::is_nothrow_constructible_v<Result<long, NothrowConversion>, Result<int, NothrowConversion>&&>
        );
        STATIC_REQUIRE(
            std::is_nothrow_constructible_v<Result<long, int>, Result<NothrowConversion, int> const&>
        );

        STATIC_REQUIRE_FALSE(
            std::is_nothrow_constructible_v<Result<long, ThrowingConversion>, Result<int, ThrowingConversion> const&>
        );
        STATIC_REQUIRE_FALSE(
            std::is_nothrow_constructible_v<Result<long, ThrowingConversion>, Result<int, ThrowingConversion>&&>
        );
        STATIC_REQUIRE_FALSE(
            std::is_nothrow_constructible_v<Result<long, int>, Result<ThrowingConversion, int> const&>
        );
    }

    SECTION("Unwrap Macros") {
        auto propagated = propagatesCustomString();
        REQUIRE(propagated.isErr());
        REQUIRE(propagated.unwrapErr() == "Custom error message");

        auto forwarded = forwardsCustomString();
        REQUIRE(forwarded.isErr());
        REQUIRE(forwarded.unwrapErr() == "Custom error message");
    }
}
