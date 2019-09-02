#include "../logic/runtime.h"

cminus::node::named_constant::named_constant(object *parent, const index_info &index, constant_type type)
	: object_with_index(parent, index), type_(type){}

cminus::node::named_constant::~named_constant() = default;

std::shared_ptr<cminus::node::object> cminus::node::named_constant::clone() const{
	return std::make_shared<named_constant>(nullptr, index_, type_);
}

void cminus::node::named_constant::print(logic::runtime &runtime) const{
	switch (type_){
	case constant_type::false_:
		runtime.writer.write_buffer("false", 5u);
		break;
	case constant_type::true_:
		runtime.writer.write_buffer("true", 4u);
		break;
	case constant_type::indeterminate:
		runtime.writer.write_buffer("indeterminate", 13u);
		break;
	case constant_type::nullptr_:
		runtime.writer.write_buffer("nullptr", 7u);
		break;
	case constant_type::nan_:
		runtime.writer.write_buffer("NaN", 3u);
		break;
	default:
		break;
	}
}

std::shared_ptr<cminus::memory::reference> cminus::node::named_constant::evaluate(logic::runtime &runtime) const{
	return runtime.global_storage->get_named_constant(type_);
}

cminus::node::named_constant::constant_type cminus::node::named_constant::get_type() const{
	return type_;
}
