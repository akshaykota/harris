#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

float minmax(float *img_array, int height, int width, int option);

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
 CvMat *gaussianimg = cvCreateMat(input_image->height, input_image->width, CV_8U);

 int sobelx[3][3] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
 float gaussian[5][5] = {0.003765, 0.015019, 0.023792, 0.015019, 0.003765,
 0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
 0.023792, 0.094907, 0.150342, 0.094907, 0.023792,
 0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
 0.003765, 0.015019, 0.023792, 0.015019, 0.003765};

 int anchor;

 for (int l = 0; l < input_image->height; l++)
    {
     for (int m = 0; m < input_image->width; m++)
        {
         anchor = 5/2;
         if (l < anchor || m < anchor || l >  input_image->height-1-anchor || m > input_image->width-1-anchor)
         {
         gaussian_image[l][m] = img_data[l][m];
         CV_MAT_ELEM(*gaussianimg, uchar, l, m) = img_data[l][m];
         }
         else
         {
          float sum = 0;
          for (int g = anchor; g >= -anchor; g--)
            {
             for (int h = anchor; h >= -anchor; h--)
             {
              sum = sum + img_data[l-g][m-h]*gaussian[anchor-g][anchor-h];
             }
            }
          gaussian_image[l][m] = sum;
          CV_MAT_ELEM(*gaussianimg, uchar, l, m) = sum;
         }
        }
    }

 printf("%f\n", minmax(gaussian_image, input_image->height, input_image->width, 1));

 for (int l = 0; l < input_image->height; l++)
    {
     for (int m = 0; m < input_image->width; m++)
        {
         anchor = 3/2;
         if (l < anchor || m < anchor || l >  input_image->height-1-anchor || m > input_image->width-1-anchor)
         {
         sobel_image[l][m] = 0;
         }
         else
         {
          float sum = 0;
          for (int g = anchor; g >= -anchor; g--)
            {
             for (int h = anchor; h >= -anchor; h--)
             {
              sum = sum + gaussian_image[l-g][m-h]*sobelx[anchor-g][anchor-h];
             }
            }
          sobel_image[l][m] = sum;
         }
        }
    }

float max_sobel = minmax(sobel_image, input_image->height, input_image->width, 1);
float min_sobel = minmax(sobel_image, input_image->height, input_image->width, 0);

for (int g = 0; g <= input_image->height-1; g++)
    {
     for (int h = 0; h<= input_image->width-1; h++)
        {
         sobel_image[g][h] = (abs(sobel_image[g][h])/max_sobel)*255.0;
         CV_MAT_ELEM(*sobelimg, uchar, g, h) = sobel_image[g][h];
        }
    }

max_sobel = minmax(sobel_image, input_image->height, input_image->width, 1);
min_sobel = minmax(sobel_image, input_image->height, input_image->width, 0);

printf("%f\t%f", max_sobel, min_sobel);

IplImage *sobel_opencv = cvCloneImage(input_image);
cvSobel(input_image, sobel_opencv, 1, 0, 3);

IplImage *threshold = cvCloneImage(input_image);
cvThreshold(sobelimg, threshold, 0.1, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);


cvNamedWindow("Sobel", CV_WINDOW_NORMAL);
cvNamedWindow("Sobel OpenCV", CV_WINDOW_NORMAL);
cvNamedWindow("Original", CV_WINDOW_NORMAL);
cvNamedWindow("Gaussian", CV_WINDOW_NORMAL);
cvNamedWindow("Threshold", CV_WINDOW_NORMAL);
cvShowImage("Sobel", sobelimg);
cvShowImage("Sobel OpenCV", sobel_opencv);
cvShowImage("Original", input_image);
cvShowImage("Gaussian", gaussianimg);
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
