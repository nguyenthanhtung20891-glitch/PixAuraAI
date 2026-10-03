#ifndef PIXAURA_DOCUMENT_CONTAINERS_HPP
#define PIXAURA_DOCUMENT_CONTAINERS_HPP

#include <string>
#include <initializer_list>
#include <vector>

namespace pixaura::document {
// MSVC debug STL default/move constructors allocate iterator proxies inside
// noexcept. Use throwing construction/copy paths so the document boundary can
// translate every allocation failure. Iterator debugging remains enabled.
// The independent allocation executable exercises these paths on every host.
#if (defined(_MSC_VER) && _ITERATOR_DEBUG_LEVEL != 0) || defined(PIXAURA_TEST_FALLIBLE_STL)
class String : public std::string {
    using Base = std::string;
public:
    using Base::Base;
    String() : Base("", std::size_t{0}) {}
    String(const Base& value) : Base(value) {}
    String(const String& value) : Base(static_cast<const Base&>(value)) {}
    String(String&& value) : Base(static_cast<const Base&>(value)) {}
    String& operator=(const String& value) { Base::operator=(value); return *this; }
    String& operator=(String&& value) { return *this = value; }
};

template<class T> class Vector : public std::vector<T> {
    using Base = std::vector<T>;
public:
    using Base::Base;
    Vector() : Base(std::initializer_list<T>{}) {}
    Vector(const Base& value) : Base(value) {}
    Vector(const Vector& value) : Base(static_cast<const Base&>(value)) {}
    Vector(Vector&& value) : Base(static_cast<const Base&>(value)) {}
    Vector& operator=(const Vector& value) { Base::operator=(value); return *this; }
    Vector& operator=(Vector&& value) { return *this = value; }
};
#else
using String = std::string;
template<class T> using Vector = std::vector<T>;
#endif
}
#endif
