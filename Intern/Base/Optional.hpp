#pragma once
#include <algorithm>
#include <compare>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include "Traits/TraitsTools.hpp"
#include "Traits/TypeTraits.hpp"
#include "Proj.hpp"

namespace BaseLib {

struct ConstructFromInvokeResultTag{
	explicit constexpr ConstructFromInvokeResultTag() noexcept {

	}
};

struct NulloptTp{
	/* 防止直接从 {} 就能构造出 NulloptTp */
	explicit NulloptTp() = default;
	friend bool operator==(NulloptTp, std::nullopt_t) { return true; }
    friend bool operator==(std::nullopt_t, NulloptTp) { return true; }
};

inline constexpr NulloptTp nullopt{};

struct NonTrivialDummyTp {
	/* 避免在对象进行值初始化时进行零初始化 */
	constexpr NonTrivialDummyTp() noexcept{} 
};

/* 平凡析构 + 指针 => 平凡指针 */
template <typename Ty, bool = std::is_trivially_destructible_v<Ty>>
struct OptionDestruct {
	static_assert(IsPointerVal<Ty>, "The type Ty should not be a pointer, pls use smart ptr");
	union {
		/* 如果你用 char 会触发零初始化机制, 导致最后多生成一条指令 */
		NonTrivialDummyTp dummy;
		std::remove_cv_t<Ty> value;
	};

	constexpr OptionDestruct() noexcept : dummy{}, bHasValue(false) {}

	template <typename Fn, typename Uty>
	constexpr  OptionDestruct(ConstructFromInvokeResultTag, Fn&& fn, Uty&& arg)
		noexcept(
			noexcept(
				static_cast<Ty>(std::invoke(std::forward<Fn>(fn),  std::forward<Uty>(arg)))
			)
		)
		: value(std::invoke(std::forward<Fn>(fn), std::forward<Uty>(arg))), bHasValue(true) {}

	template <typename... Types>
	constexpr explicit OptionDestruct(std::in_place_t, Types&&... Args)
		noexcept(std::is_nothrow_constructible_v<Ty, Types...>)
		: value(std::forward<Types>(Args)...), bHasValue(true) {}

	void __Cleanup() noexcept {
		value = nullptr;
	}
	
	bool HasValue() const noexcept {
		return value != nullptr;
	}
	void SetValue(bool bHasValue) noexcept {
		this->bHasValue = bHasValue;
	}
private:
	bool bHasValue;
};

/*
 * @function: 非平凡析构的需要手动调用 ~Ty()
 */

template <typename Ty>
struct OptionDestruct<Ty, false>{
	static_assert(IsPointerVal<Ty>, "The type Ty should not be a pointer, pls use smart ptr");
	union {
		/* 如果你用 char 会触发零初始化机制, 导致最后多生成一条指令 */
		NonTrivialDummyTp dummy;
		std::remove_cv_t<Ty> value;
	};

	constexpr OptionDestruct() noexcept : dummy{}, bHasValue(false) {}

	template <typename Fn, typename Uty>
	constexpr  OptionDestruct(ConstructFromInvokeResultTag, Fn&& fn, Uty&& arg)
		noexcept(
			noexcept(
				static_cast<Ty>(std::invoke(std::forward<Fn>(fn),  std::forward<Uty>(arg)))
			)
		)
		: value(std::invoke(std::forward<Fn>(fn), std::forward<Uty>(arg))), bHasValue(true){}

	template <typename... Types>
	constexpr explicit OptionDestruct(std::in_place_t, Types&&... Args)
		noexcept(std::is_nothrow_constructible_v<Ty, Types...>)
		: value(std::forward<Types>(Args)...), bHasValue(true) {}

	void __Cleanup() noexcept{
		bHasValue = false;
	}

	bool HasValue() const noexcept {
		return bHasValue;
	}
	void SetValue(bool bHasValue) noexcept {
		this->bHasValue = bHasValue;
	}
private:
	bool bHasValue;
};


template <typename Ty>
struct OptionConstruct : OptionDestruct<Ty> {
	using OptionDestruct<Ty>::OptionDestruct;

