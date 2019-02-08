#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>

#include <gambatte.h>
#include "src/blitterwrapper.h"
#include "builddate.h"

#include "libmenu.h"
#include "sfont_gameboy.h"

#include <string.h>
#include <string>
#include <locale>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>

#include "src/audiosink.h"


static SDL_Surface *screen;
static SFont_Font* font;
static SFont_Font* fpsfont;
static SDL_Surface *font_bitmap_surface = NULL;
static SDL_Surface *fpsfont_bitmap_surface = NULL;
static SDL_RWops *RWops;

gambatte::GB *gambatte_p;
BlitterWrapper *blitter_p;

void init_globals(gambatte::GB *gambatte, BlitterWrapper *blitter){
    blitter_p = blitter;
    gambatte_p = gambatte;
}

int init_fps_font() {

    SDL_FreeSurface(fpsfont_bitmap_surface);
    RWops = SDL_RWFromMem(sfont_gameboy_fps, 1234);
    fpsfont_bitmap_surface = IMG_LoadPNG_RW(RWops);
    SDL_FreeRW(RWops);
    if (!fpsfont_bitmap_surface) {
        fprintf(stderr, "fps: font load error\n");
        exit(1);
    }
    fpsfont = SFont_InitFont(fpsfont_bitmap_surface);
    if (!fpsfont) {
        fprintf(stderr, "fps: font init error\n");
        exit(1);
    }
    
    return 0;
}

int init_menu() {
    
    SDL_FreeSurface(font_bitmap_surface);
	RWops = SDL_RWFromMem(sfont_gameboy_black, 895);
    font_bitmap_surface = IMG_LoadPNG_RW(RWops);
    SDL_FreeRW(RWops);
    if (!font_bitmap_surface) {
        fprintf(stderr, "menu: font load error\n");
        exit(1);
    }
    font = SFont_InitFont(font_bitmap_surface);
    if (!font) {
        fprintf(stderr, "menu: font init error\n");
        exit(1);
    }

	libmenu_set_font(font);
    
	return 0;
}

void menu_set_screen(SDL_Surface *set_screen) {
	screen = set_screen;
	libmenu_set_screen(screen);
}

void show_fps(SDL_Surface *surface, int fps) {
    char buffer[8];
    sprintf(buffer, "%d", fps);
    if (showfps) {
        SFont_Write(surface, fpsfont, 0, 0, buffer);
    }
}

std::string numtohextext(int num){
    std::locale loc;
    char buffer[4];
    std::string result;
    sprintf(buffer, "%x", num);

    result = std::string(buffer);
    result = std::toupper(buffer[0],loc);

    return result;
}

static int parse_ext_pal(const struct dirent *dir) {
    if(!dir){
        return 0;
    }

    if(dir->d_type == DT_REG) {
        const char *ext = strrchr(dir->d_name,'.');
        if((!ext) || (ext == dir->d_name)) {
            return 0;
        } else {
            if(strcmp(ext, ".pal") == 0){
                return 1;
            }
        }
    }
    return 0;
}

static int parse_ext_fil(const struct dirent *dir) {
    if(!dir){
        return 0;
    }

    if(dir->d_type == DT_REG) {
        const char *ext = strrchr(dir->d_name,'.');
        if((!ext) || (ext == dir->d_name)) {
            return 0;
        } else {
            if(strcmp(ext, ".fil") == 0){
                return 1;
            }
        }
    }
    return 0;
}

static int parse_ext_png(const struct dirent *dir) {
    if(!dir){
        return 0;
    }

    if(dir->d_type == DT_REG) {
        const char *ext = strrchr(dir->d_name,'.');
        if((!ext) || (ext == dir->d_name)) {
            return 0;
        } else {
            if(strcmp(ext, ".png") == 0){
                return 1;
            }
        }
    }
    return 0;
}

/* ============================ MAIN MENU =========================== */

static void callback_return(menu_t *caller_menu);
static void callback_selectstateload(menu_t *caller_menu);
static void callback_selectstatesave(menu_t *caller_menu);
static void callback_restart(menu_t *caller_menu);
static void callback_options(menu_t *caller_menu);
static void callback_cheats(menu_t *caller_menu);
static void callback_about(menu_t *caller_menu);
static void callback_quit(menu_t *caller_menu);

static void callback_showfps(menu_t *caller_menu);
static void callback_scaler(menu_t *caller_menu);
static void callback_dmgpalette(menu_t *caller_menu);
static void callback_colorfilter(menu_t *caller_menu);
static void callback_dmgborderimage(menu_t *caller_menu);
static void callback_gbcborderimage(menu_t *caller_menu);
static void callback_usebios(menu_t *caller_menu);
static void callback_ghosting(menu_t *caller_menu);

static void callback_gamegenie(menu_t *caller_menu);
static void callback_gameshark(menu_t *caller_menu);

#ifdef VERSION_GCW0
std::string menu_main_title = ("GAMBATTE-GCWZERO");
#elif VERSION_RS97
std::string menu_main_title = ("GAMBATTE-RS97");
#else
std::string menu_main_title = ("GAMBATTE-OD");
#endif


void main_menu() {

    SDL_EnableKeyRepeat(250, 83);
    forcemenuexit = 0;

    menu_t *menu;
	menu_entry_t *menu_entry;
    enum {RETURN = 0, SAVE_STATE = 1, LOAD_STATE = 2, SELECT_STATE = 3, OPTIONS = 4, RESTART = 5, QUIT = 6};
    
    menu = new_menu();
    menu_set_header(menu, menu_main_title.c_str());
	menu_set_title(menu, "Main Menu");
	menu->back_callback = callback_return;

    menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Load state");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectstateload;

	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Save state");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectstatesave;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Reset game");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_restart;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Options");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_options;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Cheats");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_cheats;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "About");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_about;
    
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Quit");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_quit;
    
    switchToMenuAudio();

    menuin = 0;
    playMenuSound_intro();
    menu_main(menu);
    
    switchToEmulatorAudio();

    delete_menu(menu);

    SDL_EnableKeyRepeat(0, 100);
}

