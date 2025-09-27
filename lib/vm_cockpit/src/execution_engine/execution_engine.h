#pragma once

// ============================================================================
// PHASE 4.11.8: EXECUTIONENGINE_V2 ONLY - LEGACY EXECUTION ENGINE ELIMINATED
// ============================================================================
#include "execution_engine_v2.h"
// ExecutionEngine now always refers to ExecutionEngine_v2
// Legacy ExecutionEngine implementation has been completely removed
using ExecutionEngine = ExecutionEngine_v2;