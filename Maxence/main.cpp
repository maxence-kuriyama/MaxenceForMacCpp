#define PRINT_MAT(X) cout << #X << ":\n" << X << endl << endl
#define PRINT_MAT2(X,DESC) cout << DESC << ":\n" << X << endl << endl
#define PRINT_FNC    cout << "[" << __func__ << "]" << endl
#define FPS     60

#include <iostream>
#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>
#include <random>
#include <time.h>
#include <unistd.h>     // for sleep func
//#include <Eigen/Core>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
using namespace std;
//using namespace Eigen;
//using namespace cv;

void idle();
void setup();
void resize(int width, int height);
void timer(int value);
void mouse(int button, int state, int cx, int cy);
void motion(int cx, int cy);
void ps_motion(int cx, int cy);
void keyboard(unsigned char key, int cx , int cy);
void sp_key(int key, int x, int y);
void display();
void drawSphere(double x, double y, double z, double r, int div = 5);
void drawCircle(double x0, double y0, double r, int div = 20);
void render_string(float x, float y, const char* string);
bool onButton(double x, double y, double LeftUpx, double LeftUpy, double RightDownx, double RightDowny);
int initializeGame();
class fireflower {
public:
    int cnt;
    double particle[12][2];
    double velocity[12][2];
    double diffuse;
    int sound = 0;
    
    fireflower();
    void initialize();
    void tick();
    void draw();
};
class field {
public:
    int state[3][3];    // 0:None, 1:Black, -1:White
    int stone1, stone2;
    int stone1_t, stone2_t;
    
    field();
    void initialize();
    int victory();  // 0:on-game, 1:Black, -1:White, 10: Draw, 100:Error
    int filled();
    int update(int i, int j, int side);
    void draw(double baseX, double baseY, double width);
    field operator = (const field& src);
};

random_device rnd;
mt19937 mt(rnd());
uniform_real_distribution<> unif(0.0, 1.0);
normal_distribution<> gauss(0.0, 1.0);
int winw = 900; //1200;
int winh = 600; //800;
double text1_x = -1.0; double text1_y = -0.5;
double text2_x = 0.42; double text2_y = -0.5;
int text1 = 0;      // 0: White, 1: Red
int text2 = 0;
int btnFlg = 0;
double t = 0.0; double t0 = 0.0;
double s = 0.0; double s0 = 0.0;    // solid angles of camera
double r = 2.5; // radius of camera
double px, py;  // initial position of rubberband
long start = clock();
int gameFlg = 0;            // -3,..,-1: Demo, 0: Menu, 1: Game, 2: Result
field mother;
field child[3][3];
int cnt = 0;                            // number of turns
int Teban = 1;
int vict = 0;
int taijin = 0;                            // 0: vsHuman
int nextField = -1;                        // -1: anywhere
int keyboardFlg = 0;                    // 0: mouse, 1: keyboard
int selectMode = 0;                     // 0: lonely, 1: with human
string mode = "?";
int keyWait = 0;
fireflower tama[3];
//unsigned int StringColor = White;

/*--Main func-------------------------------------------------------------------------*/
int main(int argc, char * argv[]) {
    cout << "Type ctrl+C to halt the program." << endl;
    srand((unsigned)time(NULL));
//    tama[0].sound = 1;
    
    /*--Main loop-------*/
    glutInit(&argc, argv);
    glutInitWindowSize(winw, winh);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("Maxence for Mac 0.0.0");
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(ps_motion);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(sp_key);
    //    glutIdleFunc(idle);
    glutTimerFunc(1000 , timer , 0);
    setup();
    glutMainLoop();
    
    return 0;
}

