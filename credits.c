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
        char filename[9];
        filename[0] = 'a';
        filename[1] = 's';
        filename[2] = 's';
        filename[3] = 'e';
        filename[4] = 't';
        filename[5] = 's';
        filename[6] = '/';
        filename[7] = c;
        filename[8] = '\0';
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
            if (alphabets[content - 'A'][i][j] == 1) {
                printPixel(i_start + i, j_start + j, 0, 255, 255, 255);
            } else {
                printPixel(i_start + i, j_start + j, 0, 0, 0, 0);
            }
        }
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

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (*fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }

    // load characters format from txt
    loadCharacters();

    for (y = 0; y < 760; y++) {
        for (x = 0; x < 1366; x++) {
            printPixel(y, x, 0, 255, 255, 0);
        }
    }
    printCharacter(100, 0, 0, 255, 255, 255, 'A');

    munmap(fbp, screensize);
    close(fbfd);
    while (1) {
        char x;
        scanf("%c", &x);
    }
    return 0;
}
