#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

struct Color {
    float r, g, b;
    Color(float r = 0, float g = 0, float b = 0) : r(r), g(g), b(b) {}
};

class BoundaryFillRecursive {
private:
    static const int WINDOW_WIDTH = 800;
    static const int WINDOW_HEIGHT = 600;
    
    std::vector<Point> polygon;
    Color boundaryColor = Color(1.0f, 1.0f, 1.0f); // Trắng
    Color fillColor = Color(1.0f, 0.0f, 0.0f);     // Đỏ
    Color backgroundColor = Color(0.0f, 0.0f, 0.0f); // Đen
    
    GLFWwindow* window;
    std::vector<std::vector<Color>> frameBuffer;
    
public:
    BoundaryFillRecursive() {
        frameBuffer.resize(WINDOW_HEIGHT, std::vector<Color>(WINDOW_WIDTH, backgroundColor));
    }
    
    bool init() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Boundary Fill - Recursive", NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create window" << std::endl;
            glfwTerminate();
            return false;
        }
        
        glfwMakeContextCurrent(window);
        
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return false;
        }
        
        glfwSetWindowUserPointer(window, this);
        glfwSetMouseButtonCallback(window, mouseCallback);
        
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        return true;
    }
    
    bool loadPolygonFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return false;
        }
        
        int numPoints;
        file >> numPoints;
        
        polygon.clear();
        for (int i = 0; i < numPoints; i++) {
            int x, y;
            file >> x >> y;
            polygon.push_back(Point(x, y));
        }
        
        file.close();
        std::cout << "Loaded " << numPoints << " points from " << filename << std::endl;
        return true;
    }
    
    void drawLine(int x0, int y0, int x1, int y1) {
        // Bresenham's line algorithm
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        
        int x = x0, y = y0;
        
        while (true) {
            if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
                frameBuffer[y][x] = boundaryColor;
            }
            
            if (x == x1 && y == y1) break;
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x += sx;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }
    }
    
    void drawPolygon() {
        if (polygon.size() < 3) return;
        
        for (size_t i = 0; i < polygon.size(); i++) {
            size_t next = (i + 1) % polygon.size();
            drawLine(polygon[i].x, polygon[i].y, polygon[next].x, polygon[next].y);
        }
    }
    
    Color getPixel(int x, int y) {
        if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
            return boundaryColor;
        }
        return frameBuffer[y][x];
    }
    
    void setPixel(int x, int y, const Color& color) {
        if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
            frameBuffer[y][x] = color;
        }
    }
    
    bool isSameColor(const Color& c1, const Color& c2) {
        const float epsilon = 0.01f;
        return (abs(c1.r - c2.r) < epsilon && 
                abs(c1.g - c2.g) < epsilon && 
                abs(c1.b - c2.b) < epsilon);
    }
    
    // Thuật toán Boundary Fill đệ quy
    void boundaryFillRecursive(int x, int y, const Color& fillCol, const Color& boundaryCol) {
        Color currentColor = getPixel(x, y);
        
        // Điều kiện dừng: nếu là màu biên hoặc đã được tô
        if (isSameColor(currentColor, boundaryCol) || isSameColor(currentColor, fillCol)) {
            return;
        }
        
        // Tô điểm hiện tại
        setPixel(x, y, fillCol);
        
        // Đệ quy cho 4 điểm lân cận
        boundaryFillRecursive(x + 1, y, fillCol, boundaryCol);     // Phải
        boundaryFillRecursive(x - 1, y, fillCol, boundaryCol);     // Trái
        boundaryFillRecursive(x, y + 1, fillCol, boundaryCol);     // Trên
        boundaryFillRecursive(x, y - 1, fillCol, boundaryCol);     // Dưới
    }
    
    static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            BoundaryFillRecursive* app = static_cast<BoundaryFillRecursive*>(glfwGetWindowUserPointer(window));
            
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            
            // Chuyển đổi tọa độ từ window coordinate sang pixel coordinate
            int x = static_cast<int>(xpos);
            int y = static_cast<int>(WINDOW_HEIGHT - ypos); // Flip Y coordinate
            
            std::cout << "Mouse clicked at: (" << x << ", " << y << ")" << std::endl;
            
            // Thực hiện boundary fill tại vị trí click
            app->boundaryFillRecursive(x, y, app->fillColor, app->boundaryColor);
        }
    }
    
    void render() {
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Vẽ frame buffer lên màn hình
        glBegin(GL_POINTS);
        for (int y = 0; y < WINDOW_HEIGHT; y++) {
            for (int x = 0; x < WINDOW_WIDTH; x++) {
                Color& color = frameBuffer[y][x];
                glColor3f(color.r, color.g, color.b);
                glVertex2i(x, y);
            }
        }
        glEnd();
        
        glfwSwapBuffers(window);
    }
    
    void run() {
        if (!loadPolygonFromFile("polygonsmall.txt")) {
            // Tạo polygon mặc định nếu không đọc được file
            polygon.push_back(Point(200, 200));
            polygon.push_back(Point(400, 200));
            polygon.push_back(Point(500, 400));
            polygon.push_back(Point(300, 500));
            polygon.push_back(Point(100, 400));
            std::cout << "Using default polygon" << std::endl;
        }
        
        drawPolygon();
        
        std::cout << "Instructions:" << std::endl;
        std::cout << "- Click inside the polygon to fill it" << std::endl;
        std::cout << "- Press ESC to exit" << std::endl;
        
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
            
            render();
        }
    }
    
    ~BoundaryFillRecursive() {
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }
};

int main() {
    BoundaryFillRecursive app;
    
    if (!app.init()) {
        return -1;
    }
    
    app.run();
    
    return 0;
}