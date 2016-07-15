// Conversion of all 2-D Matrices to 2-D Arrays
// Border Replication and Debordering
// Run Time parameter intake - Block Size, Sobel Aperture Size, k parameter, Threshold
// Block Processing is implemented


#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

int image_height = 0, image_width = 0;
int bord_image_height = 0, bord_image_width = 0;
int bordlen = 0;

float minmax(float *img_array, int option);
void gaussianfilter(float *src, float *dst, int filtersize, float sigma, int q);
void sobelfilter(float *src, float *sobelx, float *sobely, int apsize, int q);
void matmul(float *src1, float *src2, float *dst, int q);
void responsecalc(float *x2, float *y2, float *xy, float *dst, float thresh, float k_param);
void imgwrite(char image_name[]);
void imgread(char name[], int *img_data);
void getdim(char name[]);
void borderrep(int *img, float *borderimg);
void deborderrep(float *borderimg, float *img);
void get_gaussiankernel(int size, float sigma, float *kernel);
void get_sobelkernel(int size, int *kernelx, int *kernely);

int main(int argc, char *argv[])
{
    int blocksize = atoi(argv[1]);
    int sobelaperturesize = atoi(argv[2]);
    float k_param = atof(argv[3]);
    float thresh = atof(argv[4]);
    float sigma = 1.0;

    char image[] = "left01.jpg";
    imgwrite(image);

    char image_txt[] = "left01.txt";
    getdim(image_txt);
    int img_data[image_height*image_width];
    imgread(image_txt, img_data);

    bordlen = blocksize/2;
    bord_image_height = image_height + bordlen*2;
    bord_image_width = image_width + bordlen*2;

    float *bordimg = (float*)malloc(bord_image_height*bord_image_width*sizeof(float));
    float *gaussian_image_buf = (float *) malloc(bord_image_width*(blocksize+sobelaperturesize-1)*sizeof(float));
    float *sobel_image_x_buf = (float *) malloc(bord_image_width*blocksize*sizeof(float));
    float *sobel_image_y_buf = (float *) malloc(bord_image_width*blocksize*sizeof(float));
    float *sobel_image_x2_buf = (float *) malloc(bord_image_width*blocksize*sizeof(float));
    float *sobel_image_y2_buf = (float *) malloc(bord_image_width*blocksize*sizeof(float));
    float *sobel_image_xy_buf = (float *) malloc(bord_image_width*blocksize*sizeof(float));
    float *sobel_x2_sum_buf = (float *) malloc(bord_image_width*1*sizeof(float));
    float *sobel_y2_sum_buf = (float *) malloc(bord_image_width*1*sizeof(float));
    float *sobel_xy_sum_buf = (float *) malloc(bord_image_width*1*sizeof(float));
    float *corner_response = (float *) malloc(bord_image_width*bord_image_height*sizeof(float));
    float *fin_response = (float *) malloc(image_width*image_height*sizeof(float));
    int *overlap = (int *) malloc(image_width*image_height*sizeof(int));

    borderrep(img_data, bordimg);

    gaussianfilter(&bordimg[bord_image_width*bordlen], gaussian_image_buf, blocksize, sigma, blocksize+sobelaperturesize-1);
    //printf("%f", minmax(gaussian_image_buf, 1));
    sobelfilter(&gaussian_image_buf[bord_image_width*(sobelaperturesize/2)], sobel_image_x_buf, sobel_image_y_buf, sobelaperturesize, blocksize);
    //printf("%f\t%f", minmax(gaussian_image_buf, 1), minmax(sobel_image_x_buf, 1));

    matmul(sobel_image_x_buf, sobel_image_x_buf, sobel_image_x2_buf, blocksize);
    matmul(sobel_image_y_buf, sobel_image_y_buf, sobel_image_y2_buf, blocksize);
    matmul(sobel_image_x_buf, sobel_image_y_buf, sobel_image_xy_buf, blocksize);

    gaussianfilter(&sobel_image_x2_buf[bord_image_width*(blocksize/2)], sobel_x2_sum_buf, blocksize, sigma, 1);
    gaussianfilter(&sobel_image_y2_buf[bord_image_width*(blocksize/2)], sobel_y2_sum_buf, blocksize, sigma, 1);
    gaussianfilter(&sobel_image_xy_buf[bord_image_width*(blocksize/2)], sobel_xy_sum_buf, blocksize, sigma, 1);

    responsecalc(sobel_x2_sum_buf, sobel_y2_sum_buf, sobel_xy_sum_buf, &corner_response[bord_image_width*(bordlen+(sobelaperturesize/2)+(blocksize/2))], thresh, k_param);

    for (int t = (bordlen+blocksize+sobelaperturesize); t < (bord_image_height-bordlen); t++)
    {
        for (int i = 0; i < (blocksize+sobelaperturesize-2)*bord_image_width; i++)
            gaussian_image_buf[i] = gaussian_image_buf[i+bord_image_width];

        for (int i = 0; i < (blocksize-1)*bord_image_width; i++)
        {
            sobel_image_x_buf[i] = sobel_image_x_buf[i + bord_image_width];
            sobel_image_y_buf[i] = sobel_image_y_buf[i + bord_image_width];
            sobel_image_x2_buf[i] = sobel_image_x2_buf[i + bord_image_width];
            sobel_image_y2_buf[i] = sobel_image_y2_buf[i + bord_image_width];
            sobel_image_xy_buf[i] = sobel_image_xy_buf[i + bord_image_width];
        }
        gaussianfilter(&bordimg[bord_image_width*t], &gaussian_image_buf[bord_image_width*(blocksize+sobelaperturesize-2)], blocksize, sigma, 1);

        sobelfilter(&gaussian_image_buf[bord_image_width*(blocksize+(sobelaperturesize/2)-1)], &sobel_image_x_buf[bord_image_width*(blocksize-1)], &sobel_image_y_buf[bord_image_width*(blocksize-1)], sobelaperturesize, 1);

        matmul(&sobel_image_x_buf[bord_image_width*(blocksize-1)], &sobel_image_x_buf[bord_image_width*(blocksize-1)], &sobel_image_x2_buf[bord_image_width*(blocksize-1)], 1);
        matmul(&sobel_image_y_buf[bord_image_width*(blocksize-1)], &sobel_image_y_buf[bord_image_width*(blocksize-1)], &sobel_image_y2_buf[bord_image_width*(blocksize-1)], 1);
        matmul(&sobel_image_x_buf[bord_image_width*(blocksize-1)], &sobel_image_y_buf[bord_image_width*(blocksize-1)], &sobel_image_xy_buf[bord_image_width*(blocksize-1)], 1);

        gaussianfilter(&sobel_image_x2_buf[bord_image_width*(blocksize/2)], sobel_x2_sum_buf, blocksize, sigma, 1);
        gaussianfilter(&sobel_image_y2_buf[bord_image_width*(blocksize/2)], sobel_y2_sum_buf, blocksize, sigma, 1);
        gaussianfilter(&sobel_image_xy_buf[bord_image_width*(blocksize/2)], sobel_xy_sum_buf, blocksize, sigma, 1);

        responsecalc(sobel_x2_sum_buf, sobel_y2_sum_buf, sobel_xy_sum_buf, &corner_response[bord_image_width*(bordlen+(sobelaperturesize/2)+(blocksize/2)+(t-(blocksize+sobelaperturesize-1)))], thresh, k_param);
    }

    deborderrep(corner_response, fin_response);

    for (int i = 0 ; i < image_height*image_width; i++)
    {
        if (fin_response[i] == 255)
            overlap[i] = 255;
        else
            overlap[i] = img_data[i];
    }

    IplImage *input_image = cvLoadImage("left01.jpg", 0);
    IplImage *harris_opencv = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_32F, 1);
    cvCornerHarris(input_image, harris_opencv, blocksize, sobelaperturesize, k_param);

    IplImage *test = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 1);
    uchar *test_data = (uchar *)test->imageData;

    for (int i = 0; i < image_height; i++)
    {
        for (int j = 0; j < image_width; j++)
        {
            *(test_data + i*image_width + j) = fin_response[j + i*image_width];
        }
    }

    cvNamedWindow("Original", 1);
    cvNamedWindow("Final Response", CV_WINDOW_NORMAL);
    cvNamedWindow("Harris OpenCV", 1);
    cvShowImage("Original", input_image);
    cvShowImage("Final Response", test);
    cvShowImage("Harris OpenCV", harris_opencv);

    cvSaveImage("lost01mod.jpg", test, 0);
    cvWaitKey(0);

    return 0;
}


