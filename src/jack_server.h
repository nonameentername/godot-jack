#ifndef JACKSERVER_H
#define JACKSERVER_H

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input_event_midi.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <jack/jack.h>
#include <jack/midiport.h>
#include <stdio.h>
#include <stdlib.h>

namespace godot {

class JackServer : public Node {
    GDCLASS(JackServer, Node);

private:
    bool running;
    jack_client_t *client;
    Vector<jack_port_t *> input_ports;
    Vector<jack_port_t *> output_ports;
    List<Ref<InputEventMIDI>> buffered_events;

protected:
    static void _bind_methods();
    static JackServer *singleton;

public:
    JackServer();
    ~JackServer();

    static JackServer *get_singleton();
    void process(double delta);
    void _notification(int p_what);
    void open_midi_inputs(String name, int input_size, int output_size);
    void send_midi_event(Ref<InputEventMIDI> midi_event);

    static int jack_process(jack_nframes_t nframes, void *arg);
    static void jack_shutdown(void *arg);
};
} // namespace godot

#endif
