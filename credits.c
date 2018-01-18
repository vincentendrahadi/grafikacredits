/*
To test that the Linux framebuffer is set up correctly, and that the device permissions
are correct, use the program below which opens the frame buffer and draws a gradient-
filled red square:
retrieved from:
Testing the Linux Framebuffer for Qtopia Core (qt4-x11-4.2.2)
http://cep.xor.aps.anl.gov/software/qt4-x11-4.2.2/qtopiacore-testingframebuffer.html
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

// global variable
char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
short alphabets[26][28][20];
short numbers[10][28][20];
short symbols[3][28][20];

void loadCharacters() {
    FILE *fp;

    for (char c = 'A'; c <= 'A'; c++) {
        char x;
        int i = 0, j = 0;
        char filename[2];
        filename[0] = c;
        filename[1] = '\0';
        fp = fopen(filename, "r");
        while (fscanf(fp, "%c", &x) != EOF) {
            if (x == 'X') {
                alphabets[c - 'A'][i][j] = 1;
            } else if (x == '\n') {
                i++;
                j = -1;
            } else {
                alphabets[c - 'A'][i][j] = 0;
            }
            j++;
        }
        fclose(fp);
    }
}

void printPixel(int i, int j, int opacity, int blue, int green, int red) {
    int location;
    if (j < 0 || j >= 1366 || i < 0 || i >= 768) {
        // out of bound, no need to print
    } else {
        location = (j+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                   (i+vinfo.yoffset) * finfo.line_length;
        *(fbp + location) = blue;        // Some blue
        *(fbp + location + 1) = green;     // A little green
        *(fbp + location + 2) = red;    // A lot of red
        *(fbp + location + 3) = opacity;      // No transparency
    }
}

void printCharacter(int i_start, int j_start, int opacity, int blue, int green, int red, char content) {
    int i, j;
    for (i = 0; i < 28; i++) {
        for (j = 0; j < 20; j++) {
            // printf("%d", content - 'A');
            // printf("%d", alphabets[content - 'A'][i][j]);
            if (alphabets[content - 'A'][i][j] == 1) {
                printPixel(i_start + i, j_start + j, 0, 255, 255, 255);
            } else {
                printPixel(i_start + i, j_start + j, 0, 0, 0, 0);
            }
        }
        // printf("\n");
    }
}

int main()
{
    int fbfd = 0;
    // struct fb_var_screeninfo vinfo;
    // struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    int x = 0, y = 0;
    long int location = 0;

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    // printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    // printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (*fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    // printf("The framebuffer device was mapped to memory successfully.\n");

    // load characters format from txt
    loadCharacters();

    x = 100; y = 100;       // Where we are going to put the pixel

    // Figure out where in memory to put the pixel
    // for (y = 100; y < 300; y++)
    //     for (x = 100; x < 300; x++) {

    //         location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
    //                    (y+vinfo.yoffset) * finfo.line_length;

    //         if (vinfo.bits_per_pixel == 32) {
    //             *(fbp + location) = 100;        // Some blue
    //             *(fbp + location + 1) = 15+(x-100)/2;     // A little green
    //             *(fbp + location + 2) = 200-(y-100)/5;    // A lot of red
    //             *(fbp + location + 3) = 0;      // No transparency
    //     //location += 4;
    //         } else  { //assume 16bpp
    //             int b = 10;
    //             int g = (x-100)/6;     // A little green
    //             int r = 31-(y-100)/16;    // A lot of red
    //             unsigned short int t = r<<11 | g << 5 | b;
    //             *((unsigned short int*)(fbp + location)) = t;
    //         }

    //     }

    for (y = 0; y < 760; y++) {
        for (x = 0; x < 1366; x++) {
            printPixel(y, x, 0, 255, 255, 0);
        }
    }
    printCharacter(100, 0, 0, 255, 255, 255, 'A');
    // for (y = 0; y < 500; y++)
    //     for (x = 0; x < 500; x++) {
    //         printPixel(x, y, 255, 255, 255, 0);
    //     }
    munmap(fbp, screensize);
    close(fbfd);
    while (1) {
        char x;
        scanf("%c", &x);
    }
    return 0;
}