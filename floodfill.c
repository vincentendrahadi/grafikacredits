#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

typedef struct Character {
    short points[50][3][2];
    short centerOfGravity[50][2];
    short triangleCount;
} Character;

typedef struct Pixel {
    short red;
    short green;
    short blue;
} Pixel;

char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
Pixel toBeDrawn[768][1366];
Character smallAlphabets[26], bigAlphabets[26];

void printPixel(int i, int j, int opacity, int blue, int green, int red) {
    int location;
    if (j < 0 || j >= 1366 || i < 0 || i >= 760) {
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

void printLine(int i_start, int j_start, int x0, int y0, int x1, int y1, int blue, int green, int red) {
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    toBeDrawn[i_start + x0][j_start + y0].red = red;
    toBeDrawn[i_start + x0][j_start + y0].green = green;
    toBeDrawn[i_start + x0][j_start + y0].blue = blue;
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void loadCharacter(char character) {
    FILE *fp;
    char filename[1];
    int sumI = 0, sumJ = 0;
    int i = 0;
    short x;

    filename[0] = character;
    fp = fopen(filename, "r");

    if (character >= 'a' && character <= 'z') {
        while (fscanf(fp, "%hd ", &x) != EOF) {
            if (i % 2 == 0) {
                sumI += x;
            } else {
                sumJ += x;
            }

            if (i % 6 == 5) {
                smallAlphabets[character - 'a'].centerOfGravity[i / 6][0] = sumI / 3;
                smallAlphabets[character - 'a'].centerOfGravity[i / 6][1] = sumJ / 3;
                sumI = 0;
                sumJ = 0;
            }

            smallAlphabets[character - 'a'].points[i / 6][i % 6 / 2][i % 2] = x;
            ++i;
        }
        smallAlphabets[character - 'a'].triangleCount = i / 6;
    } else if (character >= 'A' && character <= 'Z') {
        while (fscanf(fp, "%hd ", &x) != EOF) {
            if (i % 2 == 0) {
                sumI += x;
            } else {
                sumJ += x;
            }

            if (i % 6 == 5) {
                bigAlphabets[character - 'A'].centerOfGravity[i / 6][0] = sumI / 3;
                bigAlphabets[character - 'A'].centerOfGravity[i / 6][1] = sumJ / 3;
                sumI = 0;
                sumJ = 0;
            }

            bigAlphabets[character - 'A'].points[i / 6][i % 6 / 2][i % 2] = x;
            ++i;
        }
        bigAlphabets[character - 'A'].triangleCount = i / 6;
    }

    fclose(fp);
}

void floodFill(int i_start, int j_start, int i, int j, int blue, int green, int red) {
    if (i >= 0 && j >= 0 && i < 760 && j < 1366) {
        if (toBeDrawn[i_start + i][j_start + j].blue != blue && toBeDrawn[i][j].green != green && toBeDrawn[i][j].red != red) {
            toBeDrawn[i_start + i][j_start + j].blue = blue;
            toBeDrawn[i_start + i][j_start + j].green = green;
            toBeDrawn[i_start + i][j_start + j].red = red;

            floodFill(i_start, j_start, i - 1, j, blue, green, red);
            floodFill(i_start, j_start, i, j - 1, blue, green, red);
            floodFill(i_start, j_start, i, j + 1, blue, green, red);
            floodFill(i_start, j_start, i + 1, j, blue, green ,red);
        }  
    }
}

void printCharacter(char character, int i_start, int j_start, int blue, int green, int red) {
    if (character >= 'a' && character <= 'z') {
        for (int i = 0; i < smallAlphabets[character - 'a'].triangleCount; ++i) {
            for (int j = 0; j < 3; ++j) {
                short x1, y1;
                if (j < 2) {
                    x1 = smallAlphabets[character - 'a'].points[i][j + 1][0];
                    y1 = smallAlphabets[character - 'a'].points[i][j + 1][1];
                } else {
                    x1 = smallAlphabets[character - 'a'].points[i][0][0];
                    y1 = smallAlphabets[character - 'a'].points[i][0][1];
                }
                printLine(i_start, j_start, smallAlphabets[character - 'a'].points[i][j][0], smallAlphabets[character - 'a'].points[i][j][1], x1, y1, blue, green, red);
            }
            floodFill(i_start, j_start, smallAlphabets[character - 'a'].centerOfGravity[i][0], smallAlphabets[character - 'a'].centerOfGravity[i][1], blue, green, red);
        }
    } else if (character >= 'A' && character >= 'Z') {
        for (int i = 0; i < bigAlphabets[character - 'A'].triangleCount; ++i) {
            for (int j = 0; j < 3; ++j) {
                short x1, y1;
                if (j < 2) {
                    x1 = bigAlphabets[character - 'A'].points[i][j + 1][0];
                    y1 = bigAlphabets[character - 'A'].points[i][j + 1][1];
                } else {
                    x1 = bigAlphabets[character - 'A'].points[i][0][0];
                    y1 = bigAlphabets[character - 'A'].points[i][0][1];
                }
                printLine(i_start, j_start, bigAlphabets[character - 'A'].points[i][j][0], bigAlphabets[character - 'A'].points[i][j][1], x1, y1, blue, green, red);
            }
            floodFill(i_start, j_start, bigAlphabets[character - 'A'].centerOfGravity[i][0], bigAlphabets[character - 'A'].centerOfGravity[i][1], blue, green, red);
        }
    }
}

int main() {
    int fbfd = 0;
    long int screensize = 0;
    int x = 0, y = 0;
    int i_start, j_start;
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

    // Initiate toBeDrawn
    for (int i = 0; i < 760; ++i) {
    	for (int j = 0; j < 1366; ++j) {
    		toBeDrawn[i][j].red = 0;
            toBeDrawn[i][j].green = 0;
            toBeDrawn[i][j].blue = 0;
    	}
    }

    printf("%hd\n", toBeDrawn[760][1366].blue);

    // Load characters
    loadCharacter('g');

    // Print characters
    printCharacter('g', 100, 100, 255, 255, 255);

    for (int i = 0; i < 760; ++i) {
        for (int j = 0; j < 1366; ++j) {
            printPixel(i, j, 255, toBeDrawn[i][j].blue, toBeDrawn[i][j].green, toBeDrawn[i][j].red);
        }
    }
    printf("done\n");

    munmap(fbp, screensize);
    close(fbfd);
    while (1) {
        char x;
        scanf("%c", &x);
    }
    return 0;
}