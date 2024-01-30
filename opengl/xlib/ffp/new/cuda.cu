#include <stdio.h>
#include <stdint.h>

#define WIDTH 2400 // Image width
#define HEIGHT 1800 // Image height
#define MAX_ITER 1000 // Maximum number of iterations for each pixel
#define CXMIN -2.0f // Minimum real value for the subset
#define CXMAX 2.0f // Maximum real value for the subset
#define CYMIN -1.5f // Minimum imaginary value for the subset
#define CYMAX 1.5f // Maximum imaginary value for the subset


typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB;

RGB HSBtoRGB(double hue, double saturation, double brightness) {
    hue = fmod(hue, 360.0); // Ensure hue is within [0, 360) degrees
    saturation = (saturation > 1.0) ? 1.0 : ((saturation < 0.0) ? 0.0 : saturation); // Limit saturation within [0, 1]
    brightness = (brightness > 1.0) ? 1.0 : ((brightness < 0.0) ? 0.0 : brightness); // Limit brightness within [0, 1]

    double c = saturation * brightness;
    double x = c * (1 - fabs(fmod(hue / 60.0, 2) - 1));
    double m = brightness - c;

    double r1, g1, b1;

    if (hue >= 0 && hue < 60) {
        r1 = c;
        g1 = x;
        b1 = 0;
    } else if (hue >= 60 && hue < 120) {
        r1 = x;
        g1 = c;
        b1 = 0;
    } else if (hue >= 120 && hue < 180) {
        r1 = 0;
        g1 = c;
        b1 = x;
    } else if (hue >= 180 && hue < 240) {
        r1 = 0;
        g1 = x;
        b1 = c;
    } else if (hue >= 240 && hue < 300) {
        r1 = x;
        g1 = 0;
        b1 = c;
    } else {
        r1 = c;
        g1 = 0;
        b1 = x;
    }

    RGB rgb;
    rgb.red = (uint8_t)((r1 + m) * 255);
    rgb.green = (uint8_t)((g1 + m) * 255);
    rgb.blue = (uint8_t)((b1 + m) * 255);

    return rgb;
}
__global__ void mandelbrot(int *output) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < HEIGHT && col < WIDTH) {
        float zx = CXMIN + (CXMAX - CXMIN)*col/WIDTH;
        float zy = CYMIN + (CYMAX - CYMIN)*row/WIDTH;
        float cx = -0.7269;// -0.8f; //0.0f;
        float cy = 0.1889; //0.156f; //0.0f;
        uint32_t n = 0;
        for (n = 0; n < MAX_ITER; n++)
        {
            if ((zx * zx + zy * zy) >= 4.0f)
            {
                break;
            }
            float temp = zx * zx - zy * zy + cx;
            zy = 2 * zx * zy + cy;
            zx = temp;
            n++;
        }
        output[row * WIDTH + col] = n;
    }
}

void writePPM(const char *filename, int *data) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(file, "P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for (int i = 0; i < WIDTH * HEIGHT; ++i) {
        int val = data[i];

            RGB color;
        if (val < MAX_ITER) {
            float t = ((float)val *360.0f)/ MAX_ITER;
            color = HSBtoRGB(t, 1.0f, 1.0f);
        }
        else
        {
            color = {};
        }

        fprintf(file, "%d %d %d\n", color.red, color.green, color.blue);
    }

    fclose(file);
}

int main() {
    int *output, *dev_output;

    // Allocate memory for output
    output = (int *)malloc(WIDTH * HEIGHT * sizeof(int));

    // Allocate memory on the GPU
    cudaMalloc((void **)&dev_output, WIDTH * HEIGHT * sizeof(int));

    // Define grid and block dimensions for kernel execution
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((WIDTH + threadsPerBlock.x - 1) / threadsPerBlock.x, (HEIGHT + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Launch kernel
    mandelbrot<<<numBlocks, threadsPerBlock>>>(dev_output);

    // Copy result from device to host
    cudaMemcpy(output, dev_output, WIDTH * HEIGHT * sizeof(int), cudaMemcpyDeviceToHost);

    // Write computed Mandelbrot set to a PPM file
    writePPM("mandelbrot_color.ppm", output);

    // Free device and host memory
    cudaFree(dev_output);
    free(output);

    return 0;
}

