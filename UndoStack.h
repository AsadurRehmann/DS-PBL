#ifndef UNDOSTACH_H
#define UNDOSTACK_H
#include <string>
#include <stack>
using namespace std;

struct Edit{

    enum type{insert, Delete} type;
    size_t pos;
    string text;
};
class undoStack{

    stack<Edit> undoStack, redoStack;
public:
    void  pushUndo(Edit e) {undoStack.push(e); while(!redoStack.empty()) redoStack.pop();}

    bool canUndo() const{return !undoStack.empty();}
    bool canRedo() const{return !redoStack.empty();}

    Edit popUndo() {Edit e = undoStack.top(); undoStack.pop(); redoStack.push(e); return e;}
    Edit popRedo() { Edit e = redoStack.top(); redoStack.pop(); undoStack.push(e); return e; }

    void clear(){ while (!undoStack.empty()) undoStack.pop(); while(!redoStack.empty()) redoStack.pop();}

};

#endif