	template <typename... Types>
	Ty& __Construct(Types&&... Args) 
		noexcept(std::is_nothrow_constructible_v<Ty, Types...>)
	{
		std::construct_at(this->value, std::forward<Types>(Args)...);
		this->SetValue(true);
		return this->value;
	}

	template <typename Ty2>
	void __Assign(Ty2&& that)
		noexcept(std::is_nothrow_assignable_v<Ty&, Ty2> && std::is_nothrow_constructible_v<Ty, Ty2>)
	{
		if (this->HasValue()) {
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
		if (right.HasValue()){
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
		if (right.HasValue()){
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

	using MyBaseClassTp::__Cleanup;
	using MyBaseClassTp::operator*;
	/*
	 * 为了跨类型的访问 Optional<U> <-->  Optional<T>
    */
	template <class>
	friend class Optional;
public:
	using ValueType = Ty;

	constexpr Optional() noexcept {}
	constexpr Optional(NulloptTp)  noexcept {}
	constexpr Optional(std::nullopt_t)  noexcept {}

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

	template <typename Fn, typename Uty>
	constexpr Optional(ConstructFromInvokeResultTag tag, Fn&& fn, Uty&& arg)
		noexcept(
			noexcept(
				static_cast<Ty>(
					std::invoke(std::forward<Fn>(fn)), std::forward<Uty>(arg)
				)
			)
		)
		: MyBaseClassTp(tag, std::forward<Fn>(fn), std::forward<Uty>(arg)) {}

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

	CONSTEXPR20 Optional& operator=(NulloptTp) noexcept {
		this->__Cleanup();
		return *this;
	}

	CONSTEXPR20 Optional& operator=(std::nullopt_t) noexcept {
		this->__Cleanup();
		return *this;
	}

		
	template <
		class Ty2 = std::remove_cv_t<Ty>,
		std::enable_if_t<
			std::conjunction_v<
				std::negation<
					std::is_same<Optional, std::remove_cvref_t<Ty2>>
				>,
				std::negation<
					std::conjunction<
						std::is_scalar<Ty>, 
						std::is_same<Ty, std::decay_t<Ty2>>
					>
				>, 
				std::is_constructible<Ty, Ty2>,
				std::is_assignable<Ty&, Ty2>
			>,
			int
		> = 0
	>
	CONSTEXPR20 Optional& operator=(Ty2&& right)
		noexcept(
			std::is_nothrow_assignable_v<Ty&, Ty2> && 
			std::is_nothrow_constructible_v<Ty, Ty2>
		) 
	{
		this->__Assign(std::forward<Ty2>(right));
		return *this;
	}

	template <typename Ty2>
	struct __IsAllowUnwrappingAssignment : std::bool_constant<
		!std::disjunction_v<
			std::is_same<Ty, Ty2>,
			std::is_assignable<Ty&, Optional<Ty2>&>,
			std::is_assignable<Ty&, const Optional<Ty2>&>,
			std::is_assignable<Ty&, const Optional<Ty2>>,
			std::is_assignable<Ty&, Optional<Ty2>>
		>
	>{};

	template <
		typename Ty2, 
		std::enable_if_t<
			std::conjunction_v<
				__IsAllowUnwrappingAssignment<Ty2>,
				std::is_constructible<Ty, const Ty2&>,
				std::is_assignable<Ty&, const Ty2&>
			>, 
			int
		> = 0
	>
	CONSTEXPR20 Optional& operator=(const Optional<Ty2>& that)
		noexcept(std::is_nothrow_assignable_v<Ty&, const Ty2&>) 
	{
		if (that){
			this->__Assign(*that);
		}
		else{
			__Cleanup();
		}
		return *this;
	}
public:
	template <typename ...Types>
	CONSTEXPR20 Ty& Some(Types&&... args)
		noexcept(std::is_nothrow_constructible_v<Ty, Types...>)
	{
		this->__Cleanup();
		return this->__Construct(std::forward<Types>(args)...);
	}

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
	CONSTEXPR20 Ty& Some(
		std::initializer_list<Elem> list,  
		Types&&... args
	) noexcept (
		std::is_nothrow_constructible_v<
			Ty, 
			std::initializer_list<Elem>&, 
			Types...
		>
	) {
		this->__Cleanup();
		return this->__Construct(list, std::forward<Types>(args)...);
	}

	CONSTEXPR20 void Swap(Optional& that) 
		noexcept(
			std::is_nothrow_move_constructible_v<Ty> && 
			std::is_nothrow_swappable_v<Ty>
		)
	{
		if constexpr (std::is_move_constructible_v<Ty>) {
			static_assert(std::is_move_constructible_v<Ty>, 
				"Optional<T>::Swap requires Ty to be swappable"
			);
		}else{
			static_assert(false,
				"Optional<T>::Swap requires Ty to be move constructible"
			);
		}
		if constexpr (IsTriviallySwappableVal<Ty>) {
			using TrivialBaseTp = OptionDestruct<Ty>;
			std::swap(static_cast<TrivialBaseTp&>(*this), static_cast<TrivialBaseTp&>(that));
		}else{
			const bool Engaged = this->HasValue();
			if (Engaged){
				using std::swap;
				swap(**this, *that);
			}else{
				Optional& source = Engaged ? *this : that;
				Optional& target = Engaged ? that : *this;
				target.__Construct(std::move(*source));
				source.__Cleanup();
			}
		}
	}

	[[nodiscard]] constexpr const Ty* operator->() const noexcept {
		assert(this->HasValue() && "Optional::operator->() call on empty optional");
		return std::addressof(this->value);
	}
	[[nodiscard]] constexpr Ty* operator->() noexcept {
		assert(this->HasValue() && "Optional::operator->() call on empty optional");
		return std::addressof(this->value);
	}	
	[[nodiscard]] constexpr bool HasValue() const noexcept {
		return this->HasValue();
	}
	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return this->HasValue();
	}
	[[nodiscard]] constexpr const Ty& Value() const& {
		if (!this->HasValue()){
			throw std::bad_optional_access();
		}
		return this->value;
	}
	[[nodiscard]] constexpr Ty& Value() & {
		if (!this->HasValue()){
			throw std::bad_optional_access();
		}
		return this->value;
	}
	[[nodiscard]] constexpr Ty&& Value() && {
		if (!this->HasValue()){
			throw std::bad_optional_access();
		}
		return std::move(this->value);
	}
	[[nodiscard]] constexpr const Ty&& Value() const&& {
		if (!this->HasValue()){
			throw std::bad_optional_access();
		}
		return std::move(this->value);
	}
public:
	template <typename Ty2 = std::remove_cv_t<Ty>>
	[[nodiscard]] constexpr Ty&& ValueOr(Ty2&& that) && {
		static_assert(std::is_convertible_v<const Ty&, std::remove_cv_t<Ty>>,
            "The const overload of Optioanal::ValueOr requires const T& to be convertible to remove_cv_t<Ty> "
        );
        static_assert(std::is_convertible_v<Ty2, std::remove_cv_t<Ty>>,
            "Optioanal::ValueOr(U) requires U to be convertible to remove_cv_t<Ty> "
		);
		if (this->HasValue()){
			return std::move(this->value);
		}
		return static_cast<std::remove_cv_t<Ty>>(std::forward<Ty2>(that));
	}
	template <typename Ty2 = std::remove_cv_t<Ty>>
	[[nodiscard]] constexpr Ty&& ValueOr(Ty2&& that) const & {
		static_assert(std::is_convertible_v<const Ty&, std::remove_cv_t<Ty>>,
            "The const overload of Optioanal::ValueOr requires const T& to be convertible to remove_cv_t<Ty> "
        );
        static_assert(std::is_convertible_v<Ty2, std::remove_cv_t<Ty>>,
            "Optioanal::ValueOr(U) requires U to be convertible to remove_cv_t<Ty> "
		);
		if (this->HasValue()){
			return static_cast<const Ty&>(this->value);
		}
		return static_cast<std::remove_cv_t<Ty>>(std::forward<Ty2>(that));
	}

	template <typename Fn>
	constexpr auto AndThen(Fn&& fn) & {
		using Utp = std::invoke_result_t<Fn, Ty&>;
		static_assert(IsSpecializationValOf<std::remove_cvref_t<Utp>, Optional>,
			"Optional<T>::AndThen(Fn) requires the return type of Fn to be a specialization of optional"
		);
		if (this->HasValue()){
			return std::invoke(std::forward<Fn>(fn), static_cast<Ty&>(this->value));
		}else{
			return std::remove_cvref_t<Utp>{};
		}
	}
	template <typename Fn>
	constexpr auto AndThen(Fn&& fn) const & {
		using Utp = std::invoke_result_t<Fn, const Ty&>;
		static_assert(IsSpecializationValOf<std::remove_cvref_t<Utp>, Optional>,
			"Optional<T>::AndThen(Fn) requires the return type of Fn to be a specialization of optional"
		);
		if (this->HasValue()){
			return std::invoke(std::forward<Fn>(fn), static_cast<const Ty&>(this->value));
		}else{
			return std::remove_cvref_t<Utp>{};
		}
	}
	template <typename Fn>
	constexpr auto AndThen(Fn&& fn) && {
		using Utp = std::invoke_result_t<Fn, Ty>;
		static_assert(IsSpecializationValOf<std::remove_cvref_t<Utp>, Optional>,
			"Optional<T>::AndThen(Fn) requires the return type of Fn to be a specialization of optional"
		);
		if (this->HasValue()){
			return std::invoke(std::forward<Fn>(fn), static_cast<Ty&&>(this->value));
		}else{
			return std::remove_cvref_t<Utp>{};
		}
	}
	template <typename Fn>
	constexpr auto AndThen(Fn&& fn) const && {
		using Utp = std::invoke_result_t<Fn, const Ty>;
		static_assert(IsSpecializationValOf<std::remove_cvref_t<Utp>, Optional>,
			"Optional<T>::AndThen(Fn) requires the return type of Fn to be a specialization of optional"
		);
		if (this->HasValue()){
			return std::invoke(std::forward<Fn>(fn), static_cast<const Ty&&>(this->value));
		}else{
			return std::remove_cvref_t<Utp>{};
		}
	}

	template <typename Fn>
	constexpr auto Map(Fn&& fn) & {
		using Utp = std::remove_cv_t<std::invoke_result_t<Fn, Ty&>>;
		static_assert(IsAnyValOf<Utp, std::nullopt_t, std::in_place_t>, 
				"Optional<T>::Map(Fn) requires the return type of Fn to be neither std:: nor std::in_place_t"
		);
		static_assert(IsAnyValOf<Utp, NulloptTp, std::in_place_t>, 
			"Optional<T>::Map(Fn) requires the return type of Fn to be neither NulloptTp nor std::in_place_t"
		);
		static_assert(std::is_object_v<Utp> && !std::is_array_v<Utp>, 
			"Optional<T>::Map(Fn) requires the return type of Fn a non-array object type"
		);

		if  (this->HasValue()){
			return Optional<Utp>{
				ConstructFromInvokeResultTag{}, 
				std::forward<Fn>(fn), 
				static_cast<Ty&>(this->value)
			};
		}else{
			return Optional<Utp>{};
		}
	}
	template <typename Fn>
	constexpr auto Map(Fn&& fn) const & {
		using Utp = std::remove_cv_t<std::invoke_result_t<Fn, const Ty&>>;
		static_assert(IsAnyValOf<Utp, std::nullopt_t, std::in_place_t>, 
			"Optional<T>::Map(Fn) requires the return type of Fn to be neither std::nullopt_t nor std::in_place_t"
		);
		static_assert(IsAnyValOf<Utp, NulloptTp, std::in_place_t>, 
			"Optional<T>::Map(Fn) requires the return type of Fn to be neither NulloptTp nor std::in_place_t"
		);
		static_assert(std::is_object_v<Utp>  && !std::is_array_v<Utp>, 
			"Optional<T>::Map(Fn) requires the return type of Fn a non-array object type"
		);

		if  (this->HasValue()){
			return Optional<Utp>{
				ConstructFromInvokeResultTag{}, 
				std::forward<Fn>(fn), 
				static_cast<const Ty&>(this->value)
			};
		}else{
			return Optional<Utp>{};
		}
	}

	template <typename Fn>
	constexpr auto Map(Fn&& fn) && {
		using Utp = std::remove_cv_t<std::invoke_result_t<Fn, Ty>>;
		static_assert(IsAnyValOf<Utp, std::nullopt_t, std::in_place_t>, 
			"Optional<T>::Map(Fn) requires the return type of Fn to be neither std::nullopt_t nor std::in_place_t"
		);
		static_assert(IsAnyValOf<Utp, NulloptTp, std::in_place_t>, 
			"Optional<T>::Map(Fn) requires the return type of Fn to be neither NulloptTp nor std::in_place_t"
		);
		static_assert(std::is_object_v<Utp>  && !std::is_array_v<Utp>, 
			"Optional<T>::Map(Fn) requires the return type of Fn a non-array object type"
		);

		if  (this->HasValue()){
			return Optional<Utp>{
				ConstructFromInvokeResultTag{}, 
				std::forward<Fn>(fn), 
				static_cast<Ty&&>(this->value)
			};
		}else{
			return Optional<Utp>{};
		}
	}

	template <typename Fn>
	constexpr auto Map(Fn&& fn) const && {
		using Utp = std::remove_cv_t<std::invoke_result_t<Fn, Ty>>;
		static_assert(IsAnyValOf<Utp, std::nullopt_t, std::in_place_t>, 
			"Optional<T>::Map(Fn) requires the return type of Fn to be neither std::nullopt_t nor std::in_place_t"
		);
		static_assert(IsAnyValOf<Utp, NulloptTp, std::in_place_t>, 
			"Optional<T>::Map(Fn) requires the return type of Fn to be neither NulloptTp nor std::in_place_t"
		);
		static_assert(std::is_object_v<Utp>  && !std::is_array_v<Utp>, 
			"Optional<T>::Map(Fn) requires the return type of Fn a non-array object type"
		);

		if  (this->HasValue()){
			return Optional<Utp>{
				ConstructFromInvokeResultTag{}, 
				std::forward<Fn>(fn), 
				static_cast<const Ty&&>(this->value)
			};
		}else{
			return Optional<Utp>{};
		}
	}


	template <typename Fn,
		typename = std::enable_if_t<
			std::is_copy_constructible_v<Ty> &&
			std::is_invocable_v<Fn, Ty>
		>
	>
	constexpr auto OrElse(Fn&& fn) & {
		static_assert(
			std::is_same_v<
				std::remove_cvref_t<
					std::invoke_result_t<Fn>
				>,
				Optional
			>, 
			"Optional<Ty>::OrElse(Fn) requires Fn to return an Optional<Ty>"
		);

		if (this->HasValue()){
			return *this;
		}else{
			return std::forward<Fn>(fn)();
		}

	}

	template <typename Fn,
		typename = std::enable_if_t<
			std::is_move_constructible_v<Ty> &&
			std::is_invocable_v<Fn, Ty>
		>
	>
	constexpr auto OrElse(Fn&& fn) && {
		static_assert(
			std::is_same_v<
				std::remove_cvref_t<
					std::invoke_result_t<Fn>
				>,
				Optional
			>, 
			"Optional<Ty>::OrElse(Fn) requires Fn to return an Optional<Ty>"
		);

		if (this->HasValue()){
			return std::move(*this);
		}else{
			return std::forward<Fn>(fn)();
		}

	}

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
	CONSTEXPR20 auto OrElse (const Optional<Ty2>& that)
		noexcept(std::is_nothrow_constructible_v<Ty, const Ty2&>) 
	{
		if (this->HasValue()){
			return *this;
		} else {
			this->__Construct(*that);
		}
	}

	template <
		typename... Types, 
		std::enable_if_t<std::is_constructible_v<Ty, Types...>, int> = 0
	>
	constexpr auto OrElse(std::in_place_t, Types&& ... Args)
		noexcept (std::is_nothrow_constructible_v<Ty, Types...>)
	{
		if (this->HasValue()){
			return *this;
		}else {
			return Optional<Ty>{std::in_place, std::forward<Types>(Args)...};
		}
	}

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
	constexpr auto OrElse(
		std::in_place_t, 
		std::initializer_list<Elem> list,  
		Types&&... args
	) noexcept (
		std::is_nothrow_constructible_v<
			Ty, 
			std::initializer_list<Elem>&, 
			Types...
		>
	) 
	{
		if (this->HasValue()){
			return *this;
		}else{
			return Optional<Ty> {std::in_place, list, std::forward<Types>(args)...};
		}
	}

	constexpr auto Filter(){}

	constexpr auto ToList(){}
};

template <typename Ty1, typename Ty2>
[[nodiscard]] constexpr bool operator==(const Optional<Ty1>& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left == *right))
	)
	requires requires {
		{ *left == *right } -> ImplicitlyConvertibleTo<bool>;
	}
{
	const bool bLeftHasValue = left.HasValue();
	const bool bRightHasValue = right.HasValue();
	if (bLeftHasValue && bRightHasValue) {
		return *left == *right;
	}
	return bLeftHasValue == bRightHasValue;
}


template <typename Ty1, typename Ty2>
[[nodiscard]] constexpr bool operator!=(const Optional<Ty1>& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left != *right))
	)
	requires requires {
		{ *left != *right } -> ImplicitlyConvertibleTo<bool>;
	}
{
	const bool bLeftHasValue = left.HasValue();
	const bool bRightHasValue = right.HasValue();
	if (bLeftHasValue && bRightHasValue) {
		return *left != *right;
	}
	return bLeftHasValue != bRightHasValue;
}
template <typename Ty1, typename Ty2>
[[nodiscard]] constexpr bool operator>(const Optional<Ty1>& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left > *right))
	)
	requires requires {
		{ *left > *right } -> ImplicitlyConvertibleTo<bool>;
	}
{
	const bool bLeftHasValue = left.HasValue();
	const bool bRightHasValue = right.HasValue();
	if (bLeftHasValue && bRightHasValue) {
		return *left > *right;
	}
	return bLeftHasValue > bRightHasValue;
}
template <typename Ty1, typename Ty2>
[[nodiscard]] constexpr bool operator<(const Optional<Ty1>& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left < *right))
	)
	requires requires {
		{ *left < *right } -> ImplicitlyConvertibleTo<bool>;
	}
{
	const bool bLeftHasValue = left.HasValue();
	const bool bRightHasValue = right.HasValue();
	if (bLeftHasValue && bRightHasValue) {
		return *left < *right;
	}
	return bLeftHasValue < bRightHasValue;
}
template <typename Ty1, typename Ty2>
[[nodiscard]] constexpr bool operator>=(const Optional<Ty1>& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left >= *right))
	)
	requires requires {
		{ *left >= *right } -> ImplicitlyConvertibleTo<bool>;
	}
{
	const bool bLeftHasValue = left.HasValue();
	const bool bRightHasValue = right.HasValue();
	if (bLeftHasValue && bRightHasValue) {
		return *left >= *right;
	}
	return bLeftHasValue >= bRightHasValue;
}
template <typename Ty1, typename Ty2>
[[nodiscard]] constexpr bool operator<=(const Optional<Ty1>& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left <= *right))
	)
	requires requires {
		{ *left <= *right } -> ImplicitlyConvertibleTo<bool>;
	}
{
	const bool bLeftHasValue = left.HasValue();
	const bool bRightHasValue = right.HasValue();
	if (bLeftHasValue && bRightHasValue) {
		return *left <= *right;
	}
	return bLeftHasValue <= bRightHasValue;
}

