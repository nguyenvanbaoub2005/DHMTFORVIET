#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

static float angleX = 20.0f;
static float angleY = 30.0f;
static float scale = 1.0f; // Hệ số phóng to/thu nhỏ
static int windowWidth = 1000;
static int windowHeight = 600;
static int xc = 250; // Tâm vẽ hình trụ
static int yc = 300;
static int xc2 = 750; // Tâm vẽ hình nón
static int yc2 = 300;
static int displayMode = 1; // 1:  hình trụ, 2:  hình nón

// Các tham số Delta để điều chỉnh mật độ điểm
static double Delta_U = 0.06;      // Bước lặp theo góc u
static double Delta_V = 0.03;      // Bước lặp theo chiều cao v
static double Delta_U_Cone = 0.03; // Bước lặp cho hình nón
static double Delta_V_Cone = 0.1;

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
Point3D RotateX(Point3D P, float angle)
{
    Point3D result;
    float rad = angle * M_PI / 180.0f;
    result.x = P.x;
    result.y = P.y * cos(rad) - P.z * sin(rad);
    result.z = P.y * sin(rad) + P.z * cos(rad);
    return result;
}

Point3D RotateY(Point3D P, float angle)
{
    Point3D result;
    float rad = angle * M_PI / 180.0f;
    result.x = P.x * cos(rad) + P.z * sin(rad);
    result.y = P.y;
    result.z = -P.x * sin(rad) + P.z * cos(rad);
    return result;
}

Point3D RotatePoint(Point3D P)
{
    P = RotateY(P, angleY);
    P = RotateX(P, angleX);
    return P;
}

//---------------------------------------------------------
// Phép chiếu Cabinet (có áp dụng scale)
//---------------------------------------------------------
Point2D ChieuCabinet(Point3D P)
{
    Point2D P1;
    double alpha = M_PI / 4; // 45 độ
    double L = 0.5;          // Hệ số co ngắn
    P1.x = (int)(P.x * scale - L * P.z * scale * cos(alpha));
    P1.y = (int)(P.y * scale - L * P.z * scale * sin(alpha));
    return P1;
}

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
// Vẽ đường thẳng
//---------------------------------------------------------
void drawLine(int x1, int y1, int x2, int y2, float r, float g, float b)
{
    glBegin(GL_LINES);
    glColor3f(r, g, b);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glEnd();
}

//---------------------------------------------------------
// Vẽ trục tọa độ 3D sau khi chiếu
//---------------------------------------------------------
void DrawAxes(int centerX, int centerY, float length)
{
    Point3D origin = {0, 0, 0};
    Point3D axisX = {length, 0, 0};
    Point3D axisY = {0, length, 0};
    Point3D axisZ = {0, 0, length};

    // Xoay các trục
    axisX = RotatePoint(axisX);
    axisY = RotatePoint(axisY);
    axisZ = RotatePoint(axisZ);
    origin = RotatePoint(origin);

    // Chiếu các điểm
    Point2D p0 = Chieu(origin);
    Point2D px = Chieu(axisX);
    Point2D py = Chieu(axisY);
    Point2D pz = Chieu(axisZ);

    glLineWidth(2.0f);

    // Trục X - Đỏ
    drawLine(centerX + p0.x, centerY + p0.y,
             centerX + px.x, centerY + px.y,
             1.0f, 0.0f, 0.0f);

    // Trục Y - Xanh lá
    drawLine(centerX + p0.x, centerY + p0.y,
             centerX + py.x, centerY + py.y,
             0.0f, 1.0f, 0.0f);

    // Trục Z - Xanh dương
    drawLine(centerX + p0.x, centerY + p0.y,
             centerX + pz.x, centerY + pz.y,
             0.0f, 0.0f, 1.0f);

    glLineWidth(1.0f);
}

//---------------------------------------------------------
// Hàm khởi tạo OpenGL
//---------------------------------------------------------
void initGL()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glPointSize(1.0f);
}

//---------------------------------------------------------
// Thiết lập phép chiếu 2D
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
    setupProjection();
}

