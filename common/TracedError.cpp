// Implements
#include "TracedError.h"

// Dependencies
#include <string>
#include <typeinfo>

#include "Stacktrace.h"
#include "spdlog/spdlog.h"

// Took this from the uvc project
using namespace ncs;

TracedError::TracedError()
    : std::runtime_error(""), mStacktrace(ncs::stacktrace()), mMessage("") {}

TracedError::TracedError(const std::string& msg)
    : std::runtime_error(""), mStacktrace(ncs::stacktrace()), mMessage(msg) {}

std::string TracedError::stacktrace() const { return mStacktrace; }

const char* TracedError::what() const noexcept {
    if (mWhat.empty()) {
        mWhat = "\n" + mMessage + "\n\nEXCEPTION CLASS " +
                demangle(typeid(*this).name()) + "\n\n" + mStacktrace;
    }
    return mWhat.c_str();
}