static void callback_quit(menu_t *caller_menu) {
    playMenuSound_ok();
    SDL_Delay(500);
    gambatte_p->saveSavedata();
    caller_menu->quit = 1;
    SDL_Quit();
    exit(0);
}

static void callback_return(menu_t *caller_menu) {
    playMenuSound_back();
    SDL_Delay(208);
    menuout = 0;
    caller_menu->quit = 1;
}

static void callback_restart(menu_t *caller_menu) {
    playMenuSound_ok();
    SDL_Delay(250);
    if(can_reset == 1){//boot logo already ended, can reset game safely
        gambatte_p->reset();
        printOverlay("Reset ok");//print overlay text
    } else if (can_reset == 0){//boot logo is still running, can't reset game safely
        printOverlay("Unable to reset");//print overlay text
    }
    menuout = 0;
    caller_menu->quit = 1;
}

/* ==================== SELECT STATE MENU (LOAD) =========================== */

static void callback_selectedstateload(menu_t *caller_menu);
static void callback_selectstateload_back(menu_t *caller_menu);

static void callback_selectstateload(menu_t *caller_menu) {
    #define N_STATES 10
    menu_t *menu;
	menu_entry_t *menu_entry;
    int i;
    char buffer[64];
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Load State");
	menu->back_callback = callback_selectstateload_back;
	
    for (i = 0; i < N_STATES; i++) {
        menu_entry = new_menu_entry(0);
        sprintf(buffer, "State %d", i);
        menu_entry_set_text(menu_entry, buffer);
        menu_add_entry(menu, menu_entry);
        menu_entry->callback = callback_selectedstateload;
    }
    menu->selected_entry = gambatte_p->currentState();
    
    playMenuSound_in();
	menu_main(menu);
    
    delete_menu(menu);

    if(forcemenuexit > 0) {
    	menuout = 0;
    	caller_menu->quit = 1;
    }
}

static void callback_selectedstateload(menu_t *caller_menu) {
	gambatte_p->selectState_NoOsd(caller_menu->selected_entry);
	playMenuSound_ok();
    SDL_Delay(250);
    char overlaytext[20];
	if(gambatte_p->loadState_NoOsd()){
        can_reset = 1;//allow user to reset or save state once a savestate is loaded
        sprintf(overlaytext, "State %d loaded", gambatte_p->currentState());
        printOverlay(overlaytext);//print overlay text
    } else {
        sprintf(overlaytext, "State %d empty", gambatte_p->currentState());
        printOverlay(overlaytext);//print overlay text
    }
    forcemenuexit = 2;
    caller_menu->quit = 1;
}

static void callback_selectstateload_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== SELECT STATE MENU (SAVE) =========================== */

static void callback_selectedstatesave(menu_t *caller_menu);
static void callback_selectstatesave_back(menu_t *caller_menu);

static void callback_selectstatesave(menu_t *caller_menu) {
    #define N_STATES 10
    menu_t *menu;
	menu_entry_t *menu_entry;
    int i;
    char buffer[64];
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Save State");
	menu->back_callback = callback_selectstatesave_back;
	
    for (i = 0; i < N_STATES; i++) {
        menu_entry = new_menu_entry(0);
        sprintf(buffer, "State %d", i);
        menu_entry_set_text(menu_entry, buffer);
        menu_add_entry(menu, menu_entry);
        menu_entry->callback = callback_selectedstatesave;
    }
    menu->selected_entry = gambatte_p->currentState();
    
    playMenuSound_in();
	menu_main(menu);
    
    delete_menu(menu);

    if(forcemenuexit > 0) {
    	menuout = 0;
    	caller_menu->quit = 1;
    }
}

static void callback_selectedstatesave(menu_t *caller_menu) {
	gambatte_p->selectState_NoOsd(caller_menu->selected_entry);
	playMenuSound_ok();
    SDL_Delay(250);
    if(can_reset == 1){//boot logo already ended, can save state safely
        //set palette to greyscale
        Uint32 value;
        for (int i = 0; i < 3; ++i) {
            for (int k = 0; k < 4; ++k) {
                if(k == 0)
                    value = 0xF8FCF8;
                if(k == 1)
                    value = 0xA8A8A8;
                if(k == 2)
                    value = 0x505450;
                if(k == 3)
                    value = 0x000000;
                gambatte_p->setDmgPaletteColor(i, k, value);
            }
        }
        //run the emulator for 1 frame, so the screen buffer is updated without color palettes
        std::size_t fakesamples = 35112;
        Array<Uint32> const fakeBuf(35112 + 2064);
        gambatte_p->runFor(blitter_p->inBuf().pixels, blitter_p->inBuf().pitch, fakeBuf, fakesamples);
        //save state. the snapshot will now be in greyscale
        gambatte_p->saveState_NoOsd(blitter_p->inBuf().pixels, blitter_p->inBuf().pitch);
        //restore the color palette
        loadPalette(palname); //restore palette to actual colors

        char overlaytext[14];
        sprintf(overlaytext, "State %d saved", gambatte_p->currentState());
        printOverlay(overlaytext);//print overlay text
    } else if (can_reset == 0){//boot logo is still running, can't save state
        printOverlay("Unable to save");//print overlay text
    }
    forcemenuexit = 2;
    caller_menu->quit = 1;
}

static void callback_selectstatesave_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== OPTIONS MENU ================================ */

static void callback_saveconfig(menu_t *caller_menu);
static void callback_options_back(menu_t *caller_menu);

static void callback_options(menu_t *caller_menu) {
    menu_t *menu;
	menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();
        
    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Options");
	menu->back_callback = callback_options_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Show FPS");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_showfps;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Select Scaler");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_scaler;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Mono Palette");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_dmgpalette;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Color Filter");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_colorfilter;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "DMG Border");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_dmgborderimage;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "GBC Border");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_gbcborderimage;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Boot logos");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_usebios;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Ghosting");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_ghosting;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Save settings");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_saveconfig;
    
	playMenuSound_in();
    menu_main(menu);
    
    delete_menu(menu);

    if(forcemenuexit > 0) {
    	menuout = 0;
    	caller_menu->quit = 1;
    }
}

