#pragma once

// Dependencies
#include <string>

// Took this from the uvc project.

namespace ncs {
/**
 * @brief Demangles a C++ ABI symbol name into a human-readable format.
 * @param mangled_name An rvalue reference to the mangled string to be
 * processed.
 * @return A std::string containing the human-readable demangled name.
 *         Returns the original string if demangling fails.
 */
std::string demangle(std::string&&);

/**
 * @brief Captures and formats the current thread's call stack.
 * @return A std::string containing the formatted stack trace, usually
 *         including function names, addresses, and offsets.
 */
std::string stacktrace();
}  // namespace ncs
