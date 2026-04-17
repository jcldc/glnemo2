#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QString>

/**
 * @brief Manages a Frame Buffer Object (FBO) for offscreen rendering.
 *
 * Workflow:
 *   1. Call create() once after an OpenGL context is current.
 *   2. Call bind()   before rendering.
 *   3. Call unbind() after rendering.
 *   4. Call saveToFile() to dump the colour attachment to a PNG/JPEG.
 *   5. Call destroy() in the destructor or cleanup.
 */
class OffscreenRenderer
{
public:
    OffscreenRenderer() = default;
    ~OffscreenRenderer() = default;

    /**
     * @brief Allocate the FBO with a colour (RGBA8) + depth (DEPTH24) attachment.
     * @param gl     Core 3.3 function table.
     * @param width  Render target width in pixels.
     * @param height Render target height in pixels.
     */
    void create(QOpenGLFunctions_3_3_Core* gl, int width, int height);

    /** Bind FBO — subsequent draw calls render into the off-screen buffer. */
    void bind(QOpenGLFunctions_3_3_Core* gl) const;

    /** Unbind FBO — restore the default framebuffer (screen). */
    void unbind(QOpenGLFunctions_3_3_Core* gl) const;

    /**
     * @brief Read pixels from the colour attachment and save to disk.
     * @param gl       Core 3.3 function table.
     * @param filePath Destination file (extension determines format: png, jpg…).
     * @return true on success.
     */
    bool saveToFile(QOpenGLFunctions_3_3_Core* gl,
                    const QString& filePath) const;

    void destroy(QOpenGLFunctions_3_3_Core* gl);

    bool isValid() const { return m_fboId != 0; }
    int  width()   const { return m_width; }
    int  height()  const { return m_height; }

private:
    unsigned int m_fboId      = 0;  ///< Frame Buffer Object
    unsigned int m_colorTex   = 0;  ///< Color attachment (RGBA8 texture)
    unsigned int m_depthRbo   = 0;  ///< Depth attachment (Render Buffer Object)
    int          m_width      = 0;
    int          m_height     = 0;
};
