#include "camera_functions.hpp"

using namespace Core;

/**
 * This function places the camera along a hyperbole with the definition:
 *
 *   z(x) = -1/(x-1) with x ∈ [0,1]
 *
 * Camera limits are [0, 0.9], that is, the calculation is cropped when
 * those values are reached. It currently supports only one uniform
 * velocity.
 */
void CameraFunctions::hyperbolicCamera(float velocity, float& x, float& z)
{
    if (velocity > 0) {
        x = x + 0.01f;
    } else if (velocity < 0) {
        x = x - 0.01f;
    } else {
        return;
    }

    if (x < 0.0f) {
        x = 0.0f;
    } else if (x > 0.9f) {
        x = 0.9f;
    }

    z = -1.0f/(x-1); // Camera Z function determing the new position on the hyberbole
}

/**
 * This function places the camera along a linear line defined as:
 *
 *   z(x) = -0.5x
 *
 * Camera limits are [0, -10], that is, any values outside this range
 * are clamped. The X movement is calculated as a fourth of the
 * actual velocity value given, i.e. of the value read from the joystick.
 * As a result, the camera movement achieved by this camera function
 * is really smooth.
 */
void CameraFunctions::linearCamera(float velocity, float& x, float& z)
{
    x = x - velocity * 0.25;

    if (x < -10.0f) {
        x = -10.0f;
    } else if (x > 0.0f) {
        x = 0.0f;
    }

    z = -0.5 * x;
}
