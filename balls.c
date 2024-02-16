#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define Z_LEN 200
#define Z_STEP 5
#define BALLS_LEN 5

const unsigned char c_map[] = " .:-=+*%#@";
const unsigned int c_map_len = sizeof(c_map) - 1;


typedef struct {
    float x;
    float y;
    float z;
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
    float dot;
    unsigned int index;
    for (int b_i = 0; b_i < bs->len; b_i++) {
        for (int y = -half_h; y < half_h; y++) {
            for (int x = -half_w; x < half_w; x++) {
                for (int z = 1; z < Z_LEN; z += Z_STEP) {
                    i = x*z/w - bs->poss[b_i].x;
                    j = y*z/w - bs->poss[b_i].y;
                    k = z     - bs->poss[b_i].z;
                    r = bs->sizes[b_i];
                    if (i*i+j*j+k*k < r*r) {
                        index = x+half_w+(y+half_h)*w;
                        dot = (i*light.x+j*light.y+k*light.z)/r;
                        buffer[index] = (unsigned int)((dot+1)/2*c_map_len);
                        break;
                    }
                }
            }
        }
    }
}

void draw(const unsigned char* buffer, const unsigned int len) {
    static unsigned int old_len = 0;
    static unsigned int str_len = 0;
    static unsigned char* str = NULL;
    if (old_len < len) {
        old_len = len;
        str_len = len*2;
        str = realloc(str, str_len+1+1); // + newline and null terminator
        memset(str, ' ', str_len);
        str[str_len-1] = '\n';
        str[str_len] = '\0';
    }
    for (int i = 0, str_i = 0; i < len; i++, str_i += 2)
        str[str_i] = c_map[buffer[i]];
    printf("%s", str);
}

int main(void) {
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
        memset(buffer, 0, buffer_len);
        render(buffer, width, height, &bs);
        draw(buffer, buffer_len);
    }
}
