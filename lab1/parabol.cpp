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

// Thuật toán Bresenham cho vẽ parabol
void bresenhamParabola(int xc, int yc, int x1, int y1) {
    // Tính tham số a của parabol y = ax^2
    int dx = x1 - xc;
    int dy = y1 - yc;
    
    if (dx == 0) return; // Tránh chia cho 0
    
    // Tính a = dy/dx^2, nhưng dùng số nguyên
    // Sử dụng công thức: y - yc = a*(x - xc)^2
    
    // Vẽ parabol theo hướng x
    int range = abs(dx) * 2; // Phạm vi vẽ
    
    for (int i = -range; i <= range; i++) {
        int x = xc + i;
        if (x < 0 || x >= WINDOW_WIDTH) continue;
        
        // Tính y theo công thức parabol: y = yc + a*(x-xc)^2
        // a = dy/dx^2
        int y = yc + (dy * i * i) / (dx * dx);
        
        if (y >= 0 && y < WINDOW_HEIGHT) {
            drawPixel(x, y);
        }
    }
}

// Thuật toán Midpoint cho vẽ parabol  
void midpointParabola(int xc, int yc, int x1, int y1) {
    // Tính tham số a của parabol
    int dx = x1 - xc;
    int dy = y1 - yc;
    
    if (dx == 0) return; // Tránh chia cho 0
    
    // Vẽ parabol sử dụng thuật toán midpoint
    int range = abs(dx) * 2;
    
    // Region 1: |dy/dx| < 1
    int x = xc - range/2;
    int y = yc;
    
    // Decision parameter cho midpoint
    int p = 1 - 2 * abs(dy) / (dx * dx);
    
    while (x <= xc + range/2) {
        if (x >= 0 && x < WINDOW_WIDTH) {
            // Tính y chính xác cho điểm x
            int offset = x - xc;
            int y_calc = yc + (dy * offset * offset) / (dx * dx);
            
            if (y_calc >= 0 && y_calc < WINDOW_HEIGHT) {
                drawPixel(x, y_calc);
            }
        }
        x++;
    }
}

