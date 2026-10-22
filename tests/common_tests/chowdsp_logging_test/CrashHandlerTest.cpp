#include <chowdsp_logging/chowdsp_logging.h>
#include <CatchUtils.h>

#if JUCE_MAC || JUCE_LINUX
#include <atomic>
#include <csignal>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace
{
const juce::String logFileSubDir = "chowdsp/utils_test";
const juce::String logFileNameRoot = "chowdsp_utils_crash_test_Log_";

// a function that lives in this binary
[[maybe_unused]] void ourFunction() {}

// a function that lives in a system library
#if JUCE_WINDOWS
const void* foreignFunction()
{
    return reinterpret_cast<const void*> (&::GetCurrentProcessId);
}
#else
const void* foreignFunction()
{
    return reinterpret_cast<const void*> (&::getpid);
}
#endif
} // namespace

TEST_CASE ("Crash Handler Frame Attribution Test", "[common][logs]")
{
    const auto* handlerFrame = reinterpret_cast<const void*> (&chowdsp::CrashLogHelpers::signalHandler);
    const auto* ourFrame = reinterpret_cast<const void*> (&ourFunction);
    const auto* otherFrame = foreignFunction();

    SECTION ("Only foreign frames below the handler")
    {
        const void* frames[] = { handlerFrame, handlerFrame, otherFrame, otherFrame };
        REQUIRE (! chowdsp::CrashLogHelpers::framesInvolveThisBinary (frames, 4));
    }

    SECTION ("Our code below a foreign frame")
    {
        const void* frames[] = { handlerFrame, otherFrame, ourFrame, otherFrame };
        REQUIRE (chowdsp::CrashLogHelpers::framesInvolveThisBinary (frames, 4));
    }

    SECTION ("Crash directly in our code")
    {
        const void* frames[] = { handlerFrame, otherFrame, ourFrame };
        REQUIRE (chowdsp::CrashLogHelpers::framesInvolveThisBinary (frames, 3));
    }

    SECTION ("Empty stack is assumed to be ours")
    {
        REQUIRE (chowdsp::CrashLogHelpers::framesInvolveThisBinary (nullptr, 0));
    }
}

#if JUCE_MAC || JUCE_LINUX
namespace
{
constexpr int chainedExitCode = 42;

void previousHandler (int)
{
    _exit (chainedExitCode);
}

// For in-process tests (the forked child's coverage data is never written out)
std::atomic<int> previousHandlerCalls { 0 };
std::atomic<bool> previousHandlerGotInfo { false };

void returningHandler (int)
{
    ++previousHandlerCalls;
}

void returningInfoHandler (int, siginfo_t* info, void*)
{
    previousHandlerGotInfo = info != nullptr;
    ++previousHandlerCalls;
}

void installSigAbortHandler (void (*handler) (int))
{
    struct sigaction action
    {
    };
    action.sa_handler = handler;
    sigemptyset (&action.sa_mask);
    sigaction (SIGABRT, &action, nullptr);
}

struct SigAbortGuard
{
    SigAbortGuard()
    {
        struct sigaction action
        {
        };
        action.sa_handler = previousHandler;
        sigemptyset (&action.sa_mask);
        sigaction (SIGABRT, &action, &original);
    }

    ~SigAbortGuard() { sigaction (SIGABRT, &original, nullptr); }

    struct sigaction original
    {
    };
};

enum class CrashOrigin
{
    ThisBinary,
    OtherBinary,
};

// Crashes in a forked child, and returns the contents of the child's log file.
juce::String crashChildAndGetLog (CrashOrigin origin, int& childExitCode)
{
    SigAbortGuard guard; // this is the "host" handler that we should chain to

    chowdsp::Logger logger { logFileSubDir, logFileNameRoot, [] (const juce::File&) {} };
    const auto logFile = logger.getLogFile();

    const auto pid = fork();
    if (pid == 0)
    {
        if (origin == CrashOrigin::ThisBinary)
        {
            std::abort();
        }
        else
        {
            pthread_t thread {};
            // the thread's entry point is in libc, so none of our code is on the stack
            using ThreadFunc = void* (*) (void*);
            pthread_create (&thread, nullptr, reinterpret_cast<ThreadFunc> (&::abort), nullptr);
            pthread_join (thread, nullptr);
        }
        _exit (1); // should be unreachable
    }

    int status = 0;
    waitpid (pid, &status, 0);
    childExitCode = WIFEXITED (status) ? WEXITSTATUS (status) : -1;

    return logFile.loadFileAsString();
}
} // namespace

