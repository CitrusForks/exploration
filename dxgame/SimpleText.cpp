#include "StdAfx.h"
#include "SimpleText.h"


IFW1Factory *SimpleText::st_factory = nullptr;


SimpleText::SimpleText()
{
}


void SimpleText::init(ID3D11Device *device, wchar_t *fontName, int fontSize)
{
    HRESULT hr;

    m_fontSize = fontSize;
    m_color = 0xFFFFFFFF;

    if (st_factory == nullptr)
    {
        hr = FW1CreateFactory(FW1_VERSION, &st_factory);
        if (FAILED(hr))
        {
            Errors::Cry("Could not initialize FW1FontWrapper so enjoy having no text on the screen, I guess.");
            return;
        }
    }

    hr = st_factory->CreateFontWrapper(device, fontName, &m_fontWrapper);
    if (FAILED(hr))
    {
        Errors::Cry("Could not initialize font :(");
    }
}


SimpleText::~SimpleText(void)
{
}


// call this once per SimpleText instance when you don't need them:
void SimpleText::Release()
{
    m_fontWrapper->Release();
}


// call this before exiting the program, if you like deallocating all the things by hand:
void SimpleText::ReleaseFactory()
{
    st_factory->Release();
    st_factory = nullptr;
}

// color components in [0.0..1.0] range
void SimpleText::setColor( float r, float g, float b, float a )
{
    m_color = (int)(r*255) + (((int)(g*255)) << 8) + (((int)(b*255)) << 16) + (((int)(a*255)) << 24);
}

void SimpleText::write( ID3D11DeviceContext *devCtx, wchar_t *message, float x, float y, int fontsize /*= -1*/ )
{
    m_fontWrapper->DrawString(
        devCtx,
        message,// String
        (float)(fontsize == -1 ? m_fontSize : fontsize),// Font size
        x,// X position
        y,// Y position
        m_color,// Text color, 0xAaBbGgRr
        FW1_RESTORESTATE// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
        );

}