static void callback_saveconfig(menu_t *caller_menu) {
    playMenuSound_ok();
    saveConfig();
    caller_menu->quit = 1;
}

static void callback_options_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== SHOW FPS MENU =========================== */

static void callback_selectedshowfps(menu_t *caller_menu);
static void callback_showfps_back(menu_t *caller_menu);

static void callback_showfps(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Show FPS");
    menu->back_callback = callback_showfps_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "OFF");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedshowfps;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "ON");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedshowfps;

    menu->selected_entry = showfps; 
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);
}

static void callback_selectedshowfps(menu_t *caller_menu) {
    playMenuSound_ok();
    showfps = caller_menu->selected_entry;
    caller_menu->quit = 1;
}

static void callback_showfps_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== SCALER MENU =========================== */

static void callback_selectedscaler(menu_t *caller_menu);
static void callback_scaler_back(menu_t *caller_menu);

static void callback_scaler(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Select Scaler");
    menu->back_callback = callback_scaler_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "No Scaling");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedscaler;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Sw 1.50x");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedscaler;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Sw Fullscr");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedscaler;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Hw 1.25x");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedscaler;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Hw 1.36x");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedscaler;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Hw 1.50x");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedscaler;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Hw 1.66x");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedscaler;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Hw Fullscr");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedscaler;

    menu->selected_entry = selectedscaler; 
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);
}

static void callback_selectedscaler(menu_t *caller_menu) {
    playMenuSound_ok();
    selectedscaler = caller_menu->selected_entry;
    blitter_p->setScreenRes(); /* switch to selected resolution */
    clean_menu_screen(caller_menu);
    caller_menu->quit = 0;
}

static void callback_scaler_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== DMG PALETTE MENU =========================== */

struct dirent **palettelist = NULL;
int numpalettes;

static void callback_nopalette(menu_t *caller_menu);
static void callback_defaultpalette(menu_t *caller_menu);
static void callback_selectedpalette(menu_t *caller_menu);
static void callback_dmgpalette_back(menu_t *caller_menu);

static void callback_dmgpalette(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Mono Palette");
    menu->back_callback = callback_dmgpalette_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "No palette");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_nopalette;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Default");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_defaultpalette;

    std::string palettedir = (homedir + "/.gambatte/palettes");
    numpalettes = scandir(palettedir.c_str(), &palettelist, parse_ext_pal, alphasort);
    if (numpalettes <= 0) {
        printf("scandir for ./gambatte/palettes/ failed.");
    } else {
        for (int i = 0; i < numpalettes; ++i){
            menu_entry = new_menu_entry(0);
            menu_entry_set_text_no_ext(menu_entry, palettelist[i]->d_name);
            menu_add_entry(menu, menu_entry);
            menu_entry->callback = callback_selectedpalette;
        }
    }

    menu->selected_entry = currentEntryInList(menu, palname); 
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);

    for (int i = 0; i < numpalettes; ++i){
        free(palettelist[i]);
    }
    free(palettelist);
}

static void callback_nopalette(menu_t *caller_menu) {
    playMenuSound_ok();
    Uint32 value;
    for (int i = 0; i < 3; ++i) {
        for (int k = 0; k < 4; ++k) {
            if(k == 0)
                value = 0xF8FCF8;
            if(k == 1)
                value = 0xA8A8A8;
            if(k == 2)
                value = 0x505450;
            if(k == 3)
                value = 0x000000;
            gambatte_p->setDmgPaletteColor(i, k, value);
        }
    }
    set_menu_palette(0xF8FCF8, 0xA8A8A8, 0x505450, 0x000000);
    palname = "NONE";
    if(gameiscgb == 1){
        caller_menu->quit = 1;
    } else {
        caller_menu->quit = 0;
    }
}

static void callback_defaultpalette(menu_t *caller_menu) {
    playMenuSound_ok();
    palname = "DEFAULT";
    loadPalette(palname);
    if(gameiscgb == 1){
        caller_menu->quit = 1;
    } else {
        caller_menu->quit = 0;
    }
}

static void callback_selectedpalette(menu_t *caller_menu) {
    playMenuSound_ok();
    palname = palettelist[caller_menu->selected_entry - 2]->d_name; // we added 2 extra entries before the list, so we do (-2).
    loadPalette(palname);
    if(gameiscgb == 1){
        caller_menu->quit = 1;
    } else {
        caller_menu->quit = 0;
    }
}

static void callback_dmgpalette_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== COLOR FILTER MENU =========================== */

struct dirent **filterlist = NULL;
int numfilters;

static void callback_nofilter(menu_t *caller_menu);
static void callback_defaultfilter(menu_t *caller_menu);
static void callback_selectedfilter(menu_t *caller_menu);
static void callback_colorfilter_back(menu_t *caller_menu);

static void callback_colorfilter(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Color Filter");
    menu->back_callback = callback_colorfilter_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "No filter");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_nofilter;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Default");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_defaultfilter;

    std::string filterdir = (homedir + "/.gambatte/filters");
    numfilters = scandir(filterdir.c_str(), &filterlist, parse_ext_fil, alphasort);
    if (numfilters <= 0) {
        printf("scandir for ./gambatte/filters/ failed.");
    } else {
        for (int i = 0; i < numfilters; ++i){
            menu_entry = new_menu_entry(0);
            menu_entry_set_text_no_ext(menu_entry, filterlist[i]->d_name);
            menu_add_entry(menu, menu_entry);
            menu_entry->callback = callback_selectedfilter;
        }
    }

    menu->selected_entry = currentEntryInList(menu, filtername); 
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);

    for (int i = 0; i < numfilters; ++i){
        free(filterlist[i]);
    }
    free(filterlist);
}

static void callback_nofilter(menu_t *caller_menu) {
    playMenuSound_ok();
    filtername = "NONE";
    colorfilter = 0;
    if(gameiscgb == 1){
        caller_menu->quit = 0;
    } else {
        caller_menu->quit = 1;
    }
}

