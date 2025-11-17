#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <vector>

// ==================== VECTOR VÀ MA TRẬN TỰ IMPLEMENT ====================

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Vec4 {
    float x, y, z, w;
    Vec4(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}
};

struct Mat4 {
    float m[16]; // Lưu theo column-major (OpenGL convention)
    
    Mat4() {
        for (int i = 0; i < 16; i++) m[i] = 0.0f;
    }
    
    // Ma trận đơn vị
    static Mat4 identity() {
        Mat4 mat;
        mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = 1.0f;
        return mat;
    }
    
    // Nhân 2 ma trận
    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                float sum = 0.0f;
                for (int k = 0; k < 4; k++) {
                    sum += m[k * 4 + row] * other.m[col * 4 + k];
                }
                result.m[col * 4 + row] = sum;
            }
        }
        return result;
    }
    
    // Nhân ma trận với vector
    Vec4 operator*(const Vec4& v) const {
        Vec4 result;
        result.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
        result.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
        result.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
        result.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
        return result;
    }
};

// ==================== HÀM TẠO MA TRẬN BIẾN ĐỔI ====================

Mat4 rotateX(float angle) {
    Mat4 mat = Mat4::identity();
    float c = cos(angle);
    float s = sin(angle);
    mat.m[5] = c;   mat.m[9] = -s;
    mat.m[6] = s;   mat.m[10] = c;
    return mat;
}

Mat4 rotateY(float angle) {
    Mat4 mat = Mat4::identity();
    float c = cos(angle);
    float s = sin(angle);
    mat.m[0] = c;   mat.m[8] = s;
    mat.m[2] = -s;  mat.m[10] = c;
    return mat;
}

Mat4 rotateZ(float angle) {
    Mat4 mat = Mat4::identity();
    float c = cos(angle);
    float s = sin(angle);
    mat.m[0] = c;   mat.m[4] = -s;
    mat.m[1] = s;   mat.m[5] = c;
    return mat;
}

Mat4 translate(float tx, float ty, float tz) {
    Mat4 mat = Mat4::identity();
    mat.m[12] = tx;
    mat.m[13] = ty;
    mat.m[14] = tz;
    return mat;
}

Mat4 scale(float sx, float sy, float sz) {
    Mat4 mat = Mat4::identity();
    mat.m[0] = sx;
    mat.m[5] = sy;
    mat.m[10] = sz;
    return mat;
}

Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f;
    f.x = center.x - eye.x;
    f.y = center.y - eye.y;
    f.z = center.z - eye.z;
    float fLen = sqrt(f.x * f.x + f.y * f.y + f.z * f.z);
    f.x /= fLen; f.y /= fLen; f.z /= fLen;
    
    Vec3 r;
    r.x = f.y * up.z - f.z * up.y;
    r.y = f.z * up.x - f.x * up.z;
    r.z = f.x * up.y - f.y * up.x;
    float rLen = sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
    r.x /= rLen; r.y /= rLen; r.z /= rLen;
    
    Vec3 u;
    u.x = r.y * f.z - r.z * f.y;
    u.y = r.z * f.x - r.x * f.z;
    u.z = r.x * f.y - r.y * f.x;
    
    Mat4 mat = Mat4::identity();
    mat.m[0] = r.x;  mat.m[4] = r.y;  mat.m[8] = r.z;
    mat.m[1] = u.x;  mat.m[5] = u.y;  mat.m[9] = u.z;
    mat.m[2] = -f.x; mat.m[6] = -f.y; mat.m[10] = -f.z;
    mat.m[12] = -(r.x * eye.x + r.y * eye.y + r.z * eye.z);
    mat.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    mat.m[14] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);
    
    return mat;
}

Mat4 perspective(float fovy, float aspect, float near, float far) {
    Mat4 mat;
    float f = 1.0f / tan(fovy / 2.0f);
    mat.m[0] = f / aspect;
    mat.m[5] = f;
    mat.m[10] = (far + near) / (near - far);
    mat.m[11] = -1.0f;
    mat.m[14] = (2.0f * far * near) / (near - far);
    return mat;
}

Mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
    Mat4 mat = Mat4::identity();
    mat.m[0] = 2.0f / (right - left);
    mat.m[5] = 2.0f / (top - bottom);
    mat.m[10] = -2.0f / (far - near);
    mat.m[12] = -(right + left) / (right - left);
    mat.m[13] = -(top + bottom) / (top - bottom);
    mat.m[14] = -(far + near) / (far - near);
    return mat;
}

// ==================== SHADER ====================

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 ourColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

// ==================== BIẾN TOÀN CỤC ====================