float minmax(float *img_array, int option)
{
    float val = 0;
    if (option)
    {
        for (int i = 0; i < image_height*image_width; i++)
            val = val > img_array[i] ? val : img_array[i];
    }
    else
    {
        for (int i = 0; i < image_height*image_width; i++)
            val = val < img_array[i]? val : img_array[i];
    }
    return val;
}


void gaussianfilter(float *src, float *dst, int kernel_size, float sigma, int q)
{
    float gaussian[kernel_size*kernel_size];
    get_gaussiankernel(kernel_size, sigma, gaussian);
    int halfsize = kernel_size/2;

    for (int l = 0; l < bord_image_width*q; l++)
    {
        if ((l%bord_image_width) < bordlen)
            dst[l] = 0;
        else if ((l%bord_image_width) >= (bord_image_width-bordlen))
            dst[l] = 0;
        else
        {
            float sum = 0;
            for (int i = 0, m = -halfsize; i < kernel_size; i++, m++)
            {
                for (int j = 0, n = -halfsize; j < kernel_size; j++, n++)
                {
                    sum = sum + src[l + n + bord_image_width*m]*gaussian[j + kernel_size*i];
                }
            }
            dst[l] = sum;
        }
    }
}


void sobelfilter(float *src, float *sobelx, float *sobely, int side, int q)
{
    int kernelx[side*side], kernely[side*side];
    get_sobelkernel(side, kernelx, kernely);
    int halfside = side/2;

    for (int l = 0; l < bord_image_width*q; l++)
    {
        if ((l%bord_image_width) < bordlen)
            sobelx[l] = 0;
        else if ((l%bord_image_width) >= (bord_image_width-bordlen))
            sobelx[l] = 0;
        else
        {
            float sum = 0;
            for (int i = 0, m = -halfside; i < side; i++, m++)
            {
                for (int j = 0, n = -halfside; j < side; j++, n++)
                {
                    sum = sum + src[l + n + bord_image_width*m]*kernelx[j + side*i];
                }
            }
            sobelx[l] = sum;
        }
    }

    for (int l = 0; l < bord_image_width*q; l++)
    {
        if ((l%bord_image_width) < bordlen)
            sobely[l] = 0;
        else if ((l%bord_image_width) >= (bord_image_width-bordlen))
            sobely[l] = 0;
        else
        {
            float sum = 0;
            for (int i = 0, m = -halfside; i < side; i++, m++)
            {
                for (int j = 0, n = -halfside; j < side; j++, n++)
                {
                    sum = sum + src[l + n + bord_image_width*m]*kernely[j + side*i];
                }
            }
            sobely[l] = sum;
        }
    }
}



