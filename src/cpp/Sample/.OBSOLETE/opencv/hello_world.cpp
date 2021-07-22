//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 stackoverflow.com.
//
//       This file is free content, distributed under cc by-sa version 3.0,
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-3.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/3.0/us/legalcode)
//
// Extracted from-
//        Source: https://stackoverflow.com/questions/10860352/getting-started-with-opencv-2-4-and-mingw-on-windows-7
//        Author: https://stackoverflow.com/users/1365324/eyalar
//        (No code modification other than adding this copyright statement)
//
//----------------------------------------------------------------------------
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

int main(int, char**) {
    //create a gui window:
    namedWindow("Output",1);

    //initialize a 120X350 matrix of black pixels:
    Mat output = Mat::zeros( 120, 350, CV_8UC3 );

    //write text on the matrix:
    putText(output,
            "Hello World :)",
            cvPoint(15,70),
            FONT_HERSHEY_PLAIN,
            3,
            cvScalar(0,255,0),
            4);

    //display the image:
    imshow("Output", output);

    //wait for the user to press any key:
    waitKey(0);

    return 0;
}
