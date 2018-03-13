#include "AudioMath.h"

const double AudioMath::Pi = 3.1415926535897932384626433832795028841971;
const double AudioMath::Pi_2 = 1.5707963267948966192313216916397514420986;
const double AudioMath::Ln2 = 0.693147180559945309417;
const double AudioMath::Ln10 = 2.30258509299404568402;
const double AudioMath::E = 2.71828182845904523536;

std::function<double(double)> AudioMath::makeFunc_Sin()
{
    return [](double x) {
        return std::sin(x * 2 * Pi);
    };
}

std::function<double(double)> AudioMath::makeFunc_Exp(double xMin, double xMax, double yMin, double yMax)
{
    const double a = (std::log(yMax) - log(yMin)) / (xMax - xMin);
    const double b = log(yMin) - a * xMin;
    return [a, b](double d) {
        return std::exp(a * d + b);
    };
}