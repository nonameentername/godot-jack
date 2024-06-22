extends Node2D


# Called when the node enters the scene tree for the first time.
func _ready():
	JackServer.open_midi_inputs("godot", 4, 4)


func _input(input_event):
	if input_event is InputEventMIDI:
		_print_midi_info(input_event)
		JackServer.send_midi_event(input_event)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass


func _print_midi_info(midi_event):
	print()
	print(midi_event)
	print("Channel ", midi_event.channel)
	print("Message ", midi_event.message)
	print("Pitch ", midi_event.pitch)
	print("Velocity ", midi_event.velocity)
	print("Instrument ", midi_event.instrument)
	print("Pressure ", midi_event.pressure)
	print("Controller number: ", midi_event.controller_number)
	print("Controller value: ", midi_event.controller_value)
