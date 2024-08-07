#pragma once
#include <concepts>
#include <utility>
#include <stdexcept>
#include <variant>
#include <optional>

namespace geode {
    template <class OkType, class ErrType>
    class Result;

    namespace impl {
        template <class OkType>
        class OkContainer;

        template <class ErrType>
        class ErrContainer;

        template <class OkType, class ErrType>
        class ResultData;

        template <class OkType, class ErrType>
        class ResultDataWrapper;
    }

    template <class OkType>
    impl::OkContainer<OkType> Ok(OkType&& ok);

    template <class ErrType>
    impl::ErrContainer<ErrType> Err(ErrType&& err);

    impl::OkContainer<void> Ok();

    impl::ErrContainer<void> Err();

    namespace impl {
        template <class OkType>
        class OkContainer final {
        protected:
            OkType m_ok;

        public:
            OkContainer(OkType&& ok) : m_ok(std::move(ok)) {}

            OkType unwrap() {
                return std::move(m_ok);
            }
        };

        template <class OkType>
        requires std::is_reference_v<OkType>
        class OkContainer<OkType> final {
        protected:
            using ProtectedOkType = std::reference_wrapper<std::remove_reference_t<OkType>>;
            ProtectedOkType m_ok;

        public:
            OkContainer(OkType ok) : m_ok(ok) {}

            ProtectedOkType unwrap() {
                return m_ok;
            }
        };

        template <>
        class OkContainer<void> final {
        public:
            OkContainer() {}
        };

        template <class ErrType>
        class ErrContainer final {
        protected:
            ErrType m_err;

        public:
            ErrContainer(ErrType&& err) : m_err(std::move(err)) {}

            ErrType unwrap() {
                return std::move(m_err);
            }
        };

        template <class ErrType>
        requires std::is_reference_v<ErrType>
        class ErrContainer<ErrType> final {
        protected:
            using ProtectedErrType = std::reference_wrapper<std::remove_reference_t<ErrType>>;

            ProtectedErrType m_err;

        public:
            ErrContainer(ErrType err) : m_err(err) {}

            ProtectedErrType unwrap() {
                return m_err;
            }
        };

        template <>
        class ErrContainer<void> final {
        public:
            ErrContainer() {}
        };

        template <class Type>
        struct IsOptionalImpl : std::false_type {
            using type = void;
        };

        template <class Type>
        struct IsOptionalImpl<std::optional<Type>> : std::true_type {
            using type = Type;
        };

        template <class Type>
        concept IsOptional = IsOptionalImpl<Type>::value;

        template <class Type>
        using OptionalType = typename IsOptionalImpl<Type>::type;

        template <class Type>
        struct IsResultImpl : std::false_type {
            using OkType = void;
            using ErrType = void;
        };

        template <class OkType2, class ErrType2>
        struct IsResultImpl<Result<OkType2, ErrType2>> : std::true_type {
            using OkType = OkType2;
            using ErrType = ErrType2;
        };

        template <class Type>
        concept IsResult = IsResultImpl<Type>::value;

        template <class Type>
        using ResultOkType = typename IsResultImpl<Type>::OkType;

        template <class Type>
        using ResultErrType = typename IsResultImpl<Type>::ErrType;
    }

    template <class OkType>
    impl::OkContainer<OkType> Ok(OkType&& ok) {
        return impl::OkContainer<OkType>(std::forward<OkType>(ok));
    }

    template <class ErrType>
    impl::ErrContainer<ErrType> Err(ErrType&& err) {
        return impl::ErrContainer<ErrType>(std::forward<ErrType>(err));
    }

    impl::OkContainer<void> Ok() {
        return impl::OkContainer<void>();
    }

    impl::ErrContainer<void> Err() {
        return impl::ErrContainer<void>();
    }

    namespace impl {

        template <class OkType, class ErrType>
        class ResultData {
        protected:
            using ProtectedOkType = std::conditional_t<std::is_reference_v<OkType>, std::reference_wrapper<std::remove_reference_t<OkType>>, OkType>;
            using ProtectedErrType = std::conditional_t<std::is_reference_v<ErrType>, std::reference_wrapper<std::remove_reference_t<ErrType>>, ErrType>;

            std::variant<ProtectedOkType, ProtectedErrType> m_data;

            template <std::size_t Index, class ClassType>
            requires (Index == 0 ? std::constructible_from<ProtectedOkType, ClassType> : std::constructible_from<ProtectedErrType, ClassType>)
            ResultData(std::in_place_index_t<Index> index, ClassType&& ok) 
                noexcept(Index == 0 ? std::is_nothrow_constructible_v<ProtectedOkType, ClassType> : std::is_nothrow_constructible_v<ProtectedErrType, ClassType>)
                : m_data(index, std::move(ok)) {}

