//
// ----------------------------------------------------------------------------
// Dialog Box Object
// A configurable dialog box made using a component-based approach.
// ----------------------------------------------------------------------------
// Author: Alexandre
// Initial date: 2011-09-22
// Last update:  2012-02-23
// ----------------------------------------------------------------------------
//
//
// BASIC USAGE:
//
// Set $_dialogbox_text_id = <TEXT_ID> and $_dialogbox_avatar_id = <AVATAR_ID>
// before spawning '.dialogbox.default'. The DIALOGBOX_MESSAGE_<TEXTID> string,
// defined in the .lng file (in the languages/ folder) will be displayed along
// with the animation #<AVATAR_ID> of the DIALOGBOX_AVATAR sprite (defined in
// a .spr file of the sprites/ folder)
//
// The spawner object should have a state named '.dialogbox:on_destroy', which
// will be called when the dialog box gets destroyed. You may choose to take
// some action when this happens, or just 'return_to_previous_state'.
//
// If you don't want an avatar in your dialog box, just set $_dialogbox_text_id
// and spawn '.dialogbox.default[no_avatar]' instead.
// 
// Example:
//
// object foo {
//     requires 0.2.0
//     always_active // the spawner object should be always_active!
//
//     state main {
//         let $_dialogbox_text_id=0
//         let $_dialogbox_avatar_id=0
//         create_child .dialogbox.default
//         change_state freeze
//     }
//
//     state freeze {
//     }
//
//     state .dialogbox:on_destroy {
//         destroy
//     }
// }
//
//
// ADVANCED USAGE:
//
// If you want to create your own customized dialog box, script a new object
// called '.dialogbox.<PICK_A_NAME>'. Your customized dialog box may have
// any behavior you want. Here's what you need to do:
//
// 1. Upon creation, spawn the following objects:
//
//    a) .dialogbox.base.background
//    b) .dialogbox.base.avatar (optional)
//    c) .dialogbox.base.text
//    d) .dialogbox.base.button (optional)
//
//    These objects behave in a way that is documented in the base/ folder.
//    You may check that out, or read some examples below. These objects
//    provide default functionality.
//
//    This system is meant to be flexible enough, so if you need that, for
//    example, the dialog box behaves in a completely different manner,
//    you can replace its components by customized ones you write. For example,
//    you can replace '.dialogbox.base.background' by
//    '.dialogbox.base.background[slide_from_bottom]', and your dialog box will
//    suddenly become much richer.
//
//    You don't need to change existing code: just create new components, and
//    attach them to your objects. This is the beauty of the component-based
//    approach.
//
// 2. Your dialogbox must implement the following states:
//
//    a) .dialogbox.base.background:on_appear
//       Will be called when the dialog completes its "appearing" animation.
//       Normally you want to start writing the dialog text here. You do that
//       by changing the state of the text object to 'start'.
//
//    b) .dialogbox.base.background:on_disappear
//       Will be called when the dialog completes its "disappearing" animation.
//       Normally you will delete the object in this state (see #3).
//
//    c) .dialogbox.base.text:on_complete
//       Will be called when the full text is rendered to the screen.
//
//    d) .dialogbox.base.text:on_start
//       Will be called when a text begins to be rendered on the screen.
//
// 3. Upon deletion, it's mandatory to change the state of its parent object to
//    '.dialogbox:on_destroy' !
//
// Whenever you want to show your dialogbox in the game, spawn the
// '.dialogbox.<DIALOGNAME>' object in the level. Your other scripts should NOT
// have any logic to handle dialog boxes other than that. The dialog boxes
// should take care of themselves.
//
// Upon deletion, the dialog box will change the state of its parent object to
// '.dialogbox:on_destroy'. You may choose to take some action when this state
// gets called, or maybe just 'return_to_previous_state'.
//

#include "base/background.inc"
#include "base/avatar.inc"
#include "base/text.inc"
#include "base/button.inc"

// Here's an example dialogbox, so that you can study it.
// Experiment spawning it in a level.
object .dialogbox.example
{
    requires 0.2.0
    always_active