template <typename Ty1, std::three_way_comparable_with<Ty1> Ty2>
[[nodiscard]] constexpr std::compare_three_way_result_t<Ty1, Ty2>
operator<=>(const Optional<Ty1>& left, const Optional<Ty2>& right){
	const bool bLeftHasValue = left.HasValue();
	const bool bRightHasValue = right.HasValue();
	if (bLeftHasValue && bRightHasValue){
		return *left <=> *right;
	}
	return bLeftHasValue <=> bRightHasValue;
}
// Cpp20
// template <typename Ty>
// [[nodiscard]] constexpr std::strong_ordering 
// operator<=>(const Optional<Ty> &left, NulloptTp) noexcept {
// 	return left.HasValue() <=> false;
// }

template <class Ty>
[[nodiscard]] constexpr bool operator==(NulloptTp, const Optional<Ty>& right) noexcept {
    return !right.has_value();
}

template <class Ty>
[[nodiscard]] constexpr bool operator!=(const Optional<Ty>& left, NulloptTp) noexcept {
    return left.has_value();
}
template <class Ty>
[[nodiscard]] constexpr bool operator!=(NulloptTp, const Optional<Ty>& right) noexcept {
    return right.has_value();
}

template <class Ty>
[[nodiscard]] constexpr bool operator<(const Optional<Ty>&, NulloptTp) noexcept {
    return false;
}
template <class Ty>
[[nodiscard]] constexpr bool operator<(NulloptTp, const Optional<Ty>& right) noexcept {
    return right.has_value();
}

