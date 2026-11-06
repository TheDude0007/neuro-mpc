#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Skins. Each is original artwork in the spirit of a machine: NEON 2K5 (classic hardware, neon yellow underlight, disco
// purple edge lights), 3842 (far-future: magenta and ice white hairlines), GOLD (a modern flagship cast in gold).
namespace neuro::ui
{

struct Theme
{
    const char* name;
    juce::Colour bgTop, bgBottom, surface, surfaceHi, accent, glow, glow2, text, onText, screen, screenCell, screenText, wave;
    float radius;
    bool metal, hairline, disco;
};

// Literal colours only: juce::Colours constants may not be initialised yet at static-init time (they'd be transparent).
inline const std::array<Theme, 3> themes { {
    { "NEON 2K5", juce::Colour (0xff17102a), juce::Colour (0xff0b0714), juce::Colour (0xff261a3c), juce::Colour (0xff3a2758),
      juce::Colour (0xffffee00), juce::Colour (0xffb026ff), juce::Colour (0xffffee00), juce::Colour (0xfffff9c4), juce::Colour (0xff000000),
      juce::Colour (0xff0d0818), juce::Colour (0xff221638), juce::Colour (0xfffff9c4), juce::Colour (0xffffee00), 5.0f, false, false, true },
    { "3842", juce::Colour (0xff120a1e), juce::Colour (0xff06050d), juce::Colour (0xff150f24), juce::Colour (0xff241a38),
      juce::Colour (0xffff1fb4), juce::Colour (0xffe6f7ff), juce::Colour (0xffff1fb4), juce::Colour (0xffeaf8ff), juce::Colour (0xffffffff),
      juce::Colour (0xff05040c), juce::Colour (0xff17122a), juce::Colour (0xffeaf8ff), juce::Colour (0xffe6f7ff), 1.5f, false, true, false },
    { "GOLD", juce::Colour (0xffeccb62), juce::Colour (0xffa87a22), juce::Colour (0xffd9b44a), juce::Colour (0xfffbe9a0),
      juce::Colour (0xfffff4b8), juce::Colour (0xffffd700), juce::Colour (0xfffff4b8), juce::Colour (0xff1c1305), juce::Colour (0xff1c1305),
      juce::Colour (0xff140e03), juce::Colour (0xff2c2008), juce::Colour (0xffffe9a8), juce::Colour (0xffffd76a), 4.0f, true, false, false },
} };

class NeuroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NeuroLookAndFeel() { setTheme (0); }

    const Theme& theme() const noexcept { return themes[(size_t) index]; }
    int getThemeIndex() const noexcept { return index; }

    void setTheme (int newIndex)
    {
        index = juce::jlimit (0, (int) themes.size() - 1, newIndex);
        const auto& t = theme();
        using L = juce::Label; using S = juce::Slider; using C = juce::ComboBox; using P = juce::PopupMenu; using T = juce::TextButton; using G = juce::ToggleButton;
        for (auto [id, colour] : std::initializer_list<std::pair<int, juce::Colour>> {
                 { L::textColourId, t.text }, { T::textColourOffId, t.text }, { T::textColourOnId, t.onText }, { T::buttonColourId, t.surface },
                 { G::textColourId, t.text }, { G::tickColourId, t.metal ? t.onText : t.accent }, { G::tickDisabledColourId, t.text.withAlpha (0.6f) },
                 { C::backgroundColourId, t.metal ? t.surfaceHi : t.surface }, { C::textColourId, t.text }, { C::outlineColourId, t.accent.withAlpha (0.5f) }, { C::arrowColourId, t.text },
                 { S::textBoxTextColourId, t.text }, { S::textBoxBackgroundColourId, (t.metal ? t.surfaceHi : t.screen) }, { S::textBoxOutlineColourId, t.accent.withAlpha (0.4f) },
                 { P::backgroundColourId, t.screen }, { P::textColourId, t.screenText }, { P::highlightedBackgroundColourId, t.accent }, { P::highlightedTextColourId, t.metal ? t.onText : juce::Colours::black },
                 { juce::TextEditor::textColourId, t.text }, { juce::TextEditor::backgroundColourId, t.metal ? t.surfaceHi : t.screen }, { juce::TextEditor::highlightColourId, t.accent.withAlpha (0.4f) },
                 { juce::TooltipWindow::backgroundColourId, t.screen }, { juce::TooltipWindow::textColourId, t.screenText } })
            setColour (id, colour);
    }

    // Light spilling out from under a shape: nested strokes fading outwards.
    static void glowAround (juce::Graphics& g, juce::Rectangle<float> r, float radius, juce::Colour c, float strength, int layers = 3)
    {
        for (int i = layers; i > 0; --i)
        {
            g.setColour (c.withMultipliedAlpha (strength * 0.45f / (float) i));
            g.drawRoundedRectangle (r.expanded ((float) i * 0.8f), radius + (float) i * 0.8f, 1.2f);
        }
    }

