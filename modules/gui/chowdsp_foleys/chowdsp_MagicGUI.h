#pragma once

namespace chowdsp
{
/** 
 * Registers ChowDSP custom GUI elements for use within Foley's GUI magic.
 * @param builder: the foleys::MagicGUIBuilder for which to register the GUI elements
 */
inline void registerGUIClasses (std::unique_ptr<foleys::MagicGUIBuilder>& builder)
{
    builder->registerLookAndFeel ("MyLNF", std::make_unique<ChowLNF>());
    builder->registerFactory ("TooltipComp", &TooltipItem::factory);
    builder->registerFactory ("InfoComp", &InfoItem<>::factory);
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

    return builder;
}

} // namespace chowdsp
