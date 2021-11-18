#pragma once

namespace chowdsp
{
#if CHOWDSP_USE_FOLEYS_CLASSES
/** 
 * Registers ChowDSP custom GUI elements for use within Foley's GUI magic.
 * @param builder: the foleys::MagicGUIBuilder for which to register the GUI elements
 */
inline void registerGUIClasses (std::unique_ptr<foleys::MagicGUIBuilder>& builder)
{
    builder->registerLookAndFeel ("MyLNF", std::make_unique<ChowLNF>());
    builder->registerFactory ("TooltipComp", &TooltipItem::factory);
    builder->registerFactory ("InfoComp", &InfoItem::factory);
    builder->registerFactory ("TitleComp", &TitleItem::factory);
}

/**
 * Creates a foleys::MagicGUIBuilder with ChowDSP and JUCE GUI elements registered.
 * @param magicState: the foleys::MagicProcessorState to create the builder with
 */
inline std::unique_ptr<foleys::MagicGUIBuilder> createGUIBuilder (foleys::MagicProcessorState& magicState)
{
    auto builder = std::make_unique<foleys::MagicGUIBuilder> (magicState);
    builder->registerJUCEFactories();
    builder->registerJUCELookAndFeels();
    registerGUIClasses (builder);

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move") // Clang doesn't like std::move
    return std::move (builder);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}
#endif

} // namespace chowdsp
