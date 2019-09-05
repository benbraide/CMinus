#include "pointer_type.h"
#include "primitive_type.h"
#include "class_type.h"

cminus::type::raw_pointer::raw_pointer(object *raw_base_type)
	: raw_base_type_(raw_base_type){}

cminus::type::raw_pointer::~raw_pointer() = default;

bool cminus::type::raw_pointer::converts_auto(const object &target) const{
	if (object::converts_auto(target))
		return true;

	auto pointer_target = dynamic_cast<const raw_pointer *>(&target);
	return (pointer_target != nullptr && raw_base_type_->converts_auto(*pointer_target->raw_base_type_));
}

void cminus::type::raw_pointer::print(logic::runtime &runtime, bool is_qualified) const{
	raw_base_type_->print(runtime, is_qualified);
	runtime.writer.write_buffer(" *", 2u);
}

void cminus::type::raw_pointer::print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const{
	if (auto target_address = data->read_scalar<unsigned __int64>(runtime); target_address == 0u)
		runtime.writer.write_buffer("nullptr", 7u);
	else//Not null
		runtime.writer.write_scalar(logic::to_hex_string<unsigned __int64>::get(target_address));
}

std::size_t cminus::type::raw_pointer::get_size() const{
	return sizeof(void *);
}

bool cminus::type::raw_pointer::is_exact(logic::runtime &runtime, const type::object &target) const{
	auto type_target = dynamic_cast<const raw_pointer *>(&target);
	return (type_target != nullptr && raw_base_type_->is_exact(runtime, *type_target->raw_base_type_));
}

cminus::type::object::score_result_type cminus::type::raw_pointer::get_score(logic::runtime &runtime, const object &target, bool is_ref) const{
	if (converts_auto(target))
		return score_result_type::assignable;

	auto type_target = dynamic_cast<const raw_pointer *>(&target);
	if (type_target == nullptr)
		return score_result_type::nil;

	if (auto primitive_base_type = dynamic_cast<primitive *>(type_target->raw_base_type_); primitive_base_type != nullptr && primitive_base_type->get_id() == primitive::id_type::void_)
		return score_result_type::assignable;

	switch (raw_base_type_->get_score(runtime, *type_target->raw_base_type_, is_ref)){
	case score_result_type::exact:
		return score_result_type::exact;
	case score_result_type::assignable:
	case score_result_type::ancestor:
		return score_result_type::assignable;
	case score_result_type::class_compatible:
		return score_result_type::class_compatible;
	default:
		break;
	}

	return score_result_type::nil;
}

std::shared_ptr<cminus::memory::reference> cminus::type::raw_pointer::get_default_value(logic::runtime &runtime) const{
	return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nullptr_);
}

std::shared_ptr<cminus::memory::reference> cminus::type::raw_pointer::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, cast_type type) const{
	if (type != cast_type::static_ && type != cast_type::ref_static && type != cast_type::rval_static && type != cast_type::reinterpret)
		return nullptr;//Not supported

	auto pointer_target_type = dynamic_cast<const raw_pointer *>(target_type.get());
	if (pointer_target_type != nullptr){
		if (type == cast_type::reinterpret)
			return runtime.global_storage->create_scalar(data->read_scalar<unsigned __int64>(runtime));

		if (type != cast_type::static_ && type != cast_type::rval_static)
			return nullptr;

		if (raw_base_type_->is_exact(runtime, *pointer_target_type->raw_base_type_)){//Same type
			if (type != cast_type::static_)
				return data;

			return runtime.global_storage->create_scalar(data->read_scalar<unsigned __int64>(runtime));
		}

		if (auto base_class_type = dynamic_cast<class_ *>(raw_base_type_); base_class_type != nullptr && base_class_type->is_ancestor(*pointer_target_type->raw_base_type_)){//Cast to base type pointer
			auto address = (data->read_scalar<unsigned __int64>(runtime) + raw_base_type_->compute_base_offset(*pointer_target_type->raw_base_type_));
			return runtime.global_storage->create_scalar(address);
		}

		if (type != cast_type::static_)
			return nullptr;

		auto primitive_target_base_target_type = dynamic_cast<const primitive *>(pointer_target_type->raw_base_type_);
		if (primitive_target_base_target_type != nullptr && primitive_target_base_target_type->get_id() == primitive::id_type::void_)
			return runtime.global_storage->create_scalar(data->read_scalar<unsigned __int64>(runtime));

		auto primitive_base_target_type = dynamic_cast<const primitive *>(raw_base_type_);
		if (primitive_base_target_type != nullptr && primitive_base_target_type->get_id() == primitive::id_type::void_)
			return runtime.global_storage->create_scalar(data->read_scalar<unsigned __int64>(runtime));

		return nullptr;
	}

	if (type != cast_type::reinterpret)
		return nullptr;
	
	auto primitive_target_type = dynamic_cast<const primitive *>(target_type.get());
	if (primitive_target_type == nullptr)
		return nullptr;

	switch (primitive_target_type->get_id()){
	case primitive::id_type::int8_:
		return runtime.global_storage->create_scalar(static_cast<__int8>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint8_:
		return runtime.global_storage->create_scalar(static_cast<unsigned __int8>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int16_:
		return runtime.global_storage->create_scalar(static_cast<__int16>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint16_:
		return runtime.global_storage->create_scalar(static_cast<unsigned __int16>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int32_:
		return runtime.global_storage->create_scalar(static_cast<__int32>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint32_:
		return runtime.global_storage->create_scalar(static_cast<unsigned __int32>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int64_:
		return runtime.global_storage->create_scalar(static_cast<__int64>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint64_:
		return runtime.global_storage->create_scalar(data->read_scalar<unsigned __int64>(runtime));
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<cminus::evaluator::object> cminus::type::raw_pointer::get_evaluator(logic::runtime &runtime) const{
	return runtime.global_storage->get_evaluator(evaluator::id::pointer);
}

cminus::type::object *cminus::type::raw_pointer::get_raw_base_type() const{
	return raw_base_type_;
}

cminus::type::pointer::pointer(std::shared_ptr<object> base_type)
	: raw_pointer(base_type.get()), base_type_(base_type){}

cminus::type::pointer::~pointer() = default;

std::shared_ptr<cminus::type::object> cminus::type::pointer::get_base_type() const{
	return base_type_;
}
