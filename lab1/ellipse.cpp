#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

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

// Hàm vẽ 4 điểm đối xứng cho ellipse
void drawEllipsePoints(int xc, int yc, int x, int y) {
    drawPixel(xc + x, yc + y);
    drawPixel(xc - x, yc + y);
    drawPixel(xc + x, yc - y);
    drawPixel(xc - x, yc - y);
}

// Thuật toán Bresenham cho vẽ ellipse
void bresenhamEllipse(int xc, int yc, int a, int b) {
    int x = 0;
    int y = b;
    
    // Vùng 1
    double a2 = static_cast<double>(a) * a;
    double b2 = static_cast<double>(b) * b;
    double d1 = b2 - a2 * b + 0.25 * a2;
    double dx = 2.0 * b2 * x;
    double dy = 2.0 * a2 * y;
    
    // Vẽ vùng 1
    while (dx < dy) {
        drawEllipsePoints(xc, yc, x, y);
        
        if (d1 < 0) {
            x++;
            dx = 2.0 * b2 * x;
            d1 = d1 + dx + b2;
        } else {
            x++;
            y--;
            dx = 2.0 * b2 * x;
            dy = 2.0 * a2 * y;
            d1 = d1 + dx - dy + b2;
        }
    }
    
    // Vẽ điểm cuối vùng 1
    drawEllipsePoints(xc, yc, x, y);
    
    // Vùng 2
    double d2 = b2 * (x + 0.5) * (x + 0.5) + a2 * (y - 1.0) * (y - 1.0) - a2 * b2;
    
    // Vẽ vùng 2
    while (y >= 0) {
        drawEllipsePoints(xc, yc, x, y);
        
        if (d2 > 0) {
            y--;
            dy = 2.0 * a2 * y;
            d2 = d2 + a2 - dy;
        } else {
            y--;
            x++;
            dx = 2.0 * b2 * x;
            dy = 2.0 * a2 * y;
            d2 = d2 + dx - dy + a2;
        }
    }
}

// Thuật toán Midpoint cho vẽ ellipse
void midpointEllipse(int xc, int yc, int a, int b) {
    int x = 0;
    int y = b;
    
    // Vùng 1
    double a2 = static_cast<double>(a) * a;
    double b2 = static_cast<double>(b) * b;
    double d1 = b2 - a2 * b + 0.25 * a2;
    
    // Vẽ vùng 1
    while (2.0 * b2 * x < 2.0 * a2 * y) {
        drawEllipsePoints(xc, yc, x, y);
        
        if (d1 < 0) {
            x++;
            d1 += 2.0 * b2 * x + b2;
        } else {
            x++;
            y--;
            d1 += 2.0 * b2 * x - 2.0 * a2 * y + b2;
        }
    }
    
    // Vẽ điểm cuối vùng 1
    drawEllipsePoints(xc, yc, x, y);
    
    // Vùng 2
    double d2 = b2 * (x + 0.5) * (x + 0.5) + a2 * (y - 1.0) * (y - 1.0) - a2 * b2;
    
    // Vẽ vùng 2
    while (y >= 0) {
        drawEllipsePoints(xc, yc, x, y);
        
        if (d2 > 0) {
            y--;
            d2 += -2.0 * a2 * y + a2;
        } else {
            x++;
            y--;
            d2 += 2.0 * b2 * x - 2.0 * a2 * y + a2;
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
            // Điểm đầu tiên (tâm ellipse)
            points.clear();
            points.push_back(Point(x, y));
            firstPoint = false;
            std::cout << "Center point set: (" << x << ", " << y << ")" << std::endl;
        } else {
            // Điểm thứ hai (để xác định bán trục a và b)
            points.push_back(Point(x, y));
            firstPoint = true;
            
            int xc = points[0].x;
            int yc = points[0].y;
            int a = abs(x - xc); // Bán trục a
            int b = abs(y - yc); // Bán trục b
            
            // Đảm bảo a và b không bằng 0
            if (a == 0) a = 1;
            if (b == 0) b = 1;
            
            std::cout << "Corner point set: (" << x << ", " << y << ")" << std::endl;
            std::cout << "Drawing ellipse with center (" << xc << ", " << yc << "), a=" << a << ", b=" << b << std::endl;
            std::cout << "Using " << (algorithmType == 0 ? "Bresenham" : "Midpoint") << " algorithm" << std::endl;
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
    
    // Vẽ ellipse nếu đã có đủ 2 điểm
    if (points.size() >= 2) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glPointSize(2.0f);
        
        int xc = points[0].x;
        int yc = points[0].y;
        int a = abs(points[1].x - xc); // Bán trục a
        int b = abs(points[1].y - yc); // Bán trục b
        
        // Đảm bảo a và b không bằng 0
        if (a == 0) a = 1;
        if (b == 0) b = 1;
        
        if (algorithmType == 0) {
            bresenhamEllipse(xc, yc, a, b);
        } else {
            midpointEllipse(xc, yc, a, b);
        }
        
        // Vẽ hình chữ nhật bounding box để hiển thị rõ hơn
        glColor3f(0.8f, 0.8f, 0.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2i(xc - a, yc - b);
        glVertex2i(xc + a, yc - b);
        glVertex2i(xc + a, yc + b);
        glVertex2i(xc - a, yc + b);
        glEnd();
    }
    
    // Vẽ các nút giao diện
    btnBresenham.draw();
    btnMidpoint.draw();
    btnClear.draw();
    btnExit.draw();
    
    // Vẽ text cho các nút
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
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Ellipse Drawing Algorithms", NULL, NULL);
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
    
    std::cout << "Ellipse Drawing Program" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << "Instructions:" << std::endl;
    std::cout << "1. First click sets the center point of the ellipse" << std::endl;
    std::cout << "2. Second click determines the size (a and b axes) and draws the ellipse" << std::endl;
    std::cout << "3. Press 'B' or click '1' button to use Bresenham algorithm" << std::endl;
    std::cout << "4. Press 'M' or click '2' button to use Midpoint algorithm" << std::endl;
    std::cout << "5. Press 'C' or click 'C' button to clear all points" << std::endl;
    std::cout << "6. Press 'ESC' or click 'X' button to exit" << std::endl;
    std::cout << "7. Yellow rectangle shows the bounding box of the ellipse" << std::endl;
    
    // Vòng lặp chính
    while (!glfwWindowShouldClose(window)) {
        render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}