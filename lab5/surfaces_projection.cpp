#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

static int windowWidth = 800;
static int windowHeight = 600;
static int currentShape = 0; // 0: Sphere, 1: Ellipsoid, 2: Hyperboloid, 3: Toroid
static int xc = 400;         // Tâm màn hình
static int yc = 300;
static float angleX = 0.0f; // Góc xoay quanh trục X (xoay dọc)
static float angleY = 0.0f; // Góc xoay quanh trục Y (xoay ngang)
static float scale = 1.0f;  // Hệ số phóng to/thu nhỏ

// Biến Delta cho mật độ điểm
static double Delta_U = 0.1;
static double Delta_V = 0.1;
static double Delta_Hyperboloid = 0.05;

//---------------------------------------------------------
// Cấu trúc điểm 2D và 3D
//---------------------------------------------------------
struct Point2D
{
    int x, y;
};

struct Point3D
{
    double x, y, z;
};

//---------------------------------------------------------
// Hàm xoay điểm 3D
//---------------------------------------------------------
// Xoay quanh trục X
Point3D RotateX(Point3D P, float angle)
{
    Point3D result;
    float rad = angle * M_PI / 180.0f;
    result.x = P.x;
    result.y = P.y * cos(rad) - P.z * sin(rad);
    result.z = P.y * sin(rad) + P.z * cos(rad);
    return result;
}

// Xoay quanh trục Y
Point3D RotateY(Point3D P, float angle)
{
    Point3D result;
    float rad = angle * M_PI / 180.0f;
    result.x = P.x * cos(rad) + P.z * sin(rad);
    result.y = P.y;
    result.z = -P.x * sin(rad) + P.z * cos(rad);
    return result;
}

// Xoay điểm theo cả hai góc
Point3D RotatePoint(Point3D P)
{
    P = RotateY(P, angleY);
    P = RotateX(P, angleX);
    return P;
}

//---------------------------------------------------------
// Hàm chiếu từ 3D sang 2D (phép chiếu song song)
//---------------------------------------------------------

// Phép chiếu Cabinet (góc 45 độ, có áp dụng scale)
Point2D ChieuCabinet(Point3D P)
{
    Point2D P1;
    double alpha = M_PI / 4; // 45 độ
    double L = 0.5;          // Hệ số co ngắn
    P1.x = (int)(P.x * scale - L * P.z * scale * cos(alpha));
    P1.y = (int)(P.y * scale - L * P.z * scale * sin(alpha));
    return P1;
}

// Sử dụng phép chiếu nào
Point2D Chieu(Point3D P)
{
    return ChieuCabinet(P); 
}

//---------------------------------------------------------
// Hàm vẽ điểm pixel
//---------------------------------------------------------
void putpixel(int x, int y, float r, float g, float b)
{
    glBegin(GL_POINTS);
    glColor3f(r, g, b);
    glVertex2i(x, y);
    glEnd();
}

//---------------------------------------------------------
// Khởi tạo OpenGL
//---------------------------------------------------------
void initGL()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glPointSize(1.0f);
}

//---------------------------------------------------------
// Thiết lập phép chiếu 2D (vẽ điểm)
//---------------------------------------------------------
void setupProjection()
{
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//---------------------------------------------------------
// Callback khi resize cửa sổ
//---------------------------------------------------------
void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    xc = width / 2;
    yc = height / 2;
    setupProjection();
}

//---------------------------------------------------------
// Vẽ mặt cầu (Sphere) theo thuật toán
// x = R*cos(u)*cos(v)
// y = R*sin(u)*cos(v)
// z = R*sin(v)
//---------------------------------------------------------
void DrawSphere(float R)
{
    Point3D P;
    Point2D P1;
    double Pi_2 = M_PI / 2;

    for (double v = -Pi_2; v < Pi_2; v += Delta_V)
    {
        for (double u = 0; u < 2 * M_PI; u += Delta_U)
        {
            P.x = R * cos(u) * cos(v);
            P.y = R * sin(u) * cos(v);
            P.z = R * sin(v);

            P = RotatePoint(P); // Xoay điểm
            P1 = Chieu(P);
            putpixel(xc + P1.x, yc + P1.y, 0.2f, 0.6f, 1.0f); // Màu xanh dương
        }
    }
}

//---------------------------------------------------------
// Vẽ Ellipsoid theo thuật toán
// x = Rx*cos(u)*cos(v)
// y = Ry*sin(u)*cos(v)
// z = Rz*sin(v)
//---------------------------------------------------------
void DrawEllipsoid(float Rx, float Ry, float Rz)
{
    Point3D P;
    Point2D P1;
    double Pi_2 = M_PI / 2;

    for (double v = -Pi_2; v < Pi_2; v += Delta_V)
    {
        for (double u = 0; u < 2 * M_PI; u += Delta_U)
        {
            P.x = Rx * cos(u) * cos(v);
            P.y = Ry * sin(u) * cos(v);
            P.z = Rz * sin(v);

            P = RotatePoint(P); // Xoay điểm
            P1 = Chieu(P);
            putpixel(xc + P1.x, yc + P1.y, 1.0f, 0.5f, 0.2f); // Màu cam
        }
    }
}

