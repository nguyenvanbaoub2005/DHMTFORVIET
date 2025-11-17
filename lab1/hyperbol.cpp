#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

// Cấu trúc điểm 2D
struct Point {
    int x, y;
    Point(int _x = 0, int _y = 0) : x(_x), y(_y) {}
};

// Cấu trúc nút giao diện
struct Button {
    int x, y;
    int width, height;
    std::string text;
    bool isHovered;
    
    Button() : x(0), y(0), width(0), height(0), text(""), isHovered(false) {}
    
    Button(int _x, int _y, int _w, int _h, const std::string& _text = "") 
        : x(_x), y(_y), width(_w), height(_h), text(_text), isHovered(false) {}
    
    bool contains(int px, int py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
    
    void draw() const {
        // Vẽ nền nút
        glBegin(GL_QUADS);
        if (isHovered) {
            glColor3f(0.7f, 0.7f, 0.7f); // Màu xám nhạt khi hover
        } else {
            glColor3f(0.5f, 0.5f, 0.5f); // Màu xám bình thường
        }
        glVertex2i(x, y);
        glVertex2i(x + width, y);
        glVertex2i(x + width, y + height);
        glVertex2i(x, y + height);
        glEnd();
        
        // Vẽ viền nút
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
        glVertex2i(x, y);
        glVertex2i(x + width, y);
        glVertex2i(x + width, y + height);
        glVertex2i(x, y + height);
        glEnd();
    }
};

// Biến toàn cục
GLFWwindow* window;
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
std::vector<Point> points;
bool firstPoint = true;
int algorithmType = 0; // 0: Bresenham, 1: Midpoint

// Các nút giao diện
Button btnBresenham(10, 10, 120, 30, "Bresenham");
Button btnMidpoint(140, 10, 120, 30, "Midpoint");
Button btnClear(270, 10, 120, 30, "Clear");
Button btnExit(400, 10, 120, 30, "Exit");

// Hàm vẽ pixel
void drawPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

// Thuật toán Bresenham cho vẽ hyperbol (adapt từ Bresenham approach)
void bresenhamHyperbola(double a_d, double b_d, int cx, int cy) {
    long a = static_cast<long>(round(a_d));
    long b = static_cast<long>(round(b_d));
    if (a <= 0 || b <= 0) return;
    
    double x = a, y = 0;
    double d1 = (2 * a * a) - (b * b) - (2 * a * b * b);
    drawPixel(static_cast<int>(x) + cx, static_cast<int>(y) + cy);
    drawPixel(-static_cast<int>(x) + cx, static_cast<int>(y) + cy);
    drawPixel(static_cast<int>(x) + cx, -static_cast<int>(y) + cy);
    drawPixel(-static_cast<int>(x) + cx, -static_cast<int>(y) + cy);
    
    while ((a * a * y) <= (b * b * x)) {
        if (d1 <= (-1 * b * b * 0.5)) {
            d1 += 2 * a * a * (2 * y + 3);
            y++;
        } else {
            d1 += 2 * a * a * (2 * y + 3) - 4 * b * b * (x + 1);
            x++;
            y++;
        }
        drawPixel(static_cast<int>(x) + cx, static_cast<int>(y) + cy);
        drawPixel(-static_cast<int>(x) + cx, static_cast<int>(y) + cy);
        drawPixel(static_cast<int>(x) + cx, -static_cast<int>(y) + cy);
        drawPixel(-static_cast<int>(x) + cx, -static_cast<int>(y) + cy);
    }
    
    d1 = a * a * (y + 1) * (y + 1) + a * a * y * y + 2 * a * a * b * b - 2 * a * a * b * b * (x + 1) * (x + 1);
    while (y < WINDOW_HEIGHT / 2) {
        if (d1 <= (a * a * 0.5)) {
            d1 += a * a * 4 * (y + 1) - 2 - a * a * b * b * (2 * x + 3) * (2 * x + 3);
            y++;
            x++;
        } else {
            d1 += -2.0 * b * b * a * a * (2 * x + 3);
            x++;
        }
        drawPixel(static_cast<int>(x) + cx, static_cast<int>(y) + cy);
        drawPixel(-static_cast<int>(x) + cx, static_cast<int>(y) + cy);
        drawPixel(static_cast<int>(x) + cx, -static_cast<int>(y) + cy);
        drawPixel(-static_cast<int>(x) + cx, -static_cast<int>(y) + cy);
    }
}

// Thuật toán Midpoint cho vẽ hyperbol (adapt từ pseudocode)
void midpointHyperbola(double a_d, double b_d, int bound, int cx, int cy) {
    long a = static_cast<long>(round(a_d));
    long b = static_cast<long>(round(b_d));
    if (a <= 0 || b <= 0) return;
    
    long x = a, y = 0;
    long a_sqr = a * a;
    long b_sqr = b * b;
    long a22 = a_sqr + a_sqr; // 2*a^2
    long b22 = b_sqr + b_sqr; // 2*b^2
    long a42 = a22 + a22; // 4*a^2
    long b42 = b22 + b22; // 4*b^2
    long x_slope = b_sqr * a; // initial b^2 * a, but wait, in pseudocode x_slope = b42 * (x +1) ? Adjust.
    long y_slope = 0;
    long mida = a_sqr >> 1; // a^2 /2
    long midb = b_sqr >> 1; // b^2 /2
    long d = b22 * a * a - a22 * b * b + midb; // initial d for region 1, adjust based on paper

    // Adjust initial d according to pseudocode
    d = a22 - b_sqr * (1 + 2 * a) + midb;

    // Region 1
    y_slope = a42; // initial y_slope = 4*a^2 * (y+1) ? Initial y=0, y_slope = a42 for next
    x_slope = b42 * (x + 1); // 4*b^2 * (x+1)
    while (d < x_slope && y <= bound) {
        drawPixel(static_cast<int>(x) + cx, static_cast<int>(y) + cy);
        drawPixel(-static_cast<int>(x) + cx, static_cast<int>(y) + cy);
        drawPixel(static_cast<int>(x) + cx, -static_cast<int>(y) + cy);
        drawPixel(-static_cast<int>(x) + cx, -static_cast<int>(y) + cy);

        if (d >= 0) {
            d -= x_slope;
            x++;
            x_slope += b42;
        }
        d += a22 + y_slope;
        y++;
        y_slope += a42;
    }

    // Transition
    d = d - (x_slope + y_slope) / 2 + (a_sqr - b_sqr) - midb - mida;

    // Region 2
    if (a > b) {
        while (y <= bound) {
            drawPixel(static_cast<int>(x) + cx, static_cast<int>(y) + cy);
            drawPixel(-static_cast<int>(x) + cx, static_cast<int>(y) + cy);
            drawPixel(static_cast<int>(x) + cx, -static_cast<int>(y) + cy);
            drawPixel(-static_cast<int>(x) + cx, -static_cast<int>(y) + cy);

            if (d <= 0) {
                d += y_slope;
                y++;
                y_slope += a42;
            }
            d -= b22 + x_slope;
            x++;
            x_slope += b42;
        }
    }
}

// Hàm kiểm tra và cập nhật trạng thái hover cho các nút
void updateButtonHoverState(double xpos, double ypos) {
    int y = WINDOW_HEIGHT - static_cast<int>(ypos);
    btnBresenham.isHovered = btnBresenham.contains(static_cast<int>(xpos), y);
    btnMidpoint.isHovered = btnMidpoint.contains(static_cast<int>(xpos), y);
    btnClear.isHovered = btnClear.contains(static_cast<int>(xpos), y);
    btnExit.isHovered = btnExit.contains(static_cast<int>(xpos), y);
}

// Callback xử lý sự kiện di chuyển chuột
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    updateButtonHoverState(xpos, ypos);
}

