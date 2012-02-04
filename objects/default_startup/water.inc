// ---------------------------------------------------------------------------
// Open Surge Engine
// http://opensnc.sourceforge.net
//
// File:   objects/default_startup/water.inc
// Desc:   water
// Author: Alexandre
// Date:   2011-02-04
// ---------------------------------------------------------------------------

object ".default_startup.water"
{
    requires 0.2.0
    always_active

    state "main"
    {
        hide
        create_child ".default_startup.water.level0"
        create_child ".default_startup.water.level1"
        create_child ".default_startup.water.level2"
        destroy
    }
}

object ".default_startup.water.level0"
{
    requires 0.2.0
    always_active

    state "main"
    {
        set_alpha 0.25
        set_animation SD_WATERLEVEL 0
        set_animation_frame "floor($frame) mod 2"
        set_absolute_position "camera_x() - 3*screen_width()/2 + floor($x)" waterlevel()

        let "$frame += (1.0/0.15) * dt()"
        let "$speed = 50"
        let "$x = cond($x >= screen_width(), $x - screen_width(), $x += $speed * dt())"
    }
}

object ".default_startup.water.level1"
{
    requires 0.2.0
    always_active

    state "main"
    {
        set_alpha 0.25
        set_animation SD_WATERLEVEL 0
        set_animation_frame "floor($frame) mod 2"
        set_absolute_position "camera_x() - screen_width()/2 + floor($x)" waterlevel()

        let "$frame += (1.0/0.15) * dt()"
        let "$speed = 50"
        let "$x = cond($x >= screen_width(), $x - screen_width(), $x += $speed * dt())"
    }
}

object ".default_startup.water.level2"
{
    requires 0.2.0
    always_active

    state "main"
    {
        set_alpha 0.25
        set_animation SD_WATERLEVEL 0
        set_animation_frame "floor($frame) mod 2"
        set_absolute_position "camera_x() + screen_width()/2 + floor($x)" waterlevel()

        let "$frame += (1.0/0.15) * dt()"
        let "$speed = 50"
        let "$x = cond($x >= screen_width(), $x - screen_width(), $x += $speed * dt())"
    }
}
