#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

/*
* Решил сделать свой Optional, вместо использования std::optional, так как:
*   1. Не хочу пока переводить проект на c++17
*   2. Интересно реализовать свою версию!
*/

#include <stdexcept>

template <typename T>
class Optional {
public:
	using Exception = std::runtime_error;

	Optional();
	Optional(const T& value);
	Optional(const Optional& obj);
	Optional(Optional&& obj) noexcept;

	~Optional();

	Optional& operator=(const Optional& obj);
	Optional& operator=(Optional&& obj) noexcept;

	operator bool() const;

	T& value();
	const T& value() const;

private:
	T* value_;
};

template<typename T>
inline Optional<T>::Optional() :
	value_(nullptr)
{}

template<typename T>
inline Optional<T>::Optional(const T& value) :
	value_(new T(value))
{}

template<typename T>
inline Optional<T>::Optional(const Optional& obj) {
	if (obj) {
		value_ = new T(*obj.value_);
	} else {
		value_ = nullptr;
	}
}

template<typename T>
inline Optional<T>::Optional(Optional&& obj) noexcept :
	value_(obj.value_)
{
	obj.value_ = nullptr;
}

template<typename T>
inline Optional<T>::~Optional() {
	if (value_) {
		delete value_;
	}
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(const Optional& obj) {
	if (value_) {
		delete value_;
	}
	if (obj) {
		value_ = new T(obj.value_);
	} else {
		value_ = nullptr;
	}
	return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(Optional&& obj) noexcept {
	if (value_) {
		delete value_;
	}
	if (obj) {
		value_ = obj.value_;
		obj.value_ = nullptr;
	} else {
		value_ = nullptr;
	}
	return *this;
}

template<typename T>
inline Optional<T>::operator bool() const {
	return (value_ != nullptr);
}

template<typename T>
inline T& Optional<T>::value() {
	if (!value_) {
		throw Exception("Optional does not contain a value");
	}
	return *value_;
}

template<typename T>
inline const T& Optional<T>::value() const {
	if (!value_) {
		throw Exception("Optional does not contain a value");
	}
	return *value_;
}

#endif /* OPTIONAL_HPP */
