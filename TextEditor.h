#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <FL/Fl_Widget.H>
#include "GapBuffer.h"
#include "Stack.h"
#include "EditorState.h"

class TextEditor : public Fl_Widget {
private:
    GapBuffer gapBuffer;
    int cursorPos;
    int selectionStart;
    int selectionEnd;
    bool selecting;

    // Layout & Styling
    int firstVisibleLine;
    int lineHeight;
    int charWidth;
    int gutterWidth;
    int fontSize;

    char mode;
    char statusMsg[256];

    Stack<EditorState> undoStack;
    Stack<EditorState> redoStack;

    // Internal helpers (Private)
    void moveCursorUp();
    void moveCursorDown();
    void saveState();
    void updateScroll();
    int xyToIndex(int x, int y); // Helper for mouse clicks

public:
    TextEditor(int X, int Y, int W, int H);

    void draw() override;
    int handle(int event) override;

    // File Operations
    void newFile();
    void loadFromFile(const char* filename);
    void saveToFile(const char* filename);

    // Edit Operations
    void undo();
    void redo();

    // Clipboard Operations (Now Public)
    void copyToClipboard();
    void cutToClipboard();
    void pasteFromClipboard();

    // Selection Helpers
    void startSelection();
    void updateSelection();
    void clearSelection();
    void deleteSelection();
    bool hasSelection() const;

    // View Operations
    void zoomIn();
    void zoomOut();
};

#endif