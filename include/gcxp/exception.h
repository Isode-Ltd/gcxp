#pragma once
#include <exception>
#include <string>

namespace Gcxp {

// This class represents an exception thrown by the GCXP API.
class Exception : public std::exception {
public:
    Exception() noexcept {
    }
    virtual ~Exception() noexcept = default;

    Exception(const char* d) noexcept {
        what_ += std::string(": ") + d;
    }

    Exception(const std::string& d) noexcept {
        what_ += ": " + d;
    }

    Exception(const Exception& e) noexcept : what_(e.what_) {
    }

    Exception(Exception&& e) noexcept : what_(std::move(e.what_)) {
        // Note that e.what_ is not re-initialized to "GCXP Exception" as
        // the moved-from object is not expected to ever be reused.
    }

    Exception& operator=(const Exception&) = delete;
    Exception& operator=(Exception&&) = delete;

    virtual const char* what() const noexcept {
        return what_.c_str();
    }

private:
    std::string what_ = "GCXP Exception";
};
} // namespace Gcxp
