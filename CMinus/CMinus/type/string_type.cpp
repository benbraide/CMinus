#include "pointer_type.h"
#include "string_type.h"

cminus::type::string::string(logic::runtime &runtime, logic::storage::object *parent)
	: class_(runtime, "string", parent){
	declaration::variable::attribute_list_type attributes{
		runtime.global_storage->find_attribute("Private", false)
	};

	add_declaration(runtime, std::make_shared<declaration::variable>(
		attributes,																		//Attributes
		runtime.global_storage->get_primitve_type(type::primitive::id_type::uint64_),	//Type
		"size_",																		//Name
		nullptr																			//Initialization
	));

	auto char_type = runtime.global_storage->get_primitve_type(type::primitive::id_type::char_);
	add_declaration(runtime, std::make_shared<declaration::variable>(
		attributes,																		//Attributes
		std::make_shared<type::pointer>(char_type),										//Type
		"data_",																		//Name
		nullptr																			//Initialization
	));

	add_function(runtime, std::make_shared<declaration::string::default_constructor>(runtime, this));
	add_function(runtime, std::make_shared<declaration::string::destructor>(runtime, this));

	add_function(runtime, std::make_shared<declaration::string::empty>(runtime, this));
	add_function(runtime, std::make_shared<declaration::string::size>(runtime, this));

	add_function(runtime, std::make_shared<declaration::string::data>(runtime, true, this));
	add_function(runtime, std::make_shared<declaration::string::data>(runtime, false, this));

	add_function(runtime, std::make_shared<declaration::string::resize>(runtime, this));
	add_function(runtime, std::make_shared<declaration::string::clear>(runtime, this));
}

cminus::type::string::~string() = default;