static void callback_defaultfilter(menu_t *caller_menu) {
    playMenuSound_ok();
    filtername = "DEFAULT";
    loadFilter(filtername);
    if(gameiscgb == 1){
        caller_menu->quit = 0;
    } else {
        caller_menu->quit = 1;
    }
}

static void callback_selectedfilter(menu_t *caller_menu) {
    playMenuSound_ok();
    filtername = filterlist[caller_menu->selected_entry - 2]->d_name; // we added 2 extra entries before the list, so we do (-2).
    loadFilter(filtername);
    if(gameiscgb == 1){
        caller_menu->quit = 0;
    } else {
        caller_menu->quit = 1;
    }
}

static void callback_colorfilter_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== DMG BORDER IMAGE MENU =========================== */

struct dirent **dmgborderlist = NULL;
int numdmgborders;

static void callback_nodmgborder(menu_t *caller_menu);
static void callback_defaultdmgborder(menu_t *caller_menu);
static void callback_selecteddmgborder(menu_t *caller_menu);
static void callback_dmgborderimage_back(menu_t *caller_menu);

static void callback_dmgborderimage(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "DMG Border");
    menu->back_callback = callback_dmgborderimage_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "No border");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_nodmgborder;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Default");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_defaultdmgborder;

    std::string borderdir = (homedir + "/.gambatte/borders");
    numdmgborders = scandir(borderdir.c_str(), &dmgborderlist, parse_ext_png, alphasort);
    if (numdmgborders <= 0) {
        printf("scandir for ./gambatte/borders/ failed.");
    } else {
        for (int i = 0; i < numdmgborders; ++i){
            menu_entry = new_menu_entry(0);
            menu_entry_set_text_no_ext(menu_entry, dmgborderlist[i]->d_name);
            menu_add_entry(menu, menu_entry);
            menu_entry->callback = callback_selecteddmgborder;
        }
    }

    menu->selected_entry = currentEntryInList(menu, dmgbordername); 
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);

    for (int i = 0; i < numdmgborders; ++i){
        free(dmgborderlist[i]);
    }
    free(dmgborderlist);
}

static void callback_nodmgborder(menu_t *caller_menu) {
    playMenuSound_ok();
    dmgbordername = "NONE";
    if(gameiscgb == 1){
        caller_menu->quit = 1;
    } else {
        load_border(dmgbordername);
        clean_menu_screen(caller_menu);
        caller_menu->quit = 0;
    }
}

static void callback_defaultdmgborder(menu_t *caller_menu) {
    playMenuSound_ok();
    dmgbordername = "DEFAULT";
    if(gameiscgb == 1){
        caller_menu->quit = 1;
    } else {
        load_border(dmgbordername);
        clean_menu_screen(caller_menu);
        caller_menu->quit = 0;
    }
}

static void callback_selecteddmgborder(menu_t *caller_menu) {
    playMenuSound_ok();
    dmgbordername = dmgborderlist[caller_menu->selected_entry - 2]->d_name; // we added 2 extra entries before the list, so we do (-2).
    if(gameiscgb == 1){
        caller_menu->quit = 1;
    } else {
        load_border(dmgbordername);
        clean_menu_screen(caller_menu);
        caller_menu->quit = 0;
    }
}

static void callback_dmgborderimage_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== GBC BORDER IMAGE MENU =========================== */

struct dirent **gbcborderlist = NULL;
int numgbcborders;

static void callback_nogbcborder(menu_t *caller_menu);
static void callback_defaultgbcborder(menu_t *caller_menu);
static void callback_selectedgbcborder(menu_t *caller_menu);
static void callback_gbcborderimage_back(menu_t *caller_menu);

static void callback_gbcborderimage(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "GBC Border");
    menu->back_callback = callback_gbcborderimage_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "No border");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_nogbcborder;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Default");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_defaultgbcborder;

    std::string borderdir = (homedir + "/.gambatte/borders");
    numgbcborders = scandir(borderdir.c_str(), &gbcborderlist, parse_ext_png, alphasort);
    if (numgbcborders <= 0) {
        printf("scandir for ./gambatte/borders/ failed.");
    } else {
        for (int i = 0; i < numgbcborders; ++i){
            menu_entry = new_menu_entry(0);
            menu_entry_set_text_no_ext(menu_entry, gbcborderlist[i]->d_name);
            menu_add_entry(menu, menu_entry);
            menu_entry->callback = callback_selectedgbcborder;
        }
    }

    menu->selected_entry = currentEntryInList(menu, gbcbordername); 
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);

    for (int i = 0; i < numgbcborders; ++i){
        free(gbcborderlist[i]);
    }
    free(gbcborderlist);
}

static void callback_nogbcborder(menu_t *caller_menu) {
    playMenuSound_ok();
    gbcbordername = "NONE";
    if(gameiscgb == 1){
        load_border(gbcbordername);
        clean_menu_screen(caller_menu);
        caller_menu->quit = 0;
    } else {
        caller_menu->quit = 1;
    }
}

static void callback_defaultgbcborder(menu_t *caller_menu) {
    playMenuSound_ok();
    gbcbordername = "DEFAULT";
    if(gameiscgb == 1){
        load_border(gbcbordername);
        clean_menu_screen(caller_menu);
        caller_menu->quit = 0;
    } else {
        caller_menu->quit = 1;
    }
}

static void callback_selectedgbcborder(menu_t *caller_menu) {
    playMenuSound_ok();
    gbcbordername = gbcborderlist[caller_menu->selected_entry - 2]->d_name; // we added 2 extra entries before the list, so we do (-2).
    if(gameiscgb == 1){
        load_border(gbcbordername);
        clean_menu_screen(caller_menu);
        caller_menu->quit = 0;
    } else {
        caller_menu->quit = 1;
    }
}

static void callback_gbcborderimage_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== BOOT LOGOS MENU =========================== */

static void callback_selectedbios(menu_t *caller_menu);
static void callback_usebios_back(menu_t *caller_menu);

static void callback_usebios(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Boot Logos");
    menu->back_callback = callback_usebios_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "OFF");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedbios;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "ON");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedbios;

    menu->selected_entry = biosenabled; 
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);
}

