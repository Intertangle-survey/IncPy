/* The FuncMemoInfo 'class'
  
   IncPy: An auto-memoizing Python interpreter supporting incremental
   recomputation. Copyright 2009-2010 Philip J. Guo (pg@cs.stanford.edu)
   All rights reserved.
 
   This code carries the same license as the enclosing Python
   distribution: http://www.python.org/psf/license/ 

*/

#ifndef Py_MEMOIZE_FMI_H
#define Py_MEMOIZE_FMI_H
#ifdef __cplusplus
extern "C" {
#endif

#include "Python.h"

// Object that contains the memo table, dependencies, and profiling
// metadata for one function (only some fields will be serialized to disk)
typedef struct {
  // these fields below are serialized to disk
  // (for efficiency, these are NULL pointers if empty)

  // (we can't make this into a dict since not all argument values are
  // hashable as keys ... instead, we will make this a list of
  // dicts, where the keys are:
  //
  //   "args" --> argument list
  //   "global_vars_read" --> dict mapping global vars to values (OPTIONAL)
  //
  //   "files_read" --> dict mapping files read to modtimes (OPTIONAL)
  //   "files_written" --> dict mapping files written to modtimes (OPTIONAL)
  //
  //   "retval" --> return value, stored in a SINGLETON list
  //                (to facilitate mutation for COW optimization)
  //   "stdout_buf" --> buffered stdout string (OPTIONAL)
  //   "stderr_buf" --> buffered stderr string (OPTIONAL)
  //   "final_file_seek_pos" --> dict mapping filenames to their seek
  //                             positions at function exit time (OPTIONAL)
  //
  //   "runtime_ms" --> how many milliseconds it took to run
  //
  // to support lazy-loading, this field is serialized to disk as:
  //   incpy-cache/XXX.memoized_vals.pickle
  //
  // don't access this field directly ...
  //   instead use get_memoized_vals_lst()
  PyObject* memoized_vals;            // List

  /* 

    Key: cPickle.dumps([argument list])

    Value: a LIST of dicts with the following fields:

      "args" --> argument list
      "global_vars_read" --> dict mapping global vars to values (OPTIONAL)

      "files_read" --> dict mapping files read to modtimes (OPTIONAL)
      "files_written" --> dict mapping files written to modtimes (OPTIONAL)

      "retval" --> return value, stored in a SINGLETON list
                   (to facilitate mutation for COW optimization)
      "stdout_buf" --> buffered stdout string (OPTIONAL)
      "stderr_buf" --> buffered stderr string (OPTIONAL)
      "final_file_seek_pos" --> dict mapping filenames to their seek
                                positions at function exit time (OPTIONAL)

      "runtime_ms" --> how many milliseconds it took to run

    (the reason why this is a list rather than a single dict is because
    there could be MULTIPLE valid matches for a particular argument
    list, due to differing global variable values)

  */
  PyObject* memoized_vals_dict;       // Dict

  // all of these fields are serialized to disk as:
  //   incpy-cache/XXX.dependencies.pickle
  PyObject* code_dependencies;        // Dict


  // these fields below are NOT serialized to disk

  PyObject* f_code; // PyCodeObject that contains the function's
                    // canonical name as pg_canonical_name
                    // (see GET_CANONICAL_NAME macro below)

  // booleans
  char is_impure;    // is this function impure during THIS execution?
  char all_code_deps_SAT; // are all code dependencies satisfied during THIS execution?
  char memoized_vals_loaded; // have we attempted to load memoized_vals from disk yet?

  // should we not even bother memoizing this function? (but we still
  // need to track its dependencies) ... only relevant when
  // ENABLE_IGNORE_FUNC_THRESHOLD_OPTIMIZATION on
  char likely_nothing_to_memoize;

  // how many times has this function been executed and terminated
  // 'quickly' with the memoized_vals field as NULL?  only relevant when
  // ENABLE_IGNORE_FUNC_THRESHOLD_OPTIMIZATION is on
  unsigned int num_fast_calls_with_no_memoized_vals;

} FuncMemoInfo;

#define GET_CANONICAL_NAME(fmi) ((PyCodeObject*)fmi->f_code)->pg_canonical_name

#ifdef __cplusplus
}
#endif
#endif /* !Py_MEMOIZE_FMI_H */

