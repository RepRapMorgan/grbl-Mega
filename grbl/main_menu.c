#include "main_menu.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include "Menu.h"
#include "MenuDisplay.h"
#include "ButtonPress.h"
#include "timekeeper.h"

// private prototypes
void execute_winding_program(void);
void execute_loading_program(void);

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiAvrI2c display;
bool exit_at_run=false;

#define B_UP   63 // analog pin 9 (reset/stop)
#define B_DOWN 65 // analog pin 11 (cycle start)
#define B_LEFT 66
#define B_RIGHT 64 // analog pin 10 (feed hold)

// De-bounced button event handlers with 300ms de-bounce blanking time-out
ButtonPress upButton = ButtonPress(B_UP, 300);
ButtonPress downButton = ButtonPress(B_DOWN, 300);
ButtonPress leftButton = ButtonPress(B_LEFT, 300);
ButtonPress rightButton = ButtonPress(B_RIGHT, 300);


/** Event handler mapping button pushes to menu events
 */
menu_event_t buttonEvent() {
  if (upButton.pushed()) return MENU_UP;
  if (downButton.pushed()) return MENU_DOWN;
  if (leftButton.pushed()) return MENU_LEAVE;
  if (rightButton.pushed()) return MENU_SELECT;
  return NONE;
}


enum {NO_PROGRAM, WINDING, LOADING} PROGRAM;

int selected_program=NO_PROGRAM;

/** an action that uses the display and runs until user clicks "back" button
 */
bool startWinding(void* argument) {
    display.clear();
    display.print("winding...");
    exit_at_run=true;
    selected_program=WINDING;
    return false;
}

bool startLoading(void* argument) {
    display.clear();
    display.print("wire loading...");
    exit_at_run=true;
    selected_program=LOADING;
    return false;
}



// ------------ Defining the menu structure ---------------

// Value parameters, with callback (updateLED)
ParameterInt16 turns((char*)"turns", 0, 0, 50, 1);       // total number of turns for the coil
ParameterInt16 wraps((char*)"wraps/turn", 8, 0, 20, 1);  // wraps per full turn
ParameterInt16 subturns((char*)"sub-turns", 4, 0, 8, 1); // number of individual re-grips during a full turn
ParameterInt16 wirelength((char*)"wire length", 270, 0, 1000, 10); // length of wire for bobbin loading

ParameterInt16 current_turns((char*)"turns", 0, 0, 50, 1);
ParameterInt16 current_wraps((char*)"wraps", 0, 0, 20, 1);
ParameterInt16 current_subturns((char*)"sub-turns", 0, 0, 20, 1);
ParameterInt16 current_feed_override((char*)"Speed", 0, 0, 200, 1);

ParamMenuItem turnsMenu((char*)"turns", &turns);
ParamMenuItem wrapsMenu((char*)"wraps/turn", &wraps);
ParamMenuItem wirelengthMenu((char*)"wire length", &wirelength);

ParamMenuItem currentTurnsMenu((char*)"turns", &current_turns);
ParamMenuItem currentWrapsMenu((char*)"wraps", &current_wraps);
ParamMenuItem currentSubturnsMenu((char*)"sub-turns", &current_subturns);
ParamMenuItem currentSpeedMenu((char*)"Speed", &current_feed_override);

ActionMenuItem runWindingMenu((char*) "start winding", &startWinding, NULL);
ActionMenuItem loadWireMenu((char*) "load wire", &startLoading, NULL);

BackMenuItem backItem = BackMenuItem("back");
// Assembling it all into the main menu
MenuItem* settingsMenuItems[] = 
  {
    &turnsMenu, 
    &wrapsMenu,
    &wirelengthMenu, 
    &backItem
   };
Menu settingsMenu = Menu(settingsMenuItems, 4);

SubMenuItem settingsMenuItem=SubMenuItem((char*)"settings", &settingsMenu);
// Assembling it all into the main menu
MenuItem* mainMenuItems[] = 
  {
    &settingsMenuItem, 
    &loadWireMenu,
    &runWindingMenu,
   };

Menu mainMenu = Menu(mainMenuItems, 3);

MenuItem* updateMenuItems[] = 
  {
    &currentTurnsMenu, 
    &currentWrapsMenu,
    &currentSubturnsMenu,
    &currentSpeedMenu
   };

Menu updateMenu = Menu(updateMenuItems, 4);

// Instance for the menu visualisation
MenuDisplay menuDisplay = MenuDisplay(&display);


void init_menu(void){
    timekeeper_init();
    display.begin(&Adafruit128x64, I2C_ADDRESS);
    display.setFont(Arial_bold_14);
    display.clear();
    display.print("GRBL running.");

}

void run_menu(void) {
    exit_at_run=false; // reset flag for next run
    while (not exit_at_run) {
        // visualize the currently active submenu on the display
        menuDisplay.updateDisplay(mainMenu.getCurrentSubmenu());
        // run the menu navigation, based on the button events
        mainMenu.navigateMenu(buttonEvent());
    } 
    
    // set parameters at exit in  preparation for execution
    current_turns.setValue(turns.getValue());
    current_wraps.setValue(0);
    current_subturns.setValue(subturns.getValue());

}

void execute_program(void) {
    switch (selected_program) {
        case WINDING:
            execute_winding_program();
        break;
        case LOADING:
            execute_loading_program();
        break;
        default:
        break;
    }
}

void execute_loading_program(void) {
    
    int currentlength = wirelength.getValue(); 
    char valueBuffer[5]="";
    char loadCommand[16]="G1Z";
    
    itoa(currentlength, valueBuffer, 10);
    strcat(loadCommand, valueBuffer);
    
    if ((currentlength>0) && (plan_get_block_buffer_count()<2)) {
        gc_execute_line((char*)"G1F5000");      // set feedrate
        gc_execute_line((char*)"G10P1L20Z0"); // reset coordinates to zero
        gc_execute_line(loadCommand);     // execute wrapping
        
        display.clear();
        display.print(loadCommand);
        selected_program = NO_PROGRAM; // we are done.
    } 

}

void execute_winding_program(void) {
    int x_angle=90; // segment per full turn to do in one grip
    int y_angle = 360*(int32_t)wraps.getValue()/(360/x_angle); // corresponding y-angle per grip move
    char valueBuffer[5]="";
    char wrapCommand[16]="G1X0Y";
    
    itoa(y_angle, valueBuffer, 10);
    strcat(wrapCommand, valueBuffer);
    
    if ((current_turns.getValue()>0) && (plan_get_block_buffer_count()<2)) {
        gc_execute_line((char*)"G1F5000");      // set feedrate
        gc_execute_line((char*)"G10P1L20X0Y0"); // reset coordinates to zero
        gc_execute_line((char*)"M9");   // open gripper
        //gc_execute_line((char*)"G4P0.4"); // wait for 0.4 sec
        gc_execute_line((char*)"G0X90");
        gc_execute_line((char*)"M7");   // close gripper
        //gc_execute_line((char*)"G4P0.4"); // wait for 0.4 sec
        gc_execute_line(wrapCommand);     // execute wrapping
        
        current_subturns.decrement();
        if (current_subturns.getValue()<=0) {
            current_subturns.setValue(subturns.getValue());
            current_turns.decrement();
        }
        if (sys.f_override != current_feed_override.getValue()) {
            current_feed_override.setValue(sys.f_override);
            updateMenu.requestRedraw();
        }
        
        updateMenu.requestRedraw();
        menuDisplay.updateDisplay(updateMenu.getCurrentSubmenu());
        
    } else {
        gc_execute_line((char*)"M9");   // open gripper
    }

}