static void callback_selectedbios(menu_t *caller_menu) {
    playMenuSound_ok();
    biosenabled = caller_menu->selected_entry;
    caller_menu->quit = 1;
}

static void callback_usebios_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== GHOSTING MENU =========================== */

static void callback_selectedghosting(menu_t *caller_menu);
static void callback_ghosting_back(menu_t *caller_menu);

static void callback_ghosting(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Ghosting");
    menu->back_callback = callback_ghosting_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "OFF");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedghosting;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "ON");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectedghosting;

    menu->selected_entry = ghosting; 
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);
}

static void callback_selectedghosting(menu_t *caller_menu) {
    playMenuSound_ok();
    ghosting = caller_menu->selected_entry;
    caller_menu->quit = 1;
}

static void callback_ghosting_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== ABOUT MENU =========================== */


static void callback_about_back(menu_t *caller_menu);

static void callback_about(menu_t *caller_menu) {

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "About");
    menu->back_callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Gambatte");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "by Sindre Aamas");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "OpenDingux port by");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Surkow and Hi-Ban");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Special thanks to");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Senquack");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "and Pingflood");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "build version:");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, BUILDDATE);
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_about_back;
    
    playMenuSound_in();
    menu_main(menu);

    delete_menu(menu);
}

static void callback_about_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== CHEATS MENU ================================ */

static void callback_cheats_back(menu_t *caller_menu);

static void callback_cheats(menu_t *caller_menu) {
    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();
        
    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Cheats");
    menu->back_callback = callback_cheats_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Game Genie");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_gamegenie;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Game Shark");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_gameshark;
    
    playMenuSound_in();
    menu_main(menu);
    
    delete_menu(menu);

    if(forcemenuexit > 0) {
    	menuout = 0;
    	caller_menu->quit = 1;
    }
}

static void callback_cheats_back(menu_t *caller_menu) {
    playMenuSound_back();
    caller_menu->quit = 1;
}

/* ==================== GAME GENIE MENU ================================ */

static void callback_gamegenie_confirm(menu_t *caller_menu);
static void callback_gamegenie_apply(menu_t *caller_menu);
static void callback_gamegenie_apply_back(menu_t *caller_menu);
static void callback_gamegenie_edit(menu_t *caller_menu);
static void callback_gamegenie_back(menu_t *caller_menu);

static void callback_gamegenie(menu_t *caller_menu) {

    int i, j, offset, offset2;

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Game Genie");
    menu->back_callback = callback_gamegenie_back;

    for (i = 0; i < numcodes_gg; i++) {

        for (j = 0; j < 3; j++) {
            menu_entry = new_menu_entry(1);
            menu_entry_set_text(menu_entry, "");
            menu_add_entry(menu, menu_entry);
            menu_entry_add_entry(menu_entry, "0");
            menu_entry_add_entry(menu_entry, "1");
            menu_entry_add_entry(menu_entry, "2");
            menu_entry_add_entry(menu_entry, "3");
            menu_entry_add_entry(menu_entry, "4");
            menu_entry_add_entry(menu_entry, "5");
            menu_entry_add_entry(menu_entry, "6");
            menu_entry_add_entry(menu_entry, "7");
            menu_entry_add_entry(menu_entry, "8");
            menu_entry_add_entry(menu_entry, "9");
            menu_entry_add_entry(menu_entry, "A");
            menu_entry_add_entry(menu_entry, "B");
            menu_entry_add_entry(menu_entry, "C");
            menu_entry_add_entry(menu_entry, "D");
            menu_entry_add_entry(menu_entry, "E");
            menu_entry_add_entry(menu_entry, "F");
            menu_entry->selected_entry = 0;
            menu_entry->selectable = 2;
            menu_entry->callback = callback_gamegenie_edit;
        }

        menu_entry = new_menu_entry(0);
        menu_entry_set_text(menu_entry, "-");
        menu_add_entry(menu, menu_entry);
        menu_entry->selectable = 0;

        for (j = 0; j < 3; j++) {
            menu_entry = new_menu_entry(1);
            menu_entry_set_text(menu_entry, "");
            menu_add_entry(menu, menu_entry);
            menu_entry_add_entry(menu_entry, "0");
            menu_entry_add_entry(menu_entry, "1");
            menu_entry_add_entry(menu_entry, "2");
            menu_entry_add_entry(menu_entry, "3");
            menu_entry_add_entry(menu_entry, "4");
            menu_entry_add_entry(menu_entry, "5");
            menu_entry_add_entry(menu_entry, "6");
            menu_entry_add_entry(menu_entry, "7");
            menu_entry_add_entry(menu_entry, "8");
            menu_entry_add_entry(menu_entry, "9");
            menu_entry_add_entry(menu_entry, "A");
            menu_entry_add_entry(menu_entry, "B");
            menu_entry_add_entry(menu_entry, "C");
            menu_entry_add_entry(menu_entry, "D");
            menu_entry_add_entry(menu_entry, "E");
            menu_entry_add_entry(menu_entry, "F");
            menu_entry->selected_entry = 0;
            menu_entry->selectable = 2;
            menu_entry->callback = callback_gamegenie_edit;
        }

        menu_entry = new_menu_entry(0);
        menu_entry_set_text(menu_entry, "-");
        menu_add_entry(menu, menu_entry);
        menu_entry->selectable = 0;

        for (j = 0; j < 3; j++) {
            menu_entry = new_menu_entry(1);
            menu_entry_set_text(menu_entry, "");
            menu_add_entry(menu, menu_entry);
            menu_entry_add_entry(menu_entry, "0");
            menu_entry_add_entry(menu_entry, "1");
            menu_entry_add_entry(menu_entry, "2");
            menu_entry_add_entry(menu_entry, "3");
            menu_entry_add_entry(menu_entry, "4");
            menu_entry_add_entry(menu_entry, "5");
            menu_entry_add_entry(menu_entry, "6");
            menu_entry_add_entry(menu_entry, "7");
            menu_entry_add_entry(menu_entry, "8");
            menu_entry_add_entry(menu_entry, "9");
            menu_entry_add_entry(menu_entry, "A");
            menu_entry_add_entry(menu_entry, "B");
            menu_entry_add_entry(menu_entry, "C");
            menu_entry_add_entry(menu_entry, "D");
            menu_entry_add_entry(menu_entry, "E");
            menu_entry_add_entry(menu_entry, "F");
            menu_entry->selected_entry = 0;
            menu_entry->selectable = 2;
            menu_entry->callback = callback_gamegenie_edit;
        }
    }

    menu_entry->callback = callback_gamegenie_confirm; // set last entry callback to "confirm" function

    // get code values from stored
    for (i = 0; i < numcodes_gg; i++) {
        offset = 11 * i;
        offset2 = 9 * i;

        menu->entries[0 + offset]->selected_entry = ggcheats[0 + offset2];
        menu->entries[1 + offset]->selected_entry = ggcheats[1 + offset2];
        menu->entries[2 + offset]->selected_entry = ggcheats[2 + offset2];
        menu->entries[4 + offset]->selected_entry = ggcheats[3 + offset2];
        menu->entries[5 + offset]->selected_entry = ggcheats[4 + offset2];
        menu->entries[6 + offset]->selected_entry = ggcheats[5 + offset2];
        menu->entries[8 + offset]->selected_entry = ggcheats[6 + offset2];
        menu->entries[9 + offset]->selected_entry = ggcheats[7 + offset2];
        menu->entries[10 + offset]->selected_entry = ggcheats[8 + offset2];
    }
    
    playMenuSound_in();
    menu_cheat(menu);

    delete_menu(menu);
}

