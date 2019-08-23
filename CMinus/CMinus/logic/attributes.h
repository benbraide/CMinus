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

		object(const std::string &name, naming::parent *parent);

		virtual ~object();

		virtual bool handles_stage(stage_type value) const = 0;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const;

		virtual void call(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target) const;

	protected:
		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const = 0;
	};

	class bound_object : public object{
	public:
		bound_object(std::shared_ptr<object> target, const std::vector<std::shared_ptr<memory::reference>> &args);

		bound_object(std::shared_ptr<object> target, std::vector<std::shared_ptr<memory::reference>> &&args);

		virtual ~bound_object();

		virtual bool handles_stage(stage_type value) const override;

		virtual std::shared_ptr<object> get_target() const;

		virtual const std::vector<std::shared_ptr<memory::reference>> &get_args() const;

	protected:
		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;

		std::shared_ptr<object> target_;
		std::vector<std::shared_ptr<memory::reference>> args_;
	};

	class external : public object{
	public:
		explicit external(const std::string &name);

		virtual ~external();
	};

	class read_only : public external{
	public:
		read_only();

		virtual ~read_only();

		virtual bool handles_stage(stage_type value) const override;

	protected:
		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;
	};

	class write_only : public external{
	public:
		write_only();

		virtual ~write_only();

		virtual bool handles_stage(stage_type value) const override;

	protected:
		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;
	};

	class not_null : public external{
	public:
		not_null();

		virtual ~not_null();

		virtual bool handles_stage(stage_type value) const override;

	protected:
		virtual void call_(logic::runtime &runtime, stage_type stage, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args) const override;
	};
}
