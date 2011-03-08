// ---------------------------------------------------------------------------
// Open Surge Engine
// http://opensnc.sourceforge.net
//
// File:   default_companions/surge.inc
// Desc:   Surge's companion
// Author: Alexandre
// Date:   2011-02-09, 2011-02-11
// ---------------------------------------------------------------------------

object ".surge_companion"
{
    requires 0.2.0
    always_active

    state "main"
    {
        create_child ".surge_spindash_controller"
        create_child ".surge_nolightwalk_controller"
        destroy
    }
}


object ".surge_spindash_controller"
{
    requires 0.2.0
    always_active

    state "main"
    {
        hide
        observe_player "Surge"
        change_state "stand by"
    }

    state "stand by"
    {
        on_player_duck "duck"
    }

    state "duck"
    {
        let "$p = 0"
        on_button_pressed "fire1" "charge"
        on_player_duck "duck"
        change_state "stand by"
    }

    state "charge"
    {
        set_player_animation "SD_SURGE" "6"
        strong_player
        disable_player_movement
        play_sample "charge"
        let "$p += 2"
        change_state "hold"
    }

    state "hold"
    {
        set_player_animation "SD_SURGE" "6"
        let "$p *= 0.96875 * 0.96875 ^ (60 * dt() - 1)"

        // check if there's a platform underneath the player
        attach_to_player 0 20
        on_floor_collision "hold2"

        change_state "cancel"
    }

    state "hold2"
    {
        set_player_animation "SD_SURGE" "6"
        let "$p *= 0.96875 * 0.96875 ^ (60 * dt() - 1)"

        // check if the user wants to charge more, or if we can release the player
        on_button_pressed "fire1" "charge"
        on_button_down "down" "hold"

        change_state "release"
    }

    state "cancel"
    {
        enable_player_movement
        weak_player
        change_state "stand by"
    }

    state "release"
    {
        enable_player_movement
        weak_player
        set_player_xspeed "(480 + 30 * floor($p)) * player_direction()"
        roll_player
        change_state "stand by"
    }
}

object ".surge_nolightwalk_controller"
{
    requires 0.2.0
    always_active

    state "main"
    {
        hide
        observe_player "Surge"
        change_state "stand by"
    }

    state "stand by"
    {
        on_player_in_the_air "air"
    }

    state "air"
    {
        on_player_walk "walking in the air"
        on_player_run "running in the air"
        on_player_brake "braking in the air"
        on_player_in_the_air "air"
        change_state "stand by"
    }

    state "walking in the air"
    {
        on_player_spring "stand by"
        on_player_jump "stand by"
        on_player_gethit "stand by"
        on_player_death "stand by"

        set_player_animation "SD_SURGE" 18
        on_player_in_the_air "walking in the air"
        change_state "stand by"
    }

    state "running in the air"
    {
        on_player_spring "stand by"
        on_player_jump "stand by"
        on_player_gethit "stand by"
        on_player_death "stand by"

        set_player_animation "SD_SURGE" 19
        on_player_in_the_air "running in the air"
        change_state "stand by"
    }

    state "braking in the air"
    {
        on_player_spring "stand by"
        on_player_jump "stand by"
        on_player_gethit "stand by"
        on_player_death "stand by"

        set_player_animation "SD_SURGE" 20
        on_player_in_the_air "braking in the air"
        change_state "stand by"
    }
}
