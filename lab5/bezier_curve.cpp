#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// Kích thước cửa sổ
const int WIDTH = 800;
const int HEIGHT = 600;

// Cấu trúc điểm
typedef struct
{
    int x;
    int y;
} CPoint;

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

// Hàm tính điểm trên đường cong Bezier tại tham số t
CPoint TPt(CPoint P[], float t, int n)
{
    CPoint Pt;
    float B;
    int k;
    Pt.x = 0;
    Pt.y = 0;
    for (k = 0; k <= n; k++)
    {
        B = BernStein(t, n, k);
        Pt.x = Pt.x + P[k].x * B;
        Pt.y = Pt.y + P[k].y * B;
    }
    return Pt;
}

// Hàm vẽ điểm
void drawPoint(int x, int y)
{
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// Hàm vẽ đường cong Bezier
void DrawBezier(int n, CPoint P[])
{
    CPoint Pt;
    float dt, t, m;
    int i;
    t = 0;
    m = 100; // Số bước chia
    dt = 1 / m;

    // Vẽ đường cong Bezier
    glColor3f(1.0f, 0.0f, 0.0f); // Màu đỏ cho đường cong
    glBegin(GL_LINE_STRIP);
    glVertex2i(P[0].x, P[0].y); // moveto(P[0].x, P[0].y)
    for (i = 1; i <= m; i++)
    {
        Pt = TPt(P, t, n);
        glVertex2i(Pt.x, Pt.y);
        t = t + dt;
    }
    glVertex2i(P[n].x, P[n].y);
    glEnd();
}

// Hàm vẽ các điểm điều khiển và đường nối
void drawControlPoints(int n, CPoint P[])
{
    // Vẽ các điểm điều khiển
    glColor3f(0.0f, 0.0f, 1.0f); // Màu xanh dương
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i <= n; i++)
    {
        glVertex2i(P[i].x, P[i].y);
    }
    glEnd();

    // Vẽ đường nối các điểm điều khiển
    glColor3f(0.5f, 0.5f, 0.5f); // Màu xám
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= n; i++)
    {
        glVertex2i(P[i].x, P[i].y);
    }
    glEnd();
}

// Hàm hiển thị
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Định nghĩa các điểm điều khiển (n = 3 cho đường cong Bezier bậc 3)
    int n = 3;
    CPoint P[4];

    // Đường cong Bezier
    P[0].x = 150;
    P[0].y = 500;
    P[1].x = 250;
    P[1].y = 200;
    P[2].x = 500;
    P[2].y = 500;
    P[3].x = 650;
    P[3].y = 300;

    drawControlPoints(n, P);
    DrawBezier(n, P);

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
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Duong cong Bezier", NULL, NULL);
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

    // Khởi tạo OpenGL
    init();

    std::cout << "Chuong trinh ve duong cong Bezier" << std::endl;
    std::cout << "- Duong mau do: Duong cong Bezier" << std::endl;
    std::cout << "- Diem mau xanh duong: Cac diem dieu khien" << std::endl;
    std::cout << "- Duong mau xam: Duong noi cac diem dieu khien" << std::endl;

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
