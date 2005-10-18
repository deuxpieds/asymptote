/*****
 * env.h
 * Andy Hammerlindl 2002/6/20
 *
 * Keeps track of the namespaces of variables and types when traversing
 * the abstract syntax.
 *****/

#ifndef ENV_H
#define ENV_H

#include <list>
#include <map>
#include <stack>

#include "errormsg.h"
#include "entry.h"
#include "builtin.h"
#include "types.h"
#include "util.h"

namespace types {
class record;
}

namespace trans {

using std::list;

using sym::symbol;
using types::ty;
using types::function;
using types::record;

class genv;

// Keeps track of the name bindings of variables and types.  This is used for
// the fields of a record, whereas the derived class env is used for unqualified
// names in translation.
class protoenv {
protected:
  // These tables keep track of type and variable definitions.
  tenv te;
  venv ve;

  access *baseLookupCast(ty *target, ty *source, symbol *name);

public:
  // Start an environment for a file-level module.
  protoenv() {}

  protoenv(const protoenv&);
  
  void beginScope()
  {
    te.beginScope(); ve.beginScope();
  }
  void endScope()
  {
    te.endScope(); ve.endScope();
  }

  tyEntry *lookupTyEntry(symbol *s)
  {
    return te.look(s);
  }

  ty *lookupType(symbol *s)
  {
    tyEntry *ent=lookupTyEntry(s);
    return ent ? ent->t : 0;
  }

  varEntry *lookupVarByType(symbol *name, ty *t)
  {
    // Search in local vars.
    return ve.lookByType(name, t);
  }

  access *lookupInitializer(ty *t)
  {
    // The initializer's type is a function returning the desired type.
    function *it=new function(t);
    varEntry *v=lookupVarByType(symbol::initsym,it);

    // If not in the environment, try the type itself.
    return v ? v->getLocation() : t->initializer();
  }

  // Find the function that handles casting between the types.
  // The name is "cast" for implicitCasting and "ecast" for explicit (for now).
  access *lookupCast(ty *target, ty *source, symbol *name);
  bool castable(ty *target, ty *source, symbol *name);

  // Given overloaded types, this resolves which types should be the target and
  // the source of the cast.
  ty *castTarget(ty *target, ty *source, symbol *name);
  ty *castSource(ty *target, ty *source, symbol *name);

  ty *varGetType(symbol *name)
  {
    return ve.getType(name);
  }

  void addType(symbol *name, tyEntry *desc)
  {
    te.enter(name, desc);
  }
  
  void addVar(symbol *name, varEntry *desc)
  {
    // Don't check for multiple variables, as this makes adding casts
    // and initializers harder.
    ve.enter(name, desc);
  }

  // Add another environment, say from a record.
  void add(protoenv &source, varEntry *qualifier)
  {
    te.add(source.te, qualifier);
    ve.add(source.ve, qualifier);
  }

  // Add the fields and types of the record given in v to the local environment.
  void useRecord(varEntry *v);

  void list()
  {
    ve.list();
  }

private: // Non-copyable
  void operator=(const protoenv&);
};

// Environment used in translating statements and expressions at all scopes.  As
// opposed to protoenv which is suitable for keeping track of the fields of
// records, this also keeps track of the global env, for loading modules.
class env : public protoenv {
  // The global environment - keeps track of modules.
  genv &ge;
public:
  // Start an environment for a file-level module.
  env(genv &ge);

  record *getModule(symbol *id, std::string filename);
};

} // namespace trans

#endif
