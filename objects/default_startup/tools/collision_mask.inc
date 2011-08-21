//
// Collision Mask object by Alexandre
// this will detect if there is a solid brick at a specified position on the level
// will only work if it's near the camera
//
// created on: 2011-08-21
// last update: 2011-08-21
//
// how to use:
//
// +--------------------------------------------------------------------------------+
// | let "$_collisionmask_xpos = [YOUR X-COORD VALUE]"                              |
// | let "$_collisionmask_ypos = [YOUR Y-COORD VALUE]"                              |
// | change_closest_object_state ".tool.collision_mask" "check"                     |
// | if "$_collisionmask_gotbrick" "[YOUR STATE NAME WHEN THERE'S A COLLISION]"     |
// | change_state "[YOUR STATE NAME WHEN THERE ISN'T A COLLISION]"                  |
// +--------------------------------------------------------------------------------+
//

object ".tool.collision_mask"
{
    requires 0.2.0
    always_active

    state "main"
    {
        set_animation "SD_PIXEL" "0"
        hide
        let "$_collisionmask_gotbrick = 0"
        change_state "idle"
    }


    state "idle"
    {
    }

    state "check"
    {
        set_absolute_position "$_collisionmask_xpos" "$_collisionmask_ypos"
        let "$_collisionmask_gotbrick = 1"
        on_brick_collision "idle"
        on_floor_collision "idle"
        let "$_collisionmask_gotbrick = 0"
        change_state "idle"
    }
}