enum ProjectionMode { PERSPECTIVE, ORTHOGRAPHIC };
enum DisplayMode { WIREFRAME, SOLID };

ProjectionMode currentProjection = PERSPECTIVE;
DisplayMode currentDisplay = SOLID;

float theta = 45.0f;
float phi = 30.0f;
float distance = 8.0f;
float zoomFOV = 45.0f;
float scaleValue = 1.0f;

// ==================== HÀM BIÊN DỊCH SHADER ====================

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

unsigned int createShaderProgram() {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

// ==================== XỬ LÝ PHÍM BẤM ====================

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        currentProjection = PERSPECTIVE;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        currentProjection = ORTHOGRAPHIC;
    
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        currentDisplay = WIREFRAME;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        currentDisplay = SOLID;
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        theta -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        theta += 0.5f;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        phi += 0.3f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        phi -= 0.3f;
    
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        distance -= 0.05f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        distance += 0.05f;
    
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        zoomFOV -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        zoomFOV += 0.5f;
    
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        scaleValue += 0.005f;
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        scaleValue -= 0.005f;
    
    while (theta >= 360.0f) theta -= 360.0f;
    while (theta < 0.0f) theta += 360.0f;
    while (phi >= 360.0f) phi -= 360.0f;
    while (phi < 0.0f) phi += 360.0f;
    
    if (distance < 2.0f) distance = 2.0f;
    if (zoomFOV < 10.0f) zoomFOV = 10.0f;
    if (zoomFOV > 120.0f) zoomFOV = 120.0f;
    if (scaleValue < 0.1f) scaleValue = 0.1f;
    if (scaleValue > 5.0f) scaleValue = 5.0f;
}