// Hàm xử lý sự kiện chuột
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // Chuyển đổi tọa độ chuột sang tọa độ OpenGL
        int x = static_cast<int>(xpos);
        int y = static_cast<int>(WINDOW_HEIGHT - ypos); // Đảo ngược trục y
        
        // Kiểm tra xem có nhấn vào nút nào không
        if (btnBresenham.contains(x, y)) {
            algorithmType = 0;
            std::cout << "Switched to Bresenham algorithm" << std::endl;
            return;
        }
        else if (btnMidpoint.contains(x, y)) {
            algorithmType = 1;
            std::cout << "Switched to Midpoint algorithm" << std::endl;
            return;
        }
        else if (btnClear.contains(x, y)) {
            points.clear();
            firstPoint = true;
            std::cout << "Cleared all points" << std::endl;
            return;
        }
        else if (btnExit.contains(x, y)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            return;
        }
        
        if (firstPoint) {
            // Điểm đầu tiên
            points.clear();
            points.push_back(Point(x, y));
            firstPoint = false;
            std::cout << "First point set: (" << x << ", " << y << ")" << std::endl;
        } else {
            // Điểm thứ hai
            points.push_back(Point(x, y));
            firstPoint = true;
            std::cout << "Second point set: (" << x << ", " << y << ")" << std::endl;
            std::cout << "Drawing hyperbola using " << (algorithmType == 0 ? "Bresenham" : "Midpoint") << " algorithm" << std::endl;
        }
    }
}

