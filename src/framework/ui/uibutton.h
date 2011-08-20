#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "uiwidget.h"

class UIButton : public UIWidget
{
    struct ButtonStateStyle {
        ImagePtr image;
        Point textTranslate;
        Color foregroundColor;
        Color color;
    };

public:
    UIButton();

    static UIButtonPtr create();

    virtual void loadStyleFromOTML(const OTMLNodePtr& styleNode);
    void loadStateStyle(ButtonStateStyle& stateStyle, const OTMLNodePtr& stateStyleNode);
    virtual void render();

    void setOnClick(const SimpleCallback& onClick) { m_onClick = onClick; }
    void setText(const std::string& text) { m_text = text; }

    SimpleCallback getOnClick() const { return m_onClick; }
    std::string getText() const { return m_text; }
    ButtonState getState() const { return m_state; }

    UIButtonPtr asUIButton() { return std::static_pointer_cast<UIButton>(shared_from_this()); }

protected:
    virtual void onHoverChange(UIHoverEvent& event);
    virtual void onMousePress(UIMouseEvent& event);
    virtual void onMouseRelease(UIMouseEvent& event);

    ButtonState m_state;
    ButtonStateStyle m_statesStyle[3];
    SimpleCallback m_onClick;
    std::string m_text;
};

#endif