            ResultData(ResultData&& other) 
                noexcept(std::is_nothrow_move_constructible_v<ProtectedOkType> && std::is_nothrow_move_constructible_v<ProtectedErrType>)
                : m_data(std::move(other.m_data)) {}
            
        public:
            bool isOk() const noexcept {
                return m_data.index() == 0;
            }

            bool isErr() const noexcept {
                return m_data.index() == 1;
            }

            OkType unwrap() {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    throw std::runtime_error("Called unwrap on an error Result");
                }
            }

            ErrType unwrapErr() {
                if (isErr()) {
                    return std::move(std::get<1>(m_data));
                } else {
                    throw std::runtime_error("Called unwrapErr on an ok Result");
                }
            }

            Result<OkType&, ErrType&> asRef() noexcept;

            Result<OkType const&, ErrType const&> asConst() const noexcept;

            OkType unwrapOrDefault() noexcept(std::is_nothrow_default_constructible_v<OkType> && std::is_nothrow_move_constructible_v<OkType>) {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    return OkType();
                }
            }

            OkType unwrapOr(OkType&& defaultValue) noexcept(std::is_nothrow_move_constructible_v<OkType>) {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    return std::move(defaultValue);
                }
            }

            OkType unwrapOrElse(std::invocable auto&& operation) noexcept(std::is_nothrow_invocable_v<decltype(operation)>) {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    return operation();
                }
            }

            Result<std::remove_reference_t<OkType>, ErrType> copied() const noexcept(std::is_nothrow_copy_constructible_v<OkType>);

        protected:
            void inspectInternal(std::invocable<OkType const&> auto&& operation) const noexcept(noexcept(operation(std::declval<OkType const&>))) {
                if (this->isOk()) {
                    operation(std::get<0>(m_data));
                }
            }

            void inspectInternalErr(std::invocable<ErrType const&> auto&& operation) const noexcept(noexcept(operation(std::declval<ErrType const&>))) {
                if (this->isErr()) {
                    operation(std::get<1>(m_data));
                }
            }
        };

        template <class ErrType>
        class ResultData<void, ErrType> {
        protected:
            using ProtectedErrType = std::conditional_t<std::is_reference_v<ErrType>, std::reference_wrapper<std::remove_reference_t<ErrType>>, ErrType>;
            std::variant<std::monostate, ProtectedErrType> m_data;

            template <std::size_t Index, class ClassType>
            requires std::constructible_from<ErrType, ClassType>
            ResultData(std::in_place_index_t<Index> index, ClassType&& err) 
                noexcept(std::is_nothrow_constructible_v<ProtectedErrType, ClassType>)
                : m_data(index, std::move(err)) {}

            template <std::size_t Index>
            ResultData(std::in_place_index_t<Index> index) noexcept : m_data(index) {}

            ResultData(ResultData&& other) noexcept(std::is_nothrow_move_constructible_v<ProtectedErrType>)
                : m_data(std::move(other.m_data)) {}

        public:
            bool isOk() const noexcept {
                return m_data.index() == 0;
            }

            bool isErr() const noexcept {
                return m_data.index() == 1;
            }

            ErrType unwrapErr() {
                if (isErr()) {
                    return std::move(std::get<1>(m_data));
                } else {
                    throw std::runtime_error("Called unwrapErr on an ok Result");
                }
            }

            Result<void, ErrType&> asRef() noexcept;

            Result<void, ErrType const&> asConst() const noexcept;

            Result<void, ErrType> copied() const noexcept;

        protected:
            void inspectInternalErr(std::invocable<ErrType const&> auto&& operation) const noexcept(noexcept(operation(std::declval<ErrType const&>))) {
                if (this->isErr()) {
                    operation(std::get<1>(m_data));
                }
            }
        };

        template <class OkType>
        class ResultData<OkType, void> {
        protected:
            using ProtectedOkType = std::conditional_t<std::is_reference_v<OkType>, std::reference_wrapper<std::remove_reference_t<OkType>>, OkType>;
            std::variant<ProtectedOkType, std::monostate> m_data;

            template <std::size_t Index, class ClassType>
            requires std::constructible_from<OkType, ClassType>
            ResultData(std::in_place_index_t<Index> index, ClassType&& ok)
                noexcept(std::is_nothrow_constructible_v<ProtectedOkType, ClassType>)
                : m_data(index, std::move(ok)) {}

            template <std::size_t Index>
            ResultData(std::in_place_index_t<Index> index) noexcept : m_data(index) {}

            ResultData(ResultData&& other) noexcept(std::is_nothrow_move_constructible_v<ProtectedOkType>)
                : m_data(std::move(other.m_data)) {}

        public:
            bool isOk() const noexcept {
                return m_data.index() == 0;
            }

            bool isErr() const noexcept {
                return m_data.index() == 1;
            }

            OkType unwrap() {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    throw std::runtime_error("Called unwrap on an error Result");
                }
            }

            Result<OkType&, void> asRef() noexcept;

            Result<OkType const&, void> asConst() const noexcept;

            OkType unwrapOrDefault() noexcept(std::is_nothrow_default_constructible_v<OkType> && std::is_nothrow_move_constructible_v<OkType>) {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    return OkType();
                }
            }

            OkType unwrapOr(OkType&& defaultValue) noexcept(std::is_nothrow_move_constructible_v<OkType>) {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    return std::move(defaultValue);
                }
            }

            OkType unwrapOrElse(std::invocable auto&& operation) noexcept(std::is_nothrow_invocable_v<decltype(operation)>) {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    return operation();
                }
            }

            Result<std::remove_reference_t<OkType>, void> copied() const noexcept(std::is_nothrow_copy_constructible_v<OkType>);

        protected:
            void inspectInternal(std::invocable<OkType const&> auto&& operation) const noexcept(noexcept(operation(std::declval<OkType const&>))) {
                if (this->isOk()) {
                    operation(std::get<0>(m_data));
                }
            }
        };

        template <>
        class ResultData<void, void> {
        protected:
            std::variant<std::monostate, std::monostate> m_data;

            template <std::size_t Index>
            ResultData(std::in_place_index_t<Index> index) noexcept : m_data(index) {}

            ResultData(ResultData&& other) noexcept {}

        public:
            bool isOk() const noexcept {
                return m_data.index() == 0;
            }

            bool isErr() const noexcept {
                return m_data.index() == 1;
            }

            Result<void, void> asRef() noexcept;

            Result<void, void> asConst() const noexcept;

            Result<void, void> copied() const noexcept;
        };

        template <class OkType, class ErrType>
        class ResultDataWrapper : public ResultData<OkType, ErrType> {
        public:
            template <class OkType2>
            requires std::constructible_from<OkType, OkType2> 
            ResultDataWrapper(OkContainer<OkType2>&& ok) 
                noexcept(std::is_nothrow_constructible_v<ResultData<OkType, ErrType>, std::in_place_index_t<0>, OkType2>)
                : ResultData<OkType, ErrType>(std::in_place_index<0>, std::move(ok.unwrap())) {}

            template <class ErrType2>
            requires std::constructible_from<ErrType, ErrType2>
            ResultDataWrapper(ErrContainer<ErrType2>&& err) 
                noexcept(std::is_nothrow_constructible_v<ResultData<OkType, ErrType>, std::in_place_index_t<1>, ErrType2>)
                : ResultData<OkType, ErrType>(std::in_place_index<1>, std::move(err.unwrap())) {}

            ResultDataWrapper(ResultDataWrapper&& other) noexcept(std::is_nothrow_move_constructible_v<ResultData<OkType, ErrType>>)
                : ResultData<OkType, ErrType>(std::move(other)) {}
        };

        template <class OkType>
        class ResultDataWrapper<OkType, void> : public ResultData<OkType, void> {
        public:
            template <class OkType2>
            requires std::constructible_from<OkType, OkType2> 
            ResultDataWrapper(OkContainer<OkType2>&& ok) 
                noexcept(std::is_nothrow_constructible_v<ResultData<OkType, void>, std::in_place_index_t<0>, OkType2>)
                : ResultData<OkType, void>(std::in_place_index<0>, std::move(ok.unwrap())) {}

            ResultDataWrapper(ErrContainer<void>&& err) 
                noexcept(std::is_nothrow_constructible_v<ResultData<OkType, void>, std::in_place_index_t<1>>)
                : ResultData<OkType, void>(std::in_place_index<1>) {}

            ResultDataWrapper(ResultDataWrapper&& other) noexcept(std::is_nothrow_move_constructible_v<ResultData<OkType, void>>)
                : ResultData<OkType, void>(std::move(other)) {}
        };

        template <class ErrType>
        class ResultDataWrapper<void, ErrType> : public ResultData<void, ErrType> {
        public:
            template <class ErrType2>
            requires std::constructible_from<ErrType, ErrType2>
            ResultDataWrapper(ErrContainer<ErrType2>&& err) 
                noexcept(std::is_nothrow_constructible_v<ResultData<void, ErrType>, std::in_place_index_t<1>, ErrType2>)
                : ResultData<void, ErrType>(std::in_place_index<1>, std::move(err.unwrap())) {}

            ResultDataWrapper(OkContainer<void>&& ok) 
                noexcept(std::is_nothrow_constructible_v<ResultData<void, ErrType>, std::in_place_index_t<0>>)
                : ResultData<void, ErrType>(std::in_place_index<0>) {}

            ResultDataWrapper(ResultDataWrapper&& other) noexcept(std::is_nothrow_move_constructible_v<ResultData<void, ErrType>>)
                : ResultData<void, ErrType>(std::move(other)) {}
        };

        template <>
        class ResultDataWrapper<void, void> : public ResultData<void, void> {
        public:
            ResultDataWrapper(OkContainer<void>&& ok) 
                : ResultData<void, void>(std::in_place_index<0>) {}

            ResultDataWrapper(ErrContainer<void>&& err) 
                : ResultData<void, void>(std::in_place_index<1>) {}

            ResultDataWrapper(ResultDataWrapper&& other) noexcept(std::is_nothrow_move_constructible_v<ResultData<void, void>>)
                : ResultData<void, void>(std::move(other)) {}
        };
    }

    template <class OkType, class ErrType>
    class Result final : public impl::ResultDataWrapper<OkType, ErrType> {
    public:
        using impl::ResultDataWrapper<OkType, ErrType>::ResultDataWrapper;

        bool isOkAnd(std::invocable<OkType> auto&& predicate) noexcept(noexcept(predicate(std::declval<OkType>()))) {
            return this->isOk() && predicate(std::forward<OkType>(this->unwrap()));
        }

        bool isErrAnd(std::invocable<ErrType> auto&& predicate) noexcept(noexcept(predicate(std::declval<ErrType>()))) {
            return this->isErr() && predicate(std::forward<ErrType>(this->unwrapErr()));
        }

        std::optional<OkType> ok() noexcept {
            if (this->isOk()) {
                return this->unwrap();
            } else {
                return std::nullopt;
            }
        }

        std::optional<ErrType> err() noexcept {
            if (this->isErr()) {
                return this->unwrapErr();
            } else {
                return std::nullopt;
            }
        }

        template <class OkType2>
        Result<OkType2, ErrType> map(std::invocable<OkType> auto&& operation) noexcept(noexcept(predicate(std::declval<OkType>())))  {
            if (this->isOk()) {
                return Ok(operation(std::forward<OkType>(this->unwrap())));
            } else {
                return Err(this->unwrapErr());
            }
        }

        template <class OkType2>
        OkType2 mapOr(OkType2&& defaultValue, std::invocable<OkType> auto&& operation) noexcept(noexcept(predicate(std::declval<OkType>()))) {
            if (this->isOk()) {
                return operation(std::forward<OkType>(this->unwrap()));
            } else {
                return std::move(defaultValue);
            }
        }

        auto mapOrElse(std::invocable<OkType> auto&& operation, std::invocable auto&& defaultValue) noexcept(noexcept(predicate(std::declval<OkType>()))) {
            if (this->isOk()) {
                return operation(std::forward<OkType>(this->unwrap()));
            } else {
                return defaultValue();
            }
        }

        template <class ErrType2>
        Result<OkType, ErrType2> mapErr(std::invocable<ErrType> auto&& operation) noexcept(noexcept(predicate(std::declval<ErrType>()))) {
            if (this->isOk()) {
                return Ok(this->unwrap());
            } else {
                return Err(operation(std::forward<ErrType>(this->unwrapErr())));
            }
        }

        Result<OkType, ErrType>& inspect(std::invocable<OkType const&> auto&& operation) const noexcept(noexcept(operation(std::declval<OkType const&>()))) {
            this->inspectInternal(operation);
            return *this;
        }

        Result<OkType, ErrType>& inspectErr(std::invocable<ErrType const&> auto&& operation) const noexcept(noexcept(operation(std::declval<ErrType const&>()))) {
            this->inspectInternalErr(operation);
            return *this;
        }

        template <class OkType2>
        Result<OkType2, ErrType> and_(Result<OkType2, ErrType>&& other) noexcept {
            if (this->isOk()) {
                return std::move(other);
            } else {
                return Err(this->unwrapErr());
            }
        }

        auto andThen(std::invocable<OkType> auto&& operation) noexcept(noexcept(operation(std::declval<OkType>()))) {
            if (this->isOk()) {
                return operation(std::forward<OkType>(this->unwrap()));
            } else {
                return Err(this->unwrapErr());
            }
        }

        template <class ErrType2>
        Result<OkType, ErrType2> or_(Result<OkType, ErrType2>&& other) noexcept {
            if (this->isOk()) {
                return Ok(this->unwrap());
            } else {
                return std::move(other);
            }
        }

        auto orElse(std::invocable<ErrType> auto&& operation) noexcept(noexcept(operation(std::declval<ErrType>()))) {
            if (this->isOk()) {
                return Ok(this->unwrap());
            } else {
                return operation(std::forward<ErrType>(this->unwrapErr()));
            }
        }

        
        std::optional<Result<impl::OptionalType<OkType>, ErrType>> transpose() 
            noexcept(std::is_nothrow_move_constructible_v<OkType> && std::is_nothrow_move_constructible_v<ErrType>) 
            requires(impl::IsOptional<OkType>) {
            if (this->isOk()) {
                auto res = this->unwrap();
                if (res.has_value()) {
                    return Ok(std::move(res.value()));
                } else {
                    return std::nullopt;
                }
            } else {
                return Err(this->unwrapErr());
            }
        }

        Result<OkType, ErrType> flatten()
            noexcept(std::is_nothrow_move_constructible_v<OkType> && std::is_nothrow_move_constructible_v<ErrType>)
            requires(impl::IsResult<OkType> && std::same_as<ErrType, impl::ResultErrType<OkType>>) {
            if (this->isOk()) {
                return this->unwrap();
            } else {
                return Err(this->unwrapErr());
            }
        }
    };

    namespace impl {
        template <class OkType, class ErrType>
        Result<OkType&, ErrType&> ResultData<OkType, ErrType>::asRef() noexcept {
            if (this->isOk()) {
                return Ok(std::get<0>(m_data));
            } else {
                return Err(std::get<1>(m_data));
            }
        }

        template <class ErrType>
        Result<void, ErrType&> ResultData<void, ErrType>::asRef() noexcept {
            if (this->isOk()) {
                return Ok();
            } else {
                return Err(std::get<1>(m_data));
            }
        }

        template <class OkType>
        Result<OkType&, void> ResultData<OkType, void>::asRef() noexcept {
            if (this->isOk()) {
                return Ok(std::get<0>(m_data));
            } else {
                return Err();
            }
        }

        Result<void, void> ResultData<void, void>::asRef() noexcept {
            if (this->isOk()) {
                return Ok();
            } else {
                return Err();
            }
        }

        template <class OkType, class ErrType>
        Result<OkType const&, ErrType const&> ResultData<OkType, ErrType>::asConst() const noexcept {
            if (this->isOk()) {
                return Ok(std::get<0>(m_data));
            } else {
                return Err(std::get<1>(m_data));
            }
        }

        template <class ErrType>
        Result<void, ErrType const&> ResultData<void, ErrType>::asConst() const noexcept {
            if (this->isOk()) {
                return Ok();
            } else {
                return Err(std::get<1>(m_data));
            }
        }

        template <class OkType>
        Result<OkType const&, void> ResultData<OkType, void>::asConst() const noexcept {
            if (this->isOk()) {
                return Ok(std::get<0>(m_data));
            } else {
                return Err();
            }
        }

        Result<void, void> ResultData<void, void>::asConst() const noexcept {
            if (this->isOk()) {
                return Ok();
            } else {
                return Err();
            }
        }

        template <class OkType, class ErrType>
        Result<std::remove_reference_t<OkType>, ErrType> ResultData<OkType, ErrType>::copied() const noexcept(std::is_nothrow_copy_constructible_v<OkType>) {
            if (this->isOk()) {
                return Ok(std::get<0>(m_data));
            } else {
                return Err(std::get<1>(m_data));
            }
        }

        template <class ErrType>
        Result<void, ErrType> ResultData<void, ErrType>::copied() const noexcept {
            if (this->isOk()) {
                return Ok();
            } else {
                return Err(std::get<1>(m_data));
            }
        }

        template <class OkType>
        Result<std::remove_reference_t<OkType>, void> ResultData<OkType, void>::copied() const noexcept(std::is_nothrow_copy_constructible_v<OkType>) {
            if (this->isOk()) {
                return Ok(std::get<0>(m_data));
            } else {
                return Err();
            }
        }

        Result<void, void> ResultData<void, void>::copied() const noexcept {
            if (this->isOk()) {
                return Ok();
            } else {
                return Err();
            }
        }
    }
}