// Hàm xử lý sự kiện bàn phím
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        algorithmType = 0; // Chuyển sang thuật toán Bresenham
        std::cout << "Switched to Bresenham algorithm" << std::endl;
    } else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        algorithmType = 1; // Chuyển sang thuật toán Midpoint
        std::cout << "Switched to Midpoint algorithm" << std::endl;
    } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        points.clear(); // Xóa tất cả các điểm
        firstPoint = true;
        std::cout << "Cleared all points" << std::endl;
    } else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// Hàm vẽ
void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vẽ lưới tọa độ
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    for (int i = 0; i <= WINDOW_WIDTH; i += 50) {
        glVertex2i(i, 0);
        glVertex2i(i, WINDOW_HEIGHT);
    }
    for (int i = 0; i <= WINDOW_HEIGHT; i += 50) {
        glVertex2i(0, i);
        glVertex2i(WINDOW_WIDTH, i);
    }
    glEnd();
    
    // Vẽ trục tọa độ
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    glVertex2i(0, WINDOW_HEIGHT / 2);
    glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT / 2);
    glVertex2i(WINDOW_WIDTH / 2, 0);
    glVertex2i(WINDOW_WIDTH / 2, WINDOW_HEIGHT);
    glEnd();
    
    // Vẽ các điểm đã chọn
    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(5.0f);
    for (const auto& point : points) {
        drawPixel(point.x, point.y);
    }
    
    // Vẽ đường hyperbol nếu đã có đủ 2 điểm
    if (points.size() >= 2) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glPointSize(2.0f);
        
        int x1 = points[0].x;
        int y1 = points[0].y;
        int x2 = points[1].x;
        int y2 = points[1].y;
        
        int xc = (x1 + x2) / 2;
        int yc = (y1 + y2) / 2;
        
        double dx = x2 - x1;
        double dy = y2 - y1;
        double dist = std::sqrt(dx * dx + dy * dy);
        double c = dist / 2.0;
        double e = 1.25; // eccentricity > 1
        double a = c / e;
        double b = a * std::sqrt(e * e - 1);
        
        int bound = WINDOW_HEIGHT - 1;
        
        if (algorithmType == 0) {
            bresenhamHyperbola(a, b, xc, yc);
        } else {
            midpointHyperbola(a, b, bound, xc, yc);
        }
    }
    
    // Vẽ các nút giao diện
    btnBresenham.draw();
    btnMidpoint.draw();
    btnClear.draw();
    btnExit.draw();
    
    // Vẽ text cho các nút
    glColor3f(0.0f, 0.0f, 0.0f);
    
    // Bresenham button - Vẽ chữ "B" bằng các hình chữ nhật
    glBegin(GL_QUADS);
    // Phần trên của B
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 20);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 20);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 25);
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 25);
    
    // Phần giữa của B
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 12);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 12);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 17);
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 17);
    
    // Phần dưới của B
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 10);
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 10);
    
    // Phần trái của B
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 25);
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 25);
    glEnd();
    
    // Midpoint button - Vẽ chữ "M" bằng các hình chữ nhật
    glBegin(GL_QUADS);
    // Phần trái của M
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 25);
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 25);
    
    // Phần phải của M
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 25);
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 25);
    
    // Phần giữa trên của M
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 20);
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 20);
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 25);
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 25);
    
    // Phần chéo trái của M
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 15);
    glVertex2i(btnMidpoint.x + 25, btnMidpoint.y + 15);
    glVertex2i(btnMidpoint.x + 25, btnMidpoint.y + 20);
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 20);
    
    // Phần chéo phải của M
    glVertex2i(btnMidpoint.x + 25, btnMidpoint.y + 15);
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 15);
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 20);
    glVertex2i(btnMidpoint.x + 25, btnMidpoint.y + 20);
    glEnd();
    
    // Clear button - Vẽ chữ "C" bằng các hình chữ nhật
    glBegin(GL_QUADS);
    // Vẽ phần trên của chữ C
    glVertex2i(btnClear.x + 15, btnClear.y + 20);
    glVertex2i(btnClear.x + 30, btnClear.y + 20);
    glVertex2i(btnClear.x + 30, btnClear.y + 25);
    glVertex2i(btnClear.x + 15, btnClear.y + 25);
    
    // Vẽ phần trái của chữ C
    glVertex2i(btnClear.x + 15, btnClear.y + 5);
    glVertex2i(btnClear.x + 20, btnClear.y + 5);
    glVertex2i(btnClear.x + 20, btnClear.y + 25);
    glVertex2i(btnClear.x + 15, btnClear.y + 25);
    
    // Vẽ phần dưới của chữ C
    glVertex2i(btnClear.x + 15, btnClear.y + 5);
    glVertex2i(btnClear.x + 30, btnClear.y + 5);
    glVertex2i(btnClear.x + 30, btnClear.y + 10);
    glVertex2i(btnClear.x + 15, btnClear.y + 10);
    glEnd();
    
    // Exit button - Vẽ chữ "X" bằng các hình chữ nhật
    glBegin(GL_QUADS);
    // Vẽ phần chéo xuống của chữ X
    glVertex2i(btnExit.x + 15, btnExit.y + 25);
    glVertex2i(btnExit.x + 20, btnExit.y + 25);
    glVertex2i(btnExit.x + 30, btnExit.y + 5);
    glVertex2i(btnExit.x + 25, btnExit.y + 5);
    
    // Vẽ phần chéo lên của chữ X
    glVertex2i(btnExit.x + 15, btnExit.y + 5);
    glVertex2i(btnExit.x + 20, btnExit.y + 5);
    glVertex2i(btnExit.x + 30, btnExit.y + 25);
    glVertex2i(btnExit.x + 25, btnExit.y + 25);
    glEnd();
}

