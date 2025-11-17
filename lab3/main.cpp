#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

// ---- Ma trận & hàm biến đổi ----
void matMul(float C[3][3], float A[3][3], float B[3][3])
{
    float tmp[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            tmp[i][j] = 0;
            for (int k = 0; k < 3; k++)
                tmp[i][j] += A[i][k] * B[k][j];
        }
    }
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            C[i][j] = tmp[i][j];
}

void applyPoint(float M[3][3], float &x, float &y)
{
    float nx = M[0][0] * x + M[0][1] * y + M[0][2];
    float ny = M[1][0] * x + M[1][1] * y + M[1][2];
    float w = M[2][0] * x + M[2][1] * y + M[2][2];
    if (fabs(w) > 1e-6)
    {
        nx /= w;
        ny /= w;
    }
    x = nx;
    y = ny;
}

void identity(float M[3][3])
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            M[i][j] = (i == j) ? 1 : 0;
}

void translateM(float M[3][3], float tx, float ty)
{
    identity(M);
    M[0][2] = tx;
    M[1][2] = ty;
}

void scaleM(float M[3][3], float sx, float sy, float x0 = 0, float y0 = 0)
{
    float T1[3][3], S[3][3], T2[3][3], tmp[3][3];
    translateM(T1, -x0, -y0);
    identity(S);
    S[0][0] = sx;
    S[1][1] = sy;
    translateM(T2, x0, y0);
    matMul(tmp, S, T1);
    matMul(M, T2, tmp);
}

void rotateM(float M[3][3], float angle, float x0 = 0, float y0 = 0)
{
    float T1[3][3], R[3][3], T2[3][3], tmp[3][3];
    translateM(T1, -x0, -y0);
    identity(R);
    float c = cos(angle), s = sin(angle);
    R[0][0] = c;
    R[0][1] = -s;
    R[1][0] = s;
    R[1][1] = c;
    translateM(T2, x0, y0);
    matMul(tmp, R, T1);
    matMul(M, T2, tmp);
}

void reflectM(float M[3][3], float a, float b)
{
    // Đối xứng qua đường y = a*x + b
    float theta = atan(a);
    float Tdown[3][3], Tup[3][3], Rpos[3][3], Rneg[3][3], Fx[3][3];
    float tmp1[3][3], tmp2[3][3], tmp3[3][3], tmp4[3][3];
    translateM(Tdown, 0, -b);
    translateM(Tup, 0, b);
    identity(Rpos);
    identity(Rneg);
    float c = cos(theta), s = sin(theta);
    Rpos[0][0] = c;
    Rpos[0][1] = -s;
    Rpos[1][0] = s;
    Rpos[1][1] = c;
    c = cos(-theta);
    s = sin(-theta);
    Rneg[0][0] = c;
    Rneg[0][1] = -s;
    Rneg[1][0] = s;
    Rneg[1][1] = c;
    identity(Fx);
    Fx[1][1] = -1;
    matMul(tmp1, Rneg, Tdown);
    matMul(tmp2, Fx, tmp1);
    matMul(tmp3, Rpos, tmp2);
    matMul(tmp4, Tup, tmp3);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            M[i][j] = tmp4[i][j];
}

// ---- Dữ liệu hình gốc ----
float originalVertices[6] = {-0.25f, -0.15f, 0.25f, -0.15f, 0.0f, 0.25f};
float vertices[6];

void applyToTriangle(float M[3][3])
{
    for (int i = 0; i < 3; i++)
        applyPoint(M, vertices[i * 2], vertices[i * 2 + 1]);
}

