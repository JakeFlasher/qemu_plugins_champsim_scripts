/*
 * clueless --- Characterises vaLUEs Leaking as addrESSes
 * Copyright (C) 2023  Xiaoyue Chen
 *
 * This file is part of clueless.
 *
 * clueless is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * clueless is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with clueless.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROPAGATOR_H
#define PROPAGATOR_H

#include "hook.h"
#include "taint-allocator.h"
#include "taint-table.h"
#include <array>
#include <functional>
#include <vector>

namespace clueless
{

class propagator
{
public:
  struct instr
  {
    using reg_set = std::vector<unsigned char>;

    // Modify opcode enum to include branch taken/not taken
    enum class opcode
    {
      OP_REG,
      OP_LOAD,
      OP_STORE,
      OP_BRANCH_TAKEN,
      OP_BRANCH_NOT_TAKEN,
      OP_NOP,
    } op;

    unsigned long long ip;
    unsigned long long seq;

    reg_set src_reg;
    reg_set dst_reg;
    reg_set mem_reg;

    unsigned long long address;

    // Remove branch_taken field
    // bool branch_taken = false;

    // Helper function to combine opcode and branch info if needed
    int opcode_with_branch_info() const {
      return static_cast<int>(op);
    }
  };

  struct secret_exposed_hook_param
  {
    struct secret
    {
      unsigned long long secret_address;
      unsigned long long access_ip;
      size_t propagation_level;
    };

    std::vector<secret> exposed_secret;
    unsigned long long transmit_address, transmit_ip;
  };

  using secret_exposed_hook = hook<secret_exposed_hook_param>;

  void propagate (const instr &ins);

  void
  add_secret_exposed_hook (secret_exposed_hook::function f)
  {
    secret_exposed_hook_.add (f);
  }

private:
  void reg_to_reg (const instr &ins);
  void mem_to_reg (const instr &ins);
  void reg_to_mem (const instr &ins);
  void handle_mem_taint (const instr &ins);

  taint_set union_reg_taint_sets (const auto &reg_set) const;

  taint alloc_taint ();

  fifo_taint_allocator taint_allocator_;
  reg_taint_table reg_taint_ = {};
  taint_address_table taint_address_ = {};
  taint_address_table taint_ip_ = {};
  secret_exposed_hook secret_exposed_hook_ = {};
  using propagation_level_table
      = std::array<std::array<size_t, taint::N>, reg_taint_table::NREG>;
  propagation_level_table propagation_level_ = {};
  using taint_age_table
      = std::array<std::array<size_t, taint::N>, reg_taint_table::NREG>;
  taint_age_table taint_age_table_ = {};
};

}

#endif
