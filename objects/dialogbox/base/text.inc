//
// Some text components for you to choose:
//
// .dialogbox.base.text
// Your basic text
//
// .dialogbox.base.text[typewriter]
// A text that appears letter by letter
//
//
// If you want to create text components, here's what you need:
//
// *) The "destroy" state must destroy this component
//
// *) The "start" state must make the text become visible
//    (or an animation of it becoming visible)
//
// *) The "refresh" state, in case the user wants to change the displayed text
//
// *) When the text is fully visible, change its parent state to
//    ".dialogbox.base.text:on_complete"
//
// *) If $_dialogbox_text_id = <TEXT_ID>, it will display the
//    DIALOGBOX_MESSAGE_<TEXT_ID> string, written in the .lng file
//
// *) The text should be positioned relative to the dialog position,
//    $_dialogbox_base_bgx and $_dialogbox_base_bgy
//

// Your basic text
object .dialogbox.base.text
{
    requires 0.2.0
    always_active
    detach_from_camera

    state main
    {
        hide
    }

    state start
    {
        let $id=$_dialogbox_text_id
        change_state display
    }

    state refresh
    {
        let $completed=0
        change_state start
    }

    state destroy
    {
        destroy
    }

    // --------------------------------------------

    state display
    {
        let $offset_x=15+cond($_dialogbox_base_avatar_exists,$_dialogbox_base_avatar_width,0)
        let $offset_y=($_dialogbox_base_bgh-$_dialogbox_base_avatar_height)/2
        let $width=$_dialogbox_base_bgw-$offset_x-15

        set_zindex $_dialogbox_base_bgz+0.1
        set_absolute_position $_dialogbox_base_bgx+$offset_x $_dialogbox_base_bgy+$offset_y
        textout menu.small 0 0 "$DIALOGBOX_MESSAGE_$id" $width

        if $completed display
        change_parent_state .dialogbox.base.text:on_complete
        let $completed=1
    }
}

// A text that appears like a type writer
object .dialogbox.base.text[typewriter]
{
    requires 0.2.0
    always_active
    detach_from_camera

    state main
    {
        hide
    }

    state start
    {
        create_child .dialogbox.base.text[typewriter].textadvancer
        create_child .dialogbox.base.text[typewriter].audio
        let $id=$_dialogbox_text_id
        change_state display
    }

    state refresh
    {
        let $completed=0
        change_state start
    }

    state destroy
    {
        destroy
    }

    // --------------------------------------------

    state display
    {
        let $offset_x=15+cond($_dialogbox_base_avatar_exists,$_dialogbox_base_avatar_width,0)
        let $offset_y=($_dialogbox_base_bgh-$_dialogbox_base_avatar_height)/2
        let $width=$_dialogbox_base_bgw-$offset_x-15

        set_zindex $_dialogbox_base_bgz+0.1
        set_absolute_position $_dialogbox_base_bgx+$offset_x $_dialogbox_base_bgy+$offset_y
        textout menu.small 0 0 "$DIALOGBOX_MESSAGE_$id" $width 0 $_dialogbox_base_textadvancer_pos

        unless $_dialogbox_base_textadvancer_completed display

        if $completed display
        change_parent_state .dialogbox.base.text:on_complete
        let $completed=1
    }
}

object .dialogbox.base.text[typewriter].audio
{
    requires 0.2.0
    always_active

    state main
    {
        hide
        change_state idle
    }

    state idle
    {
        if $_dialogbox_base_textadvancer_completed destroy
        on_timeout 0.07 play
    }

    state destroy
    {
        destroy
    }

    state play
    {
        play_sample samples/talk.wav
        return_to_previous_state
    }
}

object .dialogbox.base.text[typewriter].textadvancer
{
    requires 0.2.0
    always_active

    state main
    {
        let $_dialogbox_base_textadvancer_pos=0
        let $_dialogbox_base_textadvancer_completed=0

        let $limit=0.01
        hide

        let $id=$_dialogbox_text_id
        textout menu.small 0 infinity() "$DIALOGBOX_MESSAGE_$id"
        let $len=$_STRLEN

        change_state idle
    }

    state idle
    {
        on_button_pressed fire1 finish
        on_button_pressed fire2 finish
        if ($t+=dt())>=$limit advance
    }

    state advance
    {
        let $t-=$limit
        let $_dialogbox_base_textadvancer_pos=($pos+=1)
        if $_dialogbox_base_textadvancer_pos>=$len finish
        return_to_previous_state
    }

    state finish
    {
        let $_dialogbox_base_textadvancer_pos=$len
        let $_dialogbox_base_textadvancer_completed=1
        destroy
    }
}
