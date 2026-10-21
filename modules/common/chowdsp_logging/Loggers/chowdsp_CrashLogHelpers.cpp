#include "chowdsp_CrashLogHelpers.h"

#define CHOWDSP_CRASH_POSIX 0
#define CHOWDSP_CRASH_WINDOWS 0
#if JUCE_MAC || JUCE_LINUX
#undef CHOWDSP_CRASH_POSIX
#define CHOWDSP_CRASH_POSIX 1
#include <csignal>
#include <dlfcn.h>
#include <execinfo.h>
#elif JUCE_WINDOWS
#undef CHOWDSP_CRASH_WINDOWS
#define CHOWDSP_CRASH_WINDOWS 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace chowdsp::CrashLogHelpers
{
// LCOV_EXCL_START
void defaultCrashLogAnalyzer (const juce::File& logFile)
{
#if JUCE_MODULE_AVAILABLE_juce_gui_basics
    auto alertOptions =
        juce::MessageBoxOptions()
            .withTitle ("Crash detected!")
            .withMessage (
                "A previous instance of this plugin has crashed! Would you like to view the logs?")
#if JUCE_IOS
            .withButton ("Copy Logs")
#else
            .withButton ("Show Log File")
#endif
            .withButton ("Cancel");

    juce::AlertWindow::showAsync (
        alertOptions,
        [logFile] (int result)
        {
            if (result == 1)
            {
#if JUCE_IOS
                juce::SystemClipboard::copyTextToClipboard (logFile.loadFileAsString());
#else
                logFile.startAsProcess();
#endif
            }
        });
#else
    jassertfalse; // Implement your own!
#endif
}
// LCOV_EXCL_END

constexpr std::string_view crashString = "Plugin crashing!!!";
constexpr std::string_view crashExaminedString = "The crash in this log file is now being examined!";

void checkLogFilesForCrashes (const LogFileHelpers::FileArray& logFiles,
                              const CrashLogAnalysisCallback& callback)
{
    for (auto& logFile : logFiles)
    {
        const auto& logString = logFile.loadFileAsString();

        if (! logString.contains (toString (crashString)))
            continue;

        if (logString.contains (toString (crashExaminedString)))
            continue;

        callback (logFile);
        logFile.appendText (toString (crashExaminedString));
    }
}

namespace
{
#if CHOWDSP_CRASH_POSIX
    const void* getImageBase (const void* address)
    {
        Dl_info info {};
        return dladdr (address, &info) != 0 ? info.dli_fbase : nullptr;
    }
#elif CHOWDSP_CRASH_WINDOWS
    const void* getImageBase (const void* address)
    {
        HMODULE module = nullptr;
        if (GetModuleHandleExA (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                static_cast<LPCSTR> (address),
                                &module)
            == 0)
            return nullptr;
        return module;
    }
#else
    const void* getImageBase (const void*)
    {
        return nullptr;
    }
#endif

    bool involvesThisBinary (const void* const* frames, int numFrames)
    {
        const auto* ourBase = getImageBase (reinterpret_cast<const void*> (&signalHandler));
        if (ourBase == nullptr || numFrames <= 0)
            return true; // we can't tell, so assume the worst

        // Skip our own handler frames at the top of the stack, and look
        // for our code anywhere in the frames below that.
        int i = 0;
        while (i < numFrames && getImageBase (frames[i]) == ourBase)
            ++i;

        for (; i < numFrames; ++i)
            if (getImageBase (frames[i]) == ourBase)
                return true;

        return false;
    }

    void logSignal (bool isOurCrash)
    {
        const auto stackTrace = juce::SystemStats::getStackBacktrace();

        if (isOurCrash)
        {
            juce::Logger::writeToLog ("Interrupt signal received!");
        }
        else
        {
            // Probably not our fault: keep the trace for reference, but don't
            // flag this log file as a crash so the user isn't prompted.
            juce::Logger::writeToLog ("Signal received, but no frames from this plugin were found in the stack trace.");
        }

        juce::Logger::writeToLog ("Stack Trace:");
        juce::Logger::writeToLog (stackTrace);

        LogFileHelpers::shutdownLogger (isOurCrash ? 1 : 0);
    }

    std::atomic<int> handlerCount { 0 };
} // namespace

bool framesInvolveThisBinary (const void* const* frames, int numFrames)
{
    return involvesThisBinary (frames, numFrames);
}

void signalHandler (void*) // NOSONAR (void* is needed here)
{
    logSignal (true);
}

namespace
{
    // Only meant to be called from within an actual signal handler,
    // since the stack is expected to look like: handler, trampoline, crash site...
    [[maybe_unused]] void reportSignal()
    {
#if CHOWDSP_CRASH_POSIX
        void* stack[128];
        const auto numFrames = backtrace (stack, 128);
        logSignal (involvesThisBinary (stack, numFrames));
#elif CHOWDSP_CRASH_WINDOWS
        void* stack[62];
        const auto numFrames = CaptureStackBackTrace (0, 62, stack, nullptr);
        logSignal (involvesThisBinary (stack, numFrames));
#else
        logSignal (true);
#endif
    }
} // namespace

#if CHOWDSP_CRASH_POSIX
namespace
{
    constexpr int crashSignals[] = { SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGABRT, SIGSYS };
    constexpr size_t numCrashSignals = sizeof (crashSignals) / sizeof (crashSignals[0]);
    struct sigaction previousActions[numCrashSignals];

    void posixCrashHandler (int signum, siginfo_t* info, void* context)
    {
        size_t index = 0;
        while (index < numCrashSignals && crashSignals[index] != signum)
            ++index;
        if (index == numCrashSignals) // LCOV_EXCL_LINE
            return; // LCOV_EXCL_LINE

        // Put the previous handler back first: if anything below goes wrong,
        // or if the previous handler just returns, we won't be re-entered.
        auto previous = previousActions[index];
        if (previous.sa_handler == SIG_IGN)
            previous.sa_handler = SIG_DFL;
        sigaction (signum, &previous, nullptr);

        reportSignal();

        // LCOV_EXCL_START
        // The default action kills the process, so this can only be tested in a child process.
        if (previous.sa_handler == SIG_DFL)
            return; // faulting instruction re-executes (or the pending signal is delivered), with the default action
        // LCOV_EXCL_STOP

        if ((previous.sa_flags & SA_SIGINFO) != 0)
            previous.sa_sigaction (signum, info, context);
        else
            previous.sa_handler (signum);
    }
} // namespace

void installCrashHandler()
{
    if (handlerCount.fetch_add (1) != 0)
        return;

    struct sigaction action
    {
    };
    action.sa_sigaction = posixCrashHandler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset (&action.sa_mask);

    for (size_t i = 0; i < numCrashSignals; ++i)
        sigaction (crashSignals[i], &action, &previousActions[i]);
}

void removeCrashHandler()
{
    if (handlerCount.fetch_sub (1) != 1)
        return;

    for (size_t i = 0; i < numCrashSignals; ++i)
    {
        // If somebody else installed a handler after us, leave it alone.
        struct sigaction current
        {
        };
        if (sigaction (crashSignals[i], nullptr, &current) == 0 && current.sa_sigaction == posixCrashHandler)
            sigaction (crashSignals[i], &previousActions[i], nullptr);
    }
}

#elif CHOWDSP_CRASH_WINDOWS
namespace
{
    LPTOP_LEVEL_EXCEPTION_FILTER previousFilter = nullptr;

    LONG WINAPI windowsCrashFilter (LPEXCEPTION_POINTERS ep)
    {
        reportSignal();
        return previousFilter != nullptr ? previousFilter (ep) : EXCEPTION_CONTINUE_SEARCH;
    }
} // namespace

void installCrashHandler()
{
    if (handlerCount.fetch_add (1) == 0)
        previousFilter = SetUnhandledExceptionFilter (windowsCrashFilter);
}

void removeCrashHandler()
{
    if (handlerCount.fetch_sub (1) != 1)
        return;

    // If somebody else installed a filter after us, leave it alone.
    if (auto* current = SetUnhandledExceptionFilter (previousFilter); current != windowsCrashFilter)
        SetUnhandledExceptionFilter (current);
}

#else
void installCrashHandler()
{
    if (handlerCount.fetch_add (1) == 0)
        juce::SystemStats::setApplicationCrashHandler (signalHandler);
}

void removeCrashHandler()
{
    handlerCount.fetch_sub (1);
}
#endif
} // namespace chowdsp::CrashLogHelpers
