#pragma once
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <array>
#include <string>
#include "CarplayNativeLogger.h"

class OesRenderer {
public:
    OesRenderer() = default;
    ~OesRenderer();

    bool init();
    void draw(GLuint oesTexId, const GLfloat* texMatrix, int width, int height); // 绘制一帧

private:
    GLuint program_ = 0;
    GLint aPos_ = -1;
    GLint aTex_ = -1;
    GLint uTexMatrix_ = -1;
    GLint uSampler_ = -1;

    bool initShader();
    GLuint compileShader(GLenum type, const char* source);

    // 顶点 & 纹理坐标
    std::array<GLfloat, 8> VERTICES{
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f,  1.0f,
            1.0f,  1.0f
    };

    std::array<GLfloat, 8> TEX_COORDS{
            0.0f, 0.0f,  // 左下
            1.0f, 0.0f,  // 右下
            0.0f, 1.0f,  // 左上
            1.0f, 1.0f   // 右上
    };
};
