# CGROW
CGROW is a C++17 library, test suite, and graphical application for identifying fatigue crack-growth model parameters from experimental data. It uses constrained, derivative-free optimization to automate parameter determination while retaining explicit parameter bounds and fit diagnostics.

## Hartman–Schijve fitting

CGROW fits the Hartman–Schijve crack-growth relation across one or more stress-ratio (`R`) datasets:

<p align="center">
  <img src="src/apps/cgrow/assets/hseqn.svg" width="240" alt="Hartman–Schijve crack-growth equation">
</p>

The model represents the threshold region through `ΔK_thr` and the high-growth asymptote through
`A (1 − R)`. CGROW identifies `D`, `p`, `ΔK_thr`, and `A` from the supplied measurements. It supports
both vertical log-space residuals and geometric distances to the model curve, making it useful when
uncertainty in both measured crack-growth rate and stress-intensity-factor range matters. Data and
parameter bounds remain the engineer's responsibility; fitted parameters should be assessed against
the experimental method and intended design application.

The implementation follows the approach proposed in [Iliopoulos *et al.*, *International Journal of Fatigue* 169 (2023), 107490](https://doi.org/10.1016/j.ijfatigue.2022.107490).

DISTRIBUTION STATEMENT A. Approved for public release; distribution is unlimited.

<div align="center">
  <img src="https://raw.githubusercontent.com/USNavalResearchLaboratory/cgrow/main/doc/images/main_interface.PNG" alt="CGROW Main GUI">
</div>

## Build

### Requirements

- CMake 3.15 or newer
- A C++17 compiler (MSVC, Clang, or GCC)
- Qt 6 with Widgets, PrintSupport, Svg, SvgWidgets, and Test components (Qt 5 is also supported by the application build)
- Internet access on the first configuration, so CMake can fetch the pinned NLopt and CXXPlot dependencies

Tell CMake where Qt is installed when it cannot find it automatically. For example, in a Visual Studio
developer PowerShell prompt:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/msvc2022_64"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

On Windows, the deployed GUI is at `build/src/apps/cgrow/bin/Release/cgrow.exe`. The CMake build
automatically runs `windeployqt` so the required Qt runtime files are placed beside the executable.
The self-contained console fitting example is built as `cgrow_basic_fit`; see
[examples/basic_fit](examples/basic_fit/README.md) for its small library workflow.

## How to cite

If CGROW contributes to published work, cite the method paper:

> Iliopoulos, A., Michopoulos, J. G., Jones, R., Kinloch, A. J., & Peng, D. (2023). A framework for automating the parameter determination of crack growth models. *International Journal of Fatigue, 169*, 107490. https://doi.org/10.1016/j.ijfatigue.2022.107490

```bibtex
@article{Iliopoulos2023CGROW,
  author  = {Iliopoulos, Athanasios and Michopoulos, John G. and Jones, Rhys and Kinloch, Anthony J. and Peng, Daren},
  title   = {A framework for automating the parameter determination of crack growth models},
  journal = {International Journal of Fatigue},
  volume  = {169},
  pages   = {107490},
  year    = {2023},
  doi     = {10.1016/j.ijfatigue.2022.107490}
}
```

The [license](license.txt) also requires published research using the software to acknowledge the NRL
CGROW computer package and its developers at the U.S. Naval Research Laboratory.

## License intent
CGROW is the work of federal employees and the relevant source code is in the public domain. Please read the [license](license.txt) to ensure you can satisfy the authorization to use and distribute requirements.

Third party libraries included with CGROW have their own respective licenses. Distribution of derivative work need to satisfy those licenses as well. If you plan to use CGROW in a non open-source manner, you need to make arrangements with the individual copyright holders. More information can be found in the [licenses](licenses) folder.

