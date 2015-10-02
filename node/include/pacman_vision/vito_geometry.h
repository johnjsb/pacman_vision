#ifndef _INCL_VITO_GEOMETRY
#define _INCL_VITO_GEOMETRY

#include <pacman_vision/utility.h>
#include <array>

//Globally define Vito Robot Geometry (Taken from meshes)
const std::array<Box,7> lwr_arm{ {
  Box(-0.06, -0.094, 0,
       0.06,  0.06,  0.269), //Link1
  Box(-0.06, -0.06,  -0.06,
       0.06,  0.094, 0.192), //Link2
  Box(-0.06, -0.06,  0,
       0.06,  0.094, 0.269), //Link3
  Box(-0.06, -0.094, -0.06,
       0.06,  0.06,  0.269), //Link4
  Box(-0.06, -0.056, 0,
       0.06,  0.06,  0.269), //Link5
  Box(-0.071, -0.056, -0.071,
       0.071, 0.08,  0.057), //Link6
  Box(-0.04,  -0.04, -0.031,
       0.04,  0.04,  0)} };
//And soft hands
const std::array<Box,21> soft_hand_right{ {
  Box(-0.038, -0.038, 0,
       0.038,  0.038, 0.068), //Base
  Box(-0.032, -0.054, -0.017,
       0.017,  0.045, 0.119), //palm_link
  Box(-0.022, -0.012, - 0.015,
       0.023,  0.012, 0.018), //kukle
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //proximal
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016), //distal
  Box(-0.022, -0.012, - 0.015,
       0.023,  0.012, 0.018), //kukle
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //proximal
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016), //distal
  Box(-0.022, -0.012, - 0.015,
       0.023,  0.012, 0.018), //kukle
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //proximal
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016), //distal
  Box(-0.022, -0.012, - 0.015,
       0.023,  0.012, 0.018), //kukle
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //proximal
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016), //distal
  Box(-0.011, -0.019, -0.016,
       0.036,  0.011,  0.017),  //Thumb
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016) //distal
} };
const std::array<Box,21> soft_hand_left{ {
  Box(-0.038, -0.038, 0,
       0.038,  0.038, 0.068), //Base
  Box(-0.032, -0.045, -0.017,
       0.017,  0.054, 0.119), //palm_link
  Box(-0.022, -0.012, - 0.015,
       0.023,  0.012, 0.018), //kukle
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //proximal
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016), //distal
  Box(-0.022, -0.012, - 0.015,
       0.023,  0.012, 0.018), //kukle
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //proximal
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016), //distal
  Box(-0.022, -0.012, - 0.015,
       0.023,  0.012, 0.018), //kukle
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //proximal
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016), //distal
  Box(-0.022, -0.012, - 0.015,
       0.023,  0.012, 0.018), //kukle
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //proximal
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016), //distal
  Box(-0.011, -0.011, -0.016,
       0.036,  0.019,  0.017),  //Thumb
  Box(-0.013, -0.012, -0.014,
       0.023,  0.012,  0.015), //middle
  Box(-0.013, -0.012, -0.014,
       0.03,   0.012,  0.016) //distal
} };
#endif
