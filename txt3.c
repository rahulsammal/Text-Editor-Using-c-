#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#define MAX 5000

/* ---------------- Node ---------------- */
typedef struct Node {
    char ch;
    struct Node *prev, *next;
} Node;

Node *head = NULL, *tail = NULL;

/* ---------------- Selection ---------------- */
Node *selectStart = NULL;
Node *selectEnd   = NULL;

/* ---------------- Clipboard ---------------- */
char clipboard[MAX];
int clipSize = 0;

/* ---------------- Undo / Redo ---------------- */
char undoStack[MAX], redoStack[MAX];
int topUndo = -1, topRedo = -1;

/* ---------------- Stack helpers ---------------- */
void pushUndo(char c) { if (topUndo < MAX - 1) undoStack[++topUndo] = c; }
char popUndo() { return (topUndo >= 0) ? undoStack[topUndo--] : '\0'; }
void pushRedo(char c) { if (topRedo < MAX - 1) redoStack[++topRedo] = c; }
char popRedo() { return (topRedo >= 0) ? redoStack[topRedo--] : '\0'; }

/* ---------------- Text operations ---------------- */
void insertChar(char c) {
    Node *n = (Node*)malloc(sizeof(Node));
    n->ch = c;
    n->next = NULL;
    n->prev = tail;

    if (tail) tail->next = n;
    else head = n;

    tail = n;
    pushUndo(c);
}

void deleteChar() {
    if (!tail) return;

    pushRedo(tail->ch);
    Node *tmp = tail;
    tail = tail->prev;

    if (tail) tail->next = NULL;
    else head = NULL;

    free(tmp);
}

void undo() {
    char c = popUndo();
    if (c) deleteChar();
}

void redo() {
    char c = popRedo();
    if (c) insertChar(c);
}

/* ---------------- Selection ---------------- */
void startSelection() {
    selectStart = tail;
}

void endSelection() {
    selectEnd = tail;
}

/* ---------------- Copy / Cut / Paste ---------------- */
void copySelection() {
    if (!selectStart || !selectEnd) return;

    Node *t = selectStart;
    clipSize = 0;

    while (t && clipSize < MAX - 1) {
        clipboard[clipSize++] = t->ch;   // includes '\n'
        if (t == selectEnd) break;
        t = t->next;
    }
    clipboard[clipSize] = '\0';
}

void cutSelection() {
    if (!selectStart || !selectEnd) return;

    copySelection();

    Node *t = selectStart;
    Node *next;

    while (t) {
        next = t->next;

        if (t->prev) t->prev->next = t->next;
        else head = t->next;

        if (t->next) t->next->prev = t->prev;
        else tail = t->prev;

        free(t);
        if (t == selectEnd) break;
        t = next;
    }

    selectStart = selectEnd = NULL;
}

void pasteText() {
	int i;
    for ( i = 0; i < clipSize; i++) {
        insertChar(clipboard[i]);   // NEWLINES PRESERVED
    }
}

/* ---------------- Display ---------------- */
void displayText() {
    Node *t = head;
    while (t) {
        putchar(t->ch);   // '\n' handled naturally
        t = t->next;
    }
}

/* ---------------- File ---------------- */
void saveFile() {
    char name[100];
    printf("\nSave as: ");
    scanf("%s", name);

    FILE *fp = fopen(name, "w");
    if (!fp) return;

    Node *t = head;
    while (t) {
        fputc(t->ch, fp);
        t = t->next;
    }
    fclose(fp);
}

void loadFile() {
    char name[100];
    printf("\nOpen file: ");
    scanf("%s", name);

    FILE *fp = fopen(name, "r");
    if (!fp) return;

    head = tail = NULL;
    topUndo = topRedo = -1;

    char c;
    while ((c = fgetc(fp)) != EOF)
        insertChar(c);

    fclose(fp);
}

/* ---------------- Search & Replace ---------------- */
void searchReplace() {
    char find[50], rep[50], buf[MAX];
    int i = 0;

    printf("\nFind: ");
    scanf("%s", find);
    printf("Replace with: ");
    scanf("%s", rep);

    Node *t = head;
    while (t) {
        buf[i++] = t->ch;
        t = t->next;
    }
    buf[i] = '\0';

    char *pos = strstr(buf, find);
    if (!pos) return;

    char newBuf[MAX];
    strncpy(newBuf, buf, pos - buf);
    newBuf[pos - buf] = '\0';
    strcat(newBuf, rep);
    strcat(newBuf, pos + strlen(find));

    head = tail = NULL;
    for (i = 0; i < strlen(newBuf); i++)
        insertChar(newBuf[i]);
}

/* ---------------- MAIN ---------------- */
int main() {
    int ch;

    while (1) {
        system("cls");
        printf("\n-----Text Editor-----\n");
        printf("Ctrl+C Copy | Ctrl+V Paste\n");
        printf("Ctrl+Z Undo | Ctrl+Y Redo | Backspace Delete\n");
        printf("F2 Save | F3 Open | F4 Search/Replace\n");
        printf("ESC Exit\n");
        printf("--------------------------------------\n");

        displayText();
        ch = getch();

        if (ch == 27) break;               // ESC
        else if (ch == 8) deleteChar();    // Backspace
        else if (ch == 26) undo();         // Ctrl+Z
        else if (ch == 25) redo();         // Ctrl+Y
        else if (ch == 3) copySelection(); // Ctrl+C
        else if (ch == 22) pasteText();    // Ctrl+V

        else if (ch == 0 || ch == 224) {
            ch = getch();
            if (ch == 60) saveFile();        // F2
            else if (ch == 61) loadFile();   // F3
            else if (ch == 62) searchReplace(); // F4
        }
        else {
            insertChar(ch);
        }
    }
    return 0;
}

