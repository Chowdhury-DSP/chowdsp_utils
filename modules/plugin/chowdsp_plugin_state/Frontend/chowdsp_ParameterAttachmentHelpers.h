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
        SetValueCallback() = default;
        explicit SetValueCallback (Attachment& a) : attach (&a) {}
        SetValueCallback (const SetValueCallback&) = default;
        SetValueCallback& operator= (const SetValueCallback&) = default;
        SetValueCallback (SetValueCallback&&) noexcept = default;
        SetValueCallback& operator= (SetValueCallback&&) noexcept = default;

        template <typename T>
        void operator() (T val)
        {
            attach->setValue (val);
        }

        Attachment* attach = nullptr;
    };

    template <typename ParamType>
    struct ParameterChangeAction : public juce::UndoableAction
    {
        using ValueType = ParameterTypeHelpers::ParameterElementType<ParamType>;

        explicit ParameterChangeAction (ParamType& parameter,
                                        ValueType oldVal,
                                        ValueType newVal,
                                        juce::AudioProcessor* processor = nullptr,
                                        bool skipFirstTime = true)
            : param (parameter),
              oldValue (oldVal),
              newValue (newVal),
              proc (processor),
              firstTime (skipFirstTime)
        {
        }

        bool perform() override
        {
            if (firstTime)
            {
                firstTime = false;
                return true;
            }

            setParameterValue (newValue);
            return true;
        }

        bool undo() override
        {
            setParameterValue (oldValue);
            return true;
        }

        int getSizeInUnits() override { return sizeof (*this); }

    private:
        void setParameterValue (ValueType val)
        {
            if (proc != nullptr)
                param.beginChangeGesture();

            ParameterTypeHelpers::setValue (val, param);

            if (proc != nullptr)
                param.endChangeGesture();
        }

        ParamType& param;
        const ValueType oldValue;
        const ValueType newValue;
        juce::AudioProcessor* proc = nullptr;
        bool firstTime = true;
    };
} // namespace ParameterAttachmentHelpers
#endif
} // namespace chowdsp
