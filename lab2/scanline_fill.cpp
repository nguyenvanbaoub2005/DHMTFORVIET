#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

struct Edge {
    int ymax;
    float x;
    float dx;
};

struct Polygon {
    std::vector<Point> vertices;
    bool fillEnabled = false;
};

static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

GLFWwindow* window;
std::vector<Polygon> polygons;

// ----------------- kiểm tra điểm trong đa giác -----------------
bool pointInPolygon(const Polygon& poly, int px, int py) {
    bool inside = false;
    int n = poly.vertices.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        int xi = poly.vertices[i].x, yi = poly.vertices[i].y;
        int xj = poly.vertices[j].x, yj = poly.vertices[j].y;
        bool intersect = ((yi > py) != (yj > py)) &&
                         (px < (xj - xi) * (py - yi) / float(yj - yi) + xi);
        if (intersect) inside = !inside;
    }
    return inside;
}

// ----------------- callback chuột -----------------
void mouse_callback(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(win, &xpos, &ypos);

        // chuyển hệ tọa độ từ GLFW (gốc trên trái) sang OpenGL (gốc dưới trái)
        ypos = WINDOW_HEIGHT - ypos;

        for (auto& poly : polygons) {
            if (pointInPolygon(poly, (int)xpos, (int)ypos)) {
                poly.fillEnabled = !poly.fillEnabled;
                std::cout << "Toggle fill polygon -> " 
                          << (poly.fillEnabled ? "ON" : "OFF") << "\n";
                break; // chỉ đổi 1 polygon
            }
        }
    }
}

// ----------------- vẽ viền đa giác -----------------
void drawPolygonOutline(const Polygon& poly) {
    glColor3f(1,1,1);
    glBegin(GL_LINE_LOOP);
    for (auto& p : poly.vertices) {
        glVertex2i(p.x, p.y);
    }
    glEnd();
}

// ----------------- tô màu bằng scanline -----------------
void scanlineFill(const Polygon& poly) {
    const auto& polygon = poly.vertices;
    if (polygon.empty()) return;

    int ymin = polygon[0].y, ymax = polygon[0].y;
    for (auto& p : polygon) {
        ymin = std::min(ymin, p.y);
        ymax = std::max(ymax, p.y);
    }

    std::vector<std::vector<Edge>> ET(ymax + 1);
    for (size_t i = 0; i < polygon.size(); i++) {
        Point p1 = polygon[i];
        Point p2 = polygon[(i + 1) % polygon.size()];
        if (p1.y == p2.y) continue; // bỏ cạnh ngang

        Edge e;
        if (p1.y < p2.y) {
            e.ymax = p2.y;
            e.x = p1.x;
            e.dx = (float)(p2.x - p1.x) / (p2.y - p1.y);
            ET[p1.y].push_back(e);
        } else {
            e.ymax = p1.y;
            e.x = p2.x;
            e.dx = (float)(p1.x - p2.x) / (p1.y - p2.y);
            ET[p2.y].push_back(e);
        }
    }

    std::vector<Edge> AET;
    for (int y = ymin; y <= ymax; y++) {
        // thêm cạnh bắt đầu tại y
        for (auto& e : ET[y]) AET.push_back(e);

        // xóa cạnh hết hạn
        AET.erase(std::remove_if(AET.begin(), AET.end(),
                                 [y](Edge& e){ return e.ymax == y; }),
                  AET.end());

        // sắp xếp theo x
        std::sort(AET.begin(), AET.end(),
                  [](Edge& a, Edge& b){ return a.x < b.x; });

        // vẽ cặp cạnh
        glColor3f(1, 0, 0); // màu tô (đỏ)
        glBegin(GL_LINES);
        for (size_t i = 0; i + 1 < AET.size(); i += 2) {
            glVertex2i((int)AET[i].x, y);
            glVertex2i((int)AET[i+1].x, y);
        }
        glEnd();

        // update x
        for (auto& e : AET) e.x += e.dx;
    }
}

// ----------------- đọc dữ liệu đa giác từ file -----------------
bool loadPolygons(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    int numPoly;
    file >> numPoly;
    polygons.clear();
    for (int i = 0; i < numPoly; i++) {
        int n;
        file >> n;
        Polygon poly;
        for (int j = 0; j < n; j++) {
            int x,y;
            file >> x >> y;
            poly.vertices.push_back(Point(x,y));
        }
        polygons.push_back(poly);
    }
    return true;
}

// ----------------- main -----------------
int main() {
    if (!glfwInit()) return -1;
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Multi-Polygon Scanline Fill", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) return -1;

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glfwSetMouseButtonCallback(window, mouse_callback);

    if (!loadPolygons("polygons.txt")) {
        // mặc định: 2 đa giác
        Polygon p1;
        p1.vertices = { {200,200},{400,200},{350,350},{250,350} };
        polygons.push_back(p1);

        Polygon p2;
        p2.vertices = { {500,200},{600,300},{550,400} };
        polygons.push_back(p2);

        std::cout << "Using default polygons\n";
    }

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto& poly : polygons) {
            drawPolygonOutline(poly);
            if (poly.fillEnabled) scanlineFill(poly);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
