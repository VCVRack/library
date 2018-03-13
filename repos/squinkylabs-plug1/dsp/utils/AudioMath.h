#pragma once
#include <cmath>
#include <functional>

class AudioMath
{
public:
    static const double Pi;
    static const double Pi_2;       // Pi / 2
    static const double Ln2;
    static const double Ln10;
    static const double E;

    static bool closeTo(double x, double y, double tolerance)
    {
        const bool ret = std::abs(x - y) < tolerance;
        return ret;
    }

    static double db(double g)
    {
        return 20 * log(g) / Ln10;
    }

    /**
     * Returns a function that generates one period of sin for x = {0..1}.
     * Range (output) is -1 to 1.
     */
    static std::function<double(double)> makeFunc_Sin();

    /*
     * Returns a function that generates an exponential defined by two points
     * At input = xMin, output will be yMin.
     * At input = xMax, output will be yMax.
     */
    static std::function<double(double)> makeFunc_Exp(double xMin, double xMax, double yMin, double yMax);
};
