# Memory Management & Efficiency Lessons

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## Key Patterns
1. **Pass-by-Const-Reference**:
   Functions receiving `Target`, `Projectile`, or `ImpactScenario` structs accept them by `const reference` (`const ImpactScenario&`) to eliminate unnecessary copy operations.
2. **Stack Allocation for Numerical Integration**:
   Time-step arrays and state variables inside the simulation loop utilize stack allocations and reserve vector capacities in advance.
