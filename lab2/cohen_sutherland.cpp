#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

const int WIN_WIDTH = 800, WIN_HEIGHT = 600;

// Lưu các điểm của đa giác
vector<pair<double, double>> polygonPoints;

// Outcode
const int INSIDE = 0; // 0000
const int LEFT   = 1; // 0001
const int RIGHT  = 2; // 0010
const int BOTTOM = 4; // 0100
const int TOP    = 8; // 1000

// Lưu 2 điểm khi click chuột
double x_start, y_start, x_end, y_end;
int clickCount = 0;
bool hasLine = false; // kiểm tra có đoạn thẳng nào chưa
bool clipped = false;
double cx0, cy0, cx1, cy1;

// Tính giá trị min/max của đa giác để dùng trong clipping
double xmin_clip, ymin_clip, xmax_clip, ymax_clip;

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
        
        // Cập nhật min/max để dùng trong clipping
        if (i == 0) {
            xmin_clip = xmax_clip = x;
            ymin_clip = ymax_clip = y;
        } else {
            xmin_clip = min(xmin_clip, x);
            xmax_clip = max(xmax_clip, x);
            ymin_clip = min(ymin_clip, y);
            ymax_clip = max(ymax_clip, y);
        }
    }
    
    file.close();
    cout << "Đọc được " << polygonPoints.size() << " điểm của đa giác từ file.\n";
}

int computeOutCode(double x, double y) {
    int code = INSIDE;
    if (x < xmin_clip) code |= LEFT;
    else if (x > xmax_clip) code |= RIGHT;
    if (y < ymin_clip) code |= BOTTOM;
    else if (y > ymax_clip) code |= TOP;
    return code;
}

bool cohenSutherlandClip(double &x0, double &y0, double &x1, double &y1) {
    int outcode0 = computeOutCode(x0, y0);
    int outcode1 = computeOutCode(x1, y1);
    bool accept = false;

    while (true) {
        if (!(outcode0 | outcode1)) { 
            accept = true; // cả 2 trong
            break;
        } else if (outcode0 & outcode1) { 
            break; // ngoài cùng phía
        } else {
            double x, y;
            int outcodeOut = outcode0 ? outcode0 : outcode1;

            if (outcodeOut & TOP) {
                x = x0 + (x1 - x0) * (ymax_clip - y0) / (y1 - y0);
                y = ymax_clip;
            } else if (outcodeOut & BOTTOM) {
                x = x0 + (x1 - x0) * (ymin_clip - y0) / (y1 - y0);
                y = ymin_clip;
            } else if (outcodeOut & RIGHT) {
                y = y0 + (y1 - y0) * (xmax_clip - x0) / (x1 - x0);
                x = xmax_clip;
            } else {
                y = y0 + (y1 - y0) * (xmin_clip - x0) / (x1 - x0);
                x = xmin_clip;
            }

            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = computeOutCode(x0, y0);
            } else {
                x1 = x;
                y1 = y;
                outcode1 = computeOutCode(x1, y1);
            }
        }
    }
    return accept;
}

void drawClippingWindow() {
    glColor3f(1, 0, 0); // Màu đỏ cho đa giác
    glBegin(GL_LINE_LOOP);
    for (const auto& point : polygonPoints) {
        glVertex2f(point.first, point.second);
    }
    glEnd();
}

void drawLine(double x0, double y0, double x1, double y1, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_LINES);
        glVertex2f(x0, y0);
        glVertex2f(x1, y1);
    glEnd();
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        ypos = WIN_HEIGHT - ypos; // Đảo trục Y

        if (clickCount == 0) {
            x_start = xpos; 
            y_start = ypos;
            clickCount = 1;
            hasLine = false;
            cout << "Điểm bắt đầu: (" << x_start << ", " << y_start << ")\n";
        } else {
            x_end = xpos; 
            y_end = ypos;
            clickCount = 0;
            hasLine = true;

            // Chuẩn bị dữ liệu cho đoạn đã xén
            cx0 = x_start; cy0 = y_start; 
            cx1 = x_end;   cy1 = y_end;
            clipped = cohenSutherlandClip(cx0, cy0, cx1, cy1);

            cout << "Điểm kết thúc: (" << x_end << ", " << y_end << ")\n";
            if (clipped) {
                cout << "Sau khi xén: (" << cx0 << ", " << cy0 << ") -> (" 
                     << cx1 << ", " << cy1 << ")\n";
            } else {
                cout << "Đoạn thẳng nằm ngoài cửa sổ!\n";
            }
        }
    }
}

int main() {
    // Đọc đa giác từ file
    readPolygonFromFile("polygon.txt");

    if (!glfwInit()) {
        cerr << "Failed to init GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Cohen Sutherland Clipping with Polygon", NULL, NULL);
    if (!window) {
        cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

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

        // Vẽ đoạn gốc
        if (hasLine) {
            drawLine(x_start, y_start, x_end, y_end, 0, 0, 1); // Xanh dương
            if (clipped) {
                drawLine(cx0, cy0, cx1, cy1, 0, 1, 0); // Xanh lá
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}