#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_draw.H>
#include <cstdlib>
#include "TextEditor.h"

TextEditor* editor = nullptr;

// Callbacks
void new_cb(Fl_Widget* w, void* data) { editor->newFile(); }
void open_cb(Fl_Widget* w, void* data) {
    Fl_Native_File_Chooser chooser;
    chooser.title("Open File");
    chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if (chooser.show() == 0) editor->loadFromFile(chooser.filename());
}
void save_cb(Fl_Widget* w, void* data) {
    Fl_Native_File_Chooser chooser;
    chooser.title("Save File");
    chooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    if (chooser.show() == 0) editor->saveToFile(chooser.filename());
}
void exit_cb(Fl_Widget* w, void* data) { exit(0); }
void undo_cb(Fl_Widget* w, void* data) { editor->undo(); }
void redo_cb(Fl_Widget* w, void* data) { editor->redo(); }
void copy_cb(Fl_Widget* w, void* data) { editor->copyToClipboard(); }
void cut_cb(Fl_Widget* w, void* data) { editor->cutToClipboard(); }
void paste_cb(Fl_Widget* w, void* data) { editor->pasteFromClipboard(); }
void zoom_in_cb(Fl_Widget* w, void* data) { editor->zoomIn(); }
void zoom_out_cb(Fl_Widget* w, void* data) { editor->zoomOut(); }

int main(int argc, char** argv) {
    Fl::scheme("gleam");
    Fl::background(35, 35, 35);
    Fl::background2(45, 45, 45);
    Fl::foreground(220, 220, 220);
    Fl::set_color(FL_SELECTION_COLOR, 60, 100, 160);

    // Attempt modern font
    Fl::set_font(FL_COURIER, "Cascadia Code");

    Fl_Double_Window* window = new Fl_Double_Window(1024, 768, "NeoVim-Style Editor");
    window->color(fl_rgb_color(30, 30, 30));

    Fl_Menu_Bar* menubar = new Fl_Menu_Bar(0, 0, 1024, 30);
    menubar->box(FL_FLAT_BOX);
    menubar->color(fl_rgb_color(25, 25, 25));
    menubar->textcolor(fl_rgb_color(200, 200, 200));
    menubar->selection_color(fl_rgb_color(60, 100, 160));
    menubar->textsize(14);

    menubar->add("File/New File",     FL_CTRL + 'n', new_cb);
    menubar->add("File/Open...",      FL_CTRL + 'o', open_cb);
    menubar->add("File/Save",         FL_CTRL + 's', save_cb);
    menubar->add("File/Exit",         FL_CTRL + 'q', exit_cb);
    menubar->add("Edit/Undo",         FL_CTRL + 'z', undo_cb);
    menubar->add("Edit/Redo",         FL_CTRL + 'y', redo_cb);
    menubar->add("Edit/Cut",          FL_CTRL + 'x', cut_cb);
    menubar->add("Edit/Copy",         FL_CTRL + 'c', copy_cb);
    menubar->add("Edit/Paste",        FL_CTRL + 'v', paste_cb);
    menubar->add("View/Zoom In",      FL_CTRL + '=', zoom_in_cb);
    menubar->add("View/Zoom Out",     FL_CTRL + '-', zoom_out_cb);

    editor = new TextEditor(0, 30, 1024, 738);

    window->end();
    window->resizable(editor);
    window->show(argc, argv);

    return Fl::run();
}