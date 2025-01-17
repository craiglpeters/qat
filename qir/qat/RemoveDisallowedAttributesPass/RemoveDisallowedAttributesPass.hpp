#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "qir/qat/Llvm/Llvm.hpp"
#include "qir/qat/QatTypes/QatTypes.hpp"

#include <functional>
#include <unordered_set>
#include <vector>

namespace microsoft::quantum
{

class RemoveDisallowedAttributesPass : public llvm::PassInfoMixin<RemoveDisallowedAttributesPass>
{
  public:
    RemoveDisallowedAttributesPass()
      : allowed_attrs_{
            static_cast<String>("EntryPoint"), static_cast<String>("InteropFriendly"),
            static_cast<String>("requiredQubits"), static_cast<String>("requiredResults"),
            static_cast<String>("replaceWith")
            /*
            static_cast<String>("nofree")
            static_cast<String>("nosync")
            static_cast<String>("nounwind")
            static_cast<String>("readnone")
            static_cast<String>("speculatable")
            static_cast<String>("willreturn")
            */
        }
    {
    }

    llvm::PreservedAnalyses run(llvm::Module& module, llvm::ModuleAnalysisManager& /*mam*/)
    {
        for (auto& fnc : module)
        {
            std::unordered_map<String, String> to_keep;
            auto                               name = static_cast<std::string>(fnc.getName());

            // Skipping any LLVM function
            if (fnc.isIntrinsic())
            {
                continue;
            }

            // Finding all valid attributes
            for (auto& attrset : fnc.getAttributes())
            {
                for (auto& attr : attrset)
                {
                    auto        r = static_cast<String>(attr.getAsString());
                    std::string value{};

                    auto p = r.find('=');
                    if (p != std::string::npos)
                    {
                        value = r.substr(p + 1, r.size() - p - 1);
                        r     = r.substr(0, p);
                    }

                    // Stripping quotes
                    if (r.size() >= 2 && r[0] == '"' && r[r.size() - 1] == '"')
                    {
                        r = r.substr(1, r.size() - 2);
                    }

                    if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"')
                    {
                        value = value.substr(1, value.size() - 2);
                    }

                    // Inserting if allowed
                    if (allowed_attrs_.find(r) != allowed_attrs_.end())
                    {
                        to_keep.insert(std::make_pair(r, value));
                    }
                }
            }

            // Deleting every
            fnc.setAttributes({});
            for (auto& attr : to_keep)
            {
                if (attr.second.empty())
                {
                    fnc.addFnAttr(attr.first);
                }
                else
                {
                    fnc.addFnAttr(attr.first, attr.second);
                }
            }

            // Updating all users attributes
            for (auto user : fnc.users())
            {
                auto call = llvm::dyn_cast<llvm::CallInst>(user);
                if (call != nullptr)
                {
                    call->setAttributes(fnc.getAttributes());
                }
            }
        }

        return llvm::PreservedAnalyses::none();
    }

  private:
    std::unordered_set<String> allowed_attrs_;
};

} // namespace microsoft::quantum
