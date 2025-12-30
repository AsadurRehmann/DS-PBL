#include "TextEditor.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cstring>
#include <fstream>
#include <cstdio>
#include <algorithm>

TextEditor::TextEditor(int X, int Y, int W, int H)
    : Fl_Widget(X, Y, W, H),
      cursorPos(0), selectionStart(-1), selectionEnd(-1), selecting(false),
      firstVisibleLine(0), fontSize(16), gutterWidth(50), mode('n') {
    strcpy(statusMsg, "-- NORMAL --");
}

void TextEditor::saveState() {
    char* text = new char[gapBuffer.getLength() + 1];
    gapBuffer.getText(text, gapBuffer.getLength() + 1);

    EditorState state(text, cursorPos, selectionStart, selectionEnd);
    undoStack.push(state);
    redoStack.clear();

    delete[] text;
}

void TextEditor::deleteSelection() {
    if (!hasSelection()) return;

    int start = std::min(selectionStart, selectionEnd);
    int end = std::max(selectionStart, selectionEnd);

    gapBuffer.moveCursorTo(start);
    for (int i = start; i < end; i++) {
        gapBuffer.deleteRight();
    }

    cursorPos = start;
    clearSelection();
}

// Ensure the cursor is always inside the visible window
void TextEditor::updateScroll() {
    int currentLine = 0;
    for(int i = 0; i < cursorPos; i++) {
        if(gapBuffer.getCharAt(i) == '\n') currentLine++;
    }

    int visibleLines = (h() - 30) / lineHeight;

    if (currentLine >= firstVisibleLine + visibleLines) {
        firstVisibleLine = currentLine - visibleLines + 1;
    }
    if (currentLine < firstVisibleLine) {
        firstVisibleLine = currentLine;
    }
}

// --- Drawing Logic ---
void TextEditor::draw() {
    fl_font(FL_COURIER, fontSize);
    lineHeight = fontSize + 4;
    charWidth = (int)fl_width('M');

    // Background
    fl_color(30, 30, 30);
    fl_rectf(x(), y(), w(), h());

    // Gutter
    fl_color(22, 22, 22);
    fl_rectf(x(), y(), gutterWidth, h());
    fl_color(50, 50, 50);
    fl_line(x() + gutterWidth, y(), x() + gutterWidth, y() + h());

    int textAreaX = x() + gutterWidth + 8;
    int cy = y() + lineHeight;

    // Calculate cursor line index
    int cursorLineIndex = 0;
    for(int i=0; i<cursorPos; i++) if(gapBuffer.getCharAt(i) == '\n') cursorLineIndex++;

    // Highlight Active Line
    int activeLineY = y() + (cursorLineIndex - firstVisibleLine) * lineHeight;
    if (activeLineY >= y() && activeLineY < y() + h() - 30) {
        fl_color(45, 45, 45);
        fl_rectf(x() + gutterWidth + 1, activeLineY, w() - gutterWidth, lineHeight);
    }

    // Text Drawing Loop
    int textLen = gapBuffer.getLength();
    int currentLine = 0;
    int cx = textAreaX;

    // Optimization: Skip lines before firstVisibleLine
    int startIndex = 0;
    if (firstVisibleLine > 0) {
        int linesSkipped = 0;
        for (int i = 0; i < textLen; i++) {
            if (gapBuffer.getCharAt(i) == '\n') {
                linesSkipped++;
                if (linesSkipped == firstVisibleLine) {
                    startIndex = i + 1;
                    currentLine = linesSkipped;
                    break;
                }
            }
        }
    }

    // Draw first line number
    fl_color(90, 90, 90);
    char lineNumStr[16];
    sprintf(lineNumStr, "%3d", currentLine + 1);
    fl_draw(lineNumStr, x() + 5, cy);

    for (int i = startIndex; i < textLen; i++) {
        if (cy > y() + h() - 30) break;

        char c = gapBuffer.getCharAt(i);

        bool isSelected = hasSelection() &&
                        ((selectionStart <= i && i < selectionEnd) ||
                         (selectionEnd <= i && i < selectionStart));

        if (isSelected) {
            fl_color(60, 100, 160);
            fl_rectf(cx, cy - lineHeight + 4, charWidth, lineHeight);
        }

        if (c == '\n') {
            cy += lineHeight;
            cx = textAreaX;
            currentLine++;

            if (cy <= y() + h() - 30) {
                fl_color(currentLine == cursorLineIndex ? 200 : 90,
                         currentLine == cursorLineIndex ? 200 : 90,
                         currentLine == cursorLineIndex ? 200 : 90);
                sprintf(lineNumStr, "%3d", currentLine + 1);
                fl_draw(lineNumStr, x() + 5, cy);
            }
        } else {
            if (isSelected) fl_color(FL_WHITE);
            else fl_color(220, 220, 220);

            char str[2] = {c, '\0'};
            fl_draw(str, cx, cy);
            cx += charWidth;
        }
    }

    // Draw Cursor
    int cursorCol = 0;
    int scanPos = cursorPos;
    while(scanPos > 0 && gapBuffer.getCharAt(scanPos-1) != '\n') {
        scanPos--;
        cursorCol++;
    }

    int cursorScreenY = y() + (cursorLineIndex - firstVisibleLine + 1) * lineHeight;
    int cursorScreenX = textAreaX + (cursorCol * charWidth);

    if (cursorLineIndex >= firstVisibleLine && cursorScreenY <= y() + h() - 25) {
        if (mode == 'i') {
            fl_color(FL_GREEN);
            fl_rectf(cursorScreenX, cursorScreenY - lineHeight + 4, 2, lineHeight);
        } else {
            fl_color(FL_WHITE);
            fl_rectf(cursorScreenX, cursorScreenY - lineHeight + 4, charWidth, lineHeight);
            if (cursorPos < textLen && gapBuffer.getCharAt(cursorPos) != '\n') {
                fl_color(FL_BLACK);
                char str[2] = {gapBuffer.getCharAt(cursorPos), '\0'};
                fl_draw(str, cursorScreenX, cursorScreenY);
            }
        }
    }

    // Status Bar
    int barHeight = 30;
    int barY = y() + h() - barHeight;
    Fl_Color statusColor = (mode == 'i') ? fl_rgb_color(0, 122, 204) : fl_rgb_color(90, 50, 150);
    fl_color(statusColor);
    fl_rectf(x(), barY, w(), barHeight);

    fl_color(FL_WHITE);
    fl_font(FL_COURIER_BOLD, 14);
    fl_draw(statusMsg, x() + 10, barY + 20);

    char posInfo[100];
    sprintf(posInfo, "Ln %d, Col %d | %d%%", cursorLineIndex + 1, cursorCol + 1, (int)(fontSize/1.6 * 10));
    fl_draw(posInfo, x() + w() - 200, barY + 20);
}

