#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glut.h>
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdint.h>


#define PI 3.1415926535
#define WIDTH 512
#define HEIGHT 512
#define FOV 60
#define NUM_RAYS WIDTH

int keys[256] = {0};
float px, py, pa; // player x, y, angle in radians
const int mapx = 16, mapy = 16, maps = 16*16;
int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1,
    1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1,
    1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,
    1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const int TILESIZE = 64;



int64_t millis() {
    struct timespec t;
    timespec_get(&t, TIME_UTC);
    int64_t milliseconds = t.tv_sec * INT64_C(1000) + t.tv_nsec / 1000000;
    return milliseconds;
}
void drawText(float x, float y, const char* text)
{
    glRasterPos2f(x, y);
    while(*text)
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *text++);
}

void drawRect(float x, float y, float w, float h){
    glBegin(GL_QUADS);
    glVertex2f(x,     y);
    glVertex2f(x+w,   y);
    glVertex2f(x+w,   y+h);
    glVertex2f(x,     y+h);
    glEnd();
}

void castRays(){
    float fovRad = FOV * PI / 180.0f;
    float halfFov = fovRad / 2.0f;
    float projDist = (WIDTH / 2.0f) / tan(halfFov);
    float rayStep = fovRad / NUM_RAYS;

    for(int col = 0; col < NUM_RAYS; col++){
        float rayA = pa - halfFov + col * rayStep;

        float cosA = cos(rayA);
        float sinA = sin(rayA);

        float hDist = 1e30f;
        float hx, hy;

        float yStep, xStep;
        float ry, rx;

        if(sinA > 0){ // ray faces down
            ry = floor(py / TILESIZE) * TILESIZE + TILESIZE;
            yStep = TILESIZE;
        } else {      // ray faces up
            ry = floor(py / TILESIZE) * TILESIZE - 0.0001f;
            yStep = -TILESIZE;
        }
        xStep = TILESIZE * cosA / fabs(sinA);
        rx = px + (ry - py) * cosA / sinA;

        for(int i = 0; i < mapy; i++){
            int mx = (int)(rx / TILESIZE);
            int my = (int)(ry / TILESIZE);
            if(mx < 0 || mx >= mapx || my < 0 || my >= mapy) break;
            if(map[my * mapx + mx] == 1){
                hDist = sqrt((rx-px)*(rx-px) + (ry-py)*(ry-py));
                hx = rx; hy = ry;
                break;
            }
            rx += xStep;
            ry += yStep;
        }

        float vDist = 1e30f;
        float vx, vy;

        if(cosA > 0){ // ray faces right
            rx = floor(px / TILESIZE) * TILESIZE + TILESIZE;
            xStep = TILESIZE;
        } else {      // ray faces left
            rx = floor(px / TILESIZE) * TILESIZE - 0.0001f;
            xStep = -TILESIZE;
        }
        yStep = TILESIZE * sinA / fabs(cosA);
        ry = py + (rx - px) * sinA / cosA;

        for(int i = 0; i < mapx; i++){
            int mx = (int)(rx / TILESIZE);
            int my = (int)(ry / TILESIZE);
            if(mx < 0 || mx >= mapx || my < 0 || my >= mapy) break;
            if(map[my * mapx + mx] == 1){
                vDist = sqrt((rx-px)*(rx-px) + (ry-py)*(ry-py));
                vx = rx; vy = ry;
                break;
            }
            rx += xStep;
            ry += yStep;
        }

        float dist, shade;
        if(hDist < vDist){
            dist = hDist;
            shade = 0.8f;
        } else {
            dist = vDist;
            shade = 1.0f;
        }

        float correctedDist = dist * cos(rayA - pa);
        if(correctedDist < 0.0001f) correctedDist = 0.0001f;

        float wallHeight = (TILESIZE / correctedDist) * projDist;

        glColor3f(shade * 0.8f, shade * 0.4f, shade * 0.2f);
        drawRect(col, HEIGHT/2 - wallHeight/2, 1, wallHeight);

        glColor3f(0.2f, 0.2f, 0.2f);
        drawRect(col, 0, 1, HEIGHT/2 - wallHeight/2);
        glColor3f(0.4f, 0.3f, 0.2f);
        drawRect(col, HEIGHT/2 + wallHeight/2, 1, HEIGHT - (HEIGHT/2 + wallHeight/2));
    }
}
void showFPS(int64_t startTime, int64_t endTime){
    int64_t deltaTime = endTime - startTime;
    float fps = 1000.0f / deltaTime;
    char* fpsText = (char*)malloc(20);
    sprintf(fpsText, "FPS: %.2f", fps);
    glColor3f(1.0f, 0.0f, 0.0f);
    drawText(10, 20, fpsText);
    free(fpsText);
}
void display(){
    int64_t startTime = millis();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    castRays();
    int64_t endTime = millis();
    showFPS(startTime, endTime);
    glutSwapBuffers();
    
}
void keyDown(unsigned char key, int x, int y)
{
    keys[key] = 1;
}

void keyUp(unsigned char key, int x, int y)
{
    keys[key] = 0;
}
void update(){
    float speed = 3.0f;
    float rotSpeed = 0.03f;
    if(keys['d']){ 
        pa += rotSpeed;
        if(pa > 2*PI) pa -= 2*PI;
    }
    if(keys['a']){
        pa -= rotSpeed;
        if(pa < 0) pa += 2*PI;
    }
    if(keys['w']){
        float pxd = cos(pa) * speed;
        float pyd = sin(pa) * speed;
        px += pxd;
        py += pyd;
        // collisions
        float epsilon = TILESIZE * 0.2f;
        float epsilonX = (pxd >= 0.0f ? epsilon : -epsilon);
        float epsilonY = (pyd >= 0.0f ? epsilon : -epsilon);
        int mx = (int)((px + epsilonX) / TILESIZE);
        int my = (int)((py + epsilonY) / TILESIZE);
        if(mx < 0 || mx >= mapx || my < 0 || my >= mapy || map[my * mapx + mx] != 0){
            px -= pxd;
            py -= pyd;
        }
    }
    if(keys['s']){
        float moveX = -cos(pa) * speed;
        float moveY = -sin(pa) * speed;
        px += moveX;
        py += moveY;
        float epsilon = TILESIZE * 0.2f;
        float epsilonX = (moveX >= 0.0f ? epsilon : -epsilon);
        float epsilonY = (moveY >= 0.0f ? epsilon : -epsilon);
        int mx = (int)((px + epsilonX) / TILESIZE);
        int my = (int)((py + epsilonY) / TILESIZE);
        if(mx < 0 || mx >= mapx || my < 0 || my >= mapy || map[my * mapx + mx] != 0){
            px -= moveX;
            py -= moveY;
        }
    }
    if(keys[27]) exit(0);
    glutPostRedisplay();
}


void init(){
    glClearColor(0.1f, 0.1f, 0.1f, 0);
    gluOrtho2D(0, WIDTH, HEIGHT, 0); // flipujemo tako da je koordinatni pocetak u gornjem levom cosku ekrana
    px = TILESIZE * 1.5f;
    py = TILESIZE * 1.5f;
    pa = 0.0f;
    //fontinit();
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Raycaster");
    init();
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutDisplayFunc(display);
    glutIdleFunc(update);
    glutMainLoop();
    return 0;
}