/*--For OpenGL-------------------------------------------------------------------------*/
void idle(void){
    glutPostRedisplay();
}
void setup(void){
    //    glClearColor(1.0, 0.99, 0.91, 1.0);       //Yellow
    //    glClearColor(0.0, 0.0, 0.1, 1.0);       //Black
    glClearColor(0.0, 0.5, 0.5, 1.0);       //Green
}
void resize(int width, int height){
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,
                   (double)width/height,
                   1.0,
                   100.0);
    gluLookAt(0.0, 0.0, 2.5,       //Position of Camera
              0.0, 0.0, 0.0,        //Position of Object
              0.0, 1.0, 0.0);       //Upward direction of Camera
    glMatrixMode(GL_MODELVIEW);
}
void timer(int value){
    glutPostRedisplay();
    glutTimerFunc(1000 / FPS , timer , 0);
    if(keyWait > 0) keyWait--;
}
void mouse(int button, int state, int cx, int cy){
//    if(button == GLUT_LEFT_BUTTON){
//        if(state == GLUT_DOWN){
//            btnFlg = 1;
//            px = cx; py = cy;
//        }else if(state == GLUT_UP){
//            btnFlg = 0;
//            t0 = t;
//            s0 = s;
//        }
//    }
    double px = 4.0 * (cx - (double)winw / 2.0) / (winh * 0.97);
    double py = -4.0 * (cy - (double)winh / 2.0) / (winh * 0.97);
    if(gameFlg == 0 && taijin == 0 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if(onButton(px, py, text1_x + 0.1, text1_y + 0.2, text1_x + 0.6, text1_y - 0.0)){
            mode = "LONELY";
            //InitializeGame();
            //keyWait = 10;
        }else if(onButton(px, py, text2_x - 0.15, text2_y + 0.2, text2_x + 0.65, text2_y - 0.0)){
            mode = "HUMAN";
            //PlayMovie("movie/battle.ogv", 1, DX_MOVIEPLAYTYPE_NORMAL);
            //            PlayMovieToGraph(MovieGraphHandle);
            //            SetBackgroundColor(0, 0, 0);
            //            while (!ScreenFlip() && !ProcessMessage() && !ClearDrawScreen()
            //                   && GetMovieStateToGraph(MovieGraphHandle)){
            //                UpdateKey(Key);
            //                if (Key[KEY_INPUT_W] == 1) {
            //                    PauseMovieToGraph(MovieGraphHandle);
            //                    break;
            //                }
            //                DrawExtendGraph(0, 60, 640, 420, MovieGraphHandle, FALSE);
            //                WaitTimer(10);
            //            }
            //            SetBackgroundColor(0, 128, 128);
            initializeGame();
            keyWait = 20;
        }
        cout << mode << endl;
    }
}
void motion(int cx, int cy){
    if(btnFlg){
        t = t0 - 0.005 * (cx - px);
        s = max(min(s0 + 0.005 * (cy - py), M_PI/2), -M_PI/2);
        glutPostRedisplay();
    }
}
void ps_motion(int cx, int cy){
    double px = 4.0 * (cx - (double)winw / 2.0) / (winh * 0.97);
    double py = -4.0 * (cy - (double)winh / 2.0) / (winh * 0.97);
    if(gameFlg == 0 && taijin == 0){
        if(onButton(px, py, text1_x + 0.1, text1_y + 0.2, text1_x + 0.6, text1_y - 0.0)) {
            text1 = 1; text2 = 0;
        }else if(onButton(px, py, text2_x - 0.15, text2_y + 0.2, text2_x + 0.65, text2_y - 0.0)) {
            text2 = 1; text1 = 0;
        }else{
            text1 = 0; text2 = 0;
            mode = "?";
        }
    }
    keyboardFlg = 0;
//    cout << px << ", " << py << endl;
}
void keyboard(unsigned char key, int cx , int cy){
    if(gameFlg == 0 && taijin == 0){
        if (keyboardFlg && (int)key == 13) {
            if(selectMode == 0){
                mode = "LONELY";
                //InitializeGame();
                //keyWait = 10;
            }else if(selectMode == 1){
                mode = "HUMAN";
                //PlayMovie("movie/battle.ogv", 1, DX_MOVIEPLAYTYPE_NORMAL);
                //            PlayMovieToGraph(MovieGraphHandle);
                //            SetBackgroundColor(0, 0, 0);
                //            while (!ScreenFlip() && !ProcessMessage() && !ClearDrawScreen()
                //                   && GetMovieStateToGraph(MovieGraphHandle)){
                //                UpdateKey(Key);
                //                if (Key[KEY_INPUT_W] == 1) {
                //                    PauseMovieToGraph(MovieGraphHandle);
                //                    break;
                //                }
                //                DrawExtendGraph(0, 60, 640, 420, MovieGraphHandle, FALSE);
                //                WaitTimer(10);
                //            }
                //            SetBackgroundColor(0, 128, 128);
                initializeGame();
                keyWait = 20;
            }
            cout << mode << endl;
        }
    }
    keyboardFlg = 1;
}
void sp_key(int key, int x, int y){
    if(gameFlg == 0 && taijin == 0){
        if (!keyWait){
            if(key == GLUT_KEY_LEFT) {
                selectMode = ((selectMode - 1) + 2) % 2;
                keyWait = 8;
            }else if(key == GLUT_KEY_RIGHT) {
                selectMode = ((selectMode - 1) + 2) % 2;
                keyWait = 8;
            }
        }
        if(selectMode == 0) { text1 = 1; text2 = 0; }
        else if(selectMode == 1) { text2 = 1; text1 = 0; }
    }
    keyboardFlg = 1;
}

