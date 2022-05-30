#pragma once

#include "chowdsp_PluginBase.h"

namespace chowdsp
{
/** Managing information relating to plugin diagnostics */
namespace PluginDiagnosticInfo
{
    /** Returns a diagnostic string for a plugin */
    template <class Processor>
    inline juce::String getDiagnosticsString (const PluginBase<Processor>& plugin)
    {
#if defined(JucePlugin_VersionString)
        const auto pluginVersionString = juce::String (JucePlugin_VersionString);
#else
        const auto pluginVersionString = juce::String {};
#endif

#if defined(CHOWDSP_PLUGIN_GIT_COMMIT_HASH)
        const auto gitHashString = juce::String (CHOWDSP_PLUGIN_GIT_COMMIT_HASH);
#else
        const auto gitHashString = juce::String {};
#endif

#if defined(CHOWDSP_PLUGIN_GIT_BRANCH)
        const auto gitBranchString = juce::String (CHOWDSP_PLUGIN_GIT_BRANCH);
#else
        const auto gitBranchString = juce::String {};
#endif

#if defined(CHOWDSP_PLUGIN_BUILD_FQDN)
        const auto buildFQDNString = juce::String (CHOWDSP_PLUGIN_BUILD_FQDN);
#else
        const auto buildFQDNString = juce::String {};
#endif

#if defined(CHOWDSP_PLUGIN_CXX_COMPILER_ID)
        const auto pluginCompilerIDString = juce::String (CHOWDSP_PLUGIN_CXX_COMPILER_ID);
#else
        const auto pluginCompilerIDString = juce::String {};
#endif

#if defined(CHOWDSP_PLUGIN_CXX_COMPILER_ID)
        const auto pluginCompilerVersionString = juce::String (CHOWDSP_PLUGIN_CXX_COMPILER_VERSION);
#else
        const auto pluginCompilerVersionString = juce::String {};
#endif

        auto getCompilationDateString = []()
        {
            return juce::Time::getCompilationDate().toString (true, true, false, true);
        };

        auto getIs64BitString = []()
        {
            return juce::SystemStats::isOperatingSystem64Bit() ? " (64-bit)" : juce::String();
        };

        auto getIsSandboxedString = []()
        {
            return juce::SystemStats::isRunningInAppExtensionSandbox() ? " (Sandboxed)" : juce::String();
        };

        auto getCPUInfoString = []()
        {
            return juce::String (juce::SystemStats::getNumCpus()) + " Core, " + juce::SystemStats::getCpuModel();
        };

        auto getBlockSizeString = [blockSize = plugin.getBlockSize()]()
        {
            return juce::String (blockSize);
        };

        auto getSampleRateString = [sampleRate = plugin.getSampleRate()]()
        {
            return juce::String (sampleRate / 1000.0, 1) + " kHz";
        };

        juce::String diagString;
        diagString += "Version: " + plugin.getName() + " " + pluginVersionString + "\n";
        diagString += "Commit: " + gitHashString + " on " + gitBranchString
                      + " with JUCE version " + juce::SystemStats::getJUCEVersion() + "\n";

        // build system info
        diagString += "Build: " + getCompilationDateString()
                      + " on " + buildFQDNString
                      + " with " + pluginCompilerIDString + "-" + pluginCompilerVersionString + "\n";

        // user system info
        diagString += "System: " + juce::SystemStats::getDeviceDescription()
                      + " with " + juce::SystemStats::getOperatingSystemName()
                      + getIs64BitString() + getIsSandboxedString()
                      + " on " + getCPUInfoString() + "\n";

        // plugin info
        juce::PluginHostType hostType {};
        diagString += "Plugin Info: " + juce::String (plugin.getWrapperTypeString())
                      + " running in " + juce::String (hostType.getHostDescription())
                      + " running at sample rate " + getSampleRateString()
                      + " with block size " + getBlockSizeString() + "\n";

        return diagString;
    }
} // namespace PluginDiagnosticInfo
} // namespace chowdsp