template <class Ty>
[[nodiscard]] constexpr bool operator>(const Optional<Ty>& left, NulloptTp) noexcept {
    return left.has_value();
}
template <class Ty>
[[nodiscard]] constexpr bool operator>(NulloptTp, const Optional<Ty>&) noexcept {
    return false;
}

template <class Ty>
[[nodiscard]] constexpr bool operator<=(const Optional<Ty>& left, NulloptTp) noexcept {
    return !left.has_value();
}
template <class Ty>
[[nodiscard]] constexpr bool operator<=(NulloptTp, const Optional<Ty>&) noexcept {
    return true;
}

template <class Ty>
[[nodiscard]] constexpr bool operator>=(const Optional<Ty>&, NulloptTp) noexcept {
    return true;
}
template <class Ty>
[[nodiscard]] constexpr bool operator>=(NulloptTp, const Optional<Ty>& right) noexcept {
    return !right.has_value();
}
template <class Ty>
[[nodiscard]] constexpr bool operator==(std::nullopt_t, const Optional<Ty>& right) noexcept {
    return !right.has_value();
}

template <class Ty>
[[nodiscard]] constexpr bool operator!=(const Optional<Ty>& left, std::nullopt_t) noexcept {
    return left.has_value();
}
template <class Ty>
[[nodiscard]] constexpr bool operator!=(std::nullopt_t, const Optional<Ty>& right) noexcept {
    return right.has_value();
}

