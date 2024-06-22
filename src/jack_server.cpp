#include "godot_cpp/variant/utility_functions.hpp"
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_midi.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "jack_server.h"

using namespace godot;

JackServer *JackServer::singleton = NULL;

int JackServer::jack_process(jack_nframes_t nframes, void *arg) {

    for (int i = 0; i < singleton->input_ports.size(); i++) {
        void *input_buf = jack_port_get_buffer(singleton->input_ports[i], nframes);

        // Get the number of MIDI events in the input buffer
        int event_count = jack_midi_get_event_count(input_buf);

        for (int j = 0; j < event_count; ++j) {
            jack_midi_event_t event;
            jack_midi_event_get(&event, input_buf, j);

            /*
            // Print the MIDI event
            godot::UtilityFunctions::print("MIDI event: time=%d, size=%d, data=", event.time, event.size);
            for (uint32_t j = 0; j < event.size; ++j) {
                godot::UtilityFunctions::print("%02x ", event.buffer[j]);
            }
            godot::UtilityFunctions::print("\n");
            */

            Ref<InputEventMIDI> input_event;
            input_event.instantiate();
            input_event->set_message(MIDIMessage(event.buffer[0]));

            if (event.size >= 1) {
                if (event.buffer[0] >= 0xF0) {
                    input_event->set_channel(16 * i + 0);
                    input_event->set_message(MIDIMessage(event.buffer[0]));
                } else {
                    input_event->set_channel(16 * i + (event.buffer[0] & 0xF));
                    input_event->set_message(MIDIMessage(event.buffer[0] >> 4));
                }
            }

            switch (input_event->get_message()) {
            case MIDIMessage::MIDI_MESSAGE_AFTERTOUCH:
                if (event.size == 3) {
                    input_event->set_pitch(event.buffer[1]);
                    input_event->set_pressure(event.buffer[2]);
                }
                break;

            case MIDIMessage::MIDI_MESSAGE_CONTROL_CHANGE:
                if (event.size == 3) {
                    input_event->set_controller_number(event.buffer[1]);
                    input_event->set_controller_value(event.buffer[2]);
                }
                break;

            case MIDIMessage::MIDI_MESSAGE_NOTE_ON:
            case MIDIMessage::MIDI_MESSAGE_NOTE_OFF:
                if (event.size == 3) {
                    input_event->set_pitch(event.buffer[1]);
                    input_event->set_velocity(event.buffer[2]);
                }
                break;

            case MIDIMessage::MIDI_MESSAGE_PITCH_BEND:
                if (event.size == 3) {
                    input_event->set_pitch((event.buffer[2] << 7) | event.buffer[1]);
                }
                break;

            case MIDIMessage::MIDI_MESSAGE_PROGRAM_CHANGE:
                if (event.size == 2) {
                    input_event->set_instrument(event.buffer[1]);
                }
                break;

            case MIDIMessage::MIDI_MESSAGE_CHANNEL_PRESSURE:
                if (event.size == 2) {
                    input_event->set_pressure(event.buffer[1]);
                }
                break;
            default:
                break;
            }

            godot::Input::get_singleton()->parse_input_event(input_event);
        }
    }

    for (int i = 0; i < singleton->output_ports.size(); i++) {
        void *output_buf = jack_port_get_buffer(singleton->output_ports[i], nframes);
        jack_midi_clear_buffer(output_buf);
    }

    while (singleton->buffered_events.front()) {
        List<Ref<InputEventMIDI>>::Element *E = singleton->buffered_events.front();
        Ref<InputEventMIDI> e = E->get();
        singleton->buffered_events.pop_front();

        int instance = e->get_channel() / 16;
        int channel = e->get_channel() % 16;
        unsigned char midi_data[3];

        jack_midi_event_t event;
        event.size = 0;
        event.buffer = midi_data;
        event.time = jack_frames_since_cycle_start(singleton->client);

        switch (e->get_message()) {
        case MIDIMessage::MIDI_MESSAGE_AFTERTOUCH:
            event.size = 3;
            midi_data[0] = (e->get_message() << 4 & 0xff) | (0x0f & channel);
            midi_data[1] = e->get_pitch() & 0x7f;
            midi_data[2] = e->get_pressure() & 0x7f;
            break;

        case MIDIMessage::MIDI_MESSAGE_CONTROL_CHANGE:
            event.size = 3;
            midi_data[0] = (e->get_message() << 4 & 0xff) | (0x0f & channel);
            midi_data[1] = e->get_controller_number() & 0x7f;
            midi_data[2] = e->get_controller_value() & 0x7f;
            break;

        case MIDIMessage::MIDI_MESSAGE_NOTE_ON:
        case MIDIMessage::MIDI_MESSAGE_NOTE_OFF:
            event.size = 3;
            midi_data[0] = (e->get_message() << 4 & 0xff) | (0x0f & channel);
            midi_data[1] = e->get_pitch() & 0x7f;
            midi_data[2] = e->get_velocity() & 0x7f;
            break;

        case MIDIMessage::MIDI_MESSAGE_PITCH_BEND:
            event.size = 3;
            midi_data[0] = (e->get_message() << 4 & 0xff) | (0x0f & channel);
            midi_data[1] = e->get_pitch() & 0x7f;
            midi_data[2] = e->get_pitch() >> 7 & 0x7f;
            break;

        case MIDIMessage::MIDI_MESSAGE_PROGRAM_CHANGE:
            event.size = 2;
            midi_data[0] = (e->get_message() << 4 & 0xff) | (0x0f & channel);
            midi_data[1] = e->get_instrument() & 0x7f;
            break;

        case MIDIMessage::MIDI_MESSAGE_CHANNEL_PRESSURE:
            event.size = 2;
            midi_data[0] = (e->get_message() << 4 & 0xff) | (0x0f & channel);
            midi_data[1] = e->get_pressure() & 0x7f;
            break;
        default:
            break;
        }

        if (event.size > 0 && instance < singleton->output_ports.size()) {
            godot::UtilityFunctions::print("sending the event ", nframes);
            void *output_buf = jack_port_get_buffer(singleton->output_ports[instance], nframes);
            // send the MIDI event to the output buffer
            jack_midi_event_write(output_buf, event.time, event.buffer, event.size);
        }
    }

    return 0;
}

