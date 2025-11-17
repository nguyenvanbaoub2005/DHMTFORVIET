#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <iostream>
using namespace std;

struct Vec3
{
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
};

// ---------- Dữ liệu vật thể (lập phương) ----------
vector<Vec3> vertices = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}};

// Cạnh cho wireframe
vector<pair<int, int>> edges = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

// Mặt cho polygon model
vector<vector<int>> faces = {
    {0, 1, 2, 3}, // mặt sau (Z = -1)
    {4, 5, 6, 7}, // mặt trước (Z = +1)
    {0, 1, 5, 4}, // đáy
    {3, 2, 6, 7}, // đỉnh
    {0, 3, 7, 4}, // trái
    {1, 2, 6, 5}  // phải
};

// ---------- Ma trận chiếu phối cảnh ----------
void buildPerspective(float fov_deg, float aspect, float znear, float zfar, float M[16])
{
    float fov = fov_deg * M_PI / 180.0f;
    float f = 1.0f / tan(fov / 2.0f);

    // Ma trận perspective chuẩn OpenGL (column-major)
    // | f/aspect    0         0              0        |
    // | 0           f         0              0        |
    // | 0           0    (f+n)/(n-f)    2*f*n/(n-f)   |
    // | 0           0        -1              0        |

    // Cột 0
    M[0] = f / aspect;
    M[1] = 0;
    M[2] = 0;
    M[3] = 0;

    // Cột 1
    M[4] = 0;
    M[5] = f;
    M[6] = 0;
    M[7] = 0;

    // Cột 2
    M[8] = 0;
    M[9] = 0;
    M[10] = (zfar + znear) / (znear - zfar);
    M[11] = -1.0f;

    // Cột 3
    M[12] = 0;
    M[13] = 0;
    M[14] = (2.0f * zfar * znear) / (znear - zfar);
    M[15] = 0;
}

// ---------- Ma trận chiếu song song ----------
void buildOrtho(float left, float right, float bottom, float top, float znear, float zfar, float M[16])
{
    M[0] = 2.0f / (right - left);
    M[4] = 0;
    M[8] = 0;
    M[12] = -(right + left) / (right - left);
    M[1] = 0;
    M[5] = 2.0f / (top - bottom);
    M[9] = 0;
    M[13] = -(top + bottom) / (top - bottom);
    M[2] = 0;
    M[6] = 0;
    M[10] = -2.0f / (zfar - znear);
    M[14] = -(zfar + znear) / (zfar - znear);
    M[3] = 0;
    M[7] = 0;
    M[11] = 0;
    M[15] = 1;
}

// ---------- Thông số toàn cục ----------
bool perspectiveMode = true;
bool wireframeMode = true; // true = wireframe, false = polygon model
float theta = 0.0f;        // Góc quay quanh trục Y
float phi = 0.0f;          // Góc nâng camera
float R = 6.0f;            // Khoảng cách camera
float fov = 45.0f;         // Góc nhìn
int winW = 800, winH = 600;

// ---------- In log ra console ----------
void logStatus()
{
    cout << "\n===== THÔNG TIN HIỆN TẠI =====\n";
    cout << "[Chế độ hiển thị]: "
              << (wireframeMode ? "Wireframe (Khung dây)" : "Polygon Model (Mặt đa giác)") << "\n";
    cout << "[Chế độ chiếu]: "
              << (perspectiveMode ? "Phối cảnh (Perspective)" : "Song song (Orthographic)") << "\n";
    cout << "[Góc θ]: " << theta << "° | [Góc φ]: " << phi << "°\n";
    cout << "[Khoảng cách R]: " << R << "\n";
    cout << "[FOV]: " << fov << " (chỉ khi phối cảnh)\n";
    cout << "===============================\n";
}

// ---------- Callback bàn phím ----------
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    const float dA = 3.0f, dR = 0.2f, dFOV = 2.0f;
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_M:
            wireframeMode = !wireframeMode;
            cout << "[INFO] Chuyển sang chế độ: "
                      << (wireframeMode ? "Wireframe (Khung dây)\n" : "Polygon Model (Mặt đa giác)\n");
            break;
        case GLFW_KEY_P:
            perspectiveMode = !perspectiveMode;
            cout << "[INFO] Đang dùng phép chiếu: "
                      << (perspectiveMode ? "Phối cảnh (Perspective)\n" : "Song song (Orthographic)\n");
            break;
        case GLFW_KEY_LEFT:
            theta -= dA;
            break;
        case GLFW_KEY_RIGHT:
            theta += dA;
            break;
        case GLFW_KEY_UP:
            phi += dA;
            if (phi > 89)
                phi = 89;
            break;
        case GLFW_KEY_DOWN:
            phi -= dA;
            if (phi < -89)
                phi = -89;
            break;
        case GLFW_KEY_Q:
            R -= dR;
            if (R < 0.5f)
                R = 0.5f;
            break;
        case GLFW_KEY_E:
            R += dR;
            break;
        case GLFW_KEY_Z:
            fov -= dFOV;
            if (fov < 10)
                fov = 10;
            break;
        case GLFW_KEY_X:
            fov += dFOV;
            if (fov > 120)
                fov = 120;
            break;
        default:
            break;
        }
        logStatus();
    }
}