//---------------------------------------------------------
// Vẽ Hyperboloid theo thuật toán
// x = u
// y = v
// z = u² - v²
// với -1 ≤ u, v ≤ 1
//---------------------------------------------------------
void DrawHyperboloid()
{
    Point3D P;
    Point2D P1;
    double scale = 100.0; // Hệ số phóng đại

    for (double u = -1; u <= 1; u += Delta_Hyperboloid)
    {
        for (double v = -1; v <= 1; v += Delta_Hyperboloid)
        {
            P.x = u * scale;
            P.y = v * scale;
            P.z = (u * u - v * v) * scale;

            P = RotatePoint(P); // Xoay điểm
            P1 = Chieu(P);
            putpixel(xc + P1.x, yc + P1.y, 0.2f, 0.9f, 0.3f); // Màu xanh lá
        }
    }
}

//---------------------------------------------------------
// Vẽ Toroid (mặt xuyến) theo thuật toán
// x = (R + a*cos(v))*cos(u)
// y = (R + a*cos(v))*sin(u)
// z = a*sin(v)
// với 0 ≤ u ≤ 2π; -π/2 ≤ v ≤ π/2
//---------------------------------------------------------
void DrawToroid(float R, float a)
{
    Point3D P;
    Point2D P1;
    double Pi_2 = M_PI / 2;

    for (double v = -Pi_2; v < Pi_2; v += Delta_V)
    {
        for (double u = 0; u < 2 * M_PI; u += Delta_U)
        {
            P.x = (R + a * cos(v)) * cos(u);
            P.y = (R + a * cos(v)) * sin(u);
            P.z = a * sin(v);

            P = RotatePoint(P); // Xoay điểm
            P1 = Chieu(P);
            putpixel(xc + P1.x, yc + P1.y, 0.9f, 0.2f, 0.7f); // Màu hồng tím
        }
    }
}

//---------------------------------------------------------
// Vẽ trục tọa độ để tham khảo
//---------------------------------------------------------
void DrawAxes()
{
    glLineWidth(2.0f);

    // Trục X - Đỏ
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2i(xc, yc);
    glVertex2i(xc + 100, yc);
    glEnd();

    // Trục Y - Xanh lá
    glBegin(GL_LINES);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2i(xc, yc);
    glVertex2i(xc, yc + 100);
    glEnd();

    // Trục Z - Xanh dương (chiếu xuống)
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2i(xc, yc);
    Point3D P = {0, 0, 100};
    Point2D P1 = Chieu(P);
    glVertex2i(xc + P1.x, yc + P1.y);
    glEnd();

    glLineWidth(1.0f);
}

//---------------------------------------------------------
// Hàm hiển thị
//---------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Vẽ trục tọa độ
    DrawAxes();

    // Vẽ hình tương ứng
    switch (currentShape)
    {
    case 0: // Sphere
        DrawSphere(100.0f);
        break;
    case 1: // Ellipsoid
        DrawEllipsoid(100.0f, 80.0f, 120.0f);
        break;
    case 2: // Hyperboloid
        DrawHyperboloid();
        break;
    case 3: // Toroid
        DrawToroid(100.0f, 40.0f);
        break;
    }
}

