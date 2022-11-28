#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
/** This API is unstable and should not be used directly! */
namespace ParameterAttachmentHelpers
{
    template <typename Attachment>
    struct SetValueCallback
    {
        explicit SetValueCallback (Attachment& a) : attach (a) {}

        template <typename T>
        void operator() (T val)
        {
            attach.setValue (val);
        }

        Attachment& attach;
    };

    template <typename AttachmentType>
    struct ParameterChangeAction : public juce::UndoableAction
    {
        using ValueType = typename AttachmentType::ParamElementType;

        explicit ParameterChangeAction (AttachmentType& attach,
                                        ValueType oldVal,
                                        ValueType newVal)
            : attachment (attach),
              oldValue (oldVal),
              newValue (newVal)
        {
        }

        bool perform() override
        {
            if (firstTime)
            {
                firstTime = false;
                return true;
            }

            attachment.setValueAsCompleteGesture (newValue);
            return true;
        }

        bool undo() override
        {
            attachment.setValueAsCompleteGesture (oldValue);
            return true;
        }

        int getSizeInUnits() override { return sizeof (*this); }

        AttachmentType& attachment;
        const ValueType oldValue;
        const ValueType newValue;
        bool firstTime = true;
    };
} // namespace ParameterAttachmentHelpers
#endif
} // namespace chowdsp
