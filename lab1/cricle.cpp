#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

// Cấu trúc lưu trữ điểm
struct Point {
    int x, y;
    Point(int _x, int _y) : x(_x), y(_y) {}
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
std::vector<Point> circlePoints; // Lưu các điểm của đường tròn
bool isFirstClick = true;
Point centerPoint(0, 0);
int radius = 0;
bool useBresenham = true; // true: Bresenham, false: Midpoint

// Các nút giao diện
Button btnBresenham(10, 10, 120, 30, "Bresenham");
Button btnMidpoint(140, 10, 120, 30, "Midpoint");
Button btnClear(270, 10, 120, 30, "Clear");
Button btnExit(400, 10, 120, 30, "Exit");

// Hàm vẽ pixel
void drawPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// Hàm vẽ 8 điểm đối xứng của đường tròn
void drawCirclePoints(int xc, int yc, int x, int y) {
    drawPixel(xc + x, yc + y);
    drawPixel(xc - x, yc + y);
    drawPixel(xc + x, yc - y);
    drawPixel(xc - x, yc - y);
    drawPixel(xc + y, yc + x);
    drawPixel(xc - y, yc + x);
    drawPixel(xc + y, yc - x);
    drawPixel(xc - y, yc - x);
}

// Thuật toán Bresenham vẽ đường tròn
void bresenhamCircle(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    
    drawCirclePoints(xc, yc, x, y);
    
    while (y >= x) {
        x++;
        
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        
        drawCirclePoints(xc, yc, x, y);
    }
}

// Thuật toán Midpoint vẽ đường tròn
void midpointCircle(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int p = 1 - r;
    
    drawCirclePoints(xc, yc, x, y);
    
    while (x < y) {
        x++;
        
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        
        drawCirclePoints(xc, yc, x, y);
    }
}

// Hàm vẽ đường tròn dựa vào thuật toán đã chọn
void drawCircle() {
    if (radius > 0) {
        if (useBresenham) {
            bresenhamCircle(centerPoint.x, centerPoint.y, radius);
        } else {
            midpointCircle(centerPoint.x, centerPoint.y, radius);
        }
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
    
    // Vẽ điểm tâm nếu đã chọn
    if (!isFirstClick || radius > 0) {
        glColor3f(1.0f, 0.0f, 0.0f);
        glPointSize(5.0f);
        drawPixel(centerPoint.x, centerPoint.y);
    }
    
    // Vẽ đường tròn nếu đã có bán kính
    if (radius > 0) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glPointSize(2.0f);
        drawCircle();
    }
    
    // Vẽ các nút giao diện
    btnBresenham.draw();
    btnMidpoint.draw();
    btnClear.draw();
    btnExit.draw();
    
    // Vẽ text cho các nút
    glColor3f(0.0f, 0.0f, 0.0f);
    
    // Vẽ text trên các nút bằng cách vẽ các hình chữ nhật màu khác nhau
    
    // Bresenham button - Vẽ số "1" bằng các hình chữ nhật
     glColor3f(0.0f, 0.0f, 0.0f);
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
    
    // Không hiển thị thông tin ở góc trên cùng bên trái
}

// Hàm kiểm tra và cập nhật trạng thái hover cho các nút
void updateButtonHoverState(double xpos, double ypos) {
    int y = WINDOW_HEIGHT - static_cast<int>(ypos);
    btnBresenham.isHovered = btnBresenham.contains(static_cast<int>(xpos), y);
    btnMidpoint.isHovered = btnMidpoint.contains(static_cast<int>(xpos), y);
    btnClear.isHovered = btnClear.contains(static_cast<int>(xpos), y);
    btnExit.isHovered = btnExit.contains(static_cast<int>(xpos), y);
}

// Callback xử lý sự kiện chuột
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // Chuyển đổi tọa độ chuột sang tọa độ OpenGL
        int x = static_cast<int>(xpos);
        int y = WINDOW_HEIGHT - static_cast<int>(ypos);
        
        // Kiểm tra xem có nhấn vào nút nào không
        if (btnBresenham.contains(x, y)) {
            useBresenham = true;
            std::cout << "Switched to Bresenham algorithm" << std::endl;
            return;
        }
        else if (btnMidpoint.contains(x, y)) {
            useBresenham = false;
            std::cout << "Switched to Midpoint algorithm" << std::endl;
            return;
        }
        else if (btnClear.contains(x, y)) {
            isFirstClick = true;
            radius = 0;
            std::cout << "Cleared circle" << std::endl;
            return;
        }
        else if (btnExit.contains(x, y)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            return;
        }
        
        if (isFirstClick) {
            // Click đầu tiên xác định tâm
            centerPoint = Point(x, y);
            isFirstClick = false;
            std::cout << "Center set at: (" << x << ", " << y << ")" << std::endl;
        } else {
            // Click thứ hai xác định bán kính
            int dx = x - centerPoint.x;
            int dy = y - centerPoint.y;
            radius = static_cast<int>(sqrt(dx*dx + dy*dy));
            isFirstClick = true; // Reset để bắt đầu vẽ đường tròn mới
            std::cout << "Radius set to: " << radius << std::endl;
        }
    }
}

// Callback xử lý sự kiện di chuyển chuột
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    updateButtonHoverState(xpos, ypos);
}

// Callback xử lý sự kiện bàn phím
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_B) {
            useBresenham = true;
            std::cout << "Using Bresenham algorithm" << std::endl;
        }
        else if (key == GLFW_KEY_M) {
            useBresenham = false;
            std::cout << "Using Midpoint algorithm" << std::endl;
        }
        else if (key == GLFW_KEY_C) {
            // Xóa đường tròn hiện tại
            radius = 0;
            isFirstClick = true;
            std::cout << "Cleared current circle" << std::endl;
        }
        else if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

// Hàm khởi tạo OpenGL
bool initOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Circle Drawing Algorithms", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    // Đăng ký callbacks
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);
    
    // Thiết lập viewport và ma trận chiếu
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Thiết lập màu nền
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    return true;
}

int main() {
    if (!initOpenGL()) {
        return -1;
    }
    
    // Khởi tạo các nút giao diện
    btnBresenham = Button(10, 10, 120, 30, "Bresenham");
    btnMidpoint = Button(140, 10, 120, 30, "Midpoint");
    btnClear = Button(270, 10, 120, 30, "Clear");
    btnExit = Button(400, 10, 120, 30, "Exit");
    
    // Đăng ký callback cho sự kiện di chuyển chuột
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    
    std::cout << "Circle Drawing Program" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << "Instructions:" << std::endl;
    std::cout << "1. First click sets the center of the circle" << std::endl;
    std::cout << "2. Second click sets the radius" << std::endl;
    std::cout << "3. Press '1' to use Bresenham algorithm" << std::endl;
    std::cout << "4. Press '2' to use Midpoint algorithm" << std::endl;
    std::cout << "5. Press 'C' to clear the current circle" << std::endl;
    std::cout << "6. Press 'X' to exit" << std::endl;
    std::cout << "7. Use buttons to control the application" << std::endl;
    
    // Vòng lặp chính
    while (!glfwWindowShouldClose(window)) {
        render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}