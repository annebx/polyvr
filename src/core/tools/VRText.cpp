#include "VRText.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <new>

//flags mit  $ pkg-config --cflags pango und $ pkg-config --libs pango :)
#include <glib.h>
#include <stdlib.h>
#include <gmodule.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <pango/pangocairo.h>

#include "core/objects/material/VRTexture.h"


using namespace std;
using namespace OSG;


VRText* VRText::get() {
    static VRText* singleton_opt = new VRText();
    return singleton_opt;
}

VRTexturePtr VRText::create(string text, string font, int res, Color4f fg, Color4f bg) {
    resolution = res;
    return createBmp(text, font, fg, bg);
}

void VRText::analyzeText() {
    Nlines = 0;
    maxLineLength = 0;

    stringstream ss(text);
    for (string line; getline(ss, line); ) {
        maxLineLength = max(maxLineLength, line.size());
        Nlines++;
    }
}

void VRText::computeTexParams() {
    texWidth = resolution*maxLineLength;
    texHeight = resolution*1.5;
    texWidth += 2*padding;
    texHeight += 2*padding;
    texHeight *= Nlines;
}

void VRText::convertData(UChar8* data, int width, int height) {
    UChar8* buffer = new UChar8[height*width*4];
    int fullSize = height*width*4;//char sind 1 byte gros!
    int rowSize = width*4;

    //drehe die zeilen um
    for (int i = 0; i < height ; i++) {
        UChar8* dataRow = data + i*4*width;
        UChar8* bufferRow = buffer + (height-1-i)*4*width;
        memcpy(bufferRow, dataRow, rowSize);
    }

    //kopiere sie zurück nach data
    memcpy(data, buffer, fullSize);

    delete buffer;
}

VRTexturePtr VRText::createBmp (string text, string font, Color4f fg, Color4f bg) {
    this->text = text;
    analyzeText();
    computeTexParams();

    cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, texWidth, texHeight);
    cairo_t* cr = cairo_create (surface);
    PangoLayout* layout = pango_cairo_create_layout (cr);
    pango_layout_set_text(layout, text.c_str(), -1);
    //pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);//MACHT IRGENDWIE NIX

    //Pango Description
    PangoFontDescription* desc = pango_font_description_from_string (font.c_str());
    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);

    //hier wird gemalt!
    //background
    cairo_set_source_rgba(cr, bg[0],bg[1],bg[2],bg[3]);
    cairo_rectangle(cr, 0, 0, texWidth, texHeight);
    cairo_fill(cr);

    //text
    cairo_set_source_rgba (cr, fg[0],fg[1],fg[2],fg[3]);
    cairo_translate(cr, padding, padding);
    pango_cairo_update_layout (cr, layout);
    pango_cairo_show_layout (cr, layout);

    cairo_set_source_rgba(cr, bg[0],bg[1],bg[2],bg[3]);
    cairo_rectangle(cr, 0, 0, texWidth, padding-1); cairo_fill(cr);
    cairo_rectangle(cr, 0, texHeight-padding-1, texWidth, padding-1); cairo_fill(cr);
    cairo_rectangle(cr, 0, 0, padding-1, texHeight); cairo_fill(cr);
    cairo_rectangle(cr, texWidth-padding-1, 0, padding-1, texHeight); cairo_fill(cr);

    UChar8* data = cairo_image_surface_get_data(surface);
    convertData(data, texWidth, texHeight);

    VRTexturePtr tex = VRTexture::create();
    tex->getImage()->set( Image::OSG_BGRA_PF, texWidth, texHeight, 1, 1, 1, 0, data);

    cairo_destroy (cr);
    cairo_surface_destroy (surface);
    return tex;
}

