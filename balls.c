#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define Z_LEN 200
#define Z_STEP 5
#define BALLS_LEN 5

const char c_map[]  = " .:-=+*%#@";
const int c_map_len = sizeof(c_map) - 1;


typedef struct {
    float x, y, z;
} vec3;

struct balls {
    unsigned int len;
    vec3* poss;
    vec3* vels;
    float* sizes;
};


struct balls init(const unsigned int len) {
    struct balls bs;
    bs.poss = malloc(sizeof(*bs.poss) * len);
    bs.vels = malloc(sizeof(*bs.vels) * len);
    bs.sizes = malloc(sizeof(*bs.sizes) * len);
    bs.len = len;
    for (int i = 0; i < len; i++) {
        bs.poss[i].x = 0;
        bs.poss[i].y = 0;
        bs.poss[i].z = Z_LEN/2; // magic
        bs.vels[i].x = rand() % 10000 / 100000.f;
        bs.vels[i].y = rand() % 10000 / 100000.f;
        bs.vels[i].z = rand() % 10000 / 100000.f;
        bs.sizes[i]  = rand() % Z_LEN/20 + Z_LEN/20; // magic
    }
    return bs;
}

void update(const int w, const int h, struct balls* bs) {
    for (int i = 0; i < bs->len; i++) {
        if (bs->poss[i].x <= -w           || bs->poss[i].x >= w)     bs->vels[i].x *= -1;
        if (bs->poss[i].y <= -h           || bs->poss[i].y >= h)     bs->vels[i].y *= -1;
        if (bs->poss[i].z <= bs->sizes[i] || bs->poss[i].z >= Z_LEN) bs->vels[i].z *= -1;
        bs->poss[i].x += bs->vels[i].x;
        bs->poss[i].y += bs->vels[i].y;
        bs->poss[i].z += bs->vels[i].z;
    }
}

void render(unsigned char* buffer, const int w, const int h, struct balls* bs) {
    const vec3 light = {.x=0.57735 , .y=-0.57735, .z=-0.57735}; // unit vector
    const int half_w = w/2;
    const int half_h = h/2;
    float i, j, k;
    float r;
    float normal;
    unsigned int index;
    for (int b_i = 0; b_i < bs->len; b_i++) {
        for (int y = -half_h; y < half_h; y++) {
            for (int x = -half_w; x < half_w; x++) {
                for (int z = 1; z < Z_LEN; z += Z_STEP) {
                    i = x*z/w-bs->poss[b_i].x;
                    j = y*z/w-bs->poss[b_i].y;
                    k = z    -bs->poss[b_i].z;
                    r = bs->sizes[b_i];
                    if (i*i+j*j+k*k < r*r) {
                        normal = (i/r)*light.x+(j/r)*light.y+(k/r)*light.z;
                        index = x+half_w+(y+half_h)*w;
                        buffer[index] = (unsigned int)((normal+1)/2*c_map_len);
                        break;
                    }
                }
            }
        }
    }
}

void draw(const unsigned char* buffer, const int w, const int h) {
    static int sw = 0;
    static int sh = 0;
    static unsigned char* str = NULL;
    char need_realloc = 0;
    if (sw < w) {
        sw = w;
        need_realloc = 1;
    }
    if (sh < h) {
        sh = h;
        need_realloc = 1;
    }
    if (need_realloc) {
        int str_len = 2*sw * sh + sh;
        str = realloc(str, sizeof(*str) * str_len + 1);
        memset(str, ' ', sizeof(*str) * str_len);
        for (int y = 0; y < sh; y++)
            str[(2*w*(y+1) + y)] = '\n';
        str[str_len] = '\0';
    }
    int str_i = 0;
    int buffer_i;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            buffer_i = x+y*w;
            if (buffer[buffer_i] == 0)
                str[str_i] = ' ';
            else
                str[str_i] = c_map[buffer[buffer_i]];
            str_i += 2;
        }
        str_i++;
    }
    printf("%s", str);
}

int main(int argc, char** argv) {
    srand(time(NULL));

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col/2;
    int height = w.ws_row;

    int buffer_len = width*height;
    unsigned char* buffer = malloc(sizeof(*buffer)*buffer_len);

    struct balls bs = init(BALLS_LEN);
    while (1) {
        update(width, height, &bs);
        memset(buffer, 0, width*height);
        render(buffer, width, height, &bs);
        draw(buffer, width, height);
    }
}
