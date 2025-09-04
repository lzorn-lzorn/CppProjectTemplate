#pragma once
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>
#include "Traits/TraitsTools.hpp"
#include "Traits/TypeTraits.hpp"
#include "Proj.hpp"
namespace BaseLib {

struct NulloptTp{
	/* 防止直接从 {} 就能构造出 NulloptTp */
	explicit NulloptTp() = default;
};

inline constexpr NulloptTp nullopt{};

struct NonTrivialDummyTp {
	/* 避免在对象进行值初始化时进行零初始化 */
	constexpr NonTrivialDummyTp() noexcept{} 
};
template <typename Ty, bool = std::is_trivially_destructible_v<Ty>>
struct OptionDestruct {
	union {
		/* 如果你用 char 会触发零初始化机制, 导致最后多生成一条指令 */
		NonTrivialDummyTp dummy;
		std::remove_cv_t<Ty> value;
	};
	bool bHasValue;
	constexpr OptionDestruct() noexcept : dummy{}, bHasValue(false) {}

	template <typename... Types>
	constexpr explicit OptionDestruct(std::in_place_t, Types&&... Args)
		noexcept(std::is_nothrow_constructible_v<Ty, Types...>)
		: value(std::forward<Types>(Args)...), bHasValue(true) {}

	void reset() noexcept {
		bHasValue = false;
	}
};

/*
 * @function: 非平凡析构的需要手动调用 ~Ty()
 */
template <typename Ty>
struct OptionDestruct<Ty, false> {
	union {
		/* 如果你用 char 会触发零初始化机制, 导致最后多生成一条指令 */
		NonTrivialDummyTp dummy;
		std::remove_cv_t<Ty> value;
	};
	bool bHasValue;
	constexpr OptionDestruct() noexcept : dummy{}, bHasValue(false) {}

	template <typename... Types>
	constexpr explicit OptionDestruct(std::in_place_t, Types&&... Args)
		noexcept(std::is_nothrow_constructible_v<Ty, Types...>)
		: value(std::forward<Types>(Args)...), bHasValue(true) {}

	~OptionDestruct() noexcept{
		__Cleanup();
	}
	OptionDestruct(const OptionDestruct&)            = default;
    OptionDestruct(OptionDestruct&&)                 = default;
    OptionDestruct& operator=(const OptionDestruct&) = default;
    OptionDestruct& operator=(OptionDestruct&&)      = default;
	

	void Reset() noexcept {
		__Cleanup();
	}
private:
	void __Cleanup() noexcept{
		if (bHasValue){
			value.~Ty();
			bHasValue = false;
		}
	}
};

template <typename Ty>
struct OptionConstruct : OptionDestruct<Ty> {
	using OptionDestruct<Ty>::OptionDestruct;

	template <typename... Types>
	Ty& __Construct(Types&&... Args) 
		noexcept(std::is_nothrow_constructible_v<Ty, Types...>)
	{
		std::construct_at(this->value, std::forward<Types>(Args)...);
		this->bHasValue = true;
		return this->value;
	}

	template <typename Ty2>
	void __Assign(Ty2&& that)
		noexcept(std::is_nothrow_assignable_v<Ty&, Ty2> && std::is_nothrow_constructible_v<Ty, Ty2>)
	{
		if (this->bHasValue) {
			static_cast<Ty&>(this->value) = std::forward<Ty2>(that);
		}else{
			Construct(std::forward<Ty2>(that));
		}
	}

	template <typename Self>
	void __ConstructFrom(Self&& right)
		noexcept(std::is_nothrow_constructible_v<
			Ty, 
			decltype(*std::forward<Self>(right))
		>)
	{
		if (right.bHasValue){
			__Construct(*std::forward<Self>(right));
		}
	}

	template <typename Self>
	void __AssignFrom(Self&& right)
		noexcept(
			std::is_nothrow_constructible_v<
				Ty, 
				decltype(*std::forward<Self>(right))
			> &&
			std::is_nothrow_assignable_v<
				Ty&, 
				decltype(*std::forward<Self>(right))
			>
		)
	{
		if (right.bHasValue){
			__Assign(*std::forward<Self>(right));
		}else{
			this->__Cleanup();
		}
	}

	[[nodiscard]] constexpr Ty& operator*() & noexcept {
		return this->value;
	}
	[[nodiscard]] constexpr const Ty& operator*() const & noexcept {
		return this->value;
	}
	/* 外部使用 std::move(opt).Value() 就会调用掉这里 */
	/* 一个函数返回 optional 在返回处 func().Value() 也是右值 */
	[[nodiscard]] constexpr Ty&& operator*() && noexcept {
		return std::move(this->value);
	}
	[[nodiscard]] constexpr const Ty&& operator*() const && noexcept {
		return std::move(this->value);
	}
};
template <typename Ty>
class Optional : private Traits::AutoControlSMF<OptionConstruct<Ty>, Ty> {
	using MyBaseClassTp = Traits::AutoControlSMF<OptionConstruct<Ty>, Ty>;

