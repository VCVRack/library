/**
 * ButterworthFilterDesigner
 * a bunch of functions for generating the parameters of butterworth filters
 */

#include "ButterworthFilterDesigner.h"
#include "DspFilter.h"
#include "BiquadFilter.h"
#include <memory>

template <typename T>
void ButterworthFilterDesigner<T>::designThreePoleLowpass(BiquadParams<T, 2>& outParams, T frequency)
{
    auto lp3 = new Dsp::ButterLowPass<3, 1>();
    lp3->SetupAs(frequency);
    assert(lp3->GetStageCount() == 2);
    BiquadFilter<T>::fillFromStages(outParams, lp3->Stages(), lp3->GetStageCount());
    delete lp3;
}

template <typename T>
void ButterworthFilterDesigner<T>::designTwoPoleLowpass(BiquadParams<T, 1>& outParams, T frequency)
{

    // TODO: use unique_ptr
    auto lp2 = new Dsp::ButterLowPass<2, 1>();
    lp2->SetupAs(frequency);
    assert(lp2->GetStageCount() == 1);
    BiquadFilter<T>::fillFromStages(outParams, lp2->Stages(), lp2->GetStageCount());
    delete lp2;
}

// Explicit instantiation, so we can put implementation into .cpp file
// TODO: option to take out float version (if we don't need it)
// Or put all in header
template class ButterworthFilterDesigner<double>;
template class ButterworthFilterDesigner<float>;
