// desk.c - Interfaccia utente tipo Desktop Environment
// Compila con: gcc -m32 -ffreestanding -fno-stack-protector -nostdlib -c desk.c -o desk.o
// Linka con: ld -m elf_i386 -Ttext 0x100000 --oformat binary desk.o -o desk.bin

// Definizioni base
#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Colori
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_BROWN 6
#define COLOR_LIGHT_GRAY 7
#define COLOR_DARK_GRAY 8
#define COLOR_LIGHT_BLUE 9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN 11
#define COLOR_LIGHT_RED 12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW 14
#define COLOR_WHITE 15

// Strutture
typedef struct {
    int x, y;
    int width, height;
    char title[32];
    int active;
} Window;

typedef struct {
    char text[16];
    int x, y;
    int width, height;
    void (*action)(void);
} Button;

// Variabili globali
Window desktop;
Window windows[4];
Button buttons[6];
int current_window = 0;
int mouse_x = 40, mouse_y = 12;

// Funzioni di base implementate direttamente in C
void write_char(char c, int x, int y) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        volatile char* video = (volatile char*)VIDEO_MEMORY;
        video[(y * SCREEN_WIDTH + x) * 2] = c;
        video[(y * SCREEN_WIDTH + x) * 2 + 1] = 0x07; // Bianco su nero
    }
}

void clear_screen(void) {
    volatile char* video = (volatile char*)VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x07; // Bianco su nero
    }
}

// Funzioni helper
void write_string(const char* str, int x, int y, int color) {
    volatile char* video = (volatile char*)VIDEO_MEMORY;
    int i = 0;
    while (str[i]) {
        video[(y * SCREEN_WIDTH + x + i) * 2] = str[i];
        video[(y * SCREEN_WIDTH + x + i) * 2 + 1] = color;
        i++;
    }
}

void draw_box(int x, int y, int width, int height, int color) {
    volatile char* video = (volatile char*)VIDEO_MEMORY;
    
    // Angoli e bordi
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            char ch = ' ';
            if (i == 0 || i == width - 1 || j == 0 || j == height - 1) {
                if ((i == 0 && j == 0) || (i == width-1 && j == 0) || 
                    (i == 0 && j == height-1) || (i == width-1 && j == height-1)) {
                    ch = '+';
                } else if (i == 0 || i == width - 1) {
                    ch = '|';
                } else {
                    ch = '-';
                }
            }
            
            if (y + j < SCREEN_HEIGHT && x + i < SCREEN_WIDTH) {
                video[((y + j) * SCREEN_WIDTH + (x + i)) * 2] = ch;
                video[((y + j) * SCREEN_WIDTH + (x + i)) * 2 + 1] = color;
            }
        }
    }
}

void draw_window(Window* win) {
    if (!win->active) return;
    
    // Bordo finestra
    draw_box(win->x, win->y, win->width, win->height, COLOR_WHITE);
    
    // Barra del titolo
    draw_box(win->x, win->y, win->width, 3, COLOR_LIGHT_BLUE);
    write_string(win->title, win->x + 2, win->y + 1, COLOR_WHITE);
    
    // Pulsanti finestra
    write_string("X", win->x + win->width - 3, win->y + 1, COLOR_LIGHT_RED);
    write_string("_", win->x + win->width - 5, win->y + 1, COLOR_YELLOW);
}

void draw_button(Button* btn) {
    draw_box(btn->x, btn->y, btn->width, btn->height, COLOR_LIGHT_GRAY);
    write_string(btn->text, btn->x + 2, btn->y + 1, COLOR_BLACK);
}

void draw_desktop() {
    // Sfondo
    volatile char* video = (volatile char*)VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = COLOR_BLUE;
    }
    
    // Barra delle applicazioni
    draw_box(0, SCREEN_HEIGHT - 3, SCREEN_WIDTH, 3, COLOR_DARK_GRAY);
    write_string("SimpleOS Desktop v1.0", 2, SCREEN_HEIGHT - 2, COLOR_WHITE);
    
    // Menu Start
    draw_box(2, SCREEN_HEIGHT - 2, 8, 1, COLOR_LIGHT_GREEN);
    write_string("Start", 3, SCREEN_HEIGHT - 2, COLOR_BLACK);
    
    // Orologio
    write_string("12:00", SCREEN_WIDTH - 8, SCREEN_HEIGHT - 2, COLOR_WHITE);
    
    // Icone desktop
    write_string("File", 5, 3, COLOR_WHITE);
    write_string("Apps", 5, 6, COLOR_WHITE);
    write_string("Settings", 5, 9, COLOR_WHITE);
}

