#pragma once

#include "function_declaration.h"

namespace cminus::declaration::string{
	class size : public function{
	public:
		explicit size(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~size();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class data : public function{
	public:
		explicit data(logic::runtime &runtime, bool read_only, logic::naming::parent *parent);

		virtual ~data();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class resize : public function{
	public:
		explicit resize(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~resize();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class clear : public function{
	public:
		explicit clear(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~clear();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};
}
