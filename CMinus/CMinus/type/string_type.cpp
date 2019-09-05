#include "pointer_type.h"
#include "string_type.h"

cminus::type::string::string(logic::runtime &runtime)
	: class_(runtime, "string", nullptr){}

cminus::type::string::~string() = default;

void cminus::type::string::print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const{
	if (auto str = runtime.global_storage->get_string_data(runtime, data); str != nullptr)
		runtime.writer.write_buffer(str, strlen(str));
}

void cminus::type::string::print(logic::runtime &runtime, bool is_qualified) const{
	runtime.writer.write_buffer("string", 6u);
}

std::shared_ptr<cminus::evaluator::object> cminus::type::string::get_evaluator(logic::runtime &runtime) const{
	return nullptr;
}

void cminus::type::string::init(logic::runtime &runtime){
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

	add_function(runtime, std::make_shared<declaration::string::destructor>(this));
	add_function(runtime, std::make_shared<declaration::string::default_constructor>(this));

	add_function(runtime, std::make_shared<declaration::string::copy_constructor>(runtime, this));
	add_function(runtime, std::make_shared<declaration::string::sub_constructor>(runtime, this));

	add_function(runtime, std::make_shared<declaration::string::assignment_constructor>(runtime, this));
	add_function(runtime, std::make_shared<declaration::string::fill_constructor>(runtime, this));

	add_function(runtime, std::make_shared<declaration::string::empty>(runtime, this));
	add_function(runtime, std::make_shared<declaration::string::size>(runtime, this));

	add_function(runtime, std::make_shared<declaration::string::data>(runtime, true, this));
	add_function(runtime, std::make_shared<declaration::string::data>(runtime, false, this));

	add_function(runtime, std::make_shared<declaration::string::at>(runtime, true, this));
	add_function(runtime, std::make_shared<declaration::string::at>(runtime, false, this));

	add_function(runtime, std::make_shared<declaration::string::resize>(runtime, this));
	add_function(runtime, std::make_shared<declaration::string::clear>(runtime, this));
}