static void callback_gamegenie_confirm(menu_t *caller_menu) {

    if (editmode == 0){ //user pressed START, he wants to apply cheats

        menu_t *menu;
        menu_entry_t *menu_entry;
        (void) caller_menu;
        menu = new_menu();

        menu_set_header(menu, menu_main_title.c_str());
        menu_set_title(menu, "Game Genie");
        menu->back_callback = callback_gamegenie_back;

        menu_entry = new_menu_entry(0);
        menu_entry_set_text(menu_entry, "Apply Codes?");
        menu_add_entry(menu, menu_entry);
        menu_entry->selectable = 0;
        menu_entry->callback = callback_gamegenie_apply;
        
        playMenuSound_in();
        menu_main(menu);

        delete_menu(menu);

        int i, offset, offset2;

        // after applying cheats the stored codes are cleared, so we must reload code values
        for (i = 0; i < numcodes_gg; i++) {
            offset = 11 * i;
            offset2 = 9 * i;

            caller_menu->entries[0 + offset]->selected_entry = ggcheats[0 + offset2];
            caller_menu->entries[1 + offset]->selected_entry = ggcheats[1 + offset2];
            caller_menu->entries[2 + offset]->selected_entry = ggcheats[2 + offset2];
            caller_menu->entries[4 + offset]->selected_entry = ggcheats[3 + offset2];
            caller_menu->entries[5 + offset]->selected_entry = ggcheats[4 + offset2];
            caller_menu->entries[6 + offset]->selected_entry = ggcheats[5 + offset2];
            caller_menu->entries[8 + offset]->selected_entry = ggcheats[6 + offset2];
            caller_menu->entries[9 + offset]->selected_entry = ggcheats[7 + offset2];
            caller_menu->entries[10 + offset]->selected_entry = ggcheats[8 + offset2]; 
        }

        if(forcemenuexit > 0) {
	    	forcemenuexit = 0;
        	caller_menu->selected_entry = 0;
	    }

    } else if (editmode == 1){ //user is in edit mode, he wants to exit edit mode

        callback_gamegenie_edit(caller_menu);
    }
}

static void callback_gamegenie_apply(menu_t *caller_menu) {

    std::string a1 = "0", a2 = "0", a3 = "0", a4 = "0", a5 = "0", a6 = "0", a7 = "0", a8 = "0", a9 = "0";
    int i, offset;
    std::string cheat_a = "", multicheat = "";

    for (i = 0; i < numcodes_gg; i++) {
        offset = 9 * i;

        // get code values from stored
        a1 = numtohextext(ggcheats[0 + offset]);
        a2 = numtohextext(ggcheats[1 + offset]);
        a3 = numtohextext(ggcheats[2 + offset]);
        a4 = numtohextext(ggcheats[3 + offset]);
        a5 = numtohextext(ggcheats[4 + offset]);
        a6 = numtohextext(ggcheats[5 + offset]);
        a7 = numtohextext(ggcheats[6 + offset]);
        a8 = numtohextext(ggcheats[7 + offset]);
        a9 = numtohextext(ggcheats[8 + offset]);

        if ((a7 == "0") && (a8 == "0") && (a9 == "0")){
            cheat_a = a1 + a2 + a3 + "-" + a4 + a5 + a6 + ";";
        } else {
            cheat_a = a1 + a2 + a3 + "-" + a4 + a5 + a6 + "-" + a7 + a8 + a9 + ";";
        }

        if ((cheat_a != "000-000-000;") && (cheat_a != "000-000;")){
            multicheat += cheat_a;
        } 
    }

    playMenuSound_ok();
    gambatte_p->setGameGenie(multicheat); // apply cheats

    // clear all codes after applying them
    for (i = 0; i < (numcodes_gg * 9); i++) {
        ggcheats[i] = 0;
    }

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();
        
    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Game Genie");
    menu->back_callback = callback_gamegenie_apply_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, " ");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_gamegenie_apply_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Codes Applied!");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_gamegenie_apply_back;

    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, " ");
    menu_add_entry(menu, menu_entry);
    menu_entry->selectable = 0;
    menu_entry->callback = callback_gamegenie_apply_back;
    
    menu_message(menu);

    delete_menu(menu);

    if(forcemenuexit > 0) {
    	menuout = 0;
    	caller_menu->quit = 1;
    }
}

static void callback_gamegenie_apply_back(menu_t *caller_menu) {
	forcemenuexit = 2;
    caller_menu->quit = 1;
}

