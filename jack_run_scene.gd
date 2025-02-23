extends SceneTree


func _init():
	var main_scene = preload("res://test.tscn")
	var main = main_scene.instantiate()

	change_scene_to_packed(main_scene)

	await create_timer(2.0).timeout

	unload_current_scene()
	quit()
