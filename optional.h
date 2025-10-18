#pragma once

#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    Optional(const T& value);
    Optional(T&& value);
    Optional(const Optional& other);
    Optional(Optional&& other);

    Optional& operator=(const T& value);
    Optional& operator=(T&& rhs);
    Optional& operator=(const Optional& rhs);
    Optional& operator=(Optional&& rhs);

    ~Optional();

    bool HasValue() const;

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*();
    const T& operator*() const;
    T* operator->();
    const T* operator->() const;

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value();
    const T& Value() const;

    void Reset();

    template <typename... Args>
    void Emplace(Args&&... args);

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};

template <typename T>
Optional<T>::Optional(const T& value){
    new (data_) T(value);
    is_initialized_ = true;
}

template <typename T>
Optional<T>::Optional(T&& value){
    new (data_) T(std::move(value));
    is_initialized_ = true;
}

template <typename T>
Optional<T>::Optional(const Optional& other){
    if (other.is_initialized_) {
        new (data_) T(*other);
        is_initialized_ = true;
    }
}

template <typename T>
Optional<T>::Optional(Optional&& other){
    if (other.is_initialized_) {
        new (data_) T(std::move(*other));
        is_initialized_ = true;
    }
}

template <typename T>
Optional<T>& Optional<T>::operator=(const T& value){
    if (is_initialized_) {
        **this = value;
    } else {
        new (data_) T(value);
        is_initialized_ = true;
    }
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(T&& rhs){
    if (is_initialized_) {
        **this = std::move(rhs);
    } else {
        new (data_) T(std::move(rhs));
        is_initialized_ = true;
    }
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(const Optional& rhs){
    if (this != &rhs) {
        if (rhs.is_initialized_) {
            *this = *rhs;
        } else {
            Reset();
        }
    }
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(Optional&& rhs){
    if (this != &rhs) {
        if (rhs.is_initialized_) {
            *this = std::move(*rhs);
        } else {
            Reset();
        }
    }
    return *this;
}

template <typename T>
Optional<T>::~Optional() {
    Reset();
}

template <typename T>
bool Optional<T>::HasValue() const{
    return is_initialized_;
}

template <typename T>
T& Optional<T>::operator*(){
    return *reinterpret_cast<T*>(data_);
}

template <typename T>
const T& Optional<T>::operator*() const{
    return *reinterpret_cast<const T*>(data_);
}

template <typename T>
T* Optional<T>::operator->(){
    return reinterpret_cast<T*>(data_);
}

template <typename T>
const T* Optional<T>::operator->() const{
    return reinterpret_cast<const T*>(data_);
}

template <typename T>
T& Optional<T>::Value(){
    if (!is_initialized_) {
        throw BadOptionalAccess();
    }
    return **this;
}

template <typename T>
const T& Optional<T>::Value() const{
    if (!is_initialized_) {
        throw BadOptionalAccess();
    }
    return **this;
}

template <typename T>
void Optional<T>::Reset(){
    if (is_initialized_) {
        reinterpret_cast<T*>(data_)->~T();
        is_initialized_ = false;
    }
}

template <typename T>
template <typename... Args>
void Optional<T>::Emplace(Args&&... args) {
    Reset(); // Уничтожение предыдущего значения
    new (data_) T(std::forward<Args>(args)...); // Конструирование нового значения на месте
    is_initialized_ = true;
}