/*--Display func-------------------------------------------------------------------------*/
void display(void){
//    start = clock();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(r*cos(s)*sin(t), r*sin(s), r*cos(s)*cos(t),
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
    
    //x-axis
    glColor3d(1.0, 0.0, 0.0);    //Red
    glBegin(GL_LINES);
    glVertex3d(-2.0, 0.0, 0.0);
    glVertex3d(2.0, 0.0, 0.0);
    glEnd();
    //y-axis
    glColor3d(0.0, 1.0, 0.0);    //Green
    glBegin(GL_LINES);
    glVertex3d(0.0, -2.0, 0.0);
    glVertex3d(0.0, 2.0, 0.0);
    glEnd();
    //z-axis
    glColor3d(0.0, 0.0, 1.0);    //Blue
    glBegin(GL_LINES);
    glVertex3d(0.0, 0.0, -2.0);
    glVertex3d(0.0, 0.0, 2.0);
    glEnd();
    
    if(gameFlg == 0){
        for (int i = 0; i < 3; ++i) {
            tama[i].draw();
            tama[i].tick();
        }
//        DrawExtendGraph(160 + (rand() % 11) - 5.0, 170, 490 + (rand() % 11) - 5.0, 260, Logo4, TRUE);
        if (taijin == 0) {
            glColor3d(1.0, 1.0, 1.0);    //White
            if(text1 == 0) render_string(text1_x, text1_y, "Lonely");
            if(text2 == 0) render_string(text2_x, text2_y, "With human");
            glColor3d(1.0, 0.0, 0.0);    //Red
            if(text1 == 1) render_string(text1_x, text1_y, "Lonely");
            if(text2 == 1) render_string(text2_x, text2_y, "With human");
        }
    }
    
//    while (clock() - start < 1000.0/60.0) {
//        sleep(0.001);
//    }
    glutSwapBuffers();
}

/*--Other func-------------------------------------------------------------------------*/
void drawSphere(double x, double y, double z, double r, int div) {
    glBegin(GL_POLYGON);
    for(int i = 0; i < div; i++){
        for(int j = 0; j < div * 2; j++){
            glVertex3d(x + r*cos(M_PI*i/div - 0.5*M_PI)*sin(M_PI*j/div),
                       y + r*cos(M_PI*i/div - 0.5*M_PI)*cos(M_PI*j/div),
                       z + r*sin(M_PI*i/div - 0.5*M_PI));
            glVertex3d(x + r*cos(M_PI*(i+1)/div - 0.5*M_PI)*sin(M_PI*j/div),
                       y + r*cos(M_PI*(i+1)/div - 0.5*M_PI)*cos(M_PI*j/div),
                       z + r*sin(M_PI*(i+1)/div - 0.5*M_PI));
            glVertex3d(x + r*cos(M_PI*(i+1)/div - 0.5*M_PI)*sin(M_PI*(j+1)/div),
                       y + r*cos(M_PI*(i+1)/div - 0.5*M_PI)*cos(M_PI*(j+1)/div),
                       z + r*sin(M_PI*(i+1)/div - 0.5*M_PI));
            glVertex3d(x + r*cos(M_PI*i/div - 0.5*M_PI)*sin(M_PI*(j+1)/div),
                       y + r*cos(M_PI*i/div - 0.5*M_PI)*cos(M_PI*(j+1)/div),
                       z + r*sin(M_PI*i/div - 0.5*M_PI));
        }
    }
    glEnd();
}
void drawCircle(double x0, double y0, double r, int div) {
    double x, y;
    glBegin(GL_POLYGON);
    for (int i=0; i<div; i++) {
        x = x0 + r * cos(2.0 * M_PI * ((double)i/div));
        y = y0 + r * sin(2.0 * M_PI * ((double)i/div));
        glVertex2d(x, y);
    }
    glEnd();
}
void render_string(float x, float y, const char* string) {
    float z = -1.0f;
    glRasterPos3f(x, y, z);
    char* p = (char*) string;
    while (*p != '\0') glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p++);
}
bool onButton(double x, double y, double LeftUpx, double LeftUpy, double RightDownx, double RightDowny) {
    return x > LeftUpx && x < RightDownx && y < LeftUpy && y > RightDowny;
}
int initializeGame() {
//    gameFlg = 1;
    nextField = -1;
    cnt = 0;
    // initializing camera
    t = 0; s = 0;
    // initializing field
    mother.initialize();
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            child[i][j].initialize();
        }
    }
//    InitializeHist(last, seclast);
    return 0;
}

/*--Class methods-------------------------------------------------------------------------*/
fireflower::fireflower() {
    initialize();
}
void fireflower::initialize() {
    cnt = rand() % 40;
    diffuse = 0.02 * unif(mt) + 0.004;
    double inix = 3.0 * unif(mt) - 1.5;
    double iniy = 1.0 * unif(mt) - 1.8;
    for (int i = 0; i < 12; ++i) {
        particle[i][0] = inix;
        particle[i][1] = iniy;
        velocity[i][0] = diffuse * cos(2.0 * M_PI * i / 12.0);
        velocity[i][1] = diffuse * sin(2.0 * M_PI * i / 12.0);
    }
}
void fireflower::tick() {
    cnt++;
    if (cnt <= 120) {
        for (int i = 0; i < 12; ++i) {
            particle[i][0] += velocity[i][0] * 0.1 * (unif(mt) - 0.5);
            particle[i][1] += 0.015 * (2.0 - particle[i][1]) * (1.0 + 0.2 * (unif(mt) - 0.5));
        }
    }
    else if (cnt <= 240) {
        for (int i = 0; i < 12; ++i) {
            particle[i][0] += velocity[i][0] + 0.01 * (unif(mt) - 0.5);
            particle[i][1] += velocity[i][1] - 0.01 * (unif(mt) - 0.3) ;
            velocity[i][0] = velocity[i][0] * 0.99;
            velocity[i][1] = velocity[i][1] * 0.99;
        }
    }
    else {
        initialize();
    }
//        if(sound == 1 && cnt == 80) PlaySound("sound/owin31.wav", NULL, SND_ASYNC);
}
void fireflower::draw() {
    double norm;
    for (int i = 0; i < 12; ++i) {
        norm = sqrt(pow(velocity[i][0], 2.0) + pow(velocity[i][1], 2.0));
        glColor3d(cos(100.0*norm), sin(100.0*norm), unif(mt));
        drawCircle(particle[i][0],
                   particle[i][1],
                   1.0 * sqrt(norm), 20);
    }
}

field::field() {
    initialize();
}
void field::initialize() {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            state[i][j] = 0;
        }
    }
}
int field::victory() {
    int vict = 0;
    int temp = 0;
    
    for (int k = 0; k < 3; ++k) {
        temp = state[0][k];
        if (temp == state[1][k] && temp == state[2][k]) {
            //if(temp != 0) cout << "â°" << k << endl;
            if (vict == 0) {
                vict = temp;
            }
            else if(vict != temp && temp != 0){
                vict = 100;
                break;
            }
        }
        temp = state[k][0];
        if (temp == state[k][1] && temp == state[k][2]) {
            //if(temp != 0) cout << "èc" << k << endl;
            if (vict == 0) {
                vict = temp;
            }
            else if (vict != temp && temp != 0) {
                vict = 100;
                break;
            }
        }
    }
    temp = state[0][0];
    if (temp == state[1][1] && temp == state[2][2]) {
        //if (temp != 0) cout << "ç∂éŒÇﬂ" << endl;
        if (vict == 0) {
            vict = temp;
        }
        else if (vict != temp && temp != 0) {
            vict = 100;
        }
    }
    temp = state[2][0];
    if (temp == state[1][1] && temp == state[0][2]) {
        //if (temp != 0) cout << "âEéŒÇﬂ" << endl;
        if (vict == 0) {
            vict = temp;
        }
        else if (vict != temp && temp != 0) {
            vict = 100;
        }
    }
    if (filled() == 1 && vict == 0) vict = 10;
    return vict;
} // 0:on-game, 1:Black, -1:White, 10: Draw, 100:Error
int field::filled() {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (state[i][j] == 0) return 0;
        }
    }
    return 1;
}
int field::update(int i, int j, int side) {
    if (victory() != 0) return -1;
    if (state[i][j] == 0) {
        state[i][j] = side;
        return 0;
    }
    return -1;
}
void field::draw(double baseX, double baseY, double width) {
    for (int k = 0; k < 3; ++k) {
        for (int l = 0; l < 3; ++l) {
            if (state[k][l] == 1) {
//                    DrawExtendGraph(baseX + width * k - 15, baseY + width * l - 15, baseX + width * k + 15, baseY + width * l + 15, stone1, TRUE);
                //DrawCircleAA(baseX + width * k, baseY + width * l, 13, 12, GetColor(50, 50, 50), TRUE);
            }
            else if (state[k][l] == -1) {
//                    DrawExtendGraph(baseX + width * k - 15, baseY + width * l - 15, baseX + width * k + 15, baseY + width * l + 15, stone2, TRUE);
                //DrawCircleAA(baseX + width * k, baseY + width * l, 13, 12, GetColor(255, 200, 100), TRUE);
            }
        }
    }
}
field field::operator = (const field& src) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            state[i][j] = src.state[i][j];
        }
    }
    return src;
}