static void callback_gamegenie_edit(menu_t *caller_menu) {

    if(editmode == 0){ //enter edit mode

        playMenuSound_in();
        selectedcode = floor(caller_menu->selected_entry / 11);
        editmode = 1;

    } else if (editmode == 1){ //exit edit mode

        playMenuSound_ok();
        editmode = 0;
        int loop = 0;
        int i, offset, offset2;

        do {
            if ((caller_menu->selected_entry % 11 == 0) || (caller_menu->selected_entry == 0)){
                //do nothing
            } else {
                --caller_menu->selected_entry;
            }
            loop++;
        } while ((caller_menu->selected_entry % 11 != 0) && (loop < 11)); //go to first entry in that line

        // save code values
        for (i = 0; i < numcodes_gg; i++) {
            offset = 11 * i;
            offset2 = 9 * i; 

            ggcheats[0 + offset2] = caller_menu->entries[0 + offset]->selected_entry;
            ggcheats[1 + offset2] = caller_menu->entries[1 + offset]->selected_entry;
            ggcheats[2 + offset2] = caller_menu->entries[2 + offset]->selected_entry;
            ggcheats[3 + offset2] = caller_menu->entries[4 + offset]->selected_entry;
            ggcheats[4 + offset2] = caller_menu->entries[5 + offset]->selected_entry;
            ggcheats[5 + offset2] = caller_menu->entries[6 + offset]->selected_entry;
            ggcheats[6 + offset2] = caller_menu->entries[8 + offset]->selected_entry;
            ggcheats[7 + offset2] = caller_menu->entries[9 + offset]->selected_entry;
            ggcheats[8 + offset2] = caller_menu->entries[10 + offset]->selected_entry;
        }
    }

    caller_menu->quit = 0;
}

static void callback_gamegenie_back(menu_t *caller_menu) {

    if(editmode == 0){ // exit to previous menu screen

        playMenuSound_back();
        selectedcode = 0;
        /*int i;

        //clear all codes on exit
        for (i = 0; i < (numcodes_gg * 9); i++) {
            ggcheats[i] = 0;
        }*/

        caller_menu->quit = 1;

    } else if (editmode == 1){ // exit edit mode without saving changes

        playMenuSound_back();
        editmode = 0;
        int loop = 0;
        int i, offset, offset2;

        do {
            if ((caller_menu->selected_entry % 11 == 0) || (caller_menu->selected_entry == 0)){
                //do nothing
            } else {
                --caller_menu->selected_entry;
            }
            loop++;
        } while ((caller_menu->selected_entry % 11 != 0) && (loop < 11)); //go to first entry in that line

        // reload code values from stored
        for (i = 0; i < numcodes_gg; i++) {
            offset = 11 * i;
            offset2 = 9 * i;

            caller_menu->entries[0 + offset]->selected_entry = ggcheats[0 + offset2];
            caller_menu->entries[1 + offset]->selected_entry = ggcheats[1 + offset2];
            caller_menu->entries[2 + offset]->selected_entry = ggcheats[2 + offset2];
            caller_menu->entries[4 + offset]->selected_entry = ggcheats[3 + offset2];
            caller_menu->entries[5 + offset]->selected_entry = ggcheats[4 + offset2];
            caller_menu->entries[6 + offset]->selected_entry = ggcheats[5 + offset2];
            caller_menu->entries[8 + offset]->selected_entry = ggcheats[6 + offset2];
            caller_menu->entries[9 + offset]->selected_entry = ggcheats[7 + offset2];
            caller_menu->entries[10 + offset]->selected_entry = ggcheats[8 + offset2]; 
        }

        caller_menu->quit = 0;
    }
}

/* ==================== GAME SHARK MENU =========================== */

static void callback_gameshark_enabledisable(menu_t *caller_menu);
static void callback_gameshark_edit(menu_t *caller_menu);
static void callback_gameshark_back(menu_t *caller_menu);

static void callback_gameshark(menu_t *caller_menu) {

    int i, j, offset, offset2, enabled;

    menu_t *menu;
    menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();

    menu_set_header(menu, menu_main_title.c_str());
    menu_set_title(menu, "Game Shark");
    menu->back_callback = callback_gameshark_back;

    for (i = 0; i < numcodes_gs; i++) {

        enabled = gscheatsenabled[i];

        menu_entry = new_menu_entry(1);
        menu_entry_set_text(menu_entry, "");
        menu_add_entry(menu, menu_entry);
        menu_entry_add_entry(menu_entry, "[ ]");
        menu_entry_add_entry(menu_entry, "[~]");
        menu_entry->selected_entry = enabled;
        menu_entry->callback = callback_gameshark_enabledisable;

        menu_entry = new_menu_entry(0);
        menu_entry_set_text(menu_entry, " ");
        menu_add_entry(menu, menu_entry);
        menu_entry->selectable = 0;

        for (j = 0; j < 8; j++) {
            menu_entry = new_menu_entry(1);
            menu_entry_set_text(menu_entry, "");
            menu_add_entry(menu, menu_entry);
            menu_entry_add_entry(menu_entry, "0");
            menu_entry_add_entry(menu_entry, "1");
            menu_entry_add_entry(menu_entry, "2");
            menu_entry_add_entry(menu_entry, "3");
            menu_entry_add_entry(menu_entry, "4");
            menu_entry_add_entry(menu_entry, "5");
            menu_entry_add_entry(menu_entry, "6");
            menu_entry_add_entry(menu_entry, "7");
            menu_entry_add_entry(menu_entry, "8");
            menu_entry_add_entry(menu_entry, "9");
            menu_entry_add_entry(menu_entry, "A");
            menu_entry_add_entry(menu_entry, "B");
            menu_entry_add_entry(menu_entry, "C");
            menu_entry_add_entry(menu_entry, "D");
            menu_entry_add_entry(menu_entry, "E");
            menu_entry_add_entry(menu_entry, "F");
            menu_entry->selected_entry = 0;
            menu_entry->selectable = 2;
            menu_entry->callback = callback_gameshark_edit;
        }
    }

    // get code values from stored
    for (i = 0; i < numcodes_gs; i++) {
        offset = 10 * i;
        offset2 = 8 * i;

        menu->entries[0 + offset]->selected_entry = gscheatsenabled[0 + i];

        menu->entries[2 + offset]->selected_entry = gscheats[0 + offset2];
        menu->entries[3 + offset]->selected_entry = gscheats[1 + offset2];
        menu->entries[4 + offset]->selected_entry = gscheats[2 + offset2];
        menu->entries[5 + offset]->selected_entry = gscheats[3 + offset2];
        menu->entries[6 + offset]->selected_entry = gscheats[4 + offset2];
        menu->entries[7 + offset]->selected_entry = gscheats[5 + offset2];
        menu->entries[8 + offset]->selected_entry = gscheats[6 + offset2];
        menu->entries[9 + offset]->selected_entry = gscheats[7 + offset2]; 
    }
    
    playMenuSound_in();
    menu_cheat(menu);

    delete_menu(menu);
}

