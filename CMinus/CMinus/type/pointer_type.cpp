#include "pointer_type.h"
#include "function_type.h"
#include "primitive_type.h"

cminus::type::pointer::pointer(std::shared_ptr<object> base_type)
	: base_type_(base_type){}

cminus::type::pointer::~pointer() = default;

void cminus::type::pointer::print(logic::runtime &runtime, bool is_qualified) const{
	base_type_->print(runtime, is_qualified);
	runtime.writer.write_buffer(" *", 2u);
}

std::size_t cminus::type::pointer::get_size() const{
	return sizeof(void *);
}

std::size_t cminus::type::pointer::compute_base_offset(const object &target) const{
	return 0u;
}

cminus::type::object::score_result_type cminus::type::pointer::get_score(logic::runtime &runtime, const object &target, bool is_ref) const{
	auto type_target = dynamic_cast<const pointer *>(&target);
	if (type_target == nullptr){
		if (auto primitive_target_type = dynamic_cast<const primitive *>(&target); primitive_target_type != nullptr && primitive_target_type->get_id() == primitive::id_type::nullptr_)
			return score_result_type::assignable;
		return score_result_type::nil;
	}

	if (auto primitive_base_type = dynamic_cast<primitive *>(base_type_.get()); primitive_base_type != nullptr && primitive_base_type->get_id() == primitive::id_type::void_)
		return score_result_type::assignable;

	switch (base_type_->get_score(runtime, *type_target->base_type_, is_ref)){
	case score_result_type::exact:
		return score_result_type::exact;
	case score_result_type::assignable:
		return score_result_type::assignable;
	case score_result_type::class_compatible:
		return score_result_type::class_compatible;
	default:
		break;
	}

	return score_result_type::nil;
}

std::shared_ptr<cminus::memory::reference> cminus::type::pointer::get_default_value(logic::runtime &runtime) const{
	return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nullptr_);
}

std::shared_ptr<cminus::memory::reference> cminus::type::pointer::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, cast_type type) const{
	if (type != cast_type::static_ && type != cast_type::reinterpret)
		return nullptr;//Not supported

	auto pointer_target_type = dynamic_cast<const pointer *>(target_type.get());
	if (pointer_target_type != nullptr){//Check for integer
		if (type != cast_type::reinterpret)
			return std::make_shared<cminus::memory::reference_with_value<unsigned __int64>>(target_type, nullptr, data->read_scalar<unsigned __int64>(runtime));

		auto score = pointer_target_type->base_type_->get_score(runtime, *base_type_, false);
		if (score == score_result_type::ancestor){//Cast to base type pointer
			auto address = (data->read_scalar<unsigned __int64>(runtime) + base_type_->compute_base_offset(*pointer_target_type->base_type_));
			return std::make_shared<cminus::memory::reference_with_value<unsigned __int64>>(target_type, nullptr, address);
		}

		auto primitive_target_base_target_type = dynamic_cast<const primitive *>(pointer_target_type->base_type_.get());
		if (primitive_target_base_target_type != nullptr && primitive_target_base_target_type->get_id() == primitive::id_type::void_)
			return std::make_shared<cminus::memory::reference_with_value<unsigned __int64>>(target_type, nullptr, data->read_scalar<unsigned __int64>(runtime));

		auto primitive_base_target_type = dynamic_cast<const primitive *>(base_type_.get());
		if (primitive_base_target_type != nullptr && primitive_base_target_type->get_id() == primitive::id_type::void_)
			return std::make_shared<cminus::memory::reference_with_value<unsigned __int64>>(target_type, nullptr, data->read_scalar<unsigned __int64>(runtime));

		return nullptr;
	}
	
	if (type != cast_type::reinterpret)
		return nullptr;
	
	auto primitive_target_type = dynamic_cast<const primitive *>(target_type.get());
	if (primitive_target_type == nullptr)
		return nullptr;

	switch (primitive_target_type->get_id()){
	case primitive::id_type::int8_:
		return std::make_shared<cminus::memory::reference_with_value<__int8>>(target_type, nullptr, static_cast<__int8>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint8_:
		return std::make_shared<cminus::memory::reference_with_value<unsigned __int8>>(target_type, nullptr, static_cast<unsigned __int8>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int16_:
		return std::make_shared<cminus::memory::reference_with_value<__int16>>(target_type, nullptr, static_cast<__int16>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint16_:
		return std::make_shared<cminus::memory::reference_with_value<unsigned __int16>>(target_type, nullptr, static_cast<unsigned __int16>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int32_:
		return std::make_shared<cminus::memory::reference_with_value<__int32>>(target_type, nullptr, static_cast<__int32>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint32_:
		return std::make_shared<cminus::memory::reference_with_value<unsigned __int32>>(target_type, nullptr, static_cast<unsigned __int32>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int64_:
		return std::make_shared<cminus::memory::reference_with_value<__int64>>(target_type, nullptr, static_cast<__int64>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint64_:
		return std::make_shared<cminus::memory::reference_with_value<unsigned __int64>>(target_type, nullptr, data->read_scalar<unsigned __int64>(runtime));
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<cminus::evaluator::object> cminus::type::pointer::get_evaluator(logic::runtime &runtime) const{
	return nullptr;
}

std::shared_ptr<cminus::type::object> cminus::type::pointer::get_base_type() const{
	return base_type_;
}