int main() {
    // Khởi tạo GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Tạo cửa sổ
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hyperbola Drawing Algorithms", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Khởi tạo GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Khởi tạo các nút giao diện
    btnBresenham = Button(10, 10, 120, 30, "Bresenham");
    btnMidpoint = Button(140, 10, 120, 30, "Midpoint");
    btnClear = Button(270, 10, 120, 30, "Clear");
    btnExit = Button(400, 10, 120, 30, "Exit");
    
    // Đăng ký callback
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    
    // Thiết lập viewport
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Thiết lập ma trận chiếu
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    
    std::cout << "Hyperbola Drawing Program" << std::endl;
    std::cout << "--------------------------" << std::endl;
    std::cout << "Instructions:" << std::endl;
    std::cout << "1. First click sets the first focus point" << std::endl;
    std::cout << "2. Second click sets the second focus point and draws the hyperbola" << std::endl;
    std::cout << "3. Press 'B' or click 'Bresenham' button to use Bresenham algorithm" << std::endl;
    std::cout << "4. Press 'M' or click 'Midpoint' button to use Midpoint algorithm" << std::endl;
    std::cout << "5. Press 'C' or click 'Clear' button to clear all points" << std::endl;
    std::cout << "6. Press 'ESC' or click 'Exit' button to exit" << std::endl;
    std::cout << "Note: The two points are used as foci to determine the hyperbola parameters." << std::endl;
    
    // Vòng lặp chính
    while (!glfwWindowShouldClose(window)) {
        render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}