// --- Helper: Mouse to Index ---
int TextEditor::xyToIndex(int mouseX, int mouseY) {
    int textAreaX = x() + gutterWidth + 8;
    int clickVisualLine = (mouseY - y()) / lineHeight;
    int targetLine = clickVisualLine + firstVisibleLine - 1;
    if (targetLine < 0) targetLine = 0;

    int targetCol = (mouseX - textAreaX + (charWidth/2)) / charWidth;
    if (targetCol < 0) targetCol = 0;

    int currentLine = 0;
    int len = gapBuffer.getLength();
    int i = 0;

    while (currentLine < targetLine && i < len) {
        if (gapBuffer.getCharAt(i) == '\n') currentLine++;
        i++;
    }

    int currentCol = 0;
    while (currentCol < targetCol && i < len && gapBuffer.getCharAt(i) != '\n') {
        i++;
        currentCol++;
    }

    return i;
}

// --- Clipboard & Zoom ---
void TextEditor::copyToClipboard() {
    if (!hasSelection()) return;
    int start = std::min(selectionStart, selectionEnd);
    int end = std::max(selectionStart, selectionEnd);
    int len = end - start;

    char* text = new char[len + 1];
    gapBuffer.moveCursorTo(start);
    for(int i=0; i<len; i++) text[i] = gapBuffer.getCharAt(start + i);
    text[len] = '\0';

    Fl::copy(text, len, 1);
    delete[] text;
    strcpy(statusMsg, "Copied");
}

void TextEditor::cutToClipboard() {
    copyToClipboard();
    saveState();
    deleteSelection();
    redraw();
    strcpy(statusMsg, "Cut");
}

void TextEditor::pasteFromClipboard() {
    Fl::paste(*this, 1);
}

void TextEditor::zoomIn() { if (fontSize < 32) { fontSize += 2; redraw(); } }
void TextEditor::zoomOut() { if (fontSize > 8) { fontSize -= 2; redraw(); } }

