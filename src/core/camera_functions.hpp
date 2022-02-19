namespace Core {

    /**
     * A namespace for functions controlling the camera. Each scene may
     * use these functions as it is required to properly steer the camera.
     * These functions, usually called "camera Z functions", take as
     * input the current position on the X axis as a value between 0
     * and 1 and the velocity to move the camera in as a value between
     * -1 and 1, where negative values are expected to move the camera forward and
     * positive values to move it backwards, relative to the screen. They
     * are expected to return 1) the new Z position for the camera and
     * 2) the new X position for the camera. The latter allows to control
     * the camera limits from within the camera Z function and to deal with
     * the velocity in a camera-function specific way. The term "camera Z function"
     * is thus terminologically not 100% clean, since they also calculate
     * and return the new X value. */
    namespace CameraFunctions {
        void hyperbolicCamera(float velocity, float& x, float& z);
        void linearCamera(float velocity, float& x, float& z);
    }

}