    state main
    {
        hide

        // it will display animation #1 of the DIALOGBOX_AVATAR sprite, defined
        // in a .spr in sprites/
        let $_dialogbox_avatar_id=1

        // it will display DIALOGBOX_MESSAGE_0, defined in the current .lng
        // file in the languages/ directory
        let $_dialogbox_text_id=0

        // it will create the components of the dialog box
        create_child .dialogbox.base.background 0 0 background
        create_child .dialogbox.base.avatar 0 0 avatar
        create_child .dialogbox.base.text 0 0 text

        // wait a bit
        disable_player_movement
        change_state wait
    }

    state wait
    {
        // do nothing!
    }

    state .dialogbox.base.background:on_appear
    {
        // when the dialog box fully appears on the screen, start the text
        // rendering routines
        change_child_state text start
        change_state wait
    }

    state .dialogbox.base.background:on_disappear
    {
        // when the dialog box fully disappears from the screen, destroy
        // its components and itself
        change_child_state background destroy
        change_child_state avatar destroy
        change_child_state text destroy
        change_parent_state .dialogbox:on_destroy
        enable_player_movement
        destroy
    }

    state .dialogbox.base.text:on_complete
    {
        // after the text is fully rendered to the screen...
        on_timeout 5.0 disappear
    }

    state .dialogbox.base.text:on_start
    {
    }

    state disappear
    {
        // disappear
        change_child_state background disappear
        change_state wait
    }

    state hide
    {
        // hide
        change_child_state background hide
        change_state wait
    }

    state show
    {
        // show the dialog box after it's been hidden
        change_child_state background show
        change_state wait
    }
}

// If you just want a default dialog box, spawn .dialogbox.default
// Set $_dialogbox_avatar_id and $dialogbase_text_id first.
object .dialogbox.default
{
    requires 0.2.0
    always_active
    detach_from_camera

    state main
    {
        hide
        create_child .dialogbox.base.background[slide_from_bottom] 0 0 background
        create_child .dialogbox.base.avatar 0 0 avatar
        create_child .dialogbox.base.text[typewriter] 0 0 text
        create_child .dialogbox.base.button 0 0 button
        disable_player_movement
        change_state wait
    }

    state wait
    {
    }

    state .dialogbox.base.background:on_appear
    {
        change_child_state text start
        change_state wait
    }

    state .dialogbox.base.background:on_disappear
    {
        change_child_state background destroy
        change_child_state avatar destroy
        change_child_state text destroy
        change_child_state button destroy
        change_parent_state .dialogbox:on_destroy
        enable_player_movement
        destroy
    }

    state .dialogbox.base.text:on_complete
    {
        change_child_state button stop
        change_state completed
    }

    state .dialogbox.base.text:on_start
    {
    }

    state completed
    {
        on_button_pressed fire1 disappear
    }

    state disappear
    {
        change_child_state background disappear
        change_state wait
    }

    state hide
    {
        // hide
        change_child_state background hide
        change_state wait
    }

    state show
    {
        // show the dialog box after it's been hidden
        change_child_state background show
        change_state wait
    }
}

// The default dialog box without an avatar
// Set $_dialogbase_text_id before spawning this.
object .dialogbox.default[no_avatar]
{
    requires 0.2.0
    always_active
    detach_from_camera

    state main
    {
        hide
        create_child .dialogbox.base.background[slide_from_bottom] 0 0 background
        create_child .dialogbox.base.text[typewriter] 0 0 text
        create_child .dialogbox.base.button 0 0 button
        disable_player_movement
        change_state wait
    }

    state wait
    {
    }

    state .dialogbox.base.background:on_appear
    {
        change_child_state text start
        change_state wait
    }

    state .dialogbox.base.background:on_disappear
    {
        change_child_state background destroy
        change_child_state text destroy
        change_child_state button destroy
        change_parent_state .dialogbox:on_destroy
        enable_player_movement
        destroy
    }

    state .dialogbox.base.text:on_complete
    {
        change_child_state button stop
        change_state completed
    }

    state .dialogbox.base.text:on_start
    {
    }

    state completed
    {
        on_button_pressed fire1 disappear
    }

    state disappear
    {
        change_child_state background disappear
        change_state wait
    }

    state hide
    {
        // hide
        change_child_state background hide
        change_state wait
    }

    state show
    {
        // show the dialog box after it's been hidden
        change_child_state background show
        change_state wait
    }
}
