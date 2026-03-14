#include <gui_interface.h>

GUIState state;

int main(void) {
    GUIInit(&state, "ddnn", "access/IBMPlexMono-Regular.ttf");

    while(!WindowShouldClose()) {
        GUIUpdate(&state);
        GUIDraw(&state);
    }

    CloseWindow();

    return 0;
}