void matmul(float *src1, float *src2, float *dst, int q)
{
    for (int i = 0; i < q*bord_image_width; i++)
        dst[i] = src1[i] * src2[i];
}



void responsecalc(float *x2, float *y2, float *xy, float *dst, float thresh, float k_param)
{
    for (int i = 0; i < bord_image_width; i++)
    {
        float det = (x2[i] * y2[i]) - pow(xy[i], 2);
        float trace = x2[i] + y2[i];
        dst[i] = det - k_param*trace*trace;
    }

    for (int i = 0; i < bord_image_width; i++)
    {
        if (dst[i] > thresh)
            dst[i] = 255;
        else
            dst[i] = 0;
    }
}


void imgwrite(char image_name[])
{
    IplImage *input_image;
    char image_text[20];
    strcpy(image_text, image_name);
    int m = 0;
    while(image_text[m] != '.')
    {
        m++;
    }
    image_text[m+1] = 't';
    image_text[m+2] = 'x';
    image_text[m+3] = 't';
    image_text[m+4] = '\0';

    input_image = cvLoadImage(image_name, 0);

    FILE *input_image_data_text;
    input_image_data_text = fopen(image_text, "w");

    uchar *input_image_data = (uchar *) input_image->imageData;

    fprintf(input_image_data_text, "%d\n", input_image->height);
    fprintf(input_image_data_text, "%d\n", input_image->width);

    int i = 0;
    while(i < input_image->width*input_image->height)
    {
        fprintf(input_image_data_text, "%d\n", input_image_data[i]);
        i++;
    }

    fclose(input_image_data_text);
}