// --- Event Handling ---
int TextEditor::handle(int event) {
    switch(event) {
        case FL_FOCUS: return 1;

        case FL_PUSH: {
            if (Fl::event_button() == FL_LEFT_MOUSE) {
                int newPos = xyToIndex(Fl::event_x(), Fl::event_y());
                selectionStart = newPos;
                selectionEnd = newPos;
                selecting = true;
                cursorPos = newPos;
                redraw();
                take_focus();
                return 1;
            }
            return 0;
        }

        case FL_DRAG: {
             if (selecting) {
                cursorPos = xyToIndex(Fl::event_x(), Fl::event_y());
                selectionEnd = cursorPos;
                redraw();
                return 1;
             }
             return 0;
        }

        case FL_RELEASE: {
            selecting = false;
            return 1;
        }

        case FL_MOUSEWHEEL: {
            firstVisibleLine += (Fl::event_dy() * 3);
            if(firstVisibleLine < 0) firstVisibleLine = 0;
            redraw();
            return 1;
        }

        case FL_PASTE: {
            saveState();
            if (hasSelection()) deleteSelection();
            const char* text = Fl::event_text();
            int len = Fl::event_length();
            gapBuffer.moveCursorTo(cursorPos);
            for(int i=0; i<len; i++) gapBuffer.insert(text[i]);
            cursorPos += len;
            updateScroll();
            strcpy(statusMsg, "Pasted");
            redraw();
            return 1;
        }

        case FL_KEYDOWN: {
            int key = Fl::event_key();

            // Shortcuts
            if (Fl::event_state(FL_CTRL)) {
                if (key == 'c') { copyToClipboard(); return 1; }
                if (key == 'v') { pasteFromClipboard(); return 1; }
                if (key == 'x') { cutToClipboard(); return 1; }
                if (key == 'a') {
                    selectionStart = 0;
                    selectionEnd = gapBuffer.getLength();
                    cursorPos = selectionEnd;
                    redraw();
                    return 1;
                }
                if (key == '=' || key == '+') { zoomIn(); return 1; }
                if (key == '-') { zoomOut(); return 1; }
                if (key == 'z') { undo(); return 1; }
                if (key == 'y') { redo(); return 1; }
            }

            // Navigation Keys
            bool shift = Fl::event_state(FL_SHIFT);
            if (key == FL_Left) {
                if (shift) startSelection(); else clearSelection();
                if (cursorPos > 0) cursorPos--;
                if (shift) updateSelection();
                updateScroll(); redraw(); return 1;
            }
            if (key == FL_Right) {
                if (shift) startSelection(); else clearSelection();
                if (cursorPos < gapBuffer.getLength()) cursorPos++;
                if (shift) updateSelection();
                updateScroll(); redraw(); return 1;
            }
            if (key == FL_Up) {
                if (shift) startSelection(); else clearSelection();
                moveCursorUp();
                if (shift) updateSelection();
                updateScroll(); redraw(); return 1;
            }
            if (key == FL_Down) {
                if (shift) startSelection(); else clearSelection();
                moveCursorDown();
                if (shift) updateSelection();
                updateScroll(); redraw(); return 1;
            }

            // Insert Mode Typing
            if (mode == 'i') {
                if (key == FL_BackSpace) {
                    if(cursorPos > 0 || hasSelection()) {
                        saveState();
                        if (hasSelection()) deleteSelection();
                        else {
                            gapBuffer.moveCursorTo(cursorPos);
                            gapBuffer.deleteLeft();
                            cursorPos--;
                        }
                        updateScroll(); redraw();
                    }
                    return 1;
                }
                if (key == FL_Enter) {
                    saveState();
                    if (hasSelection()) deleteSelection();
                    gapBuffer.moveCursorTo(cursorPos);
                    gapBuffer.insert('\n');
                    cursorPos++;
                    updateScroll(); redraw(); return 1;
                }
                const char* text = Fl::event_text();
                if (text && text[0] >= 32 && text[0] <= 126 && !Fl::event_state(FL_CTRL)) {
                    saveState();
                    if (hasSelection()) deleteSelection();
                    gapBuffer.moveCursorTo(cursorPos);
                    gapBuffer.insert(text[0]);
                    cursorPos++;
                    updateScroll(); redraw(); return 1;
                }
            }

            // Mode Switching
            if (mode == 'n' && key == 'i') { mode = 'i'; strcpy(statusMsg, "-- INSERT --"); redraw(); return 1; }
            if (mode == 'i' && key == FL_Escape) { mode = 'n'; strcpy(statusMsg, "-- NORMAL --"); redraw(); return 1; }

            // Normal Mode Commands
            if (mode == 'n') {
                if (key == 'h' && cursorPos > 0) { cursorPos--; updateScroll(); redraw(); return 1; }
                if (key == 'l' && cursorPos < gapBuffer.getLength()) { cursorPos++; updateScroll(); redraw(); return 1; }
                if (key == 'j') { moveCursorDown(); updateScroll(); redraw(); return 1; }
                if (key == 'k') { moveCursorUp(); updateScroll(); redraw(); return 1; }
                if (key == 'x') { saveState(); gapBuffer.moveCursorTo(cursorPos); gapBuffer.deleteRight(); redraw(); return 1; }
            }
            break;
        }
    }
    return Fl_Widget::handle(event);
}

