#include "VideoWidget.hpp"
#include <QMatrix4x4>

extern uint8_t io[65536];

static const char* vertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 inPos;          // quad local coords (0..1)
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec2 instanceOffset; // tile position (x,y)
layout(location = 3) in uint tileIndex;      // tile ID 0..255

uniform mat4 proj;
uniform float uvStep;

out vec2 uv;

void main() {
    uint tx = tileIndex & 15u;
    uint ty = tileIndex >> 4;

    vec2 uvBase = vec2(float(tx) * uvStep, float(ty) * uvStep);
    uv = uvBase + inUV * uvStep;

    gl_Position = proj * vec4(inPos + instanceOffset, 0.0, 1.0);
}
)";

static const char* fragmentSrc = R"(
#version 330 core
in vec2 uv;
uniform sampler2D tex;
out vec4 fragColor;

void main() {
    fragColor = texture(tex, uv);
}
)";

GLWidget::GLWidget(QWidget* parent): QOpenGLWidget(parent){
    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&GLWidget::update));
    timer->start(1000 / 60);    // 60 FPS
}

GLWidget::~GLWidget() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vboQuad);
    glDeleteBuffers(1, &vboInstance);
    delete tilemap;
    doneCurrent();
}

void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSrc);
    program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSrc);
    program.link();

    // Load tilemap
    tilemap = new QOpenGLTexture(QImage(":/tilemap.png").convertToFormat(QImage::Format_RGBA8888));
    tilemap->setMinificationFilter(QOpenGLTexture::Nearest);
    tilemap->setMagnificationFilter(QOpenGLTexture::Nearest);
    tilemap->setWrapMode(QOpenGLTexture::ClampToEdge);

    // Quad base geometry (2 triangles, coords 0..1)
    const float quadData[] = {
        // pos      uv
        0.f, 0.f,   0.f, 0.f,
        1.f, 0.f,   1.f, 0.f,
        1.f, 1.f,   1.f, 1.f,

        0.f, 0.f,   0.f, 0.f,
        1.f, 1.f,   1.f, 1.f,
        0.f, 1.f,   0.f, 1.f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboQuad);
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    // Instance buffer: vec2 offset + uint tileIndex
    struct Instance {
        float x;
        float y;
        uint32_t t;
    };

    Instance instances[TOTAL_TILES];

    for (int y = 0; y < TILES_Y; ++y) {
        int base = y * 128; // 80 visibles + padding
        for (int x = 0; x < TILES_X; ++x) {
            int idx = y * TILES_X + x;
            instances[idx].x = float(x);
            instances[idx].y = float(y);
            instances[idx].t = io[base + x];
        }
    }

    glGenBuffers(1, &vboInstance);
    glBindBuffer(GL_ARRAY_BUFFER, vboInstance);
    glBufferData(GL_ARRAY_BUFFER, sizeof(instances), instances, GL_DYNAMIC_DRAW);

    // Offsets
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Instance), (void*)0);
    glVertexAttribDivisor(2, 1);

    // Tile index
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Instance), (void*)(2 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

void GLWidget::paintGL() {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    program.bind();
    tilemap->bind(0);
    program.setUniformValue("tex", 0);

    QMatrix4x4 proj;
    proj.ortho(0.f, float(TILES_X), float(TILES_Y), 0.f, -1.f, 1.f);
    program.setUniformValue("proj", proj);

    float uvStep = 1.0f / 16.0f;   // 128/8 = 16 tiles
    program.setUniformValue("uvStep", uvStep);

    // Update instance buffer with io[]
    struct Instance {
        float x;
        float y;
        uint32_t t;
    };

    Instance instances[TOTAL_TILES];
    for (int y = 0; y < TILES_Y; ++y) {
        int base = y * 128;
        for (int x = 0; x < TILES_X; ++x) {
            int idx = y * TILES_X + x;
            instances[idx].x = float(x);
            instances[idx].y = float(y);
            instances[idx].t = io[base + x];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboInstance);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(instances), instances);

    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, TOTAL_TILES);
    glBindVertexArray(0);

    program.release();
}