//---------------------------------------------------------
// Vẽ hình trụ
// Mặt có quy tắc:
// P.x = R*cos(u); P.y = R*sin(u); P.z = v*h;
// với u từ 0 đến 2π, v từ 0 đến 1
//---------------------------------------------------------
void DrawCylinder(float R, float h, int centerX, int centerY)
{
    Point3D P;
    Point2D P1;

    // Vẽ mặt xung quanh của trụ
    for (double u = 0; u < 2 * M_PI; u += Delta_U)
    {
        for (double v = 0; v < 1; v += Delta_V)
        {
            P.x = R * cos(u);
            P.y = R * sin(u);
            P.z = v * h;

            P = RotatePoint(P);
            P1 = Chieu(P);
            putpixel(centerX + P1.x, centerY + P1.y, 0.2f, 0.6f, 1.0f); // Xanh dương
        }
    }

    // Vẽ đáy dưới (v=0, z=0)
    for (double u = 0; u < 2 * M_PI; u += Delta_U)
    {
        for (double r = 0; r <= R; r += 2.0)
        {
            P.x = r * cos(u);
            P.y = r * sin(u);
            P.z = 0;

            P = RotatePoint(P);
            P1 = Chieu(P);
            putpixel(centerX + P1.x, centerY + P1.y, 1.0f, 0.5f, 0.0f); // Cam
        }
    }

    // Vẽ đáy trên (v=1, z=h)
    for (double u = 0; u < 2 * M_PI; u += Delta_U)
    {
        for (double r = 0; r <= R; r += 2.0)
        {
            P.x = r * cos(u);
            P.y = r * sin(u);
            P.z = h;

            P = RotatePoint(P);
            P1 = Chieu(P);
            putpixel(centerX + P1.x, centerY + P1.y, 0.0f, 0.8f, 0.4f); // Xanh lá
        }
    }
}

//---------------------------------------------------------
// Vẽ hình nón
// Mặt nón (Cone)
// P.x = v*R*cos(u); P.y = v*R*sin(u); P.z = (1-v)*h;
// với u từ 0 đến 2π, v từ 0 đến 1
// v=0: đỉnh (P₀) tại z=h (dưới)
// v=1: đáy (P₁) tại z=0 (trên)
//---------------------------------------------------------
void DrawCone(float R, float h, int centerX, int centerY)
{
    Point3D P;
    Point2D P1;

    // Vẽ mặt bên của nón (từ đỉnh lên đáy)
    for (double u = 0; u < 2 * M_PI; u += Delta_U_Cone)
    {
        for (double v = 0; v <= 1; v += Delta_V_Cone)
        {
            P.x = v * R * cos(u);
            P.y = v * R * sin(u);
            P.z = (1 - v) * h; // Đỉnh ở z=h, đáy ở z=0

            P = RotatePoint(P);
            P1 = Chieu(P);
            putpixel(centerX + P1.x, centerY + P1.y, 1.0f, 0.2f, 0.2f); // Đỏ
        }
    }

    // Vẽ đáy tròn ở trên (z=0, v=1)
    for (double u = 0; u < 2 * M_PI; u += Delta_U_Cone)
    {
        for (double r = 0; r <= R; r += 2.0)
        {
            P.x = r * cos(u);
            P.y = r * sin(u);
            P.z = 0; // Đáy ở trên

            P = RotatePoint(P);
            P1 = Chieu(P);
            putpixel(centerX + P1.x, centerY + P1.y, 0.9f, 0.7f, 0.1f); // Vàng
        }
    }
}

//---------------------------------------------------------
// Hàm hiển thị
//---------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    // Chế độ 1: Chỉ hiển thị hình trụ
    if (displayMode == 1)
    {
        DrawAxes(centerX, centerY, 80.0f);
        DrawCylinder(60.0f, 120.0f, centerX, centerY);
    }
    // Chế độ 2: Chỉ hiển thị hình nón
    else if (displayMode == 2)
    {
        DrawAxes(centerX, centerY, 80.0f);
        DrawCone(60.0f, 120.0f, centerX, centerY);
    }
}

