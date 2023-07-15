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
	Optional& operator=(T*&& ptr) noexcept;

	operator bool() const;

	T& value();
	const T& value() const;

private:
	T* m_value;
};

template<typename T>
inline Optional<T>::Optional() :
	m_value(nullptr)
{}

template<typename T>
inline Optional<T>::Optional(const T& value) :
	m_value(new T(value))
{}

template<typename T>
inline Optional<T>::Optional(const Optional& obj) {
	if (obj) {
		m_value = new T(*obj.m_value);
	} else {
		m_value = nullptr;
	}
}

template<typename T>
inline Optional<T>::Optional(Optional&& obj) noexcept :
	m_value(obj.m_value)
{
	obj.m_value = nullptr;
}

template<typename T>
inline Optional<T>::~Optional() {
	if (m_value) {
		delete m_value;
	}
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(const Optional& obj) {
	if (m_value) {
		delete m_value;
	}
	if (obj) {
		m_value = new T(obj.m_value);
	} else {
		m_value = nullptr;
	}
	return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(Optional&& obj) noexcept {
	if (m_value) {
		delete m_value;
	}
	if (obj) {
		m_value = obj.m_value;
		obj.m_value = nullptr;
	} else {
		m_value = nullptr;
	}
	return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(T*&& ptr) noexcept {
	if (m_value) {
		delete m_value;
	}
	if (ptr) {
		m_value = ptr;
		ptr = nullptr;
	} else {
		m_value = nullptr;
	}
	return *this;
}

template<typename T>
inline Optional<T>::operator bool() const {
	return (m_value != nullptr);
}

template<typename T>
inline T& Optional<T>::value() {
	if (!m_value) {
		throw Exception("Optional does not contain a value");
	}
	return *m_value;
}

template<typename T>
inline const T& Optional<T>::value() const {
	if (!m_value) {
		throw Exception("Optional does not contain a value");
	}
	return *m_value;
}

#endif /* OPTIONAL_HPP */
