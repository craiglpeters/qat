// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Rules/Notation/Notation.hpp"
#include "Rules/Patterns/AnyPattern.hpp"
#include "Rules/Patterns/CallPattern.hpp"
#include "Rules/Patterns/Instruction.hpp"

#include "Llvm/Llvm.hpp"

#include <unordered_map>
#include <vector>

namespace microsoft::quantum
{
namespace notation
{

    using IOperandPrototypePtr = std::shared_ptr<IOperandPrototype>;

    IOperandPrototypePtr zExt(IOperandPrototypePtr const& value)
    {
        auto ret = std::make_shared<ZExtPattern>();

        ret->addChild(value);
        return static_cast<IOperandPrototypePtr>(ret);
    }

} // namespace notation
} // namespace microsoft::quantum