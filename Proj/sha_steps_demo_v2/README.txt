SHA_Demo

Subdirectory demo for a solution where BitFlow and optionally CLI11 are already available through FetchContent.

CMake:
    add_subdirectory(SHA_Demo)

Notes:
- This demo intentionally does not expose a factorize mode yet.
- Current BitFlow factorize/distribute rules can oscillate on forms like:
      a&b ^ a&c  <->  a&(b^c)
- The demo therefore keeps to normalize + simplify (+ SHA simplify).

Example:
    ShaStepsDemo --steps 2
    ShaStepsDemo --steps 4 --console-max 240
    ShaStepsDemo --steps 4 --out sha_out

Output:
- concise console output
- full expressions dumped to files per step:
    step_00/w.txt
    step_00/t1.txt
    step_00/t2.txt
    step_00/a_next.txt
    step_00/e_next.txt
    step_00/summary.txt