static void callback_gameshark_enabledisable(menu_t *caller_menu) {

    if (caller_menu->entries[caller_menu->selected_entry]->selected_entry == 0) {
        playMenuSound_in();
        caller_menu->entries[caller_menu->selected_entry]->selected_entry = 1;
    } else {
        playMenuSound_back();
        caller_menu->entries[caller_menu->selected_entry]->selected_entry = 0;
    }
    gscheatsenabled[(caller_menu->selected_entry / 10)] = caller_menu->entries[caller_menu->selected_entry]->selected_entry; //store the value

    caller_menu->quit = 0;
}

static void callback_gameshark_edit(menu_t *caller_menu) {

    if(editmode == 0){ //enter edit mode

        playMenuSound_in();
        selectedcode = floor(caller_menu->selected_entry / 10);
        editmode = 1;

    } else if (editmode == 1){ //exit edit mode

        playMenuSound_ok();
        editmode = 0;
        int loop = 0;
        int i, offset, offset2;

        do {
            if ((caller_menu->selected_entry % 10 == 0) || (caller_menu->selected_entry == 0)){
                //do nothing
            } else {
                --caller_menu->selected_entry;
            }
            loop++;
        } while ((caller_menu->selected_entry % 10 != 0) && (loop < 10)); //go to first entry in that line
        caller_menu->selected_entry += 2; //go to second entry in that line

        // save code values
        for (i = 0; i < numcodes_gs; i++) {
            offset = 10 * i;
            offset2 = 8 * i; 

            gscheats[0 + offset2] = caller_menu->entries[2 + offset]->selected_entry;
            gscheats[1 + offset2] = caller_menu->entries[3 + offset]->selected_entry;
            gscheats[2 + offset2] = caller_menu->entries[4 + offset]->selected_entry;
            gscheats[3 + offset2] = caller_menu->entries[5 + offset]->selected_entry;
            gscheats[4 + offset2] = caller_menu->entries[6 + offset]->selected_entry;
            gscheats[5 + offset2] = caller_menu->entries[7 + offset]->selected_entry;
            gscheats[6 + offset2] = caller_menu->entries[8 + offset]->selected_entry;
            gscheats[7 + offset2] = caller_menu->entries[9 + offset]->selected_entry;
        }
    }

    caller_menu->quit = 0;
}

static void callback_gameshark_back(menu_t *caller_menu) {

    if(editmode == 0){ // exit to previous menu screen and apply cheats

        playMenuSound_back();
        selectedcode = 0;
        std::string a1 = "0", a2 = "0", a3 = "0", a4 = "0", a5 = "0", a6 = "0", a7 = "0", a8 = "0";
        int i, offset, enabled;
        std::string cheat_a = "", multicheat = "";

        for (i = 0; i < numcodes_gs; i++) {
            offset = 8 * i;

            // get code values from stored
            a1 = numtohextext(gscheats[0 + offset]);
            a2 = numtohextext(gscheats[1 + offset]);
            a3 = numtohextext(gscheats[2 + offset]);
            a4 = numtohextext(gscheats[3 + offset]);
            a5 = numtohextext(gscheats[4 + offset]);
            a6 = numtohextext(gscheats[5 + offset]);
            a7 = numtohextext(gscheats[6 + offset]);
            a8 = numtohextext(gscheats[7 + offset]);

            cheat_a = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + ";";
            enabled = gscheatsenabled[i];

            if ((cheat_a != "00000000;") && (enabled == 1)){
                multicheat += cheat_a;
            } 
        }

        gambatte_p->setGameShark(multicheat); // apply cheats

        caller_menu->quit = 1;

    } else if (editmode == 1){ // exit edit mode without saving changes

        playMenuSound_back();
        editmode = 0;
        int loop = 0;
        int i, offset, offset2;

        do {
            if ((caller_menu->selected_entry % 10 == 0) || (caller_menu->selected_entry == 0)){
                //do nothing
            } else {
                --caller_menu->selected_entry;
            }
            loop++;
        } while ((caller_menu->selected_entry % 10 != 0) && (loop < 10)); //go to first entry in that line
        caller_menu->selected_entry += 2; //go to second entry in that line

        // reload code values from stored
        for (i = 0; i < numcodes_gs; i++) {
            offset = 10 * i;
            offset2 = 8 * i;

            caller_menu->entries[0 + offset]->selected_entry = gscheatsenabled[0 + i];

            caller_menu->entries[2 + offset]->selected_entry = gscheats[0 + offset2];
            caller_menu->entries[3 + offset]->selected_entry = gscheats[1 + offset2];
            caller_menu->entries[4 + offset]->selected_entry = gscheats[2 + offset2];
            caller_menu->entries[5 + offset]->selected_entry = gscheats[3 + offset2];
            caller_menu->entries[6 + offset]->selected_entry = gscheats[4 + offset2];
            caller_menu->entries[7 + offset]->selected_entry = gscheats[5 + offset2];
            caller_menu->entries[8 + offset]->selected_entry = gscheats[6 + offset2];
            caller_menu->entries[9 + offset]->selected_entry = gscheats[7 + offset2]; 
        }

        caller_menu->quit = 0;
    }
}
