// Implements
#include "Stacktrace.h"

// Dependencies
#include <cxxabi.h>    // __cxa_demangle
#include <execinfo.h>  // backtrace
#include <unistd.h>    // pipe

#include <cstring>
#include <string>

#include "spdlog/spdlog.h"

// Took this from the uvc project.

using std::string;
using namespace ncs;

string ncs::demangle(string&& sym) {
    int status;
    char* demangled =
        abi::__cxa_demangle(sym.c_str(), nullptr, nullptr, &status);
    if (status) return sym;  // if we can't demangle, just pass through
    string ret(demangled);
    free(demangled);
    return ret;
}

string ncs::stacktrace() {
    const int maxframes = 99;
    void* frames[maxframes];
    size_t size = backtrace(frames, maxframes);

    int p[2];  // make fd pipe: send to p[1] and read from p[0]
    if (pipe(p)) {
        SPDLOG_ERROR("Failed to create stacktrace.");
        return "Failed to create stacktrace.";
    }

    // Use backtrace_symbols_fd instead of backtrace_symbols because the latter
    // can malloc, and we might be in the middle of an out-of-memory error.
    backtrace_symbols_fd(frames, size, p[1]);
    close(p[1]);

    FILE* f = fdopen(p[0], "r");
    char buf[999];
    string ret;

    int skip = 1;  // The first frame is this function.
    while (fgets(buf, sizeof(buf), f)) {
        if (skip > 0) {
            skip--;
            continue;
        }

        // Raw backtrace lines look something like:
        //   ./test/control_tests(_ZN7testing9TestSuite3RunEv+0x12a)[0x563abda3d322]
        // Demangle the symbol between '(' and '+'.

        char* symBegin = strchr(buf, '(');
        char* symEnd = nullptr;
        if (symBegin) symEnd = strchr(symBegin, '+');
        if (!symEnd) {
            // If we don't find the symbol, return the line unchanged.  This
            // could also happen if the buf size is too small, and a single
            // entry is processed in chunks.
            ret += string(buf);
            continue;
        }

        // We're in the middle of creating an exception, so any memory
        // allocation is ill-advised.  Fix this?  Maybe do the read from p[0]
        // lazily.
        ret += string(buf, symBegin + 1 - buf);
        ret += demangle(string(symBegin + 1, symEnd - symBegin - 1));
        ret += string(symEnd);
    }

    fclose(f);
    return ret;
}
