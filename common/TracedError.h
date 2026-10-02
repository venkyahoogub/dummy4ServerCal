#pragma once

// Dependencies
#include <stdexcept>
#include <string>

// Took this from the uvc project.

namespace ncs {
/**
 * @class TracedError
 * @brief Provides a throwable error that can produce a stacktrace.
 */
class TracedError : public std::runtime_error {
   public:
    /**
     * @brief Default constructor.
     */
    TracedError();

    /**
     * @brief Constructs a TracedError with a specific descriptive message.
     * @param message A human-readable explanation of the error.
     */
    explicit TracedError(const std::string&);

    /**
     * @brief Retrieves the stack trace captured when the exception was thrown.
     * @return A std::string containing the formatted call stack.
     */
    std::string stacktrace() const;

    /**
     * @brief Returns the explanatory string describing the error.
     * @return A pointer to a null-terminated string with the error description.
     */
    virtual const char* what() const noexcept override;

   private:
    std::string mStacktrace;
    std::string mMessage;
    mutable std::string mWhat;
};
}  // namespace ncs