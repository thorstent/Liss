/*
 * Copyright 2015, IST Austria
 *
 * This file is part of Liss.
 *
 * Liss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Liss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Liss.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLACEMENT_PLACE_LOCKS_H
#define PLACEMENT_PLACE_LOCKS_H

#include "cfg/program.h"
#include <z3++.h>
#include <vector>
#include "abstraction/location.h"
#include "types.h"
#include "synthesis/trace_helpers.h"

namespace placement {

  enum class lock_type {
    lock, unlock
  };
  
  enum class position_type {
    before, after
  };
  
  struct single_placement {
    unsigned lock;
    lock_type lock_t;
    position_type position;
    abstraction::location location;
    single_placement(unsigned lock, lock_type lock_t, position_type position, abstraction::location location) : lock(lock), lock_t(lock_t), 
    position(position), location(location) {}
  };
  
  struct lock_statistics {
    unsigned locks = 0; // total number of locks
    unsigned lock_instr = 0; // number of locking instructions
    unsigned unlock_instr = 0; // number of unlock instructions
    unsigned in_lock = 0; // abstract instructions inside a lock
  };
  
  std::ostream& operator<<(std::ostream& out, const lock_statistics& ls);
  
  struct placement_result {
    std::vector<single_placement> locks;
    lock_statistics statistics;
  };
  
  enum class cost_type {
    absolute_minimum, small_locks, coarse, unoptimized
  };
  
  std::ostream& operator<<(std::ostream& out, cost_type c);
  std::string short_name(cost_type c);
  
class place_locks
{
public:
  place_locks(const cfg::program& program);
  /**
   * @brief Determines the lock placement
   * 
   * @param locks_to_place List of locations that need to share the same lock
   * The innermost is a sequence of location that need to be locked without an unlock in between. The middle layer
   * are the locations that need to be locked by the same lock. There is a cnf over that
   * @param cost_function Strategies that should be synthesised
   * @param presult The resulting placement instructions (for printing)
   * @return void
   */
  bool find_locks(const synthesis::lock_symbols& locks_to_place, placement::cost_type cost_function, placement::placement_result& presult);
private:
  
  void remove_duplicates(std::vector<single_placement>& locks);

  z3::context ctx;  
  z3::expr ztrue = ctx.bool_val(true);
  z3::expr zfalse = ctx.bool_val(false);
  const std::vector<const cfg::abstract_cfg*>& threads;
  std::vector<std::vector<z3::expr>> location_vector;
  std::unordered_map<z3::expr,abstraction::location> location_map;
  z3::sort locations = z3::sort(ctx);
  
  
  // these instructions have to be locked the same way
  std::unordered_map<clang::SourceLocation,std::vector<z3::expr>> lock_before;
  std::unordered_map<clang::SourceLocation,std::vector<z3::expr>> lock_after;
  
  struct locking_constraints {
    z3::expr ztrue;
    z3::expr inl_def = ztrue;
    z3::expr cons_loc = ztrue; // what locations may be locked or unlocked respectively
    z3::expr cons_preemption = ztrue; // do not lock preemption points
    z3::expr cons_basic = ztrue; // basic rule that nothing can be locked and unlocked at the same time
    z3::expr cons_join = ztrue; // predecessors need the same locking on join
    z3::expr cons_unl = ztrue; // to be unlocked it needs to be locked first
    z3::expr cons_lo = ztrue; // to be locked it needs to be unlocked first
    z3::expr cons_functions = ztrue; // ensure functions start and end with the same locking configuration
    z3::expr cons_sameinstr = ztrue; // same instructions need to be in the same lock
    z3::expr cons_lockorder = ztrue; // make sure lock 1 is always taken after 0 etc
    z3::sort locks;
    z3::func_decl lock_b; // lock before
    z3::func_decl unlock_b; // unlock before
    z3::func_decl lock_a; // lock after
    z3::func_decl unlock_a; // lock before
    
    std::vector<z3::expr> lock_vector;
    std::unordered_map<z3::expr, unsigned> lock_map;
    std::vector<z3::expr> lock_ids; // for each clause in locks_to_place there is one id that describes what lock is used to lock it
    
    z3::expr inle(const z3::expr& x, const z3::expr& l); // lock status after the instruction executed
    
    z3::func_decl inl;
    locking_constraints(z3::context& ctx) : ztrue(ctx.bool_val(true)), locks(ctx), lock_b(ctx), unlock_b(ctx), lock_a(ctx), unlock_a(ctx), inl(ctx) {}
  };
  
  void result_to_locklist(locking_constraints& lc, const std::vector<std::vector<z3::expr>>& result, std::vector<std::pair<unsigned, abstraction::location >>& locks);
  
  
  // functions that return the constraints for the locks
  void init_locks(locking_constraints& lc, unsigned max_lock);
  void lock_consistancy(locking_constraints& lc);
  void lock_sameinstr(locking_constraints& lc);
  void lock_order(placement::place_locks::locking_constraints& lc, const z3::expr& lock, z3::expr location, z3::expr predecessor);
  void lock_order(placement::place_locks::locking_constraints& lc, const z3::expr& lock, z3::expr location);
  
  /**
   * @brief calculates the places to that need to be locked together and adds these constraints to the solver
   * 
   * @param locks_to_place These places need to be covered by the same lock
   */
  template<class T>
  void locked_together(T& slv, placement::place_locks::locking_constraints& lc, const synthesis::lock_symbols& locks_to_place);
  // void locked_together(z3::optimize& slv, placement::place_locks::locking_constraints& lc, const synthesis::lock_symbols& locks_to_place);
  lock_statistics get_statistics(std::vector< placement::single_placement >& to_lock, z3::model& model, const placement::place_locks::locking_constraints& lc);
  
  /**
   * @brief Add constraints for the locks from_lock to to_lock to the solver (can be used to increase the number of locks )
   * 
   * These are the hard constraints only
   */
  template<class T>
  void add_constraints(T& slv, locking_constraints& lc);
    
  /**
   * @brief Adds the soft constraints to the solver
   * 
   * This function is called only once and is independent of the number of locks
   * 
   * @param locks_to_place The locking information
   * @param lock_ids The expressions representing the ids assigned to the locks
   * @return void
   */
  void cost_model(z3::optimize& slv, locking_constraints& lc, cost_type cost_function, const synthesis::lock_symbols& locks_to_place);
};
}

#endif // PLACEMENT_PLACE_LOCKS_H