//---------------------------------------------------------
// Xử lý phím
//---------------------------------------------------------
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        // Xoay hình
        if (key == GLFW_KEY_RIGHT)
            angleY += 5;
        else if (key == GLFW_KEY_LEFT)
            angleY -= 5;
        else if (key == GLFW_KEY_UP)
            angleX += 5;
        else if (key == GLFW_KEY_DOWN)
            angleX -= 5;

        // Chuyển đổi hình
        else if (key == GLFW_KEY_1)
        {
            currentShape = 0;
            std::cout << "Hinh dang hien thi: SPHERE (Mat cau)\n";
        }
        else if (key == GLFW_KEY_2)
        {
            currentShape = 1;
            std::cout << "Hinh dang hien thi: ELLIPSOID (Elip xoay)\n";
        }
        else if (key == GLFW_KEY_3)
        {
            currentShape = 2;
            std::cout << "Hinh dang hien thi: HYPERBOLOID (Mat Hyperbol)\n";
        }
        else if (key == GLFW_KEY_4)
        {
            currentShape = 3;
            std::cout << "Hinh dang hien thi: TOROID (Mat xuyen/Banh vong)\n";
        }

        // Điều chỉnh Delta_U (Q: tăng - ít điểm hơn, W: giảm - nhiều điểm hơn)
        else if (key == GLFW_KEY_Q)
        {
            if (currentShape == 2) // Hyperboloid
            {
                Delta_Hyperboloid += 0.01;
                if (Delta_Hyperboloid > 0.5)
                    Delta_Hyperboloid = 0.5;
                std::cout << "Delta_Hyperboloid = " << Delta_Hyperboloid << "\n";
            }
            else // Sphere, Ellipsoid, Toroid
            {
                Delta_U += 0.01;
                if (Delta_U > 0.5)
                    Delta_U = 0.5;
                std::cout << "Delta_U = " << Delta_U << "\n";
            }
        }
        else if (key == GLFW_KEY_W)
        {
            if (currentShape == 2) // Hyperboloid
            {
                Delta_Hyperboloid -= 0.01;
                if (Delta_Hyperboloid < 0.01)
                    Delta_Hyperboloid = 0.01;
                std::cout << "Delta_Hyperboloid = " << Delta_Hyperboloid << "\n";
            }
            else // Sphere, Ellipsoid, Toroid
            {
                Delta_U -= 0.01;
                if (Delta_U < 0.01)
                    Delta_U = 0.01;
                std::cout << "Delta_U = " << Delta_U << "\n";
            }
        }

        // Điều chỉnh Delta_V (A: tăng - ít điểm hơn, S: giảm - nhiều điểm hơn)
        else if (key == GLFW_KEY_A)
        {
            Delta_V += 0.01;
            if (Delta_V > 0.5)
                Delta_V = 0.5;
            std::cout << "Delta_V = " << Delta_V << "\n";
        }
        else if (key == GLFW_KEY_S)
        {
            Delta_V -= 0.01;
            if (Delta_V < 0.01)
                Delta_V = 0.01;
            std::cout << "Delta_V = " << Delta_V << "\n";
        }

        // Phóng to/thu nhỏ
        else if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) // Phím + hoặc +
        {
            scale += 0.1f;
            if (scale > 3.0f)
                scale = 3.0f;
            std::cout << "Scale = " << scale << " (Phong to)\n";
        }
        else if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) // Phím - hoặc -
        {
            scale -= 0.1f;
            if (scale < 0.3f)
                scale = 0.3f;
            std::cout << "Scale = " << scale << " (Thu nho)\n";
        }
        else if (key == GLFW_KEY_0 || key == GLFW_KEY_KP_0) // Phím 0 - Reset scale
        {
            scale = 1.0f;
            std::cout << "Scale = " << scale << " (Mac dinh)\n";
        }

        // Thoát
        else if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

//---------------------------------------------------------
// Hàm main
//---------------------------------------------------------
int main(int argc, char **argv)
{
    // Khởi tạo GLFW
    if (!glfwInit())
    {
        std::cerr << "Khong the khoi tao GLFW!\n";
        return -1;
    }

    // Tạo cửa sổ
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight,
                                          "LAB 5.2 - Surfaces of Revolution (Projection Method)",
                                          NULL, NULL);
    if (!window)
    {
        std::cerr << "Khong the tao cua so!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Khởi tạo GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Khong the khoi tao GLEW!\n";
        return -1;
    }

    // Thiết lập callback
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Khởi tạo OpenGL
    initGL();
    setupProjection();

    // In hướng dẫn
    std::cout << "===========================================\n";
    std::cout << "  LAB 5.2 - CAC MAT TRON XOAY\n";
    std::cout << "  (Su dung phep chieu va ve diem)\n";
    std::cout << "===========================================\n";
    std::cout << "Phim mui ten: Xoay hinh\n";
    std::cout << "  <- -> : Xoay ngang (trai/phai)\n";
    std::cout << "  ^  v  : Xoay doc (len/xuong)\n";
    std::cout << "Phim 1: Sphere (Mat cau)\n";
    std::cout << "Phim 2: Ellipsoid (Elip xoay)\n";
    std::cout << "Phim 3: Hyperboloid (Mat Hyperbol)\n";
    std::cout << "Phim 4: Toroid (Mat xuyen/Banh vong)\n";
    std::cout << "-------------------------------------------\n";
    std::cout << "Dieu chinh mat do diem:\n";
    std::cout << "  Q: Tang Delta_U (thua diem hon)\n";
    std::cout << "  W: Giam Delta_U (dac diem hon)\n";
    std::cout << "  A: Tang Delta_V (thua diem hon)\n";
    std::cout << "  S: Giam Delta_V (dac diem hon)\n";
    std::cout << "-------------------------------------------\n";
    std::cout << "Phong to / Thu nho:\n";
    std::cout << "  +: Phong to hinh\n";
    std::cout << "  -: Thu nho hinh\n";
    std::cout << "  0: Dat lai kich thuoc mac dinh\n";
    std::cout << "Phim ESC: Thoat\n";
    std::cout << "===========================================\n";
    std::cout << "Scale (Phong to) = " << scale << "\n";
    std::cout << "===========================================\n";
    std::cout << "Hinh dang hien thi: SPHERE (Mat cau)\n";

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
