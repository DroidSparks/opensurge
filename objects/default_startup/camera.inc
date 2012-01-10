// =====================================
// Open Surge
// Version: 0.2.0
// File: camera.inc
// Required: Bleeding Edge 616
// =====================================

object ".default_startup.camera"
{
    requires 0.2.0
    always_active

    state "main"
    {
		hide
		
		create_child ".default_startup.camera_focus_y_tracker"
		create_child ".default_startup.camera_focus_x_tracker"
		create_child ".default_startup.camera_focus"
		create_child ".default_startup.camera.textout_is_your_best_friend"
		change_state "track"
	}
	state "track"
	{
		let "$_player_x_to_center = player_xpos() - $_camera_focus_x"
		let "$_player_y_to_center = player_ypos() - $_camera_focus_y"
	}
}

object ".default_startup.camera.textout_is_your_best_friend"
{
	requires 0.2.0
	always_active
	detach_from_camera
	state "main"
	{
		set_zindex 0.1
		set_absolute_position 100 100
		let "$x = player_xspeed()"
		let "$y = player_yspeed()"
		textout "Default" 0 0 "player_x_to_center: $_player_x_to_center"
		textout "Default" 0 10 "player_y_to_center: $_player_y_to_center"
		textout "Default" 0 20 "camera_focus_x: $_camera_focus_x"
		textout "Default" 0 30 "camera_focus_y: $_camera_focus_y"
		textout "Default" 0 40 "camera_focus_x2: $_camera_focus_x2"
		textout "Default" 0 50 "camera_focus_y2: $_camera_focus_y2"
		textout "Default" 0 60 "xspeed: $x"
		textout "Default" 0 70 "yspeed: $y"
	}
}

object ".default_startup.camera_focus_x_tracker"
{
	requires 0.2.0
	always_active
	
	state "main"
	{
		hide
		if "$_player_x_to_center > 8" "move_up_to_16_right"
		if "$_player_x_to_center < -8" "move_up_to_16_left"
	}
	
	state "move_up_to_16_right"
	{
		let "$_camera_focus_x2 += min(player_xpos() - $_camera_focus_x, 16)"
		change_state "main"
	}
	
	state "move_up_to_16_left"
	{
		let "$_camera_focus_x2 -= min($_camera_focus_x - player_xpos(), 16)"
		change_state "main"
	}
}

object ".default_startup.camera_focus_y_tracker"
{
	requires 0.2.0
	always_active
	
	state "main"
	{
		hide
		if "($_player_y_to_center > 32) and (player_yspeed() > 6)" "move_up_to_16_down"
		if "($_player_y_to_center > 32) and (player_yspeed() <= 6)" "move_up_to_6_down"
		if "($_player_y_to_center < -32) and (player_yspeed() > 6)" "move_up_to_16_up"
		if "($_player_y_to_center < -32) and (player_yspeed() <= 6)" "move_up_to_6_up"
	}
	
	state "move_up_to_6_down"
	{
		let "$_camera_focus_y2 += min(player_ypos() - $_camera_focus_y, 6)"
		change_state "main"
	}
	
	state "move_up_to_16_down"
	{
		let "$_camera_focus_y2 += min(player_ypos() - $_camera_focus_y, 16)"
		change_state "main"
	}
	
	state "move_up_to_6_up"
	{
		let "$_camera_focus_y2 -= min($_camera_focus_y - player_ypos(), 6)"
		change_state "main"
	}
	
	state "move_up_to_16_up"
	{
		let "$_camera_focus_y2 -= min($_camera_focus_y - player_ypos(), 16)"
		change_state "main"
	}
}

object ".default_startup.camera_focus"
{
	requires 0.2.0
	always_active
	
	state "main"
	{
		set_animation "SD_CAMERA_HITBOX" 1
		set_absolute_position player_xpos() player_ypos()
		attach_to_player
		let "$_camera_focus_x2 = player_xpos()"
		let "$_camera_focus_y2 = player_ypos()"
		request_camera_focus
		change_state "listen"
	}
	
	state "listen"
	{
		let "$_camera_focus_x = xpos()"
		let "$_camera_focus_y = ypos()"
		set_absolute_position $_camera_focus_x2 $_camera_focus_y2
	}
}

