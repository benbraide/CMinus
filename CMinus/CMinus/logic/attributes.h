#pragma once

#include "naming.h"

namespace cminus::memory{
	class reference;
}

namespace cminus::logic::attributes{
	class object : public naming::single{
	public:
		enum class stage_type{
			nil,
			before_lookup,
			after_lookup,
			before_read,
			after_read,
			before_write,
			after_write,
			before_delete,
			after_delete,
			before_inheritance,
			after_inheritance,
		};

		explicit object(const std::string &name, naming::parent *parent);

		virtual ~object();

		virtual bool handles_stage(stage_type value) const = 0;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const = 0;
	};

	class external : public object{
	public:
		explicit external(const std::string &name);

		virtual ~external();
	};

	class read_only : public external{
	public:
		explicit read_only();

		virtual ~read_only();

		virtual bool handles_stage(stage_type value) const override;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;
	};

	class write_only : public external{
	public:
		explicit write_only();

		virtual ~write_only();

		virtual bool handles_stage(stage_type value) const override;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;
	};

	class not_null : public external{
	public:
		explicit not_null();

		virtual ~not_null();

		virtual bool handles_stage(stage_type value) const override;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;
	};
}
