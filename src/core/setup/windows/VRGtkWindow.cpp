#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "VRGtkWindow.h"
#include "core/gui/VRGuiUtils.h"
#include <boost/thread/recursive_mutex.hpp>

#include "../devices/VRKeyboard.h"
#include "../devices/VRMouse.h"
#include "core/utils/VRTimer.h"
#include "core/utils/VROptions.h"
#include "core/utils/VRGlobals.h"
#include "core/utils/VRProfiler.h"
#include "core/scene/VRScene.h"
#include "core/gui/VRGuiManager.h"


typedef boost::recursive_mutex::scoped_lock PLock;

using namespace OSG;
using namespace std;
namespace PL = std::placeholders;

#if GTK_MAJOR_VERSION == 2
#include "VRGtk2Window.h"
#else
//#include "VRGtk3Window.h"
#include "VRGtk32Window.h"
#endif

VRGtkWindow::~VRGtkWindow() {
    win = NULL;
    if (hmd) hmd.reset();
}

VRGtkWindowPtr VRGtkWindow::ptr() { return static_pointer_cast<VRGtkWindow>( shared_from_this() ); }
VRGtkWindowPtr VRGtkWindow::create(GtkDrawingArea* da, string msaa) { return shared_ptr<VRGtkWindow>(new VRGtkWindow(da, msaa) ); }

void VRGtkWindow::setCursor(string c) {
    GdkWindow* win = gtk_widget_get_window(widget);
    if (c == "") { gdk_window_set_cursor(win, NULL); return; }

    GdkCursorType cursor;

    if (c == "X_CURSOR") cursor = GDK_X_CURSOR;
    if (c == "ARROW") cursor = GDK_ARROW;
    if (c == "BASED_ARROW_DOWN") cursor = GDK_BASED_ARROW_DOWN;
    if (c == "BASED_ARROW_UP") cursor = GDK_BASED_ARROW_UP;
    if (c == "BOAT") cursor = GDK_BOAT;
    if (c == "BOGOSITY") cursor = GDK_BOGOSITY;
    if (c == "BOTTOM_LEFT_CORNER") cursor = GDK_BOTTOM_LEFT_CORNER;
    if (c == "BOTTOM_RIGHT_CORNER") cursor = GDK_BOTTOM_RIGHT_CORNER;
    if (c == "BOTTOM_SIDE") cursor = GDK_BOTTOM_SIDE;
    if (c == "BOTTOM_TEE") cursor = GDK_BOTTOM_TEE;
    if (c == "BOX_SPIRAL") cursor = GDK_BOX_SPIRAL;
    if (c == "CENTER_PTR") cursor = GDK_CENTER_PTR;
    if (c == "CIRCLE") cursor = GDK_CIRCLE;
    if (c == "CLOCK") cursor = GDK_CLOCK;
    if (c == "COFFEE_MUG") cursor = GDK_COFFEE_MUG;
    if (c == "CROSS") cursor = GDK_CROSS;
    if (c == "CROSS_REVERSE") cursor = GDK_CROSS_REVERSE;
    if (c == "CROSSHAIR") cursor = GDK_CROSSHAIR;
    if (c == "DIAMOND_CROSS") cursor = GDK_DIAMOND_CROSS;
    if (c == "DOT") cursor = GDK_DOT;
    if (c == "DOTBOX") cursor = GDK_DOTBOX;
    if (c == "DOUBLE_ARROW") cursor = GDK_DOUBLE_ARROW;
    if (c == "DRAFT_LARGE") cursor = GDK_DRAFT_LARGE;
    if (c == "DRAFT_SMALL") cursor = GDK_DRAFT_SMALL;
    if (c == "DRAPED_BOX") cursor = GDK_DRAPED_BOX;
    if (c == "EXCHANGE") cursor = GDK_EXCHANGE;
    if (c == "FLEUR") cursor = GDK_FLEUR;
    if (c == "GOBBLER") cursor = GDK_GOBBLER;
    if (c == "GUMBY") cursor = GDK_GUMBY;
    if (c == "HAND1") cursor = GDK_HAND1;
    if (c == "HAND2") cursor = GDK_HAND2;
    if (c == "HEART") cursor = GDK_HEART;
    if (c == "ICON") cursor = GDK_ICON;
    if (c == "IRON_CROSS") cursor = GDK_IRON_CROSS;
    if (c == "LEFT_PTR") cursor = GDK_LEFT_PTR;
    if (c == "LEFT_SIDE") cursor = GDK_LEFT_SIDE;
    if (c == "LEFT_TEE") cursor = GDK_LEFT_TEE;
    if (c == "LEFTBUTTON") cursor = GDK_LEFTBUTTON;
    if (c == "LL_ANGLE") cursor = GDK_LL_ANGLE;
    if (c == "LR_ANGLE") cursor = GDK_LR_ANGLE;
    if (c == "MAN") cursor = GDK_MAN;
    if (c == "MIDDLEBUTTON") cursor = GDK_MIDDLEBUTTON;
    if (c == "MOUSE") cursor = GDK_MOUSE;
    if (c == "PENCIL") cursor = GDK_PENCIL;
    if (c == "PIRATE") cursor = GDK_PIRATE;
    if (c == "PLUS") cursor = GDK_PLUS;
    if (c == "QUESTION_ARROW") cursor = GDK_QUESTION_ARROW;
    if (c == "RIGHT_PTR") cursor = GDK_RIGHT_PTR;
    if (c == "RIGHT_SIDE") cursor = GDK_RIGHT_SIDE;
    if (c == "RIGHT_TEE") cursor = GDK_RIGHT_TEE;
    if (c == "RIGHTBUTTON") cursor = GDK_RIGHTBUTTON;
    if (c == "RTL_LOGO") cursor = GDK_RTL_LOGO;
    if (c == "SAILBOAT") cursor = GDK_SAILBOAT;
    if (c == "SB_DOWN_ARROW") cursor = GDK_SB_DOWN_ARROW;
    if (c == "SB_H_DOUBLE_ARROW") cursor = GDK_SB_H_DOUBLE_ARROW;
    if (c == "SB_LEFT_ARROW") cursor = GDK_SB_LEFT_ARROW;
    if (c == "SB_RIGHT_ARROW") cursor = GDK_SB_RIGHT_ARROW;
    if (c == "SB_UP_ARROW") cursor = GDK_SB_UP_ARROW;
    if (c == "SB_V_DOUBLE_ARROW") cursor = GDK_SB_V_DOUBLE_ARROW;
    if (c == "SHUTTLE") cursor = GDK_SHUTTLE;
    if (c == "SIZING") cursor = GDK_SIZING;
    if (c == "SPIDER") cursor = GDK_SPIDER;
    if (c == "SPRAYCAN") cursor = GDK_SPRAYCAN;
    if (c == "STAR") cursor = GDK_STAR;
    if (c == "TARGET") cursor = GDK_TARGET;
    if (c == "TCROSS") cursor = GDK_TCROSS;
    if (c == "TOP_LEFT_ARROW") cursor = GDK_TOP_LEFT_ARROW;
    if (c == "TOP_LEFT_CORNER") cursor = GDK_TOP_LEFT_CORNER;
    if (c == "TOP_RIGHT_CORNER") cursor = GDK_TOP_RIGHT_CORNER;
    if (c == "TOP_SIDE") cursor = GDK_TOP_SIDE;
    if (c == "TOP_TEE") cursor = GDK_TOP_TEE;
    if (c == "TREK") cursor = GDK_TREK;
    if (c == "UL_ANGLE") cursor = GDK_UL_ANGLE;
    if (c == "UMBRELLA") cursor = GDK_UMBRELLA;
    if (c == "UR_ANGLE") cursor = GDK_UR_ANGLE;
    if (c == "WATCH") cursor = GDK_WATCH;
    if (c == "XTERM") cursor = GDK_XTERM;

    auto cur = gdk_cursor_new(cursor);
    gdk_window_set_cursor(win, cur);
#if GTK_MAJOR_VERSION == 2
    gdk_cursor_destroy(cur);
#endif
}

