#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define SOBELX 1
#define SOBELY 0

float minmax(float *img_array, int height, int width, int option);
void gaussianfilter(int *src, float *dst, int image_height, int image_width);
void sobelfilter(float *src, float *dst, int image_height, int image_width, int direction, CvMat *sobelimg);

int main()
{

 // Saving the image pixel data into a text file

 IplImage *input_image;
 char image_name[] = "building.jpg";
 char image_text[20];
 strcpy(image_text, image_name);
 int m = 0;
 while(image_text[m] != '.') {m++;}
 image_text[m+1] = 't';
 image_text[m+2] = 'x';
 image_text[m+3] = 't';
 image_text[m+4] = '\0';

 input_image = cvLoadImage(image_name, 0);

 FILE *input_image_data_text;
 input_image_data_text = fopen(image_text, "w");

 uchar *input_image_data = (uchar *) input_image->imageData;

 int i = 0;
 while(i < input_image->width*input_image->height)
 {
  fprintf(input_image_data_text, "%d\n", input_image_data[i]);
  i++;
 }

 fclose(input_image_data_text);

 //--------------------------
 // Reading the image pixel data from the text file and displaying
 /*
 FILE *temp;
 temp = fopen(image_text, "r");

 CvMat *imgmat = cvCreateMat(input_image->height, input_image->width, CV_8U);

 int j = 0, k = 0;
 int pix = 0;
 while(!((j+1)*k == input_image->width*input_image->height))
 {
  if(k == input_image->width)
   {j++;k = 0;}

  fscanf(temp, "%d", &pix);
  CV_MAT_ELEM( *imgmat, uchar, j, k) = pix;
  k++;
 }

 cvShowImage("Out", imgmat);
 cvWaitKey(0);

 fclose(temp);
 */


 // ----------------------------
 // Calculating derivatives in x- and y-directions using Sobel

 FILE *temp;
 temp = fopen(image_text, "r");

 int img_data[input_image->height][input_image->width];
 float gaussian_image[input_image->height][input_image->width];
 float sobel_image[input_image->height][input_image->width];

 int j = 0, k = 0;
 while(!((j+1)*k == input_image->width*input_image->height))
 {
  if(k == input_image->width)
   {j++;k = 0;}

  fscanf(temp, "%d", &img_data[j][k]);
  k++;
 }

 fclose(temp);

 CvMat *sobelimg = cvCreateMat(input_image->height, input_image->width, CV_8U);


gaussianfilter(&img_data[0][0], &gaussian_image[0][0], input_image->height, input_image->width);
sobelfilter(&gaussian_image[0][0], &sobel_image[0][0], input_image->height, input_image->width, SOBELX, sobelimg);


IplImage *sobel_opencv = cvCloneImage(input_image);
cvSobel(input_image, sobel_opencv, 1, 0, 3);

IplImage *threshold = cvCloneImage(input_image);
cvThreshold(sobelimg, threshold, 0.1, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);


cvNamedWindow("Sobel", CV_WINDOW_NORMAL);
cvNamedWindow("Sobel OpenCV", CV_WINDOW_NORMAL);
cvNamedWindow("Original", CV_WINDOW_NORMAL);
cvNamedWindow("Threshold", CV_WINDOW_NORMAL);
cvShowImage("Sobel", sobelimg);
cvShowImage("Sobel OpenCV", sobel_opencv);
cvShowImage("Original", input_image);
cvShowImage("Threshold", threshold);
cvWaitKey(0);
return 0;
}


float minmax(float *img_array, int height, int width, int option)
{
 float val = 0;
 if (option)
 {
 for (int i = 0; i <= height-1; i++)
    {
     for(int j = 0; j<= width-1; j++)
        {
         val = val > *(img_array+i*width+j) ? val : *(img_array+i*width+j);
        }
    }
  }
  else
  {
  for (int i = 0; i <= height-1; i++)
    {
     for(int j = 0; j<= width-1; j++)
        {
         val = val < *(img_array+i*width+j) ? val : *(img_array+i*width+j);
        }
    }
  }
 return val;
}


void gaussianfilter(int *src, float *dst, int image_height, int image_width)
{
 float gaussian[5][5] = {0.003765, 0.015019, 0.023792, 0.015019, 0.003765,
 0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
 0.023792, 0.094907, 0.150342, 0.094907, 0.023792,
 0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
 0.003765, 0.015019, 0.023792, 0.015019, 0.003765};

 int anchor;

 for (int l = 0; l < image_height; l++)
    {
     for (int m = 0; m < image_width; m++)
        {
         anchor = 5/2;
         if (l < anchor || m < anchor || l >  image_height-1-anchor || m > image_width-1-anchor)
         {
         *(dst + l*image_width + m) = *(src + l*image_width + m);
         }
         else
         {
          float sum = 0;
          for (int g = anchor; g >= -anchor; g--)
            {
             for (int h = anchor; h >= -anchor; h--)
             {
              sum = sum + (*(src + (l-g)*image_width + (m-h)))*(gaussian[anchor-g][anchor-h]);
             }
            }
          *(dst + l*image_width + m) = sum;
         }
        }
    }
}


void sobelfilter(float *src, float*dst, int image_height, int image_width, int direction, CvMat *sobelimg)
{
 int *sobel;

 if(direction == 1)
 {sobel = (int [3][3]){-1, 0, 1, -2, 0, 2, -1, 0, 1};}
 else
 {sobel = (int [3][3]){-1, -2, -1, 0, 0, 0, 1, 2, 1};}

 int anchor;
 for (int l = 0; l < image_height; l++)
    {
     for (int m = 0; m < image_width; m++)
        {
         anchor = 3/2;
         if (l < anchor || m < anchor || l >  image_height-1-anchor || m > image_width-1-anchor)
         {
         *(dst + l*image_width + m) = 0;
         }
         else
         {
          float sum = 0;
          for (int g = anchor; g >= -anchor; g--)
            {
             for (int h = anchor; h >= -anchor; h--)
             {
              sum = sum + (*(src + (l-g)*image_width + (m-h)))*(*(sobel + (anchor-g)*3 + (anchor-h)));
             }
            }
          *(dst + l*image_width + m) = sum;
         }
        }
    }

  float max_sobel = minmax(dst, image_height, image_width, 1);

  for (int g = 0; g <= image_height-1; g++)
    {
     for (int h = 0; h<= image_width-1; h++)
        {
         *(dst + g*image_width + h) = (abs(*(dst + g*image_width + h))/max_sobel)*255.0;
         CV_MAT_ELEM(*sobelimg, uchar, g, h) = *(dst + g*image_width + h);
        }
    }

}