// ---------- Ma trận quan sát T  ----------
void buildViewMatrix_T(float theta, float phi, float R, float T[16])
{
    float radTheta = theta * M_PI / 180.0f;
    float radPhi = phi * M_PI / 180.0f;

    float sinT = sinf(radTheta), cosT = cosf(radTheta);
    float sinP = sinf(radPhi), cosP = cosf(radPhi);

    float temp[4][4] = {
        {cosT, 0.0f, -sinT, 0.0f},
        {sinT * sinP, cosP, cosT * sinP, 0.0f},
        {sinT * cosP, -sinP, cosT * cosP, -R},
        {0.0f, 0.0f, 0.0f, 1.0f}};

    int idx = 0;
    for (int j = 0; j < 4; ++j)
        for (int i = 0; i < 4; ++i)
            T[idx++] = temp[i][j];
}

// ---------- Vẽ vật thể dạng wireframe ----------
void drawWireframe()
{
    glBegin(GL_LINES);
    for (auto &e : edges)
    {
        Vec3 a = vertices[e.first];
        Vec3 b = vertices[e.second];
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
    }
    glEnd();
}

// ---------- Vẽ vật thể dạng polygon model ----------
void drawPolygonModel()
{
    glBegin(GL_QUADS);
    for (size_t i = 0; i < faces.size(); ++i)
    {
        // Tô màu khác nhau cho từng mặt
        switch (i)
        {
        case 0:
            glColor3f(1, 0, 0);
            break; // đỏ
        case 1:
            glColor3f(0, 1, 0);
            break; // xanh lá
        case 2:
            glColor3f(0, 0, 1);
            break; // xanh dương
        case 3:
            glColor3f(1, 1, 0);
            break; // vàng
        case 4:
            glColor3f(1, 0, 1);
            break; // tím
        case 5:
            glColor3f(0, 1, 1);
            break; // cyan
        }
        for (int idx : faces[i])
        {
            Vec3 v = vertices[idx];
            glVertex3f(v.x, v.y, v.z);
        }
    }
    glEnd();
}

// ---------- MAIN ----------
int main()
{
    if (!glfwInit())
    {
        cerr << "Lỗi: Không khởi tạo được GLFW\n";
        return -1;
    }
    GLFWwindow *window = glfwCreateWindow(winW, winH, "Quan sát vật thể 3D - Combined View", NULL, NULL);
    if (!window)
    {
        cerr << "Không tạo được cửa sổ\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cerr << "Lỗi: Không khởi tạo được GLEW\n";
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);

    cout << "===== HƯỚNG DẪN =====\n"
              << "M: chuyển đổi chế độ hiển thị (Wireframe / Polygon Model)\n"
              << "P: đổi phép chiếu (song song / phối cảnh)\n"
              << "← →: quay quanh trục Y (thay đổi θ)\n"
              << "↑ ↓: quay lên / xuống (thay đổi φ)\n"
              << "Q / E: phóng to / thu nhỏ (thay đổi R)\n"
              << "Z / X: thay đổi góc nhìn FOV (chỉ khi phối cảnh)\n"
              << "ESC: thoát chương trình\n"
              << "======================\n";
    logStatus();

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &winW, &winH);
        float aspect = (float)winW / (float)winH;
        glViewport(0, 0, winW, winH);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Ma trận chiếu ---
        float P[16];
        if (perspectiveMode)
            buildPerspective(fov, aspect, 0.1f, 100.0f, P);
        else
            buildOrtho(-R * aspect, R * aspect, -R, R, -100, 100, P);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(P);

        // --- Ma trận quan sát T ---
        float V[16];
        buildViewMatrix_T(theta, phi, R, V);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(V);

        // --- Cấu hình chế độ hiển thị ---
        if (wireframeMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // --- Vẽ trục toạ độ ---
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(2, 0, 0); // X
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 2, 0); // Y
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 2); // Z
        glEnd();

        // --- Vẽ vật thể theo chế độ hiện tại ---
        if (wireframeMode)
        {
            glColor3f(1, 1, 1); // màu trắng cho wireframe
            drawWireframe();
        }
        else
        {
            drawPolygonModel();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