//---------------------------------------------------------
// Xử lý phím mũi tên để quay
//---------------------------------------------------------
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        // Xoay hình
        if (key == GLFW_KEY_RIGHT)
            angleY -= 5;
        else if (key == GLFW_KEY_LEFT)
            angleY += 5;
        else if (key == GLFW_KEY_UP)
            angleX -= 5;
        else if (key == GLFW_KEY_DOWN)
            angleX += 5;

        // Điều chỉnh Delta_U (mật độ theo góc)
        else if (key == GLFW_KEY_Q)
        {
            if (displayMode == 1)
            {
                Delta_U += 0.01;
                if (Delta_U > 0.5)
                    Delta_U = 0.5;
                std::cout << "Delta_U (Tru) = " << Delta_U << "\n";
            }
            else
            {
                Delta_U_Cone += 0.01;
                if (Delta_U_Cone > 0.5)
                    Delta_U_Cone = 0.5;
                std::cout << "Delta_U (Non) = " << Delta_U_Cone << "\n";
            }
        }
        else if (key == GLFW_KEY_W)
        {
            if (displayMode == 1)
            {
                Delta_U -= 0.01;
                if (Delta_U < 0.01)
                    Delta_U = 0.01;
                std::cout << "Delta_U (Tru) = " << Delta_U << "\n";
            }
            else
            {
                Delta_U_Cone -= 0.01;
                if (Delta_U_Cone < 0.01)
                    Delta_U_Cone = 0.01;
                std::cout << "Delta_U (Non) = " << Delta_U_Cone << "\n";
            }
        }

        // Điều chỉnh Delta_V (mật độ theo chiều cao)
        else if (key == GLFW_KEY_A)
        {
            if (displayMode == 1)
            {
                Delta_V += 0.01;
                if (Delta_V > 0.5)
                    Delta_V = 0.5;
                std::cout << "Delta_V (Tru) = " << Delta_V << "\n";
            }
            else
            {
                Delta_V_Cone += 0.01;
                if (Delta_V_Cone > 0.5)
                    Delta_V_Cone = 0.5;
                std::cout << "Delta_V (Non) = " << Delta_V_Cone << "\n";
            }
        }
        else if (key == GLFW_KEY_S)
        {
            if (displayMode == 1)
            {
                Delta_V -= 0.01;
                if (Delta_V < 0.01)
                    Delta_V = 0.01;
                std::cout << "Delta_V (Tru) = " << Delta_V << "\n";
            }
            else
            {
                Delta_V_Cone -= 0.01;
                if (Delta_V_Cone < 0.01)
                    Delta_V_Cone = 0.01;
                std::cout << "Delta_V (Non) = " << Delta_V_Cone << "\n";
            }
        }

        // Phóng to/thu nhỏ
        else if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) // Phím + hoặc =
        {
            scale += 0.1f;
            if (scale > 3.0f)
                scale = 3.0f;
            std::cout << "Scale = " << scale << " (Phong to)\n";
        }
        else if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) // Phím - hoặc _
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

        else if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (action == GLFW_PRESS)
    {
        // Phím 1: Chỉ hiển thị hình trụ
        if (key == GLFW_KEY_1)
        {
            displayMode = 1;
            std::cout << "Che do: HIEN THI HINH TRU\n";
        }
        // Phím 2: Chỉ hiển thị hình nón
        else if (key == GLFW_KEY_2)
        {
            displayMode = 2;
            std::cout << "Che do: HIEN THI HINH NON\n";
        }
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
        return -1;
    }

    // Tạo cửa sổ
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Cylinder & Cone (Projection Method)", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Đặt context hiện tại
    glfwMakeContextCurrent(window);

    // Khởi tạo GLEW
    if (glewInit() != GLEW_OK)
    {
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
    std::cout << "  HINH TRU VA HINH NON\n";
    std::cout << "  (Su dung phep chieu va ve diem)\n";
    std::cout << "===========================================\n";
    std::cout << "Phim mui ten: Xoay hinh\n";
    std::cout << "  <- -> : Xoay ngang (trai/phai)\n";
    std::cout << "  ^  v  : Xoay doc (len/xuong)\n";
    std::cout << "\n";
    std::cout << "Phim 1: HINH TRU\n";
    std::cout << "Phim 2: HINH NON\n";
    std::cout << "\n";
    std::cout << "Dieu chinh mat do diem:\n";
    std::cout << "  Q: Tang Delta_U (thua diem hon)\n";
    std::cout << "  W: Giam Delta_U (dac diem hon)\n";
    std::cout << "  A: Tang Delta_V (thua diem hon)\n";
    std::cout << "  S: Giam Delta_V (dac diem hon)\n";
    std::cout << "\n";
    std::cout << "Phong to / Thu nho:\n";
    std::cout << "  +: Phong to hinh\n";
    std::cout << "  -: Thu nho hinh\n";
    std::cout << "  0: Dat lai kich thuoc mac dinh\n";
    std::cout << "\n";
    std::cout << "Phim ESC: Thoat\n";
    std::cout << "===========================================\n";
    std::cout << "Delta_U (Tru) = " << Delta_U << "\n";
    std::cout << "Delta_V (Tru) = " << Delta_V << "\n";
    std::cout << "Delta_U (Non) = " << Delta_U_Cone << "\n";
    std::cout << "Delta_V (Non) = " << Delta_V_Cone << "\n";
    std::cout << "Scale (Phong to) = " << scale << "\n";
    std::cout << "===========================================\n";

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