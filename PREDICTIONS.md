# Predictions recorded before execution

The synthetic policy is age18..65 and90days since last donation; it is a programming fixture, not clinical eligibility guidance. The fixed reporting day is11Nov2026. A1000-donor district baseline grows100times to100000 donors distributed across25hospitals.

B1: digital root9875 ->29 ->11 ->2. power(2,30)=1073741824; linear31calls, halving6. For exponents10,100,1000,10000 with base1: linear11,101,1001,10001; halving5,8,11,15. Duplicating two child calls for exponent30 gives63calls. Retaining the unused original half call as well gives364calls. A stack-constrained, optimization-disabled child should eventually fail for deep linear recursion; its exact failure point is platform-dependent. Iteration should remain constant-stack.

A4/B4: adding5 records gives capacities2,2,4,4,8. At100000 records doubling ends at131072capacity,17growths,131070record-copy upper bound,31072unused slots. Plus10ends at100000capacity,10000growths and499950000potential copied records. The allocator may extend in place, so these are logical copy bounds rather than measured physical moves.

B3: text and raw binary round trips must preserve all fields. Binary size is n*sizeof(Donor); text size depends on field lengths. Record50000 is zero-based index49999. In-place binary update touches one record, while text rewrite processes the whole file. No exact timing ordering is promised before measurement.

B4: generated record i has age18+i%48, last donation i%180days ago and hospital1+i%25. Eligible by day+7 means i%180>=83. For100000 records,555complete180-record blocks contribute53835 and the remaining100 contribute17: expected53852 donors. Each of25hospitals has4000records. All donor allocations must be freed at exit.
