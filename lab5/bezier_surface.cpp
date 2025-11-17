#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>

// Kích thước cửa sổ
const int WIDTH = 1000;
const int HEIGHT = 800;

// Góc xoay
static float angleX = 30.0f;
static float angleY = 45.0f;
static float scale = 1.0f;
static bool wireframe = true;

// Độ phóng to/thu nhỏ
static float zoomFactor = 1.0f;

// Mật độ lưới
static int steps_u = 30; // Số bước chia theo u (có thể thay đổi)
static int steps_v = 30; // Số bước chia theo v (có thể thay đổi)

// Cấu trúc điểm 3D
typedef struct
{
    float x;
    float y;
    float z;
} CPoint3D;

// Hàm tính giai thừa
int fact(int n)
{
    if (n == 0)
        return 1;
    else
        return n * fact(n - 1);
}

// Hàm tính lũy thừa
float power(float a, int n)
{
    if (n == 0)
        return 1;
    else
        return a * power(a, n - 1);
}

// Hàm tính hệ số Bernstein
float BernStein(float t, int n, int k)
{
    float ckn, kq;
    ckn = fact(n) / (fact(k) * fact(n - k));
    kq = ckn * power(1 - t, n - k) * power(t, k);
    return kq;
}

// Hàm tính điểm trên mặt cong Bezier tại tham số (u, v)
// P là mảng 2D các điểm điều khiển [n+1][m+1]
CPoint3D BezierSurface(CPoint3D P[][4], float u, float v, int n, int m)
{
    CPoint3D Pt;
    float Bu, Bv;
    int i, j;

    Pt.x = 0;
    Pt.y = 0;
    Pt.z = 0;

    // Tính tổng có trọng số theo 2 chiều u và v
    for (i = 0; i <= n; i++)
    {
        for (j = 0; j <= m; j++)
        {
            Bu = BernStein(u, n, i);
            Bv = BernStein(v, m, j);
            Pt.x = Pt.x + P[i][j].x * Bu * Bv;
            Pt.y = Pt.y + P[i][j].y * Bu * Bv;
            Pt.z = Pt.z + P[i][j].z * Bu * Bv;
        }
    }

    return Pt;
}

// Hàm xoay điểm 3D quanh trục X
CPoint3D RotateX(CPoint3D P, float angle)
{
    CPoint3D result;
    float rad = angle * M_PI / 180.0f;
    result.x = P.x;
    result.y = P.y * cos(rad) - P.z * sin(rad);
    result.z = P.y * sin(rad) + P.z * cos(rad);
    return result;
}

// Hàm xoay điểm 3D quanh trục Y
CPoint3D RotateY(CPoint3D P, float angle)
{
    CPoint3D result;
    float rad = angle * M_PI / 180.0f;
    result.x = P.x * cos(rad) + P.z * sin(rad);
    result.y = P.y;
    result.z = -P.x * sin(rad) + P.z * cos(rad);
    return result;
}

// Hàm chiếu phối cảnh
void Project(CPoint3D P3D, int &x2D, int &y2D)
{
    float d = 500.0f; // Khoảng cách từ mắt đến màn hình
    float scale_factor = d / (d + P3D.z);
    x2D = (int)(P3D.x * scale_factor * zoomFactor) + WIDTH / 2;
    y2D = (int)(P3D.y * scale_factor * zoomFactor) + HEIGHT / 2;
}

// Hàm vẽ mặt cong Bezier
void DrawBezierSurface(CPoint3D P[][4], int n, int m)
{
    float du = 1.0f / steps_u;
    float dv = 1.0f / steps_v;

    // Vẽ lưới mặt cong
    for (int i = 0; i < steps_u; i++)
    {
        for (int j = 0; j < steps_v; j++)
        {
            float u = i * du;
            float v = j * dv;

            // Tính 4 điểm của một ô lưới
            CPoint3D P00 = BezierSurface(P, u, v, n, m);
            CPoint3D P10 = BezierSurface(P, u + du, v, n, m);
            CPoint3D P11 = BezierSurface(P, u + du, v + dv, n, m);
            CPoint3D P01 = BezierSurface(P, u, v + dv, n, m);

            // Xoay và chiếu các điểm
            P00 = RotateX(P00, angleX);
            P00 = RotateY(P00, angleY);
            P10 = RotateX(P10, angleX);
            P10 = RotateY(P10, angleY);
            P11 = RotateX(P11, angleX);
            P11 = RotateY(P11, angleY);
            P01 = RotateX(P01, angleX);
            P01 = RotateY(P01, angleY);

            int x00, y00, x10, y10, x11, y11, x01, y01;
            Project(P00, x00, y00);
            Project(P10, x10, y10);
            Project(P11, x11, y11);
            Project(P01, x01, y01);

            if (wireframe)
            {
                // Vẽ khung lưới
                glColor3f(0.0f, 0.0f, 0.0f);
                glBegin(GL_LINE_LOOP);
                glVertex2i(x00, y00);
                glVertex2i(x10, y10);
                glVertex2i(x11, y11);
                glVertex2i(x01, y01);
                glEnd();
            }
            else
            {
                // Vẽ mặt đầy màu
                // Tính màu dựa trên vị trí (tạo hiệu ứng gradient)
                float colorR = 0.3f + 0.7f * u;
                float colorG = 0.3f + 0.7f * v;
                float colorB = 0.5f;

                glColor3f(colorR, colorG, colorB);
                glBegin(GL_QUADS);
                glVertex2i(x00, y00);
                glVertex2i(x10, y10);
                glVertex2i(x11, y11);
                glVertex2i(x01, y01);
                glEnd();

                // Vẽ đường viền
                glColor3f(0.0f, 0.0f, 0.0f);
                glBegin(GL_LINE_LOOP);
                glVertex2i(x00, y00);
                glVertex2i(x10, y10);
                glVertex2i(x11, y11);
                glVertex2i(x01, y01);
                glEnd();
            }
        }
    }
}