// --- Standard Helper Methods ---
void TextEditor::moveCursorUp() {
    if (cursorPos == 0) return;
    int lineStart = cursorPos;
    while (lineStart > 0 && gapBuffer.getCharAt(lineStart - 1) != '\n') lineStart--;
    if (lineStart == 0) return;
    int col = cursorPos - lineStart;
    int prevLineEnd = lineStart - 1;
    int prevLineStart = prevLineEnd;
    while (prevLineStart > 0 && gapBuffer.getCharAt(prevLineStart - 1) != '\n') prevLineStart--;
    int prevLineLen = prevLineEnd - prevLineStart;
    cursorPos = (col > prevLineLen) ? prevLineEnd : prevLineStart + col;
}

void TextEditor::moveCursorDown() {
    int textLen = gapBuffer.getLength();
    if (cursorPos >= textLen) return;
    int lineStart = cursorPos;
    while (lineStart > 0 && gapBuffer.getCharAt(lineStart - 1) != '\n') lineStart--;
    int col = cursorPos - lineStart;
    int lineEnd = cursorPos;
    while (lineEnd < textLen && gapBuffer.getCharAt(lineEnd) != '\n') lineEnd++;
    if (lineEnd >= textLen) return;
    int nextLineStart = lineEnd + 1;
    int nextLineEnd = nextLineStart;
    while (nextLineEnd < textLen && gapBuffer.getCharAt(nextLineEnd) != '\n') nextLineEnd++;
    int nextLineLen = nextLineEnd - nextLineStart;
    cursorPos = (col > nextLineLen) ? nextLineEnd : nextLineStart + col;
}

void TextEditor::undo() {
    if (undoStack.isEmpty()) return;
    char* currentText = new char[gapBuffer.getLength() + 1];
    gapBuffer.getText(currentText, gapBuffer.getLength() + 1);
    EditorState currentState(currentText, cursorPos, selectionStart, selectionEnd);
    redoStack.push(currentState);
    delete[] currentText;
    EditorState prevState = undoStack.pop();
    gapBuffer.loadFromString(prevState.text);
    cursorPos = prevState.cursorPos;
    selectionStart = prevState.selStart;
    selectionEnd = prevState.selEnd;
    redraw();
}

void TextEditor::redo() {
    if (redoStack.isEmpty()) return;
    char* currentText = new char[gapBuffer.getLength() + 1];
    gapBuffer.getText(currentText, gapBuffer.getLength() + 1);
    EditorState currentState(currentText, cursorPos, selectionStart, selectionEnd);
    undoStack.push(currentState);
    delete[] currentText;
    EditorState nextState = redoStack.pop();
    gapBuffer.loadFromString(nextState.text);
    cursorPos = nextState.cursorPos;
    selectionStart = nextState.selStart;
    selectionEnd = nextState.selEnd;
    redraw();
}

void TextEditor::startSelection() { if (!selecting) { selectionStart = cursorPos; selecting = true; } }
void TextEditor::updateSelection() { selectionEnd = cursorPos; }
void TextEditor::clearSelection() { selectionStart = -1; selectionEnd = -1; selecting = false; }
bool TextEditor::hasSelection() const { return selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd; }

void TextEditor::saveToFile(const char* filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        int len = gapBuffer.getLength();
        for (int i = 0; i < len; i++) file << gapBuffer.getCharAt(i);
        file.close();
        sprintf(statusMsg, "Saved %s", filename);
        redraw();
    }
}

void TextEditor::loadFromFile(const char* filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        int size = file.tellg();
        file.seekg(0, std::ios::beg);
        char* content = new char[size + 1];
        file.read(content, size);
        content[size] = '\0';
        gapBuffer.loadFromString(content);
        cursorPos = 0;
        clearSelection();
        delete[] content;
        file.close();
        sprintf(statusMsg, "Loaded %s", filename);
        redraw();
    }
}

void TextEditor::newFile() {
    gapBuffer.clear();
    cursorPos = 0;
    clearSelection();
    undoStack.clear();
    redoStack.clear();
    strcpy(statusMsg, "New file");
    redraw();
}