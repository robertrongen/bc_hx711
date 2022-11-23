#include <application.h>

#define SCALE_FREQUENCY 10000

// LED instance
twr_led_t led;

// scale instance
hx711_t scale;                          // HX711 scale module configuration
float _scale_value;
bool _scale_on = true;

static int _rmenu_level=0;
static int _rmenu_pos=-1;
#define MENULEFT 10
#define MENURIGHT 90

void _turn_on();
void _turn_off();
void _lcd_rewrite();
void _lcd_write_menu();
void _lcd_navigate(bool rbutton);

void lcd_button_left_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) event_param;
    if (event == TWR_BUTTON_EVENT_CLICK)
	{
        if (_rmenu_level!=0)
        {
            // accept menu selection
            _lcd_navigate(false);
        }
        else if (_scale_on)
        {
            // measure
            hx711_measure(&scale);
        }
    }
    else if (event == TWR_BUTTON_EVENT_HOLD)
    {
        if (_scale_on)
        {
            _turn_off();
        }
        else
        {
            _turn_on();
        }
    }

}

void lcd_button_right_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) event_param;

	if (event == TWR_BUTTON_EVENT_CLICK)
	{
        _lcd_navigate(true);
    }
}

void hx711_event_handler(hx711_t *self, hx711_event_t event, double value, void *event_param)
{
    (void) event_param;

    _scale_value = value;;
    _lcd_rewrite();

    long val = hx711_read_raw(&scale);

    char buffer[100];
    sprintf(buffer, "event:%d; raw:%d; scale;%f\r\n", (int)event, (int)val, value);
    twr_usb_cdc_write(buffer, strlen(buffer));
}

// switch ON scale and LCD
void _turn_on(){
    twr_module_lcd_on();
    twr_module_lcd_init();
    twr_module_lcd_clear();

    twr_module_lcd_draw_circle(64,64,25,false);
    twr_module_lcd_update();

    hx711_set_update_interval(&scale, SCALE_FREQUENCY);
    hx711_power_up(&scale);
    _scale_on = true;

    hx711_measure(&scale);

    twr_usb_cdc_write("ON\r\n",4);
}

// switch OFF scale and LCD
void _turn_off(){
    twr_module_lcd_off();
    hx711_set_update_interval(&scale, TWR_TICK_INFINITY);
    hx711_power_down(&scale);
    _scale_on = false;

    twr_usb_cdc_write("OFF\r\n",5);
}

// write measured weight
// value - measured units will be rounded to 2 decimal digits
void _lcd_rewrite()
{
    twr_module_lcd_clear();

    if (scale._state==HX711_STATE_INITIALIZE)
    {
        twr_module_lcd_set_font(&twr_font_ubuntu_15);
        twr_module_lcd_draw_string(10, 40, "NOT calibrated", true);
    }
    else
    {
        char buffer[10];
        sprintf(buffer, "%.2f", 0.01*round(_scale_value*100));
        twr_module_lcd_set_font(&twr_font_ubuntu_28);
        twr_module_lcd_draw_string(40, 40, buffer, true);
    }

    _lcd_write_menu();

    twr_module_lcd_update();
}

// write menu depends on state and level
void _lcd_write_menu()
{
    twr_module_lcd_set_font(&twr_font_ubuntu_11);

    if (_rmenu_level==0)
    {
        twr_module_lcd_draw_string(MENULEFT, 115, "Get / Off", true);
        twr_module_lcd_draw_string(MENURIGHT, 115, "Settings", true);
    }
    else if (_rmenu_level==1)
    {
        twr_module_lcd_draw_string(MENULEFT, 115, "Select", true);

        twr_module_lcd_draw_rectangle(MENURIGHT-5, 69+_rmenu_pos*15,127, 84+_rmenu_pos*15, true);
        twr_module_lcd_draw_string(MENURIGHT, 70, "<back>", true);
        twr_module_lcd_draw_string(MENURIGHT, 85, "Tare", true);
        twr_module_lcd_draw_string(MENURIGHT, 100, "Calibrate", true);
        twr_module_lcd_draw_string(MENURIGHT, 115, "Save", true);
    }

    float volt;
    char vtxt[10];
    twr_module_battery_measure();
    twr_module_battery_get_voltage(&volt);
    sprintf(vtxt, "%.2f V", 0.01*round(volt*100));
    twr_module_lcd_draw_string(MENULEFT, 1, vtxt, true);
}

// navigate in the right menu
// rbutton - right LCD module button pressed
void _lcd_navigate(bool rbutton)
{
    if (rbutton)
    {
        if (_rmenu_level==0){
            _rmenu_pos=-1;
            _rmenu_level++;
        }
        _rmenu_pos++;
        if (_rmenu_pos>3)
            _rmenu_pos=0;

        _lcd_rewrite();
    }
    else
    {
        if (_rmenu_level==0)
            return;

        switch (_rmenu_pos)
        {
            case 1:
                hx711_tare(&scale);
                break;
            case 2:
                hx711_calibrate(&scale,100);
                twr_led_pulse(&led, 2000);
                break;
            case 3:
                hx711_save(&scale);
                break;
            default:
                break;
        }

        _rmenu_level = 0;
        _rmenu_pos = -1;

        hx711_measure(&scale);
    }
}


// initialize display
void lcd_init(){
    // Initialize LCD
    twr_module_lcd_init();

    twr_module_lcd_clear();
    twr_module_lcd_set_font(&twr_font_ubuntu_24);
    twr_module_lcd_draw_string(35, 25, "DALI", true);
    twr_module_lcd_draw_string(30, 50, "Scale", true);
    twr_module_lcd_set_font(&twr_font_ubuntu_13);
    twr_module_lcd_draw_string(30, 80, "(C) 2020 Matej", true);
    twr_module_lcd_draw_string(30, 100, "(C) 2022 DALI", true);
    twr_module_lcd_update();

    // Initialize LCD buttons
    static twr_button_t lcd_left;
    twr_button_init_virtual(&lcd_left, TWR_MODULE_LCD_BUTTON_LEFT, twr_module_lcd_get_button_driver(), false);
    twr_button_set_event_handler(&lcd_left, lcd_button_left_event_handler, NULL);
    static twr_button_t lcd_right;
    twr_button_init_virtual(&lcd_right, TWR_MODULE_LCD_BUTTON_RIGHT, twr_module_lcd_get_button_driver(), false);
    twr_button_set_event_handler(&lcd_right, lcd_button_right_event_handler, NULL);
}

// initialize application
void application_init(void){
    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);

    // Initialize USB
    twr_usb_cdc_init();

    // Initialize battery
    twr_module_battery_init();

    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);
    twr_led_set_mode(&led, TWR_LED_MODE_ON);

    // initialize LCD module
    lcd_init();

    // initialize scale
    hx711_init(&scale, DTPIN, CLKPIN, HX711_CHANNEL_A);
    // hx711_set_reads(&scale, 3);
    // hx711_tare(&scale);
    // hx711_set_scale(&scale, 23800);

    hx711_load(&scale);
    hx711_set_update_interval(&scale, SCALE_FREQUENCY);
    hx711_set_event_handler(&scale, hx711_event_handler, NULL);

    twr_led_set_mode(&led, TWR_LED_MODE_OFF);

}