TEST_CASE ("Crash Handler Chaining Test", "[common][logs]")
{
    juce::InterProcessLock mutex { "plugin_logger_lock" };
    juce::InterProcessLock::ScopedLockType lock { mutex };
    jassert (lock.isLocked());

    auto logsDirectory = juce::FileLogger::getSystemLogFileFolder().getChildFile (logFileSubDir);

    SECTION ("Crash in this binary")
    {
        int exitCode = 0;
        const auto log = crashChildAndGetLog (CrashOrigin::ThisBinary, exitCode);
        REQUIRE (exitCode == chainedExitCode);
        REQUIRE (log.contains ("Interrupt signal received!"));
        REQUIRE (log.contains ("Plugin crashing!!!"));
    }

    SECTION ("Crash in another binary")
    {
        int exitCode = 0;
        const auto log = crashChildAndGetLog (CrashOrigin::OtherBinary, exitCode);
        REQUIRE (exitCode == chainedExitCode);
        REQUIRE (log.contains ("no frames from this plugin"));
        REQUIRE (! log.contains ("Plugin crashing!!!"));
    }

    // These run the handlers in this process, so that they show up in the coverage report.
    SECTION ("Chaining in-process: this binary")
    {
        SigAbortGuard guard;
        installSigAbortHandler (returningHandler);
        previousHandlerCalls = 0;

        juce::String log;
        {
            chowdsp::Logger logger { logFileSubDir, logFileNameRoot, [] (const juce::File&) {} };
            std::raise (SIGABRT);
            log = logger.getLogFile().loadFileAsString();
        }

        REQUIRE (previousHandlerCalls == 1);
        REQUIRE (log.contains ("Plugin crashing!!!"));
    }

    SECTION ("Chaining in-process: other binary")
    {
        SigAbortGuard guard;
        installSigAbortHandler (returningHandler);
        previousHandlerCalls = 0;

        juce::String log;
        {
            chowdsp::Logger logger { logFileSubDir, logFileNameRoot, [] (const juce::File&) {} };

            // the thread's entry point is in libc, so none of our code is on the stack
            using ThreadFunc = void* (*) (void*);
            pthread_t thread {};
            pthread_create (&thread, nullptr, reinterpret_cast<ThreadFunc> (&::raise), reinterpret_cast<void*> (static_cast<intptr_t> (SIGABRT)));
            pthread_join (thread, nullptr);
            log = logger.getLogFile().loadFileAsString();
        }

        REQUIRE (previousHandlerCalls == 1);
        REQUIRE (log.contains ("no frames from this plugin"));
        REQUIRE (! log.contains ("Plugin crashing!!!"));
    }

    SECTION ("Chaining in-process: previous handler wants signal info")
    {
        SigAbortGuard guard;
        struct sigaction action
        {
        };
        action.sa_sigaction = returningInfoHandler;
        action.sa_flags = SA_SIGINFO;
        sigemptyset (&action.sa_mask);
        sigaction (SIGABRT, &action, nullptr);
        previousHandlerCalls = 0;
        previousHandlerGotInfo = false;

        {
            chowdsp::Logger logger { logFileSubDir, logFileNameRoot, [] (const juce::File&) {} };
            std::raise (SIGABRT);
        }

        REQUIRE (previousHandlerCalls == 1);
        REQUIRE (previousHandlerGotInfo);
    }

    SECTION ("Handlers installed after ours are left alone")
    {
        SigAbortGuard guard;
        {
            chowdsp::Logger logger { logFileSubDir, logFileNameRoot, [] (const juce::File&) {} };
            installSigAbortHandler (returningHandler);
        }

        struct sigaction after
        {
        };
        sigaction (SIGABRT, nullptr, &after);
        REQUIRE (after.sa_handler == returningHandler);
    }

    SECTION ("Multiple loggers share one handler")
    {
        SigAbortGuard guard;
        struct sigaction before
        {
        };
        sigaction (SIGABRT, nullptr, &before);
        {
            auto logger1 = std::make_unique<chowdsp::Logger> (logFileSubDir, logFileNameRoot, [] (const juce::File&) {});
            auto logger2 = std::make_unique<chowdsp::Logger> (logFileSubDir, logFileNameRoot, [] (const juce::File&) {});
            logger1.reset();

            struct sigaction during
            {
            };
            sigaction (SIGABRT, nullptr, &during);
            REQUIRE (during.sa_sigaction != before.sa_sigaction); // still installed
        }

        struct sigaction after
        {
        };
        sigaction (SIGABRT, nullptr, &after);
        REQUIRE (after.sa_handler == before.sa_handler);
    }

    SECTION ("Handlers are restored")
    {
        SigAbortGuard guard;
        struct sigaction before
        {
        };
        sigaction (SIGABRT, nullptr, &before);
        {
            chowdsp::Logger logger { logFileSubDir, logFileNameRoot, [] (const juce::File&) {} };
            struct sigaction during
            {
            };
            sigaction (SIGABRT, nullptr, &during);
            REQUIRE (during.sa_sigaction != before.sa_sigaction);
        }
        struct sigaction after
        {
        };
        sigaction (SIGABRT, nullptr, &after);
        REQUIRE (after.sa_handler == before.sa_handler);
    }

    logsDirectory.deleteRecursively();
}
#endif
