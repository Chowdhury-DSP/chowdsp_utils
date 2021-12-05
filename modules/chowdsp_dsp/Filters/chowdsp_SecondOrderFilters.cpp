namespace chowdsp
{
//================================================================
template <typename T>
void SecondOrderLPF<T>::calcCoefs (T fc, T qVal, T fs)
{
    using namespace Bilinear;

    const auto wc = juce::MathConstants<T>::twoPi * fc;
    const auto K = computeKValue (fc, fs);

    auto kSqTerm = (T) 1 / (wc * wc);
    auto kTerm = (T) 1 / (qVal * wc);

    BilinearTransform<T, 3>::call (this->b, this->a, { (T) 0, (T) 0, (T) 1 }, { kSqTerm, kTerm, (T) 1 }, K);
}

//================================================================
template <typename T>
void SecondOrderHPF<T>::calcCoefs (T fc, T qVal, T fs)
{
    using namespace Bilinear;

    const auto wc = juce::MathConstants<T>::twoPi * fc;
    const auto K = computeKValue (fc, fs);

    auto kSqTerm = (T) 1 / (wc * wc);
    auto kTerm = (T) 1 / (qVal * wc);

    BilinearTransform<T, 3>::call (this->b, this->a, { kSqTerm, (T) 0, (T) 0 }, { kSqTerm, kTerm, (T) 1 }, K);
}

//================================================================
template <typename T>
void SecondOrderBPF<T>::calcCoefs (T fc, T qVal, T fs)
{
    using namespace Bilinear;

    const auto wc = juce::MathConstants<T>::twoPi * fc;
    const auto K = computeKValue (fc, fs);

    auto kSqTerm = (T) 1 / (wc * wc);
    auto kTerm = (T) 1 / (qVal * wc);

    BilinearTransform<T, 3>::call (this->b, this->a, { (T) 0, kTerm, (T) 0 }, { kSqTerm, kTerm, (T) 1 }, K);
}

//================================================================
template <typename T>
void NotchFilter<T>::calcCoefs (T fc, T qVal, T fs)
{
    using namespace Bilinear;

    const auto wc = juce::MathConstants<T>::twoPi * fc;
    const auto K = computeKValue (fc, fs);

    auto kSqTerm = (T) 1 / (wc * wc);
    auto kTerm = (T) 1 / (qVal * wc);

    BilinearTransform<T, 3>::call (this->b, this->a, { kSqTerm, (T) 0, (T) 1 }, { kSqTerm, kTerm, (T) 1 }, K);
}

//================================================================
template <typename T>
void PeakingFilter<T>::calcCoefs (T fc, T qVal, T gain, T fs)
{
    using namespace Bilinear;

    const auto wc = juce::MathConstants<T>::twoPi * fc;
    const auto K = computeKValue (fc, fs);

    auto kSqTerm = (T) 1 / (wc * wc);
    auto kTerm = (T) 1 / (qVal * wc);
    auto kNum = gain > (T) 1 ? kTerm * gain : kTerm;
    auto kDen = gain < (T) 1 ? kTerm / gain : kTerm;

    BilinearTransform<T, 3>::call (this->b, this->a, { kSqTerm, kNum, (T) 1 }, { kSqTerm, kDen, (T) 1 }, K);
}

template <typename T>
void PeakingFilter<T>::calcCoefsDB (T fc, T qVal, T gainDB, T fs)
{
    calcCoefs (fc, qVal, juce::Decibels::decibelsToGain (gainDB), fs);
}

//================================================================
template <typename T>
void LowShelfFilter<T>::calcCoefs (T fc, T qVal, T gain, T fs)
{
    using namespace Bilinear;

    const auto wc = juce::MathConstants<T>::twoPi * fc;
    const auto K = computeKValue (fc, fs);

    auto A = std::sqrt (gain);
    auto Aroot = std::sqrt (A);

    auto kSqTerm = (T) 1 / (wc * wc);
    auto kTerm = Aroot / (qVal * wc);

    BilinearTransform<T, 3>::call (this->b, this->a, { A * kSqTerm, A * kTerm, A * A }, { A * kSqTerm, kTerm, (T) 1 }, K);
}

template <typename T>
void LowShelfFilter<T>::calcCoefsDB (T fc, T qVal, T gainDB, T fs)
{
    calcCoefs (fc, qVal, juce::Decibels::decibelsToGain (gainDB), fs);
}

//================================================================
template <typename T>
void HighShelfFilter<T>::calcCoefs (T fc, T qVal, T gain, T fs)
{
    using namespace Bilinear;

    const auto wc = juce::MathConstants<T>::twoPi * fc;
    const auto K = computeKValue (fc, fs);

    auto A = std::sqrt (gain);
    auto Aroot = std::sqrt (A);

    auto kSqTerm = (T) 1 / (wc * wc);
    auto kTerm = Aroot / (qVal * wc);

    BilinearTransform<T, 3>::call (this->b, this->a, { A * A * kSqTerm, A * kTerm, A }, { kSqTerm, kTerm, (T) A }, K);
}

template <typename T>
void HighShelfFilter<T>::calcCoefsDB (T fc, T qVal, T gainDB, T fs)
{
    calcCoefs (fc, qVal, juce::Decibels::decibelsToGain (gainDB), fs);
}
} // namespace chowdsp
