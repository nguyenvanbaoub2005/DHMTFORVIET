#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

using namespace std;

const int WIN_WIDTH = 800;
const int WIN_HEIGHT = 600;

// Lưu các điểm của đa giác
vector<pair<double, double>> polygonPoints;

// Biến toàn cục cho bounding box của đa giác
double Xmin, Ymin, Xmax, Ymax;

const int LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8;

struct Pt { double x, y; };

void readPolygonFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Không thể mở file " << filename << endl;
        return;
    }

    int n;
    file >> n; // Đọc số lượng điểm
    polygonPoints.clear();
    
    double x, y;
    for (int i = 0; i < n; ++i) {
        file >> x >> y;
        polygonPoints.push_back({x, y});
        
        // Cập nhật min/max cho bounding box
        if (i == 0) {
            Xmin = Xmax = x;
            Ymin = Ymax = y;
        } else {
            Xmin = min(Xmin, x);
            Xmax = max(Xmax, x);
            Ymin = min(Ymin, y);
            Ymax = max(Ymax, y);
        }
    }
    
    file.close();
    cout << "Đọc được " << polygonPoints.size() << " điểm của đa giác từ file.\n";
}

bool pointInside(const Pt &p) {
    return (p.x >= Xmin && p.x <= Xmax && p.y >= Ymin && p.y <= Ymax);
}

int outCode(const Pt &p) {
    int c = 0;
    if (p.x < Xmin) c |= LEFT;
    else if (p.x > Xmax) c |= RIGHT;
    if (p.y < Ymin) c |= BOTTOM;
    else if (p.y > Ymax) c |= TOP;
    return c;
}

// Recursively search for t-intervals where the parametric segment is inside.
// foundT0 = smallest t found inside, foundT1 = largest t found inside
void binaryRec(double t0, double t1, const Pt &P, const Pt &Q,
               double &foundT0, double &foundT1, double eps)
{
    Pt A = { P.x + (Q.x - P.x) * t0, P.y + (Q.y - P.y) * t0 };
    Pt B = { P.x + (Q.x - P.x) * t1, P.y + (Q.y - P.y) * t1 };

    int codeA = outCode(A);
    int codeB = outCode(B);

    // If both endpoints of this param interval are outside in the same region -> no intersection here
    if ((codeA & codeB) != 0) return;

    // If both inside -> update found interval
    if (pointInside(A) && pointInside(B)) {
        if (t0 < foundT0) foundT0 = t0;
        if (t1 > foundT1) foundT1 = t1;
        return;
    }

    // If interval is small enough, test midpoint
    if (t1 - t0 < eps) {
        double tm = 0.5 * (t0 + t1);
        Pt M = { P.x + (Q.x - P.x) * tm, P.y + (Q.y - P.y) * tm };
        if (pointInside(M)) {
            if (t0 < foundT0) foundT0 = t0;
            if (t1 > foundT1) foundT1 = t1;
        }
        return;
    }

    // Otherwise split and recurse
    double tm = 0.5 * (t0 + t1);
    binaryRec(t0, tm, P, Q, foundT0, foundT1, eps);
    binaryRec(tm, t1, P, Q, foundT0, foundT1, eps);
}

// Top-level binary clip: returns true if there is a clipped segment, and fills rx0,ry0,rx1,ry1
bool binaryClip(const Pt &P, const Pt &Q, double &rx0, double &ry0, double &rx1, double &ry1) {
    double foundT0 = 1e9;
    double foundT1 = -1e9;
    const double epsT = 1e-4;

    binaryRec(0.0, 1.0, P, Q, foundT0, foundT1, epsT);

    if (foundT0 <= foundT1) {
        rx0 = P.x + (Q.x - P.x) * foundT0;
        ry0 = P.y + (Q.y - P.y) * foundT0;
        rx1 = P.x + (Q.x - P.x) * foundT1;
        ry1 = P.y + (Q.y - P.y) * foundT1;
        return true;
    }
    return false;
}

/* ---------- OpenGL app (mouse input and drawing) ---------- */

double x_start = 0, y_start = 0, x_end = 0, y_end = 0;
bool hasLine = false;
int clickCount = 0;
double clipped_x0, clipped_y0, clipped_x1, clipped_y1;
bool clipped_valid = false;

void drawClippingWindow() {
    glColor3f(0.0f, 1.0f, 0.0f); // Màu xanh lá cho đa giác
    glBegin(GL_LINE_LOOP);
    for (const auto& point : polygonPoints) {
        glVertex2d(point.first, point.second);
    }
    glEnd();
}

void drawLine(double x0, double y0, double x1, double y1, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_LINES);
        glVertex2d(x0, y0);
        glVertex2d(x1, y1);
    glEnd();
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        ypos = WIN_HEIGHT - ypos; // flip Y

        if (clickCount == 0) {
            x_start = xpos; y_start = ypos;
            clickCount = 1;
            hasLine = false;
            clipped_valid = false;
            cout << "Point A: (" << x_start << ", " << y_start << ")\n";
        } else {
            x_end = xpos; y_end = ypos;
            clickCount = 0;
            hasLine = true;

            Pt P = { x_start, y_start };
            Pt Q = { x_end, y_end };
            double rx0, ry0, rx1, ry1;
            bool ok = binaryClip(P, Q, rx0, ry0, rx1, ry1);

            cout << "Point B: (" << x_end << ", " << y_end << ")\n";
            if (ok) {
                cout << "Clipped segment: (" << rx0 << ", " << ry0 << ") -> (" << rx1 << ", " << ry1 << ")\n";
                clipped_x0 = rx0; clipped_y0 = ry0;
                clipped_x1 = rx1; clipped_y1 = ry1;
                clipped_valid = true;
            } else {
                cout << "Segment is fully outside clipping window.\n";
                clipped_valid = false;
            }
        }
    }
}

int main() {
    // Đọc đa giác từ file
    readPolygonFromFile("polygon.txt");

    if (!glfwInit()) {
        cerr << "glfwInit failed\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Clipping - Binary Division with Polygon", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        cerr << "glewInit failed\n";
        return -1;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_WIDTH, 0, WIN_HEIGHT);

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Vẽ đa giác clipping
        if (!polygonPoints.empty()) {
            drawClippingWindow();
        }

        // Vẽ đoạn thẳng gốc (xanh dương)
        if (hasLine) {
            drawLine(x_start, y_start, x_end, y_end, 0.0f, 0.0f, 1.0f);
        }

        // Vẽ đoạn thẳng đã được xén (đỏ)
        if (clipped_valid) {
            drawLine(clipped_x0, clipped_y0, clipped_x1, clipped_y1, 1.0f, 0.0f, 0.0f);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}