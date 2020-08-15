#define _USE_MATH_DEFINES
#include <cmath>

#include "builtincontext.h"
#include "builtin.h"
#include "expression.h"
#include "function.h"
#include "ModuleInstantiation.h"
#include "printutils.h"
#include "evalcontext.h"
#include "boost-utils.h"

BuiltinContext::BuiltinContext() : Context()
{
}

void BuiltinContext::init()
{
	for(const auto &assignment : Builtins::instance()->getAssignments()) {
		this->set_variable(assignment->getName(), assignment->getExpr()->evaluate(shared_from_this()));
	}

	this->set_constant("PI", ValuePtr(M_PI));
}

ValuePtr BuiltinContext::evaluate_function(const std::string &name, const std::shared_ptr<EvalContext>& evalctx) const
{
	const auto &search = Builtins::instance()->getFunctions().find(name);
	if (search != Builtins::instance()->getFunctions().end()) {
		AbstractFunction *f = search->second;
		if (f->is_enabled()) return f->evaluate((const_cast<BuiltinContext *>(this))->get_shared_ptr(), evalctx);
		else LOG(boostfs_uncomplete(evalctx->loc.filePath(),this->documentPath()).generic_string(),evalctx->loc.firstLine(),getFormatted("Experimental builtin function '%1$s' is not enabled",name),message_group::Warning);
	}
	return Context::evaluate_function(name, evalctx);
}

class AbstractNode *BuiltinContext::instantiate_module(const class ModuleInstantiation &inst, const std::shared_ptr<EvalContext>& evalctx) const
{
	const std::string &name = inst.name();
	const auto &search = Builtins::instance()->getModules().find(name);
	if (search != Builtins::instance()->getModules().end()) {
		AbstractModule *m = search->second;
		if (!m->is_enabled()) {
			LOG(boostfs_uncomplete(evalctx->loc.filePath(),this->documentPath()).generic_string(),evalctx->loc.firstLine(),getFormatted("Experimental builtin module '%1$s' is not enabled",name),message_group::Warning);
		}
		std::string replacement = Builtins::instance()->instance()->isDeprecated(name);
		if (!replacement.empty()) {
			PRINT_DEPRECATION("The %s() module will be removed in future releases. Use %s instead. %s", name % replacement % evalctx->loc.toRelativeString(this->documentPath()));
		}
		return m->instantiate((const_cast<BuiltinContext *>(this))->get_shared_ptr(), &inst, evalctx);
	}
	return Context::instantiate_module(inst, evalctx);
}

