#ifndef SIMPLETEXT_H
#define SIMPLETEXT_H

#include <d3d11.h>
#include <FW1FontWrapper.h> // see http://fw1.codeplex.com/documentation

class SimpleText
{
private:
    static IFW1Factory *st_factory;

    IFW1FontWrapper *m_fontWrapper;
    int m_fontSize;

    unsigned int m_color; // 0xAaBbGgRr because that's what the library uses

public:
    SimpleText();
    ~SimpleText(void);

    void init(ID3D11Device *device, wchar_t *fontName, int fontSize = 24);
    void write(ID3D11DeviceContext *, wchar_t *message, float x, float y, int fontsize = -1);
    void setColor(float r, float g, float b, float a);

    void Release();
    static void ReleaseFactory(); // bleh
};


#endif