// ==================== HÀM MAIN ====================

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1400, 900, "LAB 04 - Hinh Chop Tu Dien (Pyramid)", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    glViewport(0, 0, 1400, 900);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.0f);
    
    unsigned int shaderProgram = createShaderProgram();
    
    // ==================== ĐỊNH NGHĨA HÌNH CHÓP TƯ DIỆN ====================
    // Hình chóp với đáy vuông và đỉnh ở trên
    // Trung tâm đặt tại (3, 1, 0) - cách gốc tọa độ
    
    float pyramidVertices[] = {
        // Vị trí              // Màu sắc
        // Đỉnh chóp (apex)
        3.0f, 3.0f, 0.0f,     1.0f, 0.0f, 0.0f,  // 0 - Đỏ
        
        // Đáy vuông (base) - 4 đỉnh
        2.0f, 0.0f, 1.0f,     0.0f, 1.0f, 0.0f,  // 1 - Xanh lá (góc trước trái)
        4.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,  // 2 - Xanh dương (góc trước phải)
        4.0f, 0.0f, -1.0f,    1.0f, 1.0f, 0.0f,  // 3 - Vàng (góc sau phải)
        2.0f, 0.0f, -1.0f,    1.0f, 0.0f, 1.0f,  // 4 - Tím (góc sau trái)
    };
    
    // Định nghĩa vertices cho wireframe với MÀU TRẮNG
    float pyramidWireVertices[] = {
        // Vị trí              // Màu sắc (TRẮNG)
        3.0f, 3.0f, 0.0f,     1.0f, 1.0f, 1.0f,  // 0 - Đỉnh chóp
        2.0f, 0.0f, 1.0f,     1.0f, 1.0f, 1.0f,  // 1
        4.0f, 0.0f, 1.0f,     1.0f, 1.0f, 1.0f,  // 2
        4.0f, 0.0f, -1.0f,    1.0f, 1.0f, 1.0f,  // 3
        2.0f, 0.0f, -1.0f,    1.0f, 1.0f, 1.0f,  // 4
    };
    
    // Indices cho mặt đa giác (solid)
    unsigned int pyramidIndices[] = {
        // 4 mặt bên (tam giác)
        0, 1, 2,  // Mặt trước
        0, 2, 3,  // Mặt phải
        0, 3, 4,  // Mặt sau
        0, 4, 1,  // Mặt trái
        
        // Đáy (2 tam giác tạo thành hình vuông)
        1, 4, 3,
        1, 3, 2
    };
    
    // Indices cho khung kết nối (wireframe)
    unsigned int pyramidWireIndices[] = {
        // Các cạnh từ đỉnh xuống đáy
        0, 1,
        0, 2,
        0, 3,
        0, 4,
        
        // Các cạnh đáy
        1, 2,
        2, 3,
        3, 4,
        4, 1
    };
    
    // VAO, VBO, EBO cho solid
    unsigned int pyramidVAO, pyramidVBO, pyramidEBO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glGenBuffers(1, &pyramidEBO);
    
    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // VAO, VBO, EBO cho wireframe
    unsigned int pyramidWireVAO, pyramidWireVBO, pyramidWireEBO;
    glGenVertexArrays(1, &pyramidWireVAO);
    glGenBuffers(1, &pyramidWireVBO);
    glGenBuffers(1, &pyramidWireEBO);
    
    glBindVertexArray(pyramidWireVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidWireVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidWireVertices), pyramidWireVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidWireEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidWireIndices), pyramidWireIndices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // ==================== TẠO TRỤC TỌA ĐỘ ====================
    float axisVertices[] = {
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // X axis (ĐỎ)
        6.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Y axis (XANH LÁ)
        0.0f, 6.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Z axis (XANH DƯƠNG)
        0.0f, 0.0f, 6.0f,  0.0f, 0.0f, 1.0f
    };
    
    unsigned int axisVAO, axisVBO;
    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(1, &axisVBO);
    
    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // ==================== RENDER LOOP ====================
    std::cout << "\n========== LAB 04 - HINH CHOP TU DIEN ==========\n";
    std::cout << "PHEP CHIEU:\n";
    std::cout << "  P: Perspective (Phoi canh)\n";
    std::cout << "  O: Orthographic (Song song)\n\n";
    std::cout << "CHE DO HIEN THI:\n";
    std::cout << "  1: Wireframe (Khung ket noi)\n";
    std::cout << "  2: Solid (Mat da giac)\n\n";
    std::cout << "XOAY VAT THE:\n";
    std::cout << "  A/D: Xoay ngang (theta)\n";
    std::cout << "  W/S: Xoay doc (phi) - 360 do\n\n";
    std::cout << "CAMERA:\n";
    std::cout << "  Q/E: Gan/Xa\n";
    std::cout << "  Z/X: Zoom\n";
    std::cout << "  +/-: Phong to/Thu nho\n\n";
    std::cout << "ESC: Thoat\n";
    std::cout << "================================================\n\n";
    
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        // Ma trận Model
        Mat4 model = scale(scaleValue, scaleValue, scaleValue);
        
        // Ma trận View
        float thetaRad = theta * M_PI / 180.0f;
        float phiRad = phi * M_PI / 180.0f;
        
        Vec3 cameraPos;
        cameraPos.x = distance * cos(phiRad) * cos(thetaRad);
        cameraPos.y = distance * sin(phiRad);
        cameraPos.z = distance * cos(phiRad) * sin(thetaRad);
        
        Mat4 view = lookAt(cameraPos, Vec3(3.0f, 1.5f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
        
        // Ma trận Projection
        Mat4 projection;
        if (currentProjection == PERSPECTIVE) {
            projection = perspective(zoomFOV * M_PI / 180.0f, 1400.0f / 900.0f, 0.1f, 100.0f);
        } else {
            float orthoSize = distance * 0.5f;
            projection = ortho(-orthoSize, orthoSize, -orthoSize * 0.64f, orthoSize * 0.64f, 0.1f, 100.0f);
        }
        
        // Truyền ma trận vào shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.m);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.m);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.m);
        
        // Vẽ trục tọa độ
        glBindVertexArray(axisVAO);
        glDrawArrays(GL_LINES, 0, 6);
        
        // Vẽ hình chóp
        if (currentDisplay == WIREFRAME) {
            glBindVertexArray(pyramidWireVAO);
            glDrawElements(GL_LINES, 16, GL_UNSIGNED_INT, 0);
        } else if (currentDisplay == SOLID) {
            glBindVertexArray(pyramidVAO);
            glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
        }
        
        // Hiển thị thông tin
        std::cout << "\r";
        std::cout << "Mode: " << (currentDisplay == WIREFRAME ? "WIREFRAME" : "SOLID    ");
        std::cout << " | Proj: " << (currentProjection == PERSPECTIVE ? "PERSP" : "ORTHO");
        std::cout << " | θ:" << (int)theta << "° | Φ:" << (int)phi << "°";
        std::cout << " | D:" << distance << " | FOV:" << (int)zoomFOV << "°";
        std::cout << " | Scale:" << scaleValue << "   " << std::flush;
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Dọn dẹp
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteVertexArrays(1, &pyramidWireVAO);
    glDeleteVertexArrays(1, &axisVAO);
    glDeleteBuffers(1, &pyramidVBO);
    glDeleteBuffers(1, &pyramidEBO);
    glDeleteBuffers(1, &pyramidWireVBO);
    glDeleteBuffers(1, &pyramidWireEBO);
    glDeleteBuffers(1, &axisVBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}