template <class Ty>
[[nodiscard]] constexpr bool operator<(const Optional<Ty>&, std::nullopt_t) noexcept {
    return false;
}
template <class Ty>
[[nodiscard]] constexpr bool operator<(std::nullopt_t, const Optional<Ty>& right) noexcept {
    return right.has_value();
}

template <class Ty>
[[nodiscard]] constexpr bool operator>(const Optional<Ty>& left, std::nullopt_t) noexcept {
    return left.has_value();
}
template <class Ty>
[[nodiscard]] constexpr bool operator>(std::nullopt_t, const Optional<Ty>&) noexcept {
    return false;
}

template <class Ty>
[[nodiscard]] constexpr bool operator<=(const Optional<Ty>& left, std::nullopt_t) noexcept {
    return !left.has_value();
}
template <class Ty>
[[nodiscard]] constexpr bool operator<=(std::nullopt_t, const Optional<Ty>&) noexcept {
    return true;
}

template <class Ty>
[[nodiscard]] constexpr bool operator>=(const Optional<Ty>&, std::nullopt_t) noexcept {
    return true;
}
template <class Ty>
[[nodiscard]] constexpr bool operator>=(std::nullopt_t, const Optional<Ty>& right) noexcept {
    return !right.has_value();
}

template <typename Ty>
using EnableIfBoolConvertible = std::enable_if_t<std::is_convertible_v<Ty, bool>, int>;

