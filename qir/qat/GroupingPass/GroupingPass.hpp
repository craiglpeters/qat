#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "GroupingPass/GroupingPassConfiguration.hpp"
#include "Logging/ILogger.hpp"
#include "Profile/Profile.hpp"
#include "QatTypes/QatTypes.hpp"

#include "Llvm/Llvm.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace microsoft
{
namespace quantum
{

    ///
    /// ```
    /// ┌─────────────────────────────────┐│┌──────────────────────────────────┐
    /// │         No measurements         │││        With measurements         │
    /// └─────────────────────────────────┘│└──────────────────────────────────┘
    ///                                    │
    ///      Before             After      │       Before            After
    /// ─────────────────┬─────────────────┼──────────────────┬─────────────────
    ///                                    │
    ///                          [entry]   │                          [entry]
    ///                  │                 │                  │
    ///                                    │                    ┌───
    ///      [entry]                       │       [entry]      │
    ///                  │ ┌───            │                  │ │  [bb head cc]
    ///                    │               │                    │
    ///                    │     [bb cc]   │                    │
    ///                  │ │               │                  │ │     [bb qc]
    ///   [mixed qc/cc]  ──▶               │  [mixed qc/mc/cc]──▶
    ///                    │               │                    │
    ///                  │ │     [bb qc]   │                  │ │     [bb mc]
    ///                    │               │                    │
    ///                    └───            │                    │
    ///       [bb2]      │                 │        [bb2]     │ │  [bb tail cc]
    ///                                    │                    │
    ///                           [bb2]    │                    └───
    ///                  │                 │                  │
    ///                                    │                           [bb2]
    ///                                    │
    /// ```
    ///

    class GroupingPass : public llvm::PassInfoMixin<GroupingPass>
    {
      public:
        using Instruction   = llvm::Instruction;
        using Value         = llvm::Value;
        using ILoggerPtr    = std::shared_ptr<ILogger>;
        using BlockSet      = std::unordered_set<llvm::BasicBlock*>;
        using SharedBuilder = std::shared_ptr<llvm::IRBuilder<>>;
        // Construction and destruction configuration.
        //

        explicit GroupingPass(GroupingPassConfiguration const& cfg)
          : config_{cfg}
        //    , logger_{logger}
        {
        }

        /// Copy construction is banned.
        GroupingPass(GroupingPass const&) = delete;

        /// We allow move semantics.
        GroupingPass(GroupingPass&&) = default;

        /// Default destruction.
        ~GroupingPass() = default;
        void prepareBlockModification(llvm::Module& module, llvm::BasicBlock* block);
        void nextQuantumCycle(llvm::Module& module, llvm::BasicBlock* block);
        void expandBlockQuantumMeasureClassical(llvm::Module& module, llvm::BasicBlock* block);

        llvm::PreservedAnalyses run(llvm::Module& module, llvm::ModuleAnalysisManager& mam);

        void runBlockAnalysis(llvm::Module& module);

        /// Whether or not this pass is required to run.
        static bool isRequired();

      private:
        GroupingPassConfiguration config_{};

        // Basic blocks used to build

        llvm::BasicBlock* post_classical_block_{nullptr};
        llvm::BasicBlock* readout_block_{nullptr};
        llvm::BasicBlock* quantum_block_{nullptr};
        llvm::BasicBlock* pre_classical_block_{nullptr};

        // Builders
        //

        SharedBuilder pre_classical_builder_{};
        SharedBuilder quantum_builder_{};
        SharedBuilder readout_builder_{};
        SharedBuilder post_classical_builder_{};

        BlockSet   visited_blocks_;
        ILoggerPtr logger_{nullptr};
    };

} // namespace quantum
} // namespace microsoft