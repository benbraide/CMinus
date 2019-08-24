#include "runtime.h"

cminus::logic::attributes::object::object(const std::string &name, naming::parent *parent)
	: single(name, parent){}

bool cminus::logic::attributes::object::is_required_on_ref_destination(logic::runtime &runtime) const{
	return false;
}

bool cminus::logic::attributes::object::is_included_in_comparison(logic::runtime &runtime) const{
	return false;
}

void cminus::logic::attributes::object::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	return call_(runtime, stage, target, args);
}

void cminus::logic::attributes::object::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const{
	return call_(runtime, stage, target, std::vector<std::shared_ptr<memory::reference>>{});
}

cminus::logic::attributes::object::~object() = default;

cminus::logic::attributes::bound_object::bound_object(std::shared_ptr<object> target, const std::vector<std::shared_ptr<memory::reference>> &args)
	: object(target->get_naming_value(), target->get_naming_parent()), target_(target), args_(args){}

cminus::logic::attributes::bound_object::bound_object(std::shared_ptr<object> target, std::vector<std::shared_ptr<memory::reference>> &&args)
	: object(target->get_naming_value(), target->get_naming_parent()), target_(target), args_(std::move(args)){}

cminus::logic::attributes::bound_object::~bound_object() = default;

bool cminus::logic::attributes::bound_object::handles_stage(logic::runtime &runtime, stage_type value) const{
	return target_->handles_stage(runtime, value);
}

bool cminus::logic::attributes::bound_object::is_required_on_ref_destination(logic::runtime &runtime) const{
	return target_->is_required_on_ref_destination(runtime);
}

bool cminus::logic::attributes::bound_object::is_included_in_comparison(logic::runtime &runtime) const{
	return target_->is_included_in_comparison(runtime);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::attributes::bound_object::get_target() const{
	return target_;
}

const std::vector<std::shared_ptr<cminus::memory::reference>> &cminus::logic::attributes::bound_object::get_args() const{
	return args_;
}

void cminus::logic::attributes::bound_object::call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (args.empty())
		return target_->call(runtime, stage, target, args_);

	std::vector<std::shared_ptr<memory::reference>> combined_args;
	combined_args.reserve(args_.size() + args.size());

	for (auto arg : args_)
		combined_args.push_back(arg);

	for (auto arg : args)
		combined_args.push_back(arg);

	return target_->call(runtime, stage, target, combined_args);
}

cminus::logic::attributes::external::external(const std::string &name)
	: object(name, nullptr){}

cminus::logic::attributes::external::~external() = default;

cminus::logic::attributes::read_only::read_only()
	: external("ReadOnly"){}

cminus::logic::attributes::read_only::~read_only() = default;

bool cminus::logic::attributes::read_only::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_write);
}

bool cminus::logic::attributes::read_only::is_required_on_ref_destination(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::read_only::is_included_in_comparison(logic::runtime &runtime) const{
	return true;
}

void cminus::logic::attributes::read_only::call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (stage != stage_type::before_write)
		return;
	
	if (args.size() == 1u/* && args[0].IsStringType()*/)
		throw logic::exception(""/*args[0].ReadString()*/, 0u, 0u);
	else//Default message
		throw logic::exception("Cannot write to object. Object is read-only!", 0u, 0u);
}

cminus::logic::attributes::write_only::write_only()
	: external("WriteOnly"){}

cminus::logic::attributes::write_only::~write_only() = default;

bool cminus::logic::attributes::write_only::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_read);
}

bool cminus::logic::attributes::write_only::is_required_on_ref_destination(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::write_only::is_included_in_comparison(logic::runtime &runtime) const{
	return true;
}

void cminus::logic::attributes::write_only::call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (stage != stage_type::before_read)
		return;

	if (args.size() == 1u/* && args[0].IsStringType()*/)
		throw logic::exception(""/*args[0].ReadString()*/, 0u, 0u);
	else//Default message
		throw logic::exception("Cannot read to object. Object is write-only!", 0u, 0u);
}

cminus::logic::attributes::not_null::not_null()
	: external("NotNull"){}

cminus::logic::attributes::not_null::~not_null() = default;

bool cminus::logic::attributes::not_null::handles_stage(logic::runtime &runtime, stage_type value) const{
	return (value == stage_type::before_write);
}

bool cminus::logic::attributes::not_null::is_required_on_ref_destination(logic::runtime &runtime) const{
	return true;
}

bool cminus::logic::attributes::not_null::is_included_in_comparison(logic::runtime &runtime) const{
	return true;
}

void cminus::logic::attributes::not_null::call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (stage != stage_type::before_write || args.size() < 1u/* || !target->get_type()->IsPointer() || !args[0]->IsNull*/)
		return;

	if (args.size() == 2u/* && args[1].IsStringType()*/)
		throw logic::exception(""/*args[1].ReadString()*/, 0u, 0u);
	else//Default message
		throw logic::exception("Cannot write null value to object!", 0u, 0u);
}