bool VRGtkWindow::on_button(GdkEventButton* event) {
    gtk_widget_grab_focus(widget);
    int state = 1;
    if (event->type == GDK_BUTTON_PRESS) state = 0;

    if (getMouse() == 0) return false;
    getMouse()->mouse(event->button -1,state ,event->x ,event->y);

	/*printf("\nbutton: %i %i", event->button, event->type);
	printf("\n   x : %f", event->x);
	printf("\n   y : %f", event->y);
	printf("\n");*/
	return true;
}

bool VRGtkWindow::on_motion(GdkEventMotion * event) {
    //gtk_widget_grab_focus((GtkWidget*)drawArea->gobj());
    if (getMouse() == 0) return false;
    getMouse()->motion(event->x ,event->y);

	/*printf("\nevent: %i", event->type);
	printf("\n   x : %f", event->x);
	printf("\n   y : %f", event->y);
	printf("\n");*/
	return true;
}

bool VRGtkWindow::on_key(GdkEventKey *event) {
    if (event->keyval >= 65470 && event->keyval <= 65481) return false; //F keys
    //VRKeyboard::get()->keyboard(event->keyval, 0, 0); // TODO: check the values!!
	//printf("\n KEY: %i %i %i\n", event->keyval, event->type, event->state);
	//cout << "\n KEY: " << event->keyval << " " << event->type << " " << event->state << endl;

    if (getKeyboard() == 0) return false;
    getKeyboard()->setGtkEvent(event);
    getKeyboard()->keyboard(event->keyval, (event->type == 8), 0, 0);

	return true;
}

bool VRGtkWindow::on_scroll(GdkEventScroll * event) {
    int button = 999;
    if (event->direction == GDK_SCROLL_DOWN) button = 3;
    if (event->direction == GDK_SCROLL_UP) button = 4;
    if (button == 999) return false;

    if (getMouse() == 0) return false;
    getMouse()->mouse(button, 0 ,event->x ,event->y);

    return true;
}

void VRGtkWindow::on_resize(GdkRectangle* allocation) {
    initialExpose = true;
    resize(allocation->width, allocation->height);
}

void printGLversion() {
    //const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString (GL_VERSION); // version as a string
    cout << "Supported OpenGL version: " << version << endl;
}

void VRGtkWindow::save(XMLElementPtr node) { VRWindow::save(node); }
void VRGtkWindow::load(XMLElementPtr node) { VRWindow::load(node); }






