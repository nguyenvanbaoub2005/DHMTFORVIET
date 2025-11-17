#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <stack>
#include <cmath>

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

struct Color {
    float r, g, b;
    Color(float r = 0, float g = 0, float b = 0) : r(r), g(g), b(b) {}
};

class BoundaryFillIterative {
private:
    static const int WINDOW_WIDTH = 800;
    static const int WINDOW_HEIGHT = 600;
    
    std::vector<Point> polygon;
    Color boundaryColor = Color(1.0f, 1.0f, 1.0f); // Trắng
    Color fillColor = Color(0.0f, 1.0f, 0.0f);     // Xanh lá
    Color backgroundColor = Color(0.0f, 0.0f, 0.0f); // Đen
    
    GLFWwindow* window;
    std::vector<std::vector<Color>> frameBuffer;
    
public:
    BoundaryFillIterative() {
        frameBuffer.resize(WINDOW_HEIGHT, std::vector<Color>(WINDOW_WIDTH, backgroundColor));
    }
    
    bool init() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Boundary Fill - Iterative", NULL, NULL);
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
    
    // Thuật toán Boundary Fill không đệ quy (sử dụng stack)
    void boundaryFillIterative(int startX, int startY, const Color& fillCol, const Color& boundaryCol) {
        std::stack<Point> pixelStack;
        pixelStack.push(Point(startX, startY));
        
        while (!pixelStack.empty()) {
            Point current = pixelStack.top();
            pixelStack.pop();
            
            int x = current.x;
            int y = current.y;
            
            // Kiểm tra bounds
            if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
                continue;
            }
            
            Color currentColor = getPixel(x, y);
            
            // Nếu là màu biên hoặc đã được tô thì bỏ qua
            if (isSameColor(currentColor, boundaryCol) || isSameColor(currentColor, fillCol)) {
                continue;
            }
            
            // Tô điểm hiện tại
            setPixel(x, y, fillCol);
            
            // Thêm 4 điểm lân cận vào stack
            pixelStack.push(Point(x + 1, y));     // Phải
            pixelStack.push(Point(x - 1, y));     // Trái  
            pixelStack.push(Point(x, y + 1));     // Trên
            pixelStack.push(Point(x, y - 1));     // Dưới
            
            // Thêm 4 điểm chéo để tô đầy đủ hơn
            pixelStack.push(Point(x + 1, y + 1)); // Phải-trên
            pixelStack.push(Point(x - 1, y + 1)); // Trái-trên
            pixelStack.push(Point(x + 1, y - 1)); // Phải-dưới
            pixelStack.push(Point(x - 1, y - 1)); // Trái-dưới
        }
    }
    
    // Phiên bản tối ưu hơn: Scanline-based boundary fill
    void boundaryFillScanline(int startX, int startY, const Color& fillCol, const Color& boundaryCol) {
        std::stack<Point> seedStack;
        seedStack.push(Point(startX, startY));
        
        while (!seedStack.empty()) {
            Point seed = seedStack.top();
            seedStack.pop();
            
            int x = seed.x;
            int y = seed.y;
            
            if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
                continue;
            }
            
            Color currentColor = getPixel(x, y);
            if (isSameColor(currentColor, boundaryCol) || isSameColor(currentColor, fillCol)) {
                continue;
            }
            
            // Tìm leftmost và rightmost pixel trên scanline
            int leftmost = x;
            while (leftmost > 0) {
                Color leftColor = getPixel(leftmost - 1, y);
                if (isSameColor(leftColor, boundaryCol) || isSameColor(leftColor, fillCol)) {
                    break;
                }
                leftmost--;
            }
            
            int rightmost = x;
            while (rightmost < WINDOW_WIDTH - 1) {
                Color rightColor = getPixel(rightmost + 1, y);
                if (isSameColor(rightColor, boundaryCol) || isSameColor(rightColor, fillCol)) {
                    break;
                }
                rightmost++;
            }
            
            // Tô toàn bộ scanline
            for (int i = leftmost; i <= rightmost; i++) {
                setPixel(i, y, fillCol);
            }
            
            // Kiểm tra scanline trên và dưới
            for (int i = leftmost; i <= rightmost; i++) {
                // Scanline trên
                if (y + 1 < WINDOW_HEIGHT) {
                    Color aboveColor = getPixel(i, y + 1);
                    if (!isSameColor(aboveColor, boundaryCol) && !isSameColor(aboveColor, fillCol)) {
                        seedStack.push(Point(i, y + 1));
                    }
                }
                
                // Scanline dưới
                if (y - 1 >= 0) {
                    Color belowColor = getPixel(i, y - 1);
                    if (!isSameColor(belowColor, boundaryCol) && !isSameColor(belowColor, fillCol)) {
                        seedStack.push(Point(i, y - 1));
                    }
                }
            }
        }
    }
    
    static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
        BoundaryFillIterative* app = static_cast<BoundaryFillIterative*>(glfwGetWindowUserPointer(window));
        
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            
            int x = static_cast<int>(xpos);
            int y = static_cast<int>(WINDOW_HEIGHT - ypos);
            
            std::cout << "Left click at: (" << x << ", " << y << ") - Using simple iterative fill" << std::endl;
            app->boundaryFillIterative(x, y, app->fillColor, app->boundaryColor);
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            
            int x = static_cast<int>(xpos);
            int y = static_cast<int>(WINDOW_HEIGHT - ypos);
            
            std::cout << "Right click at: (" << x << ", " << y << ") - Using scanline fill" << std::endl;
            app->boundaryFillScanline(x, y, app->fillColor, app->boundaryColor);
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
        if (!loadPolygonFromFile("polygon.txt")) {
            // Tạo polygon mặc định nếu không đọc được file
            polygon.push_back(Point(300, 150));
            polygon.push_back(Point(500, 150));
            polygon.push_back(Point(600, 350));
            polygon.push_back(Point(400, 450));
            polygon.push_back(Point(200, 350));
            std::cout << "Using default polygon" << std::endl;
        }
        
        drawPolygon();
        
        std::cout << "Instructions:" << std::endl;
        std::cout << "- Left click: Simple iterative boundary fill" << std::endl;
        std::cout << "- Right click: Scanline-based boundary fill (faster)" << std::endl;
        std::cout << "- Press R to reset" << std::endl;
        std::cout << "- Press ESC to exit" << std::endl;
        
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
            
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                // Reset frame buffer
                for (int y = 0; y < WINDOW_HEIGHT; y++) {
                    for (int x = 0; x < WINDOW_WIDTH; x++) {
                        frameBuffer[y][x] = backgroundColor;
                    }
                }
                drawPolygon();
                std::cout << "Reset!" << std::endl;
            }
            
            render();
        }
    }
    
    ~BoundaryFillIterative() {
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }
};

int main() {
    BoundaryFillIterative app;
    
    if (!app.init()) {
        return -1;
    }
    
    app.run();
    
    return 0;
}