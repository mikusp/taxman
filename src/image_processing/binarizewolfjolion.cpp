/**************************************************************
 * Binarization with several methods
 * (0) Niblacks method
 * (1) Sauvola & Co.
 *     ICDAR 1997, pp 147-152
 * (2) by myself - Christian Wolf
 *     Research notebook 19.4.2001, page 129
 * (3) by myself - Christian Wolf
 *     20.4.2007
 *
 * See also:
 * Research notebook 24.4.2001, page 132 (Calculation of s)
 **************************************************************/

#include "binarizewolfjolion.h"

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
//#include <highgui.h>

using namespace std;
using namespace cv;

#define BINARIZEWOLF_VERSION	"2.3 (February 26th, 2013)"

#define uget(x,y)    at<unsigned char>(y,x)
#define uset(x,y,v)  at<unsigned char>(y,x)=v;
#define fget(x,y)    at<float>(y,x)
#define fset(x,y,v)  at<float>(y,x)=v;



double calcLocalStats (Mat &im, Mat &map_m, Mat &map_s, int winx, int winy) {

	double m,s,max_s, sum, sum_sq, foo;
	int wxh	= winx/2;
	int wyh	= winy/2;
	int x_firstth= wxh;
	int y_lastth = im.rows-wyh-1;
	int y_firstth= wyh;
	double winarea = winx*winy;

	max_s = 0;
	for	(int j = y_firstth ; j<=y_lastth; j++) 
	{
		// Calculate the initial window at the beginning of the line
		sum = sum_sq = 0;
		for	(int wy=0 ; wy<winy; wy++)
			for	(int wx=0 ; wx<winx; wx++) {
				foo = im.uget(wx,j-wyh+wy);
				sum    += foo;
				sum_sq += foo*foo;
			}
		m  = sum / winarea;
		s  = sqrt ((sum_sq - (sum*sum)/winarea)/winarea);
		if (s > max_s)
			max_s = s;
		map_m.fset(x_firstth, j, m);
		map_s.fset(x_firstth, j, s);

		// Shift the window, add and remove	new/old values to the histogram
		for	(int i=1 ; i <= im.cols-winx; i++) {

			// Remove the left old column and add the right new column
			for (int wy=0; wy<winy; ++wy) {
				foo = im.uget(i-1,j-wyh+wy);
				sum    -= foo;
				sum_sq -= foo*foo;
				foo = im.uget(i+winx-1,j-wyh+wy);
				sum    += foo;
				sum_sq += foo*foo;
			}
			m  = sum / winarea;
			s  = sqrt ((sum_sq - (sum*sum)/winarea)/winarea);
			if (s > max_s)
				max_s = s;
			map_m.fset(i+wxh, j, m);
			map_s.fset(i+wxh, j, s);
		}
	}
	
	return max_s;
}


/**********************************************************
 * The binarization routine
 **********************************************************/


