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

#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

typedef int16_t state_id;
typedef int16_t reward_t;
const state_id no_state = 0;
const unsigned max_states = INT16_MAX;

const unsigned max_variables = UINT16_MAX;
const unsigned max_locks = 64;
const unsigned max_conditionals = 64; 

typedef uint16_t variable_type;
typedef uint8_t lock_type;
typedef uint8_t conditional_type;


typedef int8_t thread_id_type;
const unsigned max_threads = INT8_MAX;

#endif 