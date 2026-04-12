#pragma once
#include "creeper-qt/utility/wrapper/layout.hh"
#include <memory>

namespace creeper::util {

template <typename T> class Singleton {
public:
    static T& instance();

    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    struct token { };
    Singleton() = default;
};

template <typename T> inline T& Singleton<T>::instance() {
    static const std::unique_ptr<T> instance { new T { token {} } };
    return *instance;
}

class GlobalLayout : public Singleton<GlobalLayout> {
    using Singleton::Singleton;
    friend class Singleton<GlobalLayout>;
public:
    auto theme() const noexcept -> const QString& { return theme_; }
    void set_theme(QString theme) noexcept { theme_ = std::move(theme); }
private:
    explicit GlobalLayout(token) noexcept { }
    QString theme_;
};

}