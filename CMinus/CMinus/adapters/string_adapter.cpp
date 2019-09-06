#include "string_adapter.h"

cminus::adapter::string::string(cminus::logic::runtime &runtime, const std::string &value)
	: runtime_(&runtime), value_(runtime.global_storage->create_string(runtime, value)){}

cminus::adapter::string::string(cminus::logic::runtime &runtime, std::shared_ptr<cminus::memory::reference> value)
	: runtime_(&runtime), value_(runtime.global_storage->create_string(runtime, runtime.global_storage->get_string_data(runtime, value))){}

const char *cminus::adapter::string::data() const{
	auto data = call_(*runtime_, call_info{ "data", value_ });
	auto data_block = runtime_->memory_object.get_block(data->read_scalar<unsigned __int64>(*runtime_));
	return reinterpret_cast<char *>(data_block->get_data());
}

std::size_t cminus::adapter::string::size() const{
	return call_(*runtime_, call_info{ "size", value_ })->read_scalar<std::size_t>(*runtime_);
}

bool cminus::adapter::string::empty() const{
	return (call_(*runtime_, call_info{ "empty", value_ })->read_scalar<node::named_constant::constant_type>(*runtime_) == node::named_constant::constant_type::true_);
}

char cminus::adapter::string::at(std::size_t position) const{
	return call_(*runtime_, call_info{
		"at",
		value_,
		std::vector<std::shared_ptr<cminus::memory::reference>>{ runtime_->global_storage->create_scalar(position) } }
	)->read_scalar<char>(*runtime_);
}

void cminus::adapter::string::clear(){
	call_(*runtime_, call_info{ "clear", value_ });
}

std::shared_ptr<cminus::memory::reference> cminus::adapter::string::get_value() const{
	return value_;
}

std::shared_ptr<cminus::memory::reference> cminus::adapter::string::call_(cminus::logic::runtime &runtime, const call_info &info){
	auto string_type = dynamic_cast<cminus::type::string *>(info.context->get_type().get());
	auto function = string_type->find(runtime, cminus::logic::storage::object::search_options{ nullptr, info.context, info.name, false });

	auto callable = dynamic_cast<cminus::memory::function_reference *>(function->get_non_raw());
	return callable->get_value()->call(runtime, callable->get_context(), info.args);
}
