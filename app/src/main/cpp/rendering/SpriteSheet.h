#pragma once
#include <raylib.h>
#include <string>
#include <vector>

/**
 * Animation
 * ---------
 * A named sequence of frames within a sprite sheet texture.
 */
struct Animation {
    std::string name;
    int   startFrame = 0;
    int   frameCount = 1;
    float fps        = 12.0f;
    bool  loop       = true;
};

/**
 * SpriteSheet
 * -----------
 * A texture split into a grid of equal-sized frames.
 * Supports named animations with per-animation fps and loop settings.
 *
 * Coordinate system: top-left origin, matches virtual 640×480 canvas.
 */
class SpriteSheet {
public:
    SpriteSheet() = default;
    ~SpriteSheet();

    // Non-copyable (owns GPU texture handle)
    SpriteSheet(const SpriteSheet&)            = delete;
    SpriteSheet& operator=(const SpriteSheet&) = delete;

    // Movable: transfers GPU texture ownership, source left unloaded
    SpriteSheet(SpriteSheet&& o) noexcept;
    SpriteSheet& operator=(SpriteSheet&& o) noexcept;

    /**
     * Load from APK assets.
     * @param path   Path within assets/ (e.g. "sprites/player.png")
     * @param frameW Width of a single frame in pixels
     * @param frameH Height of a single frame in pixels
     */
    bool load(const std::string& path, int frameW, int frameH);

    /** Add an animation definition. */
    void addAnimation(const Animation& anim);

    /** Set active animation by name. Resets time if different. */
    void setAnimation(const std::string& name);

    /** Advance animation timer. Call once per frame. */
    void update(float dt);

    /**
     * Draw current frame.
     * @param x, y   Top-left position in virtual coords.
     * @param flipX  Mirror horizontally (for left-facing sprites).
     * @param tint   Colour tint (WHITE = no tint).
     */
    void draw(float x, float y, bool flipX = false, Color tint = WHITE) const;

    /** Draw at a specific frame (bypasses animation state). */
    void drawFrame(int frame, float x, float y, bool flipX = false, Color tint = WHITE) const;

    bool isLoaded()  const { return m_loaded; }
    int  frameWidth()  const { return m_frameW; }
    int  frameHeight() const { return m_frameH; }
    bool isAnimationDone() const;  // true when non-looping anim reached last frame

    void unload();

private:
    Texture2D m_texture{};
    bool      m_loaded  = false;
    int       m_frameW  = 0;
    int       m_frameH  = 0;
    int       m_cols    = 0;

    std::vector<Animation> m_animations;
    int   m_currentAnim   = -1;
    int   m_currentFrame  = 0;  // absolute frame index in sheet
    float m_frameTimer    = 0.0f;
    bool  m_animDone      = false;

    Rectangle frameRect(int absFrame) const;
    const Animation* findAnim(const std::string& name) const;
};