template <typename Lhs, typename Rhs>
using EnableIfComparableWithEqual = 
	EnableIfBoolConvertible<decltype(std::declval<const Lhs&>()== std::declval<const Rhs&>())>;

template <typename Lhs, typename Rhs>
using EnableIfComparableWithNotEqual = 
	EnableIfBoolConvertible<decltype(std::declval<const Lhs&>() != std::declval<const Rhs&>())>;

template <typename Lhs, typename Rhs>
using EnableIfComparableWithLess = 
	EnableIfBoolConvertible<decltype(std::declval<const Lhs&>() < std::declval<const Rhs&>())>;

template <typename Lhs, typename Rhs>
using EnableIfComparableWithGreater = 
	EnableIfBoolConvertible<decltype(std::declval<const Lhs&>() > std::declval<const Rhs&>())>;

template <typename Lhs, typename Rhs>
using EnableIfComparableWithLessEqual = 
	EnableIfBoolConvertible<decltype(std::declval<const Lhs&>() <= std::declval<const Rhs&>())>;

template <typename Lhs, typename Rhs>
using EnableIfComparableWithGreaterEqual = 
	EnableIfBoolConvertible<decltype(std::declval<const Lhs&>() >= std::declval<const Rhs&>())>;

template <typename Ty1, typename Ty2, EnableIfComparableWithEqual<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator==(const Optional<Ty1>& left, const Ty2& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left == right))
	)
{
	if (left) {
		return *left == right;
	}
	return false;
}
template <typename Ty1, typename Ty2, EnableIfComparableWithEqual<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator==(const Ty1& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(left == *right))
	)
{
	if (right) {
		return left == *right;
	}
	return false;
}


