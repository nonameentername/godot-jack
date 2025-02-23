extends Node2D


@onready
var midi_events: ItemList = $ItemList


func _ready():
	print ("godot-jack version: ", JackServer.get_version(), " build: ", JackServer.get_build())
	JackServer.open_midi_inputs("godot-midi", 4, 4)


func _input(input_event):
	if input_event is InputEventMIDI:
		if input_event.message == MIDI_MESSAGE_NOTE_ON:
			midi_events.add_item("note_on channel = %s note = %s velocity = %s" % [input_event.channel, input_event.pitch, input_event.velocity])
		if input_event.message == MIDI_MESSAGE_NOTE_OFF:
			midi_events.add_item("note_off channel = %s note = %s velocity = %s" % [input_event.channel, input_event.pitch, input_event.velocity])
		if midi_events.item_count > 0:
			midi_events.select(midi_events.item_count - 1, true)
			midi_events.ensure_current_is_visible()
		_print_midi_info(input_event)
		JackServer.send_midi_event(input_event)


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