void imgread(char image_text[], int *img_data)
{
    FILE *temp;
    temp = fopen(image_text, "r");

    fscanf(temp, "%d", &image_height);
    fscanf(temp, "%d", &image_width);

    int k = 0;
    while(!(k == image_width*image_height))
    {
        fscanf(temp, "%d", &img_data[k]);
        k++;
    }
    fclose(temp);
}

void getdim(char image_text[])
{
    FILE *temp;
    temp = fopen(image_text, "r");

    fscanf(temp, "%d", &image_height);
    fscanf(temp, "%d", &image_width);

    fclose(temp);
}

void borderrep(int *img, float *borderimg)
{
    int j = 0;
    for (int i = 0; i < bord_image_height*bord_image_width; i++)
    {
        if (i % bord_image_width == 0 && i > 0)
            j++;
        if (i % bord_image_width < bordlen && i < bord_image_width*bordlen)
            borderimg[i] = img[0];
        else if ((i % bord_image_width) >= (bord_image_width-bordlen) && i < bord_image_width*bordlen)
            borderimg[i] = img[image_width-1];
        else if (i < bord_image_width*bordlen)
            borderimg[i] = img[(i%bord_image_width) - bordlen];
        else if ((i % bord_image_width) < bordlen && i >= bord_image_width*bordlen && i < bord_image_width*(image_height+bordlen))
            borderimg[i] = img[(j-bordlen)*image_width];
        else if ((i % bord_image_width) >= (bord_image_width-bordlen) && i >= bord_image_width*bordlen && i < bord_image_width*(image_height+bordlen))
            borderimg[i] = img[(j-bordlen+1)*image_width-1];
        else if (i >= bord_image_width*bordlen && i < bord_image_width*(image_height+bordlen))
            borderimg[i] = img[(i%bord_image_width) - bordlen + (j-bordlen)*image_width];
        else if ((i % bord_image_width) < bordlen && i >= bord_image_width*(image_height+bordlen))
            borderimg[i] = img[(image_height-1)*image_width];
        else if ((i % bord_image_width) >= (bord_image_width-bordlen))
            borderimg[i] = img[image_height*image_width-1];
        else
            borderimg[i] = img[(i%bord_image_width) - bordlen + (image_height-1)*image_width];
    }
}

void deborderrep(float *borderimg, float *deborderimg)
{
    int j = 0;
    for (int i = 0; i < image_height*image_width; i++)
    {
        if (i%image_width == 0 && i > 0)
            j++;
        deborderimg[i] = borderimg[bord_image_width*bordlen + i + j*bordlen*2];
    }
}

void get_gaussiankernel(int size, float sigma, float *kernel)
{
    int halfsize = size/2;
    float gaussian[size][size];
    float sum = 0;

    for (int i = -halfsize; i <= halfsize; i++)
    {
        for (int j = -halfsize; j <= halfsize; j++)
        {
            gaussian[i+halfsize][j+halfsize] = exp(-(i*i+j*j)/(2*sigma*sigma))/(2*3.142*sigma*sigma);
            sum += gaussian[i+halfsize][j+halfsize];
        }
    }

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            gaussian[i][j] /= sum;
            kernel[j + i*size] = gaussian[i][j];
        }
    }
}

void get_sobelkernel(int side, int *kernelx, int *kernely)
{
    int Kx[side][side], Ky[side][side];
    int halfSide = side / 2;
    for (int i = 0; i < side; i++)
    {
        int k = (i <= halfSide) ? (halfSide + i) : (side + halfSide - i - 1);
        for (int j = 0; j < side; j++)
        {
            if (j < halfSide)
                Kx[i][j] = Ky[j][i] = j - k;
            else if (j > halfSide)
                Kx[i][j] = Ky[j][i] = k - (side - j - 1);
            else
                Kx[i][j] = Ky[j][i] = 0;
        }
    }

    for (int i = 0; i < side; i++)
    {
        for (int j = 0; j < side; j++)
        {
            kernelx[i*side+j] = Kx[i][j];
            kernely[(side*side)-(i*side+j)-1] = Ky[i][j];
        }
    }
}
