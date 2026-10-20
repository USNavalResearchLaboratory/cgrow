# Basic C++ fitting example

This is the smallest console workflow for fitting Hartman–Schijve parameters:

1. Include `cgrow.hpp`.
2. Store measurements as `test_data_t<double>` (`R` plus `DeltaK`/`dadN` points).
3. Provide lower and upper bounds in the order `D`, `p`, `DeltaK_thr`, `A`.
4. Call `Hartman_Schijve::fit` with an atomic stop flag.
5. Evaluate the returned parameters with `objective_function`.

From the repository root:

```powershell
cmake -S . -B build
cmake --build build --config Debug --target cgrow_basic_fit
ctest --test-dir build -C Debug -R cgrow_basic_fit --output-on-failure
```

The example uses synthetic data so it is self-contained. Replace the point-building loop in
`main.cpp` with your file or database loader when integrating CGROW into another application.