template <typename Ty1, typename Ty2, EnableIfComparableWithNotEqual<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator!=(const Optional<Ty1>& left, const Ty2& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left != right))
	)
{
	if (left) {
		return *left != right;
	}
	return true;
}
template <typename Ty1, typename Ty2, EnableIfComparableWithNotEqual<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator!=(const Ty1& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(left != *right))
	)
{
	if (right) {
		return left != *right;
	}
	return true;
}

template <typename Ty1, typename Ty2, EnableIfComparableWithLess<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator<(const Optional<Ty1>& left, const Ty2& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left < right))
	)
{
	if (left) {
		return *left < right;
	}
	return true;
}
template <typename Ty1, typename Ty2, EnableIfComparableWithLess<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator<(const Ty1& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(left < *right))
	)
{
	if (right) {
		return left < *right;
	}
	return false;
}


template <typename Ty1, typename Ty2, EnableIfComparableWithGreater<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator>(const Optional<Ty1>& left, const Ty2& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left > right))
	)
{
	if (left) {
		return *left > right;
	}
	return false;
}
template <typename Ty1, typename Ty2, EnableIfComparableWithGreater<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator>(const Ty1& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(left > *right))
	)
{
	if (right) {
		return left > *right;
	}
	return true;
}

template <typename Ty1, typename Ty2, EnableIfComparableWithGreaterEqual<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator>=(const Optional<Ty1>& left, const Ty2& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left >= right))
	)
{
	if (left) {
		return *left >= right;
	}
	return false;
}
template <typename Ty1, typename Ty2, EnableIfComparableWithGreaterEqual<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator>=(const Ty1& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(left >= *right))
	)
{
	if (right) {
		return left >= *right;
	}
	return false;
}
template <typename Ty1, typename Ty2, EnableIfComparableWithLessEqual<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator<=(const Optional<Ty1>& left, const Ty2& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(*left <= right))
	)
{
	if (left) {
		return *left <= right;
	}
	return true;
}
template <typename Ty1, typename Ty2, EnableIfComparableWithLessEqual<Ty1, Ty2> = 0>
[[nodiscard]] constexpr bool operator<=(const Ty1& left, const Optional<Ty2>& right)
	noexcept(
		noexcept(FakeCopyInit<bool>(left <= *right))
	)
{
	if (right) {
		return left <= *right;
	}
	return false;
}

template <typename Ty>
constexpr auto Flatten(){}

template <typename Ty>
constexpr auto Fold(){}

template <typename Ty, typename Elem, typename... Types,
	std::enable_if_t<
		std::is_constructible_v<
			Ty, 
			std::initializer_list<Elem>&, 
			Types...
		>, 
		int
	> = 0
>
[[nodiscard]] constexpr auto MakeOptional(std::initializer_list<Elem> list, Types&&... args)
	noexcept(
		noexcept(
			Optional<Ty>{std::in_place, list, std::forward<Types>(args)...}
		)
	)
{
	return Optional<Ty> {std::in_place, list, std::forward<Types>(args)...};
}


}