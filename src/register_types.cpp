#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "jack_server.h"

using namespace godot;

static JackServer *jack_module;

void initialize_jackgodot_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ClassDB::register_class<JackServer>();
    jack_module = memnew(JackServer);
    Engine::get_singleton()->register_singleton("JackServer", JackServer::get_singleton());
}

void uninitialize_jackgodot_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    Engine::get_singleton()->unregister_singleton("JackServer");
    memdelete(jack_module);
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT jackgodot_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                                  const GDExtensionClassLibraryPtr p_library,
                                                  GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_jackgodot_module);
    init_obj.register_terminator(uninitialize_jackgodot_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