// Thuật toán vẽ parabol cải tiến (sử dụng công thức tham số)
void drawParabola(int xc, int yc, int x1, int y1, bool useBresenham) {
    int dx = x1 - xc;
    int dy = y1 - yc;
    
    if (dx == 0) return;
    
    // Tính hệ số a cho parabol y = a(x-xc)^2 + yc
    float a = (float)dy / (dx * dx);
    
    // Vẽ parabol trong phạm vi rộng hơn
    int range = std::max(abs(dx) * 2, 100);
    
    if (useBresenham) {
        // Thuật toán Bresenham cho parabol
        for (int i = -range; i <= range; i++) {
            int x = xc + i;
            if (x < 0 || x >= WINDOW_WIDTH) continue;
            
            int y = yc + (int)(a * i * i);
            
            if (y >= 0 && y < WINDOW_HEIGHT) {
                drawPixel(x, y);
            }
        }
    } else {
        // Thuật toán Midpoint cho parabol
        for (int i = -range; i <= range; i++) {
            int x = xc + i;
            if (x < 0 || x >= WINDOW_WIDTH) continue;
            
            // Sử dụng làm tròn midpoint
            int y = yc + (int)(a * i * i + 0.5f);
            
            if (y >= 0 && y < WINDOW_HEIGHT) {
                drawPixel(x, y);
            }
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
            // Điểm đầu tiên (đỉnh parabol)
            points.clear();
            points.push_back(Point(x, y));
            firstPoint = false;
            std::cout << "Vertex point set: (" << x << ", " << y << ")" << std::endl;
        } else {
            // Điểm thứ hai (điểm trên parabol)
            points.push_back(Point(x, y));
            firstPoint = true;
            std::cout << "Control point set: (" << x << ", " << y << ")" << std::endl;
            std::cout << "Drawing parabola using " << (algorithmType == 0 ? "Bresenham" : "Midpoint") << " algorithm" << std::endl;
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
    glPointSize(8.0f);
    for (size_t i = 0; i < points.size(); i++) {
        drawPixel(points[i].x, points[i].y);
    }
    
    // Vẽ parabol nếu đã có đủ 2 điểm
    if (points.size() >= 2) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glPointSize(2.0f);
        
        drawParabola(points[0].x, points[0].y, points[1].x, points[1].y, algorithmType == 0);
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
    // Vẽ thanh dọc trái của chữ B
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 25);
    glVertex2i(btnBresenham.x + 15, btnBresenham.y + 25);
    
    // Vẽ thanh ngang trên của chữ B
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 20);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 20);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 25);
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 25);
    
    // Vẽ thanh ngang giữa của chữ B
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 12);
    glVertex2i(btnBresenham.x + 28, btnBresenham.y + 12);
    glVertex2i(btnBresenham.x + 28, btnBresenham.y + 17);
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 17);
    
    // Vẽ thanh ngang dưới của chữ B
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 5);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 10);
    glVertex2i(btnBresenham.x + 20, btnBresenham.y + 10);
    
    // Vẽ thanh dọc phải trên của chữ B
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 17);
    glVertex2i(btnBresenham.x + 35, btnBresenham.y + 17);
    glVertex2i(btnBresenham.x + 35, btnBresenham.y + 20);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 20);
    
    // Vẽ thanh dọc phải dưới của chữ B
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 10);
    glVertex2i(btnBresenham.x + 35, btnBresenham.y + 10);
    glVertex2i(btnBresenham.x + 35, btnBresenham.y + 12);
    glVertex2i(btnBresenham.x + 30, btnBresenham.y + 12);
    glEnd();
    
    // Midpoint button - Vẽ chữ "M" bằng các hình chữ nhật
    glBegin(GL_QUADS);
    // Vẽ thanh dọc trái của chữ M
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 25);
    glVertex2i(btnMidpoint.x + 15, btnMidpoint.y + 25);
    
    // Vẽ thanh dọc phải của chữ M
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 5);
    glVertex2i(btnMidpoint.x + 35, btnMidpoint.y + 25);
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 25);
    
    // Vẽ thanh chéo trái của chữ M
    glVertex2i(btnMidpoint.x + 20, btnMidpoint.y + 15);
    glVertex2i(btnMidpoint.x + 25, btnMidpoint.y + 15);
    glVertex2i(btnMidpoint.x + 25, btnMidpoint.y + 25);
    glVertex2i(btnMidpoint.x + 22, btnMidpoint.y + 25);
    
    // Vẽ thanh chéo phải của chữ M
    glVertex2i(btnMidpoint.x + 25, btnMidpoint.y + 15);
    glVertex2i(btnMidpoint.x + 30, btnMidpoint.y + 15);
    glVertex2i(btnMidpoint.x + 28, btnMidpoint.y + 25);
    glVertex2i(btnMidpoint.x + 25, btnMidpoint.y + 25);
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
    
    // Hiển thị thông tin thuật toán hiện tại
    glColor3f(1.0f, 1.0f, 1.0f);
    // Có thể thêm text hiển thị thuật toán đang sử dụng ở đây
}

int main() {
    // Khởi tạo GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Tạo cửa sổ
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Parabola Drawing Algorithms", NULL, NULL);
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
    
    // Thiết lập màu nền
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    std::cout << "Parabola Drawing Program" << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << "Instructions:" << std::endl;
    std::cout << "1. First click sets the vertex of the parabola" << std::endl;
    std::cout << "2. Second click sets a point on the parabola and draws it" << std::endl;
    std::cout << "3. Press 'B' or click 'B' button to use Bresenham algorithm" << std::endl;
    std::cout << "4. Press 'M' or click 'M' button to use Midpoint algorithm" << std::endl;
    std::cout << "5. Press 'C' or click 'C' button to clear all points" << std::endl;
    std::cout << "6. Press 'ESC' or click 'X' button to exit" << std::endl;
    std::cout << "7. Red dots show the control points, green shows the parabola" << std::endl;
    
    // Vòng lặp chính
    while (!glfwWindowShouldClose(window)) {
        render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}