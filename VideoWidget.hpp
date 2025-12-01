#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QTimer>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:
    explicit GLWidget(QWidget* parent = nullptr);
    ~GLWidget();

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    QOpenGLShaderProgram program;
    QOpenGLTexture* tilemap = nullptr;

    GLuint vao = 0;
    GLuint vboQuad = 0;
    GLuint vboInstance = 0;   // offsets + tileIndex

    static constexpr int TILES_X = 80;
    static constexpr int TILES_Y = 60;
    static constexpr int TOTAL_TILES = TILES_X * TILES_Y;
};
