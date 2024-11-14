#include "chowdsp_BaseLogger.h"

namespace chowdsp
{
BaseLogger* BaseLogger::global_logger = nullptr;

void set_global_logger (BaseLogger* logger)
{
    BaseLogger::global_logger = logger;
    juce::Logger::setCurrentLogger (logger);
}

BaseLogger* get_global_logger()
{
    return BaseLogger::global_logger;
}
} // namespace chowdsp
