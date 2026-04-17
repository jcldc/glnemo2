#include "offscreenrenderer.h"

#include <QImage>
#include <QDebug>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
void OffscreenRenderer::create(QOpenGLFunctions_3_3_Core* gl,
                                int width, int height)
{
    m_width  = width;
    m_height = height;

    // ── 1. Colour attachment: RGBA8 texture ──────────────────────────────────
    gl->glGenTextures(1, &m_colorTex);
    gl->glBindTexture(GL_TEXTURE_2D, m_colorTex);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
                     width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glBindTexture(GL_TEXTURE_2D, 0);

    // ── 2. Depth attachment: 24-bit Render Buffer ────────────────────────────
    gl->glGenRenderbuffers(1, &m_depthRbo);
    gl->glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                               width, height);
    gl->glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // ── 3. Frame Buffer Object ────────────────────────────────────────────────
    gl->glGenFramebuffers(1, &m_fboId);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    // Attach colour texture
    gl->glFramebufferTexture2D(GL_FRAMEBUFFER,
                                GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D,
                                m_colorTex, 0);

    // Attach depth RBO
    gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                   GL_DEPTH_ATTACHMENT,
                                   GL_RENDERBUFFER,
                                   m_depthRbo);

    // ── 4. Completeness check ─────────────────────────────────────────────────
    GLenum status = gl->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        qCritical() << "[FBO] Framebuffer is NOT complete. Status:" << status;
    }
    else
    {
        qDebug() << "[FBO] Created offscreen FBO"
                 << width << "x" << height;
    }

    gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
void OffscreenRenderer::bind(QOpenGLFunctions_3_3_Core* gl) const
{
    gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    gl->glViewport(0, 0, m_width, m_height);
}

// ─────────────────────────────────────────────────────────────────────────────
void OffscreenRenderer::unbind(QOpenGLFunctions_3_3_Core* gl) const
{
    gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
bool OffscreenRenderer::saveToFile(QOpenGLFunctions_3_3_Core* gl,
                                    const QString& filePath) const
{
    if (!m_fboId)
    {
        qWarning() << "[FBO] saveToFile() called but FBO is not valid.";
        return false;
    }

    // Bind for reading
    gl->glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    // Read pixels: RGBA, 1 byte per channel
    std::vector<unsigned char> pixels(
        static_cast<size_t>(m_width) * m_height * 4);

    // Ensure tight packing
    gl->glPixelStorei(GL_PACK_ALIGNMENT, 1);

    gl->glReadPixels(0, 0, m_width, m_height,
                     GL_RGBA, GL_UNSIGNED_BYTE,
                     pixels.data());

    gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ── Build QImage (OpenGL origin is bottom-left, Qt is top-left) ──────────
    QImage img(m_width, m_height, QImage::Format_RGBA8888);

    for (int row = 0; row < m_height; ++row)
    {
        // Flip vertically
        int srcRow = m_height - 1 - row;
        const unsigned char* src =
            pixels.data() + static_cast<size_t>(srcRow) * m_width * 4;
        unsigned char* dst = img.scanLine(row);
        std::memcpy(dst, src, static_cast<size_t>(m_width) * 4);
    }

    bool ok = img.save(filePath);
    if (ok)
        qDebug() << "[FBO] Saved offscreen render to:" << filePath;
    else
        qWarning() << "[FBO] Failed to save image to:" << filePath;

    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
void OffscreenRenderer::destroy(QOpenGLFunctions_3_3_Core* gl)
{
    if (m_fboId)     { gl->glDeleteFramebuffers(1,  &m_fboId);    m_fboId    = 0; }
    if (m_colorTex)  { gl->glDeleteTextures(1,      &m_colorTex); m_colorTex = 0; }
    if (m_depthRbo)  { gl->glDeleteRenderbuffers(1, &m_depthRbo); m_depthRbo = 0; }
}