// Vẽ các điểm điều khiển
void DrawControlPoints(CPoint3D P[][4], int n, int m)
{
    glColor3f(1.0f, 0.0f, 0.0f); // Màu đỏ
    glPointSize(6.0f);
    glBegin(GL_POINTS);

    for (int i = 0; i <= n; i++)
    {
        for (int j = 0; j <= m; j++)
        {
            CPoint3D pt = P[i][j];
            pt = RotateX(pt, angleX);
            pt = RotateY(pt, angleY);
            int x2D, y2D;
            Project(pt, x2D, y2D);
            glVertex2i(x2D, y2D);
        }
    }
    glEnd();

    // Vẽ lưới điểm điều khiển
    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(1.0f);

    // Vẽ theo hàng
    for (int i = 0; i <= n; i++)
    {
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= m; j++)
        {
            CPoint3D pt = P[i][j];
            pt = RotateX(pt, angleX);
            pt = RotateY(pt, angleY);
            int x2D, y2D;
            Project(pt, x2D, y2D);
            glVertex2i(x2D, y2D);
        }
        glEnd();
    }

    // Vẽ theo cột
    for (int j = 0; j <= m; j++)
    {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= n; i++)
        {
            CPoint3D pt = P[i][j];
            pt = RotateX(pt, angleX);
            pt = RotateY(pt, angleY);
            int x2D, y2D;
            Project(pt, x2D, y2D);
            glVertex2i(x2D, y2D);
        }
        glEnd();
    }
}

// Hàm vẽ trục tọa độ 3D
void DrawAxes()
{
    float axisLength = 200.0f;

    // Trục X (màu đỏ)
    CPoint3D origin = {0, 0, 0};
    CPoint3D xAxis = {axisLength, 0, 0};

    origin = RotateX(origin, angleX);
    origin = RotateY(origin, angleY);
    xAxis = RotateX(xAxis, angleX);
    xAxis = RotateY(xAxis, angleY);

    int x0, y0, x1, y1;
    Project(origin, x0, y0);
    Project(xAxis, x1, y1);

    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2i(x0, y0);
    glVertex2i(x1, y1);
    glEnd();

    // Trục Y (màu xanh lá)
    CPoint3D yAxis = {0, axisLength, 0};
    yAxis = RotateX(yAxis, angleX);
    yAxis = RotateY(yAxis, angleY);
    Project(yAxis, x1, y1);

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2i(x0, y0);
    glVertex2i(x1, y1);
    glEnd();

    // Trục Z (màu xanh dương)
    CPoint3D zAxis = {0, 0, axisLength};
    zAxis = RotateX(zAxis, angleX);
    zAxis = RotateY(zAxis, angleY);
    Project(zAxis, x1, y1);

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex2i(x0, y0);
    glVertex2i(x1, y1);
    glEnd();
}

// Hàm hiển thị
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Vẽ trục tọa độ
    DrawAxes();

    // Định nghĩa lưới các điểm điều khiển 4x4 (n=3, m=3)
    int n = 3, m = 3;
    CPoint3D P[4][4];

    // Khởi tạo các điểm điều khiển tạo thành mặt cong hình yên ngựa
    float spacing = 80.0f;
    float offsetX = 150.0f; // Dịch chuyển mặt cong ra xa trục tọa độ
    float offsetY = 100.0f;
    float offsetZ = 50.0f;

    for (int i = 0; i <= n; i++)
    {
        for (int j = 0; j <= m; j++)
        {
            P[i][j].x = (i - 1.5f) * spacing + offsetX;
            P[i][j].y = (j - 1.5f) * spacing + offsetY;

            // Tạo độ cao z tạo hiệu ứng mặt cong
            float xi = (i - 1.5f) / 1.5f;
            float yj = (j - 1.5f) / 1.5f;
            P[i][j].z = 50.0f * (xi * xi - yj * yj) + offsetZ; // Hình yên ngựa
        }
    }

    // Vẽ mặt cong Bezier
    DrawBezierSurface(P, n, m);

    // Vẽ các điểm điều khiển
    DrawControlPoints(P, n, m);

    glFlush();
}

