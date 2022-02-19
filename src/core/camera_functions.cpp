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