void NiblackSauvolaWolfJolion (Mat im, Mat output, NiblackVersion version,
	int winx, int winy, double k, double dR) {

	
	double m, s, max_s;
	double th=0;
	double min_I, max_I;
	int wxh	= winx/2;
	int wyh	= winy/2;
	int x_firstth= wxh;
	int x_lastth = im.cols-wxh-1;
	int y_lastth = im.rows-wyh-1;
	int y_firstth= wyh;
	int mx, my;

	// Create local statistics and store them in a double matrices
	Mat map_m = Mat::zeros (im.rows, im.cols, CV_32F);
	Mat map_s = Mat::zeros (im.rows, im.cols, CV_32F);
	max_s = calcLocalStats (im, map_m, map_s, winx, winy);
	
	minMaxLoc(im, &min_I, &max_I);
			
	Mat thsurf (im.rows, im.cols, CV_32F);
			
	// Create the threshold surface, including border processing
	// ----------------------------------------------------

	for	(int j = y_firstth ; j<=y_lastth; j++) {

		// NORMAL, NON-BORDER AREA IN THE MIDDLE OF THE WINDOW:
		for	(int i=0 ; i <= im.cols-winx; i++) {

			m  = map_m.fget(i+wxh, j);
    		s  = map_s.fget(i+wxh, j);

    		// Calculate the threshold
    		switch (version) {

    			case NIBLACK:
    				th = m + k*s;
    				break;

    			case SAUVOLA:
	    			th = m * (1 + k*(s/dR-1));
	    			break;

    			case WOLFJOLION:
    				th = m + k * (s/max_s-1) * (m-min_I);
    				break;
    				
    			default:
    				cerr << "Unknown threshold type in ImageThresholder::surfaceNiblackImproved()\n";
    				exit (1);
    		}
    		
    		thsurf.fset(i+wxh,j,th);

    		if (i==0) {
        		// LEFT BORDER
        		for (int i=0; i<=x_firstth; ++i)
                	thsurf.fset(i,j,th);

        		// LEFT-UPPER CORNER
        		if (j==y_firstth)
        			for (int u=0; u<y_firstth; ++u)
        			for (int i=0; i<=x_firstth; ++i)
        				thsurf.fset(i,u,th);

        		// LEFT-LOWER CORNER
        		if (j==y_lastth)
        			for (int u=y_lastth+1; u<im.rows; ++u)
        			for (int i=0; i<=x_firstth; ++i)
        				thsurf.fset(i,u,th);
    		}

			// UPPER BORDER
			if (j==y_firstth)
				for (int u=0; u<y_firstth; ++u)
					thsurf.fset(i+wxh,u,th);

			// LOWER BORDER
			if (j==y_lastth)
				for (int u=y_lastth+1; u<im.rows; ++u)
					thsurf.fset(i+wxh,u,th);
		}

		// RIGHT BORDER
		for (int i=x_lastth; i<im.cols; ++i)
        	thsurf.fset(i,j,th);

  		// RIGHT-UPPER CORNER
		if (j==y_firstth)
			for (int u=0; u<y_firstth; ++u)
			for (int i=x_lastth; i<im.cols; ++i)
				thsurf.fset(i,u,th);

		// RIGHT-LOWER CORNER
		if (j==y_lastth)
			for (int u=y_lastth+1; u<im.rows; ++u)
			for (int i=x_lastth; i<im.cols; ++i)
				thsurf.fset(i,u,th);
	}
//	cerr << "surface created" << endl;
	
	
	for	(int y=0; y<im.rows; ++y) 
	for	(int x=0; x<im.cols; ++x) 
	{
    	if (im.uget(x,y) >= thsurf.fget(x,y))
    	{
    		output.uset(x,y,255);
    	}
    	else
    	{
    	    output.uset(x,y,0);
    	}
    }
}

/**********************************************************
 * The main function
 **********************************************************/

/*int main ()
{
	char version;
	int c;
	int winx=0, winy=0;
	float optK=0.5;
	bool didSpecifyK=false;
	NiblackVersion versionCode;	
	char *inputname, *outputname, *versionstring;	
	
	cerr << "===========================================================\n"
	     << "Christian Wolf, LIRIS Laboratory, Lyon, France.\n"
		 << "christian.wolf@liris.cnrs.fr\n"
		 << "Version " << BINARIZEWOLF_VERSION << endl
		 << "===========================================================\n";
	
	cerr << "Adaptive binarization\n"
		 << "Threshold calculation: ";
	
	
	cerr << "parameter k=" << optK << endl;
	
	if (!didSpecifyK)
		cerr << "Setting k to default value " << optK << endl;

    // Load the image in grayscale mode
    Mat input = imread(inputname,CV_LOAD_IMAGE_GRAYSCALE);

    // Treat the window size
    if (winx==0||winy==0) {
        cerr << "Input size: " << input.cols << "x" << input.rows << endl;
        winy = (int) (2.0 * input.rows-1)/3;
        winx = (int) input.cols-1 < winy ? input.cols-1 : winy;
        // if the window is too big, than we asume that the image
        // is not a single text box, but a document page: set
        // the window size to a fixed constant.
        if (winx > 100)
            winx = winy = 40;
        cerr << "Setting window size to [" << winx
            << "," << winy << "].\n";
    }
    
    // Threshold
    Mat output (input.rows, input.cols, CV_8U);
    NiblackSauvolaWolfJolion (input, output, versionCode, winx, winy, optK, 128);

    // Write the tresholded file
    cerr << "Writing binarized image to file '" << outputname << "'.\n";
    imwrite (outputname, output);	
        
    return 0;    
}*/
