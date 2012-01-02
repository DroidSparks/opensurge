// ---------------------------------------------------------------------------
// Open Surge Engine
// http://opensnc.sourceforge.net
//
// File:   objects/default_startup/camera.inc
// Desc:   the original camera of the engine just locks the player at the
//         center of the screen. Can we make it better?
// Author: Alexandre
// Date:   2012-01-01
// ---------------------------------------------------------------------------

object ".default_startup.camera"
{
    requires 0.2.0
    always_active

    state "main"
    {
        //hide
        set_zindex 1
set_animation SD_BLUERING 0

        let "$LAG_FACTOR = 0.13" // the lower this value, the fastest the camera is
        change_state "reposition"
    }

    state "reposition"
    {
        on_level_cleared "drop"

        observe_active_player
        attach_to_player 0 0
        request_camera_focus

        let "$cam_x = xpos()"
        let "$cam_y = ypos()"

        change_state "watch"
    }

    state "watch"
    {
        request_camera_focus
        on_player_rect_collision -8 -16 8 16 watch //-$h_border -$v_border $h_border $v_border watch
        change_state move
    }

    state move
    {
        request_camera_focus
        let "$dx = player_xpos() - xpos()"
        let "$dy = player_ypos() - ypos()"
        let "$len = sqrt($dx^2 + $dy^2)"
        let "$vel = $len / $LAG_FACTOR"
        let "$dx = $vel * $dx / $len"
        let "$dy = $vel * $dy / $len"
        move $dx $dy
        //on_player_rect_collision -8 -16 8 16 watch
        if "floor(abs($dx)) <= 2 and floor(abs($dy)) <= 2" watch
    }

    state "drop"
    {
        drop_camera_focus
        destroy
    }
}