void draw_cursor() {
    volatile char* video = (volatile char*)VIDEO_MEMORY;
    if (mouse_x >= 0 && mouse_x < SCREEN_WIDTH && mouse_y >= 0 && mouse_y < SCREEN_HEIGHT) {
        video[(mouse_y * SCREEN_WIDTH + mouse_x) * 2] = '*';
        video[(mouse_y * SCREEN_WIDTH + mouse_x) * 2 + 1] = COLOR_YELLOW;
    }
}

// Azioni pulsanti
void action_file_manager() {
    Window* win = &windows[0];
    win->x = 10;
    win->y = 5;
    win->width = 50;
    win->height = 15;
    win->active = 1;
    for (int i = 0; i < 32; i++) win->title[i] = 0;
    const char* title = "File Manager";
    int i = 0;
    while (title[i]) {
        win->title[i] = title[i];
        i++;
    }
    
    draw_window(win);
    write_string("Files:", win->x + 2, win->y + 4, COLOR_BLACK);
    write_string("- document.txt", win->x + 2, win->y + 6, COLOR_BLACK);
    write_string("- image.bmp", win->x + 2, win->y + 8, COLOR_BLACK);
    write_string("- program.exe", win->x + 2, win->y + 10, COLOR_BLACK);
}

void action_calculator() {
    Window* win = &windows[1];
    win->x = 20;
    win->y = 8;
    win->width = 25;
    win->height = 12;
    win->active = 1;
    for (int i = 0; i < 32; i++) win->title[i] = 0;
    const char* title = "Calculator";
    int i = 0;
    while (title[i]) {
        win->title[i] = title[i];
        i++;
    }
    
    draw_window(win);
    write_string("Result: 0", win->x + 2, win->y + 4, COLOR_BLACK);
    
    // Pulsanti calcolatrice
    write_string("7 8 9 +", win->x + 2, win->y + 6, COLOR_BLACK);
    write_string("4 5 6 -", win->x + 2, win->y + 7, COLOR_BLACK);
    write_string("1 2 3 *", win->x + 2, win->y + 8, COLOR_BLACK);
    write_string("0 . = /", win->x + 2, win->y + 9, COLOR_BLACK);
}

void action_text_editor() {
    Window* win = &windows[2];
    win->x = 15;
    win->y = 4;
    win->width = 45;
    win->height = 18;
    win->active = 1;
    for (int i = 0; i < 32; i++) win->title[i] = 0;
    const char* title = "Text Editor";
    int i = 0;
    while (title[i]) {
        win->title[i] = title[i];
        i++;
    }
    
    draw_window(win);
    write_string("Welcome to SimpleOS Text Editor!", win->x + 2, win->y + 4, COLOR_BLACK);
    write_string("", win->x + 2, win->y + 6, COLOR_BLACK);
    write_string("This is a basic text editor.", win->x + 2, win->y + 8, COLOR_BLACK);
    write_string("You can edit text here...", win->x + 2, win->y + 10, COLOR_BLACK);
}

void init_ui() {
    // Inizializza finestre
    for (int i = 0; i < 4; i++) {
        windows[i].active = 0;
    }
    
    // Inizializza pulsanti
    buttons[0] = (Button){"File Mgr", 15, SCREEN_HEIGHT - 2, 10, 1, action_file_manager};
    buttons[1] = (Button){"Calc", 26, SCREEN_HEIGHT - 2, 8, 1, action_calculator};
    buttons[2] = (Button){"Editor", 35, SCREEN_HEIGHT - 2, 9, 1, action_text_editor};
}

void handle_input() {
    // Simulazione input (normalmente da tastiera/mouse)
    static int counter = 0;
    counter++;
    
    if (counter == 1000) {
        action_file_manager();
    } else if (counter == 2000) {
        action_calculator();
    } else if (counter == 3000) {
        action_text_editor();
    }
    
    // Movimento cursore simulato
    if (counter % 500 == 0) {
        mouse_x = (mouse_x + 1) % SCREEN_WIDTH;
        if (mouse_x == 0) mouse_y = (mouse_y + 1) % SCREEN_HEIGHT;
    }
}

// Funzione main
int main() {
    // Pulisci schermo
    clear_screen();
    
    // Inizializza UI
    init_ui();
    
    // Mostra messaggio di benvenuto
    write_string("SimpleOS Desktop Environment", 25, 1, COLOR_WHITE);
    write_string("Caricamento completato!", 30, 2, COLOR_LIGHT_GREEN);
    
    // Loop principale
    while (1) {
        // Disegna desktop
        draw_desktop();
        
        // Disegna finestre attive
        for (int i = 0; i < 4; i++) {
            if (windows[i].active) {
                draw_window(&windows[i]);
            }
        }
        
        // Disegna cursore
        draw_cursor();
        
        // Gestisci input
        handle_input();
        
        // Delay semplice
        for (volatile int i = 0; i < 1000000; i++);
    }
    
    return 0;
}

// Punto d'ingresso assembly
void _start() {
    main();
}