    // A control's face: underlit from below, or polished metal.
    static void fillFace (juce::Graphics& g, juce::Rectangle<float> r, const Theme& t, float light)
    {
        if (t.metal)
            g.setGradientFill (juce::ColourGradient::vertical (t.surfaceHi, r.getY(), t.surface.darker (0.25f), r.getBottom()));
        else
            g.setGradientFill (juce::ColourGradient::vertical (t.surface, r.getY(), t.surface.interpolatedWith (t.accent, light), r.getBottom()));

        g.fillRoundedRectangle (r, t.radius);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour& bg, bool over, bool down) override
    {
        const auto& t = theme();
        auto r = b.getLocalBounds().toFloat().reduced (2.0f);
        const auto on = b.getToggleState() || down;

        if (b.isColourSpecified (juce::TextButton::buttonColourId)) // step grids and the like carry their own state colour
        {
            g.setColour (bg);
            g.fillRoundedRectangle (r, t.radius);
            return;
        }

        if (on)
        {
            glowAround (g, r, t.radius, t.accent, 1.0f);
            g.setColour (t.accent);
            g.fillRoundedRectangle (r, t.radius);
        }
        else
        {
            glowAround (g, r, t.radius, t.metal ? t.onText : t.accent, over ? 0.6f : 0.3f, 2);
            fillFace (g, r, t, over ? 0.45f : 0.28f);
        }

        g.setColour (t.hairline ? t.glow.withAlpha (on ? 1.0f : 0.55f) : t.metal ? t.onText.withAlpha (0.45f) : t.accent.withAlpha (on ? 1.0f : 0.6f));
        g.drawRoundedRectangle (r, t.radius, t.hairline ? 0.7f : 1.0f);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override
    {
        const auto& t = theme();
        auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
        const auto size = juce::jmin (r.getWidth(), r.getHeight());
        const auto c = r.getCentre();
        const auto radius = size * 0.5f, ring = t.hairline ? 1.5f : 3.0f, angle = start + pos * (end - start);
        auto arc = [&] (float from, float to, float rad) { juce::Path p; p.addCentredArc (c.x, c.y, rad, rad, 0.0f, from, to, true); return p; };

        if (t.metal)
        {
            g.setGradientFill (juce::ColourGradient (t.surfaceHi, c.x - radius, c.y - radius, t.surface.darker (0.45f), c.x + radius, c.y + radius, false));
            g.fillEllipse (c.x - radius * 0.72f, c.y - radius * 0.72f, radius * 1.44f, radius * 1.44f);
        }

        g.setColour ((t.metal ? t.onText : t.surfaceHi).withAlpha (0.6f));
        g.strokePath (arc (start, end, radius - ring), juce::PathStrokeType (ring));

        for (int i = 3; i > 0; --i) // lit arc with bloom
        {
            g.setColour ((t.metal ? t.onText : t.accent).withAlpha (t.metal ? 0.0f : 0.14f));
            g.strokePath (arc (start, angle, radius - ring), juce::PathStrokeType (ring + (float) i * 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour (t.metal ? t.onText : t.accent);
        g.strokePath (arc (start, angle, radius - ring), juce::PathStrokeType (ring, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        const auto tip = c.getPointOnCircumference (radius - ring * 2.5f, angle), root = c.getPointOnCircumference (radius * 0.28f, angle);
        g.setColour (t.hairline ? t.glow : t.metal ? t.onText : t.accent);
        g.drawLine ({ root, tip }, t.hairline ? 1.0f : 2.0f);
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float minPos, float maxPos, juce::Slider::SliderStyle style, juce::Slider& s) override
    {
        if (style != juce::Slider::LinearHorizontal)
            return LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, pos, minPos, maxPos, style, s);

        const auto& t = theme();
        const auto cy = (float) y + (float) h * 0.5f, track = t.hairline ? 1.0f : 3.0f;
        const auto lit = t.metal ? t.onText : t.accent;
        g.setColour ((t.metal ? t.onText : t.surfaceHi).withAlpha (0.45f));
        g.fillRoundedRectangle ((float) x, cy - track * 0.5f, (float) w, track, track * 0.5f);
        g.setColour (lit);
        g.fillRoundedRectangle ((float) x, cy - track * 0.5f, pos - (float) x, track, track * 0.5f);
        g.setColour (lit.withAlpha (t.metal ? 0.0f : 0.3f));
        g.fillEllipse (pos - 9.0f, cy - 9.0f, 18.0f, 18.0f);
        g.setColour (t.hairline ? t.glow : lit);
        g.fillEllipse (pos - 5.0f, cy - 5.0f, 10.0f, 10.0f);
    }

private:
    int index = 0;
};

inline const Theme& themeOf (const juce::Component& c)
{
    if (auto* laf = dynamic_cast<NeuroLookAndFeel*> (&c.getLookAndFeel()))
        return laf->theme();
    return themes[0];
}

} // namespace neuro::ui
