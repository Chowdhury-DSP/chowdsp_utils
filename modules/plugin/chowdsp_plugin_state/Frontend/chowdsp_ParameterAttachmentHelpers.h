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

    template <typename ParamType>
    struct ParameterChangeAction : public juce::UndoableAction
    {
        using ValueType = ParameterTypeHelpers::ParameterElementType<ParamType>;

        explicit ParameterChangeAction (ParamType& parameter,
                                        ValueType oldVal,
                                        ValueType newVal)
            : param (parameter),
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
            param.beginChangeGesture();
            ParameterTypeHelpers::setValue (val, param);
            param.endChangeGesture();
        }

        ParamType& param;
        const ValueType oldValue;
        const ValueType newValue;
        bool firstTime = true;
    };
} // namespace ParameterAttachmentHelpers
#endif
} // namespace chowdsp
