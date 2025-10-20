#include<FL/Fl.H> 
#include<FL/Fl_Box.H>
#include<FL/Fl_Window.H>
#include<FL/Fl_Widget.H>
#include<FL/fl_draw.H>

int main(){
Fl_Window window(1280,720, "Text Editor");
window.color(0x14141400);

Fl_Box box(40,40,1200,50);
box.box(FL_UP_BOX);
box.color(0x1E1E1E00);
box.labelsize(24);
box.labelfont(FL_HELVETICA_BOLD);
box.labelcolor(0xFFFFFF00);

window.end();
window.show();
return Fl::run();
}