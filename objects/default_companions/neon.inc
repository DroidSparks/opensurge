// ---------------------------------------------------------------------------
// Open Neon Engine
// http://opensnc.sourceforge.net
//
// File:   default_companions/neon.inc
// Desc:   Neon's companion
// Author: Alexandre
// Date:   2011-02-09, 2011-02-11
// ---------------------------------------------------------------------------

object ".neon_companion"
{
    requires 0.2.0
    always_active

    state "main"
    {
        create_child ".neon_spindash_controller"
        destroy
    }
}


object ".neon_spindash_controller"
{
    requires 0.2.0
    always_active

    state "main"
    {
        hide
        observe_player "Neon"
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
        set_player_animation "SD_NEON" "6"
        strong_player
        disable_player_movement
        play_sample "charge"
        let "$p += 2"
        change_state "hold"
    }

    state "hold"
    {
        set_player_animation "SD_NEON" "6"
        let "$p *= 0.96875 * 0.96875 ^ (60 * dt() - 1)"

        // check if there's a platform underneath the player
        attach_to_player 0 20
        on_floor_collision "hold2"

        change_state "cancel"
    }

    state "hold2"
    {
        set_player_animation "SD_NEON" "6"
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
