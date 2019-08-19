#include "attributes.h"

cminus::logic::attributes::object::object(const std::string &name)
	: single(name, nullptr){}

cminus::logic::attributes::object::~object() = default;

cminus::logic::attributes::external::external(const std::string &name)
	: object(name){}

cminus::logic::attributes::external::~external() = default;

cminus::logic::attributes::read_only::read_only()
	: external("ReadOnly"){}

cminus::logic::attributes::read_only::~read_only() = default;

bool cminus::logic::attributes::read_only::handles_stage(stage_type value) const{
	return (value == stage_type::before_write);
}

void cminus::logic::attributes::read_only::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
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

bool cminus::logic::attributes::write_only::handles_stage(stage_type value) const{
	return (value == stage_type::before_read);
}

void cminus::logic::attributes::write_only::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
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

bool cminus::logic::attributes::not_null::handles_stage(stage_type value) const{
	return (value == stage_type::before_write);
}

void cminus::logic::attributes::not_null::call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (stage != stage_type::before_write || args.size() < 1u/* || !target->get_type()->IsPointer() || !args[0]->IsNull*/)
		return;

	if (args.size() == 2u/* && args[1].IsStringType()*/)
		throw logic::exception(""/*args[1].ReadString()*/, 0u, 0u);
	else//Default message
		throw logic::exception("Cannot write null value to object!", 0u, 0u);
}
