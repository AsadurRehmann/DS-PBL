#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "EditorWidget.h"

int main() {
    Fl_Window win(800, 600, "Simple C++ Text Editor");
    EditorWidget editor(10, 10, 780, 580);
    win.end();
    win.show();
    return Fl::run();
}