// Hàm khởi tạo
void init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Nền trắng
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
}

// Hàm xử lý phím
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_LEFT:
            angleY += 5.0f;
            break;
        case GLFW_KEY_RIGHT:
            angleY -= 5.0f;
            break;
        case GLFW_KEY_UP:
            angleX += 5.0f;
            break;
        case GLFW_KEY_DOWN:
            angleX -= 5.0f;
            break;
        case GLFW_KEY_W:
            wireframe = !wireframe;
            std::cout << "Wireframe mode: " << (wireframe ? "ON" : "OFF") << std::endl;
            break;
        case GLFW_KEY_Q:
            if (steps_u > 5)
            {
                steps_u -= 5;
                std::cout << "Delta U giam: steps_u = " << steps_u << " (du = " << (1.0f / steps_u) << ")" << std::endl;
            }
            break;
        case GLFW_KEY_E:
            if (steps_u < 100)
            {
                steps_u += 5;
                std::cout << "Delta U tang: steps_u = " << steps_u << " (du = " << (1.0f / steps_u) << ")" << std::endl;
            }
            break;
        case GLFW_KEY_A:
            if (steps_v > 5)
            {
                steps_v -= 5;
                std::cout << "Delta V giam: steps_v = " << steps_v << " (dv = " << (1.0f / steps_v) << ")" << std::endl;
            }
            break;
        case GLFW_KEY_D:
            if (steps_v < 100)
            {
                steps_v += 5;
                std::cout << "Delta V tang: steps_v = " << steps_v << " (dv = " << (1.0f / steps_v) << ")" << std::endl;
            }
            break;
        case GLFW_KEY_Z:
            if (zoomFactor > 0.2f)
            {
                zoomFactor -= 0.1f;
                std::cout << "Thu nho: zoom = " << zoomFactor << std::endl;
            }
            break;
        case GLFW_KEY_X:
            if (zoomFactor < 3.0f)
            {
                zoomFactor += 0.1f;
                std::cout << "Phong to: zoom = " << zoomFactor << std::endl;
            }
            break;
        case GLFW_KEY_R:
            angleX = 30.0f;
            angleY = 45.0f;
            steps_u = 30;
            steps_v = 30;
            zoomFactor = 1.0f;
            std::cout << "Reset view, mat do va zoom" << std::endl;
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
}

// Hàm xử lý thay đổi kích thước cửa sổ
void reshape(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
}

// Hàm main
int main()
{
    // Khởi tạo GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Tạo cửa sổ
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Mat cong Bezier 3D", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Khởi tạo GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // Thiết lập callback
    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, key_callback);

    // Khởi tạo OpenGL
    init();

    std::cout << "=== CHUONG TRINH VE MAT CONG BEZIER 3D ===" << std::endl;
    std::cout << "Cac phim dieu khien:" << std::endl;
    std::cout << "  XOA MAT:" << std::endl;
    std::cout << "    - Mui ten TRAI/PHAI: Xoay quanh truc Y" << std::endl;
    std::cout << "    - Mui ten LEN/XUONG: Xoay quanh truc X" << std::endl;
    std::cout << "  MAT DO LUOI:" << std::endl;
    std::cout << "    - Q: Giam Delta U (tang mat do theo U)" << std::endl;
    std::cout << "    - E: Tang Delta U (giam mat do theo U)" << std::endl;
    std::cout << "    - A: Giam Delta V (tang mat do theo V)" << std::endl;
    std::cout << "    - D: Tang Delta V (giam mat do theo V)" << std::endl;
    std::cout << "  PHONG TO/THU NHO:" << std::endl;
    std::cout << "    - Z: Thu nho" << std::endl;
    std::cout << "    - X: Phong to" << std::endl;
    std::cout << "  KHAC:" << std::endl;
    std::cout << "    - W: Bat/tat che do khung luoi (wireframe)" << std::endl;
    std::cout << "    - R: Reset tat ca" << std::endl;
    std::cout << "    - ESC: Thoat chuong trinh" << std::endl;
    std::cout << "  TRUC TOA DO:" << std::endl;
    std::cout << "    - Truc X: Mau DO" << std::endl;
    std::cout << "    - Truc Y: Mau XANH LA" << std::endl;
    std::cout << "    - Truc Z: Mau XANH DUONG" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Mat do hien tai: steps_u = " << steps_u << ", steps_v = " << steps_v << std::endl;
    std::cout << "Zoom hien tai: " << zoomFactor << std::endl;

    // Vòng lặp chính
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
