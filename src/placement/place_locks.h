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
    absolute_minimum, small_locks
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
   * @param to_place a vector of the results in the same order as the cost function
   * @return void
   */
  bool find_locks(const synthesis::lock_symbols& locks_to_place, const std::vector<cost_type>& cost_function, std::vector<placement_result>& to_place);
private:
  
  void result_to_locklist(const std::vector<std::vector<z3::expr>>& result, std::vector<std::pair<unsigned, abstraction::location >>& locks);
  bool minsmt(z3::optimize& slv, const synthesis::lock_symbols& locks_to_place, cost_type cost_function, placement_result& result);
  void remove_duplicates(std::vector<single_placement>& locks);

  z3::context ctx;  
  z3::expr ztrue = ctx.bool_val(true);
  z3::expr zfalse = ctx.bool_val(false);
  const std::vector<const cfg::abstract_cfg*>& threads;
  std::vector<std::vector<z3::expr>> location_vector;
  std::unordered_map<z3::expr,abstraction::location> location_map;
  z3::sort locations = z3::sort(ctx);
  
  z3::sort locks = ctx.int_sort();
  z3::func_decl lock_b = z3::func_decl(ctx); // lock before
  z3::func_decl unlock_b = z3::func_decl(ctx); // unlock before
  z3::func_decl lock_a = z3::func_decl(ctx); // lock after
  z3::func_decl unlock_a = z3::func_decl(ctx); // lock before
    
  z3::func_decl inl = z3::func_decl(ctx);
  
  z3::expr inle(const z3::expr& x, const z3::expr& l); // lock status after the instruction executed
  
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
    locking_constraints(z3::context& ctx) : ztrue(ctx.bool_val(true)) {}
  };
  
  // functions that return the constraints for the locks (both bounds inclusive)
  void lock_consistancy(locking_constraints& lc, unsigned from_lock, unsigned to_lock);
  void lock_sameinstr(locking_constraints& lc, unsigned from_lock, unsigned to_lock);
  void lock_order(locking_constraints& lc, unsigned lock, z3::expr location, z3::expr predecessor);
  void lock_order(locking_constraints& lc, unsigned lock, z3::expr location);
  
  /**
   * @brief calculates the places to that need to be locked together
   * 
   * @param locks_to_place These places need to be covered by the same lock
   * @return a pair of an expression representing the lock_id for this lock and an expression making sure all locations of the lock are locked together
   */
  std::vector<std::pair<z3::expr,z3::expr>> locked_together(const synthesis::lock_symbols& locks_to_place);
  lock_statistics get_statistics(std::vector<single_placement>& to_lock, z3::model& model);
  
  /**
   * @brief Add constraints for the locks from_lock to to_lock to the solver (can be used to increase the number of locks )
   * 
   * These are the hard constraints only
   */
  void add_constraints(z3::optimize& slv, placement::cost_type cost_function, unsigned int from_lock, unsigned int to_lock);
  
  
  /**
   * @brief This function needs to be called every time we increase the number of locks
   * 
   */
  void cost_model(z3::optimize& slv, cost_type cost_function, unsigned from_lock, unsigned to_lock);
  
  /**
   * @brief The cost function for the locks
   * 
   * This function is called only once and is independent of the number of locks
   * 
   * @param locks_to_place The locking information
   * @param lock_ids The expressions representing the ids assigned to the locks
   * @return void
   */
  void cost_model_locks(z3::optimize& slv, cost_type cost_function, const synthesis::lock_symbols& locks_to_place, const std::vector<z3::expr> lock_ids);
};
}

#endif // PLACEMENT_PLACE_LOCKS_H
