#include "OesRenderer.h"

bool OesRenderer::init() {
    return initShader();
}

OesRenderer::~OesRenderer() {
    if (program_) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

bool OesRenderer::initShader() {
    const char* vShader =
            "attribute vec4 aPosition;\n"
            "attribute vec4 aTexCoord;\n"
            "uniform mat4 uTexMatrix;\n"
            "varying vec2 vTexCoord;\n"
            "void main() {\n"
            "  gl_Position = aPosition;\n"
            "  vTexCoord = (uTexMatrix * aTexCoord).xy;\n"
            "}";

    const char* fShader =
            "#extension GL_OES_EGL_image_external : require\n"
            "precision highp float;\n"
            "uniform samplerExternalOES sTexture;\n"
            "varying highp vec2 vTexCoord;\n"
            "void main() {\n"
            "  gl_FragColor = texture2D(sTexture, vTexCoord);\n"
            "}";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fShader);
    if (!vs || !fs) return false;

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);

    GLint linked;
    glGetProgramiv(program_, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint len;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, 0);
        glGetProgramInfoLog(program_, len, nullptr, log.data());
        LOGD("Shader link error: %s", log.c_str());
        glDeleteProgram(program_);
        program_ = 0;
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    aPos_ = glGetAttribLocation(program_, "aPosition");
    aTex_ = glGetAttribLocation(program_, "aTexCoord");
    uTexMatrix_ = glGetUniformLocation(program_, "uTexMatrix");
    uSampler_ = glGetUniformLocation(program_, "sTexture");

    LOGD("OES shader init success.");
    return true;
}

GLuint OesRenderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, 0);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        LOGD("Shader compile error: %s", log.c_str());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void OesRenderer::draw(GLuint oesTexId, const GLfloat* texMatrix, int width, int height) {
    if (!program_) return;

    glViewport(0, 0, width, height);

    glUseProgram(program_);

    glVertexAttribPointer(aPos_, 2, GL_FLOAT, GL_FALSE, 0, VERTICES.data());
    glEnableVertexAttribArray(aPos_);

    glVertexAttribPointer(aTex_, 2, GL_FLOAT, GL_FALSE, 0, TEX_COORDS.data());
    glEnableVertexAttribArray(aTex_);

    glUniformMatrix4fv(uTexMatrix_, 1, GL_FALSE, texMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, oesTexId);
    glUniform1i(uSampler_, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(aPos_);
    glDisableVertexAttribArray(aTex_);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
}