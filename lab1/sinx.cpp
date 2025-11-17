#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm> // For std::min and std::max

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

// Thuật toán Bresenham cho vẽ đoạn thẳng
void bresenhamLine(int x1, int y1, int x2, int y2) {
    // Trường hợp 1: 0 < m < 1
    if (abs(x2 - x1) >= abs(y2 - y1) && x1 <= x2) {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int yi = 1;
        
        if (dy < 0) {
            yi = -1;
            dy = -dy;
        }
        
        int p = 2 * dy - dx;
        int y = y1;
        
        for (int x = x1; x <= x2; x++) {
            drawPixel(x, y);
            if (p > 0) {
                y += yi;
                p += 2 * (dy - dx);
            } else {
                p += 2 * dy;
            }
        }
    }
    // Trường hợp 2: m > 1
    else if (abs(x2 - x1) < abs(y2 - y1) && y1 <= y2) {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int xi = 1;
        
        if (dx < 0) {
            xi = -1;
            dx = -dx;
        }
        
        int p = 2 * dx - dy;
        int x = x1;
        
        for (int y = y1; y <= y2; y++) {
            drawPixel(x, y);
            if (p > 0) {
                x += xi;
                p += 2 * (dx - dy);
            } else {
                p += 2 * dx;
            }
        }
    }
    // Trường hợp 3: -1 < m < 0
    else if (abs(x2 - x1) >= abs(y2 - y1) && x1 > x2) {
        bresenhamLine(x2, y2, x1, y1); // Đổi điểm đầu và điểm cuối
    }
    // Trường hợp 4: m < -1
    else {
        bresenhamLine(x2, y2, x1, y1); // Đổi điểm đầu và điểm cuối
    }
}

// Thuật toán Midpoint cho vẽ đoạn thẳng
void midpointLine(int x1, int y1, int x2, int y2) {
    // Trường hợp 1: 0 < m < 1
    if (abs(x2 - x1) >= abs(y2 - y1) && x1 <= x2) {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int yi = 1;
        
        if (dy < 0) {
            yi = -1;
            dy = -dy;
        }
        
        int d = dy - (dx / 2);
        int y = y1;
        
        for (int x = x1; x <= x2; x++) {
            drawPixel(x, y);
            if (d > 0) {
                y += yi;
                d += (dy - dx);
            } else {
                d += dy;
            }
        }
    }
    // Trường hợp 2: m > 1
    else if (abs(x2 - x1) < abs(y2 - y1) && y1 <= y2) {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int xi = 1;
        
        if (dx < 0) {
            xi = -1;
            dx = -dx;
        }
        
        int d = dx - (dy / 2);
        int x = x1;
        
        for (int y = y1; y <= y2; y++) {
            drawPixel(x, y);
            if (d > 0) {
                x += xi;
                d += (dx - dy);
            } else {
                d += dx;
            }
        }
    }
    // Trường hợp 3: -1 < m < 0
    else if (abs(x2 - x1) >= abs(y2 - y1) && x1 > x2) {
        midpointLine(x2, y2, x1, y1); // Đổi điểm đầu và điểm cuối
    }
    // Trường hợp 4: m < -1
    else {
        midpointLine(x2, y2, x1, y1); // Đổi điểm đầu và điểm cuối
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
            int x_start = std::min(points[0].x, points[1].x);
            int x_end = std::max(points[0].x, points[1].x);
            std::cout << "Second point set: (" << x << ", " << y << ")" << std::endl;
            std::cout << "Drawing sin(x) from x=" << x_start << " to x=" << x_end << " using " << (algorithmType == 0 ? "Bresenham" : "Midpoint") << " for segments" << std::endl;
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
    
    // Vẽ đường sin(x) nếu đã có đủ 2 điểm
    if (points.size() >= 2) {
        int x_start = std::min(points[0].x, points[1].x);
        int x_end = std::max(points[0].x, points[1].x);
        
        if (x_start == x_end) return; // Không vẽ nếu x giống nhau
        
        double pi = acos(-1.0);
        double amp = 100.0;
        double center_y = WINDOW_HEIGHT / 2.0;
        double period = static_cast<double>(x_end - x_start) / 4.0; // 4 chu kỳ
        double freq = 2 * pi / period;
        
        glColor3f(0.0f, 1.0f, 0.0f);
        glPointSize(2.0f);
        
        int prev_x = x_start;
        double prev_y_d = center_y + amp * sin(freq * (prev_x - x_start));
        int prev_y = static_cast<int>(round(prev_y_d));
        
        for (int x = x_start + 1; x <= x_end; ++x) {
            double y_d = center_y + amp * sin(freq * (x - x_start));
            int y = static_cast<int>(round(y_d));
            
            // Vẽ đoạn thẳng ngắn từ (prev_x, prev_y) đến (x, y) sử dụng thuật toán được chọn
            if (algorithmType == 0) {
                bresenhamLine(prev_x, prev_y, x, y);
            } else {
                midpointLine(prev_x, prev_y, x, y);
            }
            
            prev_x = x;
            prev_y = y;
        }
    }
    
    // Vẽ các nút giao diện
    btnBresenham.draw();
    btnMidpoint.draw();
    btnClear.draw();
    btnExit.draw();
    
    // Vẽ text cho các nút (giữ nguyên như code gốc)
    glColor3f(0.0f, 0.0f, 0.0f);
    
    // Bresenham button - Vẽ số "1" bằng các hình chữ nhật
    glBegin(GL_QUADS);
    // Vẽ số 1
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 25);
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 25);
    glEnd();
    
    // Midpoint button - Vẽ số "2" bằng các hình chữ nhật
    glBegin(GL_QUADS);
    // Vẽ phần trên của số 2
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 20);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 20);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 25);
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 25);
    
    // Vẽ phần giữa của số 2
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 12);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 12);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 17);
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 17);
    
    // Vẽ phần dưới của số 2
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 10);
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 10);
    
    // Vẽ phần phải trên của số 2
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 17);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 17);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 20);
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 20);
    
    // Vẽ phần trái dưới của số 2
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 10);
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 10);
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 12);
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 12);
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
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sin Curve Drawing Algorithms", NULL, NULL);
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
    
    std::cout << "Sin Curve Drawing Program" << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "Instructions:" << std::endl;
    std::cout << "1. First click sets the start x of the sin curve" << std::endl;
    std::cout << "2. Second click sets the end x and draws the sin(x) curve (y-coordinates ignored)" << std::endl;
    std::cout << "3. Press 'B' or click 'Bresenham' button to use Bresenham algorithm for curve segments" << std::endl;
    std::cout << "4. Press 'M' or click 'Midpoint' button to use Midpoint algorithm for curve segments" << std::endl;
    std::cout << "5. Press 'C' or click 'Clear' button to clear all points" << std::endl;
    std::cout << "6. Press 'ESC' or click 'Exit' button to exit" << std::endl;
    std::cout << "Note: The curve has amplitude 100, 4 cycles in the x-range." << std::endl;
    
    // Vòng lặp chính
    while (!glfwWindowShouldClose(window)) {
        render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}