#pragma once
#include <concepts>
#include <utility>
#include <stdexcept>
#include <variant>

namespace geode {
    template <class OkType, class ErrType>
    class Result;

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

        template <class OkType, class ErrType>
        class ResultData {
        protected:
            using ProtectedOkType = std::conditional_t<std::is_reference_v<OkType>, std::reference_wrapper<std::remove_reference_t<OkType>>, OkType>;
            using ProtectedErrType = std::conditional_t<std::is_reference_v<ErrType>, std::reference_wrapper<std::remove_reference_t<ErrType>>, ErrType>;

            std::variant<ProtectedOkType, ProtectedErrType> m_data;

            template <std::size_t Index, class ClassType>
            requires (Index == 1 ? std::constructible_from<ErrType, ClassType> : std::constructible_from<OkType, ClassType>)
            ResultData(std::in_place_index_t<Index> index, ClassType&& ok) : m_data(index, std::move(ok)) {}
            
        public:
            bool isOk() const {
                return m_data.index() == 0;
            }

            bool isErr() const {
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
        };

        template <class ErrType>
        class ResultData<void, ErrType> {
        protected:
            using ProtectedErrType = std::conditional_t<std::is_reference_v<ErrType>, std::reference_wrapper<std::remove_reference_t<ErrType>>, ErrType>;
            std::variant<std::monostate, ProtectedErrType> m_data;

            template <std::size_t Index, class ClassType>
            requires std::constructible_from<ErrType, ClassType>
            ResultData(std::in_place_index_t<Index> index, ClassType&& err) : m_data(index, std::move(err)) {}

            template <std::size_t Index>
            ResultData(std::in_place_index_t<Index> index) : m_data(index) {}

        public:
            bool isOk() const {
                return m_data.index() == 0;
            }

            bool isErr() const {
                return m_data.index() == 1;
            }

            ErrType unwrapErr() {
                if (isErr()) {
                    return std::move(std::get<1>(m_data));
                } else {
                    throw std::runtime_error("Called unwrapErr on an ok Result");
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
            ResultData(std::in_place_index_t<Index> index, ClassType&& ok) : m_data(index, std::move(ok)) {}

            template <std::size_t Index>
            ResultData(std::in_place_index_t<Index> index) : m_data(index) {}

        public:
            bool isOk() const {
                return m_data.index() == 0;
            }

            bool isErr() const {
                return m_data.index() == 1;
            }

            OkType unwrap() {
                if (isOk()) {
                    return std::move(std::get<0>(m_data));
                } else {
                    throw std::runtime_error("Called unwrap on an error Result");
                }
            }
        };

        template <>
        class ResultData<void, void> {
        protected:
            std::variant<std::monostate, std::monostate> m_data;

            template <std::size_t Index>
            ResultData(std::in_place_index_t<Index> index) : m_data(index) {}

        public:
            bool isOk() const {
                return m_data.index() == 0;
            }

            bool isErr() const {
                return m_data.index() == 1;
            }
        };

        template <class OkType, class ErrType>
        class ResultDataWrapper : public ResultData<OkType, ErrType> {
        public:
            template <class OkType2>
            requires std::constructible_from<OkType, OkType2> 
            ResultDataWrapper(OkContainer<OkType2>&& ok) 
                : ResultData<OkType, ErrType>(std::in_place_index<0>, std::move(ok.unwrap())) {}

            template <class ErrType2>
            requires std::constructible_from<ErrType, ErrType2>
            ResultDataWrapper(ErrContainer<ErrType2>&& err) 
                : ResultData<OkType, ErrType>(std::in_place_index<1>, std::move(err.unwrap())) {}
        };

        template <class OkType>
        class ResultDataWrapper<OkType, void> : public ResultData<OkType, void> {
        public:
            template <class OkType2>
            requires std::constructible_from<OkType, OkType2> 
            ResultDataWrapper(OkContainer<OkType2>&& ok) 
                : ResultData<OkType, void>(std::in_place_index<0>, std::move(ok.unwrap())) {}

            ResultDataWrapper(ErrContainer<void>&& err) 
                : ResultData<OkType, void>(std::in_place_index<1>) {}
        };

        template <class ErrType>
        class ResultDataWrapper<void, ErrType> : public ResultData<void, ErrType> {
        public:
            template <class ErrType2>
            requires std::constructible_from<ErrType, ErrType2>
            ResultDataWrapper(ErrContainer<ErrType2>&& err) 
                : ResultData<void, ErrType>(std::in_place_index<1>, std::move(err.unwrap())) {}

            ResultDataWrapper(OkContainer<void>&& ok) 
                : ResultData<void, ErrType>(std::in_place_index<0>) {}
        };

        template <>
        class ResultDataWrapper<void, void> : public ResultData<void, void> {
        public:
            ResultDataWrapper(OkContainer<void>&& ok) 
                : ResultData<void, void>(std::in_place_index<0>) {}

            ResultDataWrapper(ErrContainer<void>&& err) 
                : ResultData<void, void>(std::in_place_index<1>) {}
        };
    }

    template <class OkType, class ErrType>
    class Result final : public impl::ResultDataWrapper<OkType, ErrType> {
    public:
        using impl::ResultDataWrapper<OkType, ErrType>::ResultDataWrapper;
    };

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
}