#pragma once
#include <raylib.h>

/**
 * Renderer
 * --------
 * Maintains a virtual 640×480 render target (original game resolution).
 * Scales it to fit the physical screen with letterboxing, preserving 4:3.
 *
 * Usage:
 *   beginFrame()      — begin drawing to virtual render target
 *   [draw calls]
 *   endFrame()        — blit render target to screen with letterbox
 */
class Renderer {
public:
    // Original game resolution (4:3)
    static constexpr int VIRTUAL_W = 640;
    static constexpr int VIRTUAL_H = 480;

    Renderer(int screenW, int screenH);
    ~Renderer();

    void init();
    void beginFrame();
    void endFrame();

    /** Convert virtual coords → screen coords (for input mapping). */
    float virtualToScreenX(float vx) const;
    float virtualToScreenY(float vy) const;

    /** Convert screen coords → virtual coords. */
    float screenToVirtualX(float sx) const;
    float screenToVirtualY(float sy) const;

    float getScale()        const { return m_scale; }
    int   getOffsetX()      const { return m_offsetX; }
    int   getOffsetY()      const { return m_offsetY; }
    int   getScreenWidth()  const { return m_screenW; }
    int   getScreenHeight() const { return m_screenH; }

private:
    int m_screenW, m_screenH;
    float m_scale;
    int   m_offsetX, m_offsetY;

    RenderTexture2D m_renderTarget{};
};