// ---- Vẽ trục toạ độ ----
void drawAxes(float worldExtent = 1.0f)
{
    // Đường trục chính (dày hơn)
    glLineWidth(2.0f);
    glColor3f(1.0f, 1.0f, 1.0f); // màu trắng
    glBegin(GL_LINES);
    // Ox (một chút dài hơn worldExtent để thấy rõ)
    glVertex2f(-worldExtent * 1.1f, 0.0f);
    glVertex2f(worldExtent * 1.1f, 0.0f);
    // Oy
    glVertex2f(0.0f, -worldExtent * 1.1f);
    glVertex2f(0.0f, worldExtent * 1.1f);
    glEnd();

    // Vẽ các vạch nhỏ (ticks) để dễ nhìn khi cửa sổ lớn
    glLineWidth(1.0f);
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINES);
    const int N = 10;
    for (int i = -N; i <= N; ++i)
    {
        float t = (float)i / (float)N * worldExtent;
        // vertical ticks on Ox
        glVertex2f(t, -0.01f * worldExtent);
        glVertex2f(t, 0.01f * worldExtent);
        // horizontal ticks on Oy
        glVertex2f(-0.01f * worldExtent, t);
        glVertex2f(0.01f * worldExtent, t);
    }
    glEnd();
}

// ---- Xử lý phím ----
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        float M[3][3];
        if (key == GLFW_KEY_R)
        { // reset
            for (int i = 0; i < 6; i++)
                vertices[i] = originalVertices[i];
        }
        else if (key == GLFW_KEY_1)
        { // tịnh tiến
            translateM(M, 0.0f, 0.1f);
            applyToTriangle(M);
        }
        else if (key == GLFW_KEY_2)
        { // scale đều quanh gốc
            scaleM(M, 1.2f, 1.2f, 0.0f, 0.0f);
            applyToTriangle(M);
        }
        else if (key == GLFW_KEY_3)
        { // rotate
            rotateM(M, M_PI / 6, 0.0f, 0.0f);
            applyToTriangle(M);
        }
        else if (key == GLFW_KEY_4)
        { // reflect qua Ox: y = 0
            reflectM(M, 0.0f, 0.0f);
            applyToTriangle(M);
        }
    }
}

// ---- Resize / Projection ----
// Khi cửa sổ thay đổi kích thước, cập nhật viewport + phép chiếu orthographic
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);

    // Thiết lập phép chiếu orthographic sao cho không bị méo:
    // Nếu aspect >= 1 (rộng hơn cao), mở rộng theo trục X; ngược lại mở rộng theo Y.
    float aspect = (float)width / (float)height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (aspect >= 1.0f)
    {
        // x mở rộng
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
    }
    else
    {
        // y mở rộng
        glOrtho(-1.0, 1.0, -1.0f / aspect, 1.0f / aspect, -1.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main()
{
    if (!glfwInit())
        return -1;

    // Tạo cửa sổ lớn hơn
    const int initialW = 1000;
    const int initialH = 800;
    GLFWwindow *window = glfwCreateWindow(initialW, initialH, "2D Transform + Axes (resizable)", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to init GLEW\n";
        return -1;
    }

    // Khởi tạo dữ liệu tam giác
    for (int i = 0; i < 6; i++)
        vertices[i] = originalVertices[i];

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Force initial projection
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    framebuffer_size_callback(window, w, h);

    // Vòng lặp render
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Lấy kích thước hiện tại để tính worldExtent cho vạch ticks (tùy chọn)
        glfwGetFramebufferSize(window, &w, &h);
        float aspect = (float)w / (float)h;
        // worldExtent dùng để vẽ các ticks; set =1 (mặc định) vì phép chiếu đã chuẩn hóa
        float worldExtent = 1.0f;

        // Vẽ trục toạ độ
        drawAxes(worldExtent);

        // Vẽ tam giác (dùng vertex array cũ)
        glEnableClientState(GL_VERTEX_ARRAY);
        glColor3f(0.2f, 0.7f, 0.9f);
        glPointSize(6.0f);
        glVertexPointer(2, GL_FLOAT, 0, vertices);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableClientState(GL_VERTEX_ARRAY);

        // Vẽ viền tam giác để dễ nhìn khi phóng to
        glLineWidth(1.5f);
        glColor3f(0.0f, 0.0f, 0.0f);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, vertices);
        glDrawArrays(GL_LINE_LOOP, 0, 3);
        glDisableClientState(GL_VERTEX_ARRAY);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