void JackServer::jack_shutdown(void *arg) {
    exit(1);
}

JackServer::JackServer() {
    set_process_internal(true);
    running = false;
    singleton = this;
}

JackServer::~JackServer() {
    jack_client_close(client);
    singleton = NULL;
}

JackServer *JackServer::get_singleton() {
    return singleton;
}

void JackServer::process(double delta) {
}

void JackServer::_notification(int p_what) {
    switch (p_what) {
    case NOTIFICATION_INTERNAL_PROCESS: {
        process(get_process_delta_time());
    }
    }
}

void JackServer::open_midi_inputs(String name, int input_size, int output_size) {
    bool is_editor = godot::Engine::get_singleton()->is_editor_hint();

    if (!is_editor && !running) {
        // name = godot::ProjectSettings::get_singleton()->get_setting("application/config/name", name);

        input_ports.resize(input_size);
        output_ports.resize(output_size);

        running = true;
        jack_options_t options = JackNullOption;
        jack_status_t status;

        // Open a client connection to the JACK server
        client = jack_client_open(name.ascii(), options, &status);
        if (client == NULL) {
            godot::UtilityFunctions::push_error("jack_client_open() failed, status = 0x%2.0x\n", status);
            if (status & JackServerFailed) {
                godot::UtilityFunctions::push_error(stderr, "Unable to connect to JACK server\n");
            }
            exit(1);
        }

        // Register the shutdown callback
        jack_on_shutdown(client, JackServer::jack_shutdown, 0);

        // Register the process callback
        jack_set_process_callback(client, JackServer::jack_process, 0);

        // Create an input MIDI port
        for (int i = 0; i < input_size; i++) {
            singleton->input_ports.write[i] =
                jack_port_register(client, vformat("input_%d", i).ascii(), JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
            if (singleton->input_ports[i] == NULL) {
                godot::UtilityFunctions::push_error(stderr, "no more JACK ports available\n");
                exit(1);
            }
        }

        // Create an output MIDI port
        for (int i = 0; i < input_size; i++) {
            singleton->output_ports.write[i] = jack_port_register(client, vformat("output_%d", i).ascii(),
                                                                  JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
            if (singleton->output_ports[i] == NULL) {
                godot::UtilityFunctions::push_error(stderr, "no more JACK ports available\n");
                exit(1);
            }
        }

        // Activate the JACK client
        if (jack_activate(client)) {
            godot::UtilityFunctions::push_error(stderr, "cannot activate client");
            exit(1);
        }

        // Connect the input and output ports to external MIDI devices as needed
        // For example:
        // jack_connect(client, "your_midi_device:output", "jack_midi_example:input");
        // jack_connect(client, "jack_midi_example:output", "your_midi_device:input");
    }
}

void JackServer::send_midi_event(Ref<InputEventMIDI> midi_event) {
    buffered_events.push_back(midi_event);
}

void JackServer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("process", "delta"), &JackServer::process);
    ClassDB::bind_method(D_METHOD("open_midi_inputs", "name"), &JackServer::open_midi_inputs);
    ClassDB::bind_method(D_METHOD("send_midi_event", "midi_event"), &JackServer::send_midi_event);
}
