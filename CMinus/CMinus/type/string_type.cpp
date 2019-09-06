#include "pointer_type.h"
#include "string_type.h"

cminus::type::string::string(logic::runtime &runtime)
	: class_(runtime, "string", nullptr){}

cminus::type::string::~string() = default;

void cminus::type::string::print(logic::runtime &runtime, bool is_qualified) const{
	runtime.writer.write_buffer("string", 6u);
}

void cminus::type::string::print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const{
	if (auto str = runtime.global_storage->get_string_data(runtime, data); str != nullptr)
		runtime.writer.write_buffer(str, strlen(str));
}

cminus::type::object::score_result_type cminus::type::string::get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const{
	if (&target == this)
		return score_result_type::exact;

	if (converts_auto(target))
		return score_result_type::assignable;

	if (is_ancestor(target))
		return score_result_type::ancestor;

	auto primitive_target_type = dynamic_cast<const primitive *>(&target);
	return ((primitive_target_type == nullptr || !primitive_target_type->is_numeric()) ? score_result_type::nil : score_result_type::assignable);
}

std::shared_ptr<cminus::memory::reference> cminus::type::string::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, cast_type type) const{
	if (type != cast_type::static_ && type != cast_type::rval_static && type != cast_type::ref_static)
		return nullptr;

	if (target_type.get() == this)
		return ((type == cast_type::static_) ? runtime.global_storage->create_string(runtime, runtime.global_storage->get_string_data(runtime, data)) : data);

	if (type != cast_type::static_ && type != cast_type::rval_static)
		return nullptr;

	auto primitive_target_type = dynamic_cast<primitive *>(target_type.get());
	if (primitive_target_type == nullptr)
		return nullptr;

	switch (primitive_target_type->get_id()){
	case primitive::id_type::int16_:
		return runtime.global_storage->create_scalar(static_cast<__int16>(std::atoi(runtime.global_storage->get_string_data(runtime, data))));
	case primitive::id_type::uint16_:
		return runtime.global_storage->create_scalar(static_cast<unsigned __int16>(std::atoi(runtime.global_storage->get_string_data(runtime, data))));
	case primitive::id_type::int32_:
		return runtime.global_storage->create_scalar(std::atoi(runtime.global_storage->get_string_data(runtime, data)));
	case primitive::id_type::uint32_:
		return runtime.global_storage->create_scalar(static_cast<unsigned __int32>(std::strtoul(runtime.global_storage->get_string_data(runtime, data), nullptr, 10)));
	case primitive::id_type::int64_:
		return runtime.global_storage->create_scalar(std::strtoll(runtime.global_storage->get_string_data(runtime, data), nullptr, 10));
	case primitive::id_type::uint64_:
		return runtime.global_storage->create_scalar(std::strtoull(runtime.global_storage->get_string_data(runtime, data), nullptr, 10));
	case primitive::id_type::float_:
		return runtime.global_storage->create_scalar(std::strtof(runtime.global_storage->get_string_data(runtime, data), nullptr));
	case primitive::id_type::double_:
		return runtime.global_storage->create_scalar(std::strtod(runtime.global_storage->get_string_data(runtime, data), nullptr));
	case primitive::id_type::ldouble:
		return runtime.global_storage->create_scalar(std::strtold(runtime.global_storage->get_string_data(runtime, data), nullptr));
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<cminus::evaluator::object> cminus::type::string::get_evaluator(logic::runtime &runtime) const{
	return runtime.global_storage->get_evaluator(evaluator::id::string);
}

void cminus::type::string::init(logic::runtime &runtime){
	declaration::variable::attribute_list_type attributes{
		runtime.global_storage->find_attribute("Private", false)
	};

	add_declaration(std::make_shared<declaration::variable>(
		attributes,																		//Attributes
		runtime.global_storage->get_primitve_type(type::primitive::id_type::uint64_),	//Type
		"size_",																		//Name
		nullptr																			//Initialization
	));

	auto char_type = runtime.global_storage->get_primitve_type(type::primitive::id_type::char_);
	add_declaration(std::make_shared<declaration::variable>(
		attributes,																		//Attributes
		std::make_shared<type::pointer>(char_type),										//Type
		"data_",																		//Name
		nullptr																			//Initialization
	));

	add_declaration(std::make_shared<declaration::string::destructor>(this));
	add_declaration(std::make_shared<declaration::string::default_constructor>(this));

	add_declaration(std::make_shared<declaration::string::copy_constructor>(runtime, this));
	add_declaration(std::make_shared<declaration::string::sub_constructor>(runtime, this));

	add_declaration(std::make_shared<declaration::string::assignment_constructor>(runtime, this));
	add_declaration(std::make_shared<declaration::string::fill_constructor>(runtime, this));

	add_declaration(std::make_shared<declaration::string::empty>(runtime, this));
	add_declaration(std::make_shared<declaration::string::size>(runtime, this));

	add_declaration(std::make_shared<declaration::string::data>(runtime, true, this));
	add_declaration(std::make_shared<declaration::string::data>(runtime, false, this));

	add_declaration(std::make_shared<declaration::string::at>(runtime, true, this));
	add_declaration(std::make_shared<declaration::string::at>(runtime, false, this));

	add_declaration(std::make_shared<declaration::string::resize>(runtime, this));
	add_declaration(std::make_shared<declaration::string::clear>(runtime, this));
	add_declaration(std::make_shared<declaration::string::swap>(runtime, this));

	build();
}