	/*
	 * 为了跨类型的访问 Optional<U> <-->  Optional<T>
    */
	template <class>
	friend class Optional;
public:
	using ValueType = Ty;

	constexpr Optional() noexcept {}
	constexpr Optional(NulloptTp)  noexcept {}

	template <
		typename... Types, 
		std::enable_if_t<std::is_constructible_v<Ty, Types...>, int> = 0
	>
	constexpr explicit Optional(std::in_place_t, Types&& ... Args)
		noexcept (std::is_nothrow_constructible_v<Ty, Types...>)
		: MyBaseClassTp(std::in_place, std::forward<Types>(Args)...){}

	template <
		typename Elem, 
		typename... Types,
		std::enable_if_t<
			std::is_constructible_v<
				Ty, 
				std::initializer_list<Elem>&, 
				Types...
			>, 
			int 
		> = 0
	>
	constexpr explicit Optional(
		std::in_place_t, 
		std::initializer_list<Elem> list,  
		Types&&... args
	) noexcept (
		std::is_nothrow_constructible_v<
			Ty, 
			std::initializer_list<Elem>&, 
			Types...
		>
	)  : MyBaseClassTp (std::in_place, list, std::forward<Types>(args)...) {}

	template <typename Ty2>
	using __IsAllowDirectConversion = std::bool_constant<
		std::conjunction_v< 
			/* 禁止 optional 之间直接转换 */
			std::negation<
				std::is_same<
					std::remove_cvref_t<Ty2>, 
					Optional
				>
			>,
			/* 禁止 std::in_place_t 之间直接转换 */
			std::negation<
				std::is_same<
					std::remove_cvref_t<Ty2>, 
					std::in_place_t
				>
			>,
			std::negation<
				/* 如果 _Ty 是 bool 类型，并且 _Ty2 是 optional 的特化 */
				/* 为了避免 optional<bool> 从 optional<X> 直接转换成布尔值造成歧义 */
				/* Eg: int -> bool 是可以隐式转换的, 但是 Optional<int> -> Optional<bool> 则不可 */
				std::conjunction<
					std::is_same<
						std::remove_cvref_t<Ty>, 
						bool
					>,
					IsSpecializationOf<
						std::remove_cvref_t<Ty2>, 
						Optional
					>
				>
			>,
			/* 可以用 _Ty2 构造 _Ty */
			std::is_constructible<Ty, Ty2>
		>
	>;

	template <
		typename Ty2 = std::remove_cv_t<Ty>, 
		std::enable_if_t<__IsAllowDirectConversion<Ty2>::value, int> = 0
	>
	constexpr explicit(!std::is_convertible_v<Ty2, Ty>) Optional(Ty2&& right)
		noexcept(std::is_nothrow_constructible_v<Ty, Ty2>)
		: MyBaseClassTp(std::in_place, std::forward<Ty2>(right)) {}


	/* 用于判断是否允许将 optional<_Ty2> 自动解包为 _Ty，只有在特殊类型(如 bool)*/
	/* 或者没有安全的构造/转换器存在时才允许自动 unwrap，防止类型歧义和不安全转换*/
	template <typename Ty2>
	using __IsAllowUnwrapping = std::bool_constant<
		std::disjunction_v<
			std::is_same<std::remove_cv_t<Ty>, bool>,
			std::negation<
				std::disjunction<
					std::is_same<Ty, Ty2>,
					std::is_constructible<Ty, Optional<Ty2>&>,
					std::is_constructible<Ty, const Optional<Ty2>&>,
					std::is_constructible<Ty, const Optional<Ty2>>,
					std::is_constructible<Ty, Optional<Ty2>>,
					std::is_convertible<Optional<Ty2>&, Ty>,
					std::is_convertible<const Optional<Ty2>&, Ty>,
					std::is_convertible<const Optional<Ty2>, Ty>,
					std::is_convertible<Optional<Ty2>, Ty>
				>
			>
		>
	>;

	template <
		typename Ty2,
		std::enable_if_t<
			std::conjunction_v<
				__IsAllowUnwrapping<Ty2>,
				std::is_constructible<Ty, const Ty2&>
			>, 
			int
		> = 0
	>
	CONSTEXPR20 explicit(!std::is_convertible_v<const Ty2&, Ty>) Optional (const Optional<Ty2>& that)
		noexcept(std::is_nothrow_constructible_v<Ty, const Ty2&>) 
	{
		if (that){
			this->__Construct(*that);
		}
	}


	template <
		typename Ty2,
		std::enable_if_t<
			std::conjunction_v<
				__IsAllowUnwrapping<Ty2>,
				std::is_constructible<Ty, Ty2&>
			>, 
			int
		> = 0
	>
	CONSTEXPR20 explicit(!std::is_convertible_v<const Ty2&, Ty>) Optional (const Optional<Ty2>&& that)
		noexcept(std::is_nothrow_constructible_v<Ty, Ty2>) 
	{
		if (that){
			this->__Construct(std::move(*that));
		}
	}
};	
}