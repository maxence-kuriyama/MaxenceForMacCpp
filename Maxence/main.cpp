#define PRINT_MAT(X) cout << #X << ":\n" << X << endl << endl
#define PRINT_MAT2(X,DESC) cout << DESC << ":\n" << X << endl << endl
#define PRINT_FNC    cout << "[" << __func__ << "]" << endl
#define FPS     60

#include <iostream>
#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>
#include <random>
#include <time.h>
#include <unistd.h>     // for sleep and getcwd
#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
//#include <Eigen/Core>
#include "jfont.h"
using namespace std;
namespace fs = boost::filesystem;
//using namespace Eigen;
using namespace cv;

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
void init(fs::path cur_path);
void drawSphere(double x, double y, double z, double r, int div = 5);
void drawCircle(double x0, double y0, double r, int div = 20);
void drawBox(double ld_x, double ld_y, double ru_x, double ru_y, bool filled = false);
void render_string(float x, float y, const char* string);
bool onButton(double x, double y, double ld_x, double ld_y, double ru_x, double ru_y);
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
    cv::Mat stone1, stone2;
    cv::Mat stone1_t, stone2_t;
    
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
double text1_x = -1.0; double text1_y = -0.75;
double text2_x = 0.42; double text2_y = -0.75;
double text3_x = -0.4; double text3_y = -0.5;
double text4_x = -0.7; double text4_y = 0.0;
double text5_x = 0.2; double text5_y = 0.0;
double field_x = -1.35; double field_y = -1.35;
double unit_w = 0.3; double unit_h = 0.3;
int text1 = 0;      // 0: White, 1: Red
int text2 = 0;
int text3 = 0;
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
int keyboardFlg = 0;                    // 0: mouse, 1: keyboard
int selectMode = 0;                     // 0: lonely, 1: with human
string mode = "?";
int keyWait = 0;
int drawFlgCnt = 0;                     // for avoiding infinite loop
int last[5];
int seclast[5];
int cancelCnt = 0;
int nextField = -1;                        // -1: anywhere
int corGx = 1;
int corGy = 1;
int corLx = 1;
int corLy = 1;
fireflower tama[3];
cv::Mat logo4;
//unsigned int StringColor = White;

/*--Main func-------------------------------------------------------------------------*/
int main(int argc, char * argv[]) {
    cout << "Type ctrl+C to halt." << endl;
    srand((unsigned)time(NULL));
//    tama[0].sound = 1;
    
    fs::path cur_path(fs::initial_path<fs::path>());
    cur_path = fs::system_complete(fs::path(argv[0]));
    cout << "Current Directory : " << cur_path.parent_path() << endl;
    init(cur_path);
    
    /*--Main loop-------*/
    glutInit(&argc, argv);
    glutInitWindowSize(winw, winh);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("Maxence for Mac 0.1.1");
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
    
    //Judge victory
    vict = mother.victory();
    if (vict != 0) {
        gameFlg = 2;
        if(gameFlg != 2) keyWait = 20;
    }
    //For infinite loop
    if (drawFlgCnt > 10000) gameFlg = 2;
    if (taijin == 2 || taijin == 3 || taijin == 4) drawFlgCnt++;
}
void motion(int cx, int cy){
    if(btnFlg){
        t = t0 - 0.005 * (cx - px);
        s = max(min(s0 + 0.005 * (cy - py), M_PI/2), -M_PI/2);
        glutPostRedisplay();
    }
}
void mouse(int button, int state, int cx, int cy){
    if(button == GLUT_LEFT_BUTTON){
        if(state == GLUT_DOWN){
            btnFlg = 1;
            px = cx; py = cy;
        }else if(state == GLUT_UP){
            btnFlg = 0;
            t0 = t;
            s0 = s;
        }
    }
    double px = 4.0 * (cx - (double)winw / 2.0) / (winh * 0.97);
    double py = -4.0 * (cy - (double)winh / 2.0) / (winh * 0.97);
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if(gameFlg == 0 && taijin == 0) {
            if(onButton(px, py, text1_x + 0.1, text1_y - 0.0, text1_x + 0.6, text1_y + 0.2)){
                mode = "LONELY";
                //InitializeGame();
                //keyWait = 10;
            }else if(onButton(px, py, text2_x - 0.15, text2_y - 0.0, text2_x + 0.65, text2_y + 0.2)){
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
        else if(gameFlg == 1){
            for(int i = 0; i < 3; ++i){
                for(int j = 0; j < 3; ++j){
                    for (int k = 0; k < 3; ++k) {
                        for (int l = 0; l < 3; ++l) {
                            if (!keyboardFlg &&
                                onButton(px, py,
                                         field_x + unit_w * (3.0 * i + k),
                                         field_y + unit_h * (3.0 * j + l),
                                         field_x + unit_w * (3.0 * i + (k + 1)),
                                         field_y + unit_h * (3.0 * j + (l + 1)))){
                                //Updation of local
                                if (nextField == 3 * i + j || nextField == -1) {
                                    if (child[i][j].update(k, l, 1 - 2 * (cnt % 2)) == 0) {
                                        cnt++;
                                        cout << "count: " << cnt << endl;
                                        //Update history
                                        for (int i1 = 0; i1 < 5; i1++) {
                                            seclast[i1] = last[i1];
                                        }
                                        last[0] = i; last[1] = j;
                                        last[2] = k; last[3] = l;
                                        last[4] = nextField;
                                        if (cancelCnt < 2) cancelCnt++;
                                        //Updation of global
                                        mother.update(i, j, child[i][j].victory());
                                        if (child[k][l].victory() != 0) { nextField = -1; }
                                        else { nextField = k * 3 + l; }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if(gameFlg == 2){
            if(onButton(px, py, text3_x + 0.05, text3_y - 0.0, text3_x + 0.75, text3_y + 0.2)){
                initializeGame();
                keyWait = 20;
            }
        }
    }
}
void ps_motion(int cx, int cy){
    double px = 4.0 * (cx - (double)winw / 2.0) / (winh * 0.97);
    double py = -4.0 * (cy - (double)winh / 2.0) / (winh * 0.97);
    if(gameFlg == 0 && taijin == 0){
        if(onButton(px, py, text1_x + 0.1, text1_y - 0.0, text1_x + 0.6, text1_y + 0.2)) {
            text1 = 1; text2 = 0;
        }else if(onButton(px, py, text2_x - 0.15, text2_y - 0.0, text2_x + 0.65, text2_y + 0.2)) {
            text2 = 1; text1 = 0;
        }else{
            text1 = 0; text2 = 0;
            mode = "?";
        }
    }
    else if(gameFlg == 1){
        for (int i = 0; i < 3; ++i){
            for (int j = 0; j < 3; ++j){
                for (int k = 0; k < 3; ++k) {
                    for (int l = 0; l < 3; ++l) {
                        if (!keyboardFlg &&
                            onButton(px, py,
                                     field_x + unit_w * (3.0 * i + k),
                                     field_y + unit_h * (3.0 * j + l),
                                     field_x + unit_w * (3.0 * i + (k + 1)),
                                     field_y + unit_h * (3.0 * j + (l + 1)))){
                            corGx = i; corGy = j; corLx = k; corLy = l;
//                            cout << "global: (" << i << "," << j << "), local: (" << k << "," << l << ")" << endl;
                        }
                    }
                }
            }
        }
    }
    else if(gameFlg == 2){
        if(onButton(px, py, text3_x + 0.05, text3_y - 0.0, text3_x + 0.75, text3_y + 0.2)) {
            text3 = 1;
        }else{
            text3 = 0;
        }
    }
    keyboardFlg = 0;
//    cout << px << ", " << py << endl;
}
void keyboard(unsigned char key, int cx, int cy){
    keyboardFlg = 1;    // Always enable keyboard
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
    else if(gameFlg == 1){
        if(!keyWait && keyboardFlg){
            switch(key){
                case 'w':
                    corLy++;
                    if (corLy > 2) {
                        if (corGy < 2) {
                            corGy++; corLy -= 3;
                        }
                        else {
                            corLy = 2;
                        }
                    }
                    keyWait = 6;
                    break;
                case 's':
                    corLy--;
                    if (corLy < 0) {
                        if (corGy > 0) {
                            corGy--; corLy += 3;
                        }
                        else {
                            corLy = 0;
                        }
                    }
                    keyWait = 6;
                    break;
                case 'd':
                    corLx++;
                    if (corLx > 2) {
                        if (corGx < 2) {
                            corGx++; corLx -= 3;
                        }
                        else {
                            corLx = 2;
                        }
                    }
                    keyWait = 6;
                    break;
                case 'a':
                    corLx--;
                    if (corLx < 0) {
                        if (corGx > 0) {
                            corGx--; corLx += 3;
                        }
                        else {
                            corLx = 0;
                        }
                    }
                    keyWait = 6;
                    break;
                case 'f':
                case (char)13:  // return key
                    //Updation of local
                    if (nextField == 3 * corGx + corGy || nextField == -1) {
                        if (child[corGx][corGy].update(corLx, corLy, 1 - 2 * (cnt % 2)) == 0) {
                            cnt++;
                            cout << "count: " << cnt << endl;
                            //Update history
                            for (int i = 0; i < 5; i++) {
                                seclast[i] = last[i];
                            }
                            last[0] = corGx; last[1] = corGy;
                            last[2] = corLx; last[3] = corLy;
                            last[4] = nextField;
                            if (cancelCnt < 2) cancelCnt++;
                            //Updation of global
                            mother.update(corGx, corGy, child[corGx][corGy].victory());
                            if (child[corLx][corLy].victory() != 0) { nextField = -1; }
                            else { nextField = corLx * 3 + corLy; }
                        }
                    }
                    break;
                case 'z':
                case (char)127:     //backspace key
                    if (cancelCnt > 0) {
                        child[last[0]][last[1]].state[last[2]][last[3]] = 0;
                        mother.state[last[0]][last[1]] = 0;
                        mother.update(last[0], last[1], child[last[0]][last[1]].victory());
                        nextField = last[4];
                        for (int i = 0; i < 5; i++) {
                            last[i] = seclast[i];
                            seclast[i] = 0;
                        }
                        cnt--;
                        cancelCnt--;
                    }
                    break;
//                default:
//                    cout << (int)key << endl;
//                    break;
            }
        }
    }
    else if(gameFlg == 2){
        if(!keyWait && keyboardFlg){
            switch(key){
                case 'z':
                case (char)127:     //backspace key
                    if (cancelCnt > 0) {
                        child[last[0]][last[1]].state[last[2]][last[3]] = 0;
                        mother.state[last[0]][last[1]] = 0;
                        mother.update(last[0], last[1], child[last[0]][last[1]].victory());
                        nextField = last[4];
                        for (int i = 0; i < 5; i++) {
                            last[i] = seclast[i];
                            seclast[i] = 0;
                        }
                        cnt--;
                        cancelCnt--;
                        gameFlg = 1;
                    }
                    break;
                case 'f':
                case (char)13:
                    initializeGame();
                    keyWait = 20;
                    break;
            }
        }
    }
//    keyboardFlg = 1;  // Switch keyboard
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
    }else if(gameFlg == 1){
        if(!keyWait && keyboardFlg){
            switch(key){
                case GLUT_KEY_UP:
                    corLy++;
                    if (corLy > 2) {
                        if (corGy < 2) {
                            corGy++; corLy -= 3;
                        }
                        else {
                            corLy = 2;
                        }
                    }
                    keyWait = 6;
                    break;
                case GLUT_KEY_DOWN:
                    corLy--;
                    if (corLy < 0) {
                        if (corGy > 0) {
                            corGy--; corLy += 3;
                        }
                        else {
                            corLy = 0;
                        }
                    }
                    keyWait = 6;
                    break;
                case GLUT_KEY_RIGHT:
                    corLx++;
                    if (corLx > 2) {
                        if (corGx < 2) {
                            corGx++; corLx -= 3;
                        }
                        else {
                            corLx = 2;
                        }
                    }
                    keyWait = 6;
                    break;
                case GLUT_KEY_LEFT:
                    corLx--;
                    if (corLx < 0) {
                        if (corGx > 0) {
                            corGx--; corLx += 3;
                        }
                        else {
                            corLx = 0;
                        }
                    }
                    keyWait = 6;
                    break;
            }
        }
    }
    keyboardFlg = 1;
}

/*--Display func-------------------------------------------------------------------------*/
void display(void){
//    start = clock();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
//    gluLookAt(r*cos(s)*sin(t), r*sin(s), r*cos(s)*cos(t),
    gluLookAt(0.0, 0.0, r,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0);
    
    if(gameFlg == 0){
        for (int i = 0; i < 3; ++i) {
            tama[i].draw();
            tama[i].tick();
        }
        glRasterPos3f(-1.75, 0.0, 0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawPixels(logo4.cols, logo4.rows, GL_RGBA, GL_UNSIGNED_BYTE, logo4.data);
        glDisable(GL_BLEND);
        if (taijin == 0) {
            glColor3d(1.0, 1.0, 1.0);    //White
            if(text1 == 0) render_jstring(text1_x, text1_y, (unsigned char *)"ぼっちで");
            if(text2 == 0) render_jstring(text2_x, text2_y, (unsigned char *)"隣の人と");
            glColor3d(1.0, 0.0, 0.0);    //Red
            if(text1 == 1) render_jstring(text1_x, text1_y, (unsigned char *)"ぼっちで");
            if(text2 == 1) render_jstring(text2_x, text2_y, (unsigned char *)"隣の人と");
        }
    }
    else if (gameFlg == 1){
        // Draw field
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (mother.state[i][j] != 0) {
                    if (mother.state[i][j] == 1) {
                        glColor3d(0.5, 0.26, 0.26);
                    }else if (mother.state[i][j] == -1) {
                        glColor3d(0.26, 0.26, 0.5);
                    }else {
                        glColor3d(0.26, 0.5, 0.26);
                    }
                    drawBox(field_x + 3.0 * unit_w * i,         field_y + 3.0 * unit_h * j,
                            field_x + 3.0 * unit_w * (i + 1),   field_y + 3.0 * unit_h * (j + 1), true);
                }
                glColor3d(0.0, 0.0, 0.0);   //Black
                drawBox(field_x + 3.0 * unit_w * i,         field_y + 3.0 * unit_h * j,
                        field_x + 3.0 * unit_w * (i + 1),   field_y + 3.0 * unit_h * (j + 1));
            }
        }
        // Next small field
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (nextField == 3 * i + j) {
                    glColor3d(1.0, 0.0, 0.0);   //Red
                    drawBox(field_x + 3.0 * unit_w * i,         field_y + 3.0 * unit_h * j,
                            field_x + 3.0 * unit_w * (i + 1),   field_y + 3.0 * unit_h * (j + 1));
                    drawBox(field_x + 3.0 * unit_w * i + 0.005,         field_y + 3.0 * unit_h * j + 0.005,
                            field_x + 3.0 * unit_w * (i + 1) - 0.005,   field_y + 3.0 * unit_h * (j + 1) - 0.005);
                }else if (nextField == -1 && child[i][j].victory() == 0) {
                    glColor3d(1.0, 0.0, 0.0);   //Red
                    drawBox(field_x + 3.0 * unit_w * i,         field_y + 3.0 * unit_h * j,
                            field_x + 3.0 * unit_w * (i + 1),   field_y + 3.0 * unit_h * (j + 1));
                }
                child[i][j].draw(field_x + 3.0 * unit_w * i, field_y + 3.0 * unit_h * j, unit_w);
            }
        }
        // Current position
        glColor3d(0.0, 0.0, 0.0);   //Black
        drawBox(field_x + unit_w * (3 * corGx + corLx),
                field_y + unit_h * (3 * corGy + corLy),
                field_x + unit_w * (3 * corGx + (corLx + 1)),
                field_y + unit_h * (3 * corGy + (corLy + 1)));
        // Previous position
        if (cancelCnt > 0) {
            glColor3d(1.0, 0.8, 0.8);   //Thin pink
            drawBox(field_x + unit_w * (3 * last[0] + last[2]),
                    field_y + unit_h * (3 * last[1] + last[3]),
                    field_x + unit_w * (3 * last[0] + (last[2] + 1)),
                    field_y + unit_h * (3 * last[1] + (last[3] + 1)));
        }
//            DrawExtendGraph(titleX, titleY, titleX + 190, titleY + 60, Logo4, TRUE);
//            titleX += AcRate * (rand() % 11 - 5.0); if (titleX <= -10) titleX = -10;
//            if (titleX >= 640 - 160) titleX = 640 - 160;
//            titleY += AcRate * (rand() % 11 - 5.0); if (titleY <= -10) titleY = -10;
//            if (titleY >= 480 - 80) titleY = 480 - 80;
    }
    else if (gameFlg == 2){
        // Draw field
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (mother.state[i][j] != 0) {
                    if (mother.state[i][j] == 1) {
                        glColor3d(0.5, 0.26, 0.26);
                    }else if (mother.state[i][j] == -1) {
                        glColor3d(0.26, 0.26, 0.5);
                    }else {
                        glColor3d(0.26, 0.5, 0.26);
                    }
                    drawBox(field_x + 3.0 * unit_w * i,         field_y + 3.0 * unit_h * j,
                            field_x + 3.0 * unit_w * (i + 1),   field_y + 3.0 * unit_h * (j + 1), true);
                }
                glColor3d(0.0, 0.0, 0.0);   //Black
                drawBox(field_x + 3.0 * unit_w * i,         field_y + 3.0 * unit_h * j,
                        field_x + 3.0 * unit_w * (i + 1),   field_y + 3.0 * unit_h * (j + 1));
            }
        }
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                child[i][j].draw(field_x + 3.0 * unit_w * i, field_y + 3.0 * unit_h * j, unit_w);
            }
        }
        // Previous position
        if (cancelCnt > 0) {
            glColor3d(1.0, 0.8, 0.8);   //Thin pink
            drawBox(field_x + unit_w * (3 * last[0] + last[2]),
                    field_y + unit_h * (3 * last[1] + last[3]),
                    field_x + unit_w * (3 * last[0] + (last[2] + 1)),
                    field_y + unit_h * (3 * last[1] + (last[3] + 1)));
        }
        glColor3d(1.0, 1.0, 1.0);    //White
        if(vict == 1) { render_string(text4_x, text4_y, "Player 1"); }
        else if(vict == -1) { render_string(text4_x, text4_y, "Player 2"); }
        else { render_string(text4_x, text4_y, "No one"); }
        render_string(text5_x, text5_y, "Won!!");
        if(text3 == 1) glColor3d(1.0, 0.0, 0.0);    //Red
        render_string(text3_x, text3_y, "Once again");
    }
    
//    while (clock() - start < 1000.0/60.0) {
//        sleep(0.001);
//    }
    glutSwapBuffers();
}

/*--Other func-------------------------------------------------------------------------*/
void init(fs::path cur_path){
    // パスの取得
    string str_cur_path = cur_path.parent_path().string();
//    cout << str_cur_path << endl;
    // ロゴ画像の作成
    logo4 = cv::imread(str_cur_path + "/graph/Maxence_after4.png", cv::IMREAD_UNCHANGED);
    cv::flip(logo4, logo4, 0);
    cv::cvtColor(logo4, logo4, COLOR_BGRA2RGBA);
    resize(logo4, logo4, cv::Size(), 1.0, 0.75);
    // 置石画像の作成
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            child[i][j].stone1 = cv::imread(str_cur_path + "/graph/stone1.png", cv::IMREAD_UNCHANGED);
            cv::flip(child[i][j].stone1, child[i][j].stone1, 0);
            cv::cvtColor(child[i][j].stone1, child[i][j].stone1, COLOR_BGRA2RGBA);
            resize(child[i][j].stone1, child[i][j].stone1,
                   cv::Size(), 45.0/child[i][j].stone1.cols, 45.0/child[i][j].stone1.rows);
            child[i][j].stone2 = cv::imread(str_cur_path + "/graph/stone2.png", cv::IMREAD_UNCHANGED);
            cv::flip(child[i][j].stone2, child[i][j].stone2, 0);
            cv::cvtColor(child[i][j].stone2, child[i][j].stone2, COLOR_BGRA2RGBA);
            resize(child[i][j].stone2, child[i][j].stone2,
                   cv::Size(), 45.0/child[i][j].stone2.cols, 45.0/child[i][j].stone2.rows);
        }
    }
}
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
void drawBox(double ld_x, double ld_y, double ru_x, double ru_y, bool filled) {
    if(!filled){
        glBegin(GL_LINE_LOOP);
        glVertex2d(ld_x, ld_y);
        glVertex2d(ld_x, ru_y);
        glVertex2d(ru_x, ru_y);
        glVertex2d(ru_x, ld_y);
        glEnd();
    }else{
        glBegin(GL_QUADS);
        glVertex2d(ld_x, ld_y);
        glVertex2d(ld_x, ru_y);
        glVertex2d(ru_x, ru_y);
        glVertex2d(ru_x, ld_y);
        glEnd();
    }
}
void render_string(float x, float y, const char* string) {
    float z = -1.0f;
    glRasterPos3f(x, y, z);
    char* p = (char*) string;
    while (*p != '\0') glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p++);
//    while (*p != '\0') glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p++);
}
bool onButton(double x, double y, double ld_x, double ld_y, double ru_x, double ru_y) {
    if(ld_x >= ru_x) cout << "ERROR(onButton): null range of x." << endl;
    if(ld_y >= ru_y) cout << "ERROR(onButton): null range of y." << endl;
    return x > ld_x && x < ru_x && y > ld_y && y < ru_y;
}
int initializeGame() {
    gameFlg = 1;
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
    cancelCnt = 0;
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
void field::draw(double base_x, double base_y, double width) {
    for (int k = 0; k < 3; ++k) {
        for (int l = 0; l < 3; ++l) {
            if (state[k][l] == 1) {
                glRasterPos3f(base_x + width * k,
                              base_y + width * l,
                              0);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDrawPixels(stone1.cols, stone1.rows, GL_RGBA, GL_UNSIGNED_BYTE, stone1.data);
                glDisable(GL_BLEND);
            }
            else if (state[k][l] == -1) {
                glRasterPos3f(base_x + width * k,
                              base_y + width * l,
                              0);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDrawPixels(stone2.cols, stone2.rows, GL_RGBA, GL_UNSIGNED_BYTE, stone2.data);
                glDisable(GL_BLEND);
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


