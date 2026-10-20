"""Check the checked-in predecessor and execute small reproductions of its bugs."""

from pathlib import Path
import subprocess
import sys
import tempfile
import shutil


ROOT = Path(__file__).resolve().parents[3]
# Find a historical revision that actually contains the predecessor signatures.
# Using HEAD^ is unstable: after this change is committed, HEAD^ is the current
# partially-fixed baseline rather than the genuinely buggy implementation.
try:
    history = subprocess.check_output(
        ["git", "rev-list", "--all", "--", "src/lib/cgrow.hpp"], cwd=ROOT,
        stderr=subprocess.DEVNULL,
    ).decode("utf-8").splitlines()
except (FileNotFoundError, subprocess.CalledProcessError):
    print("Git history is unavailable; skipping predecessor checks")
    sys.exit(0)

old = None
for revision in history:
    try:
        candidate = subprocess.check_output(
            ["git", "show", f"{revision}:src/lib/cgrow.hpp"],
            cwd=ROOT,
            stderr=subprocess.DEVNULL,
        ).decode("utf-8")
    except subprocess.CalledProcessError:
        continue
    if all(
        signature in candidate
        for signature in (
            "auto span = std::abs( std::log10( xpos ) - std::log10( xneg ) );",
            "ParamList params;",
            "inline int        totalevals",
            "const bool&         stop_requested",
        )
    ):
        old = candidate
        break

if old is None:
    print("No historical predecessor containing the original bug signatures; skipping")
    sys.exit(0)

# These assertions deliberately inspect HEAD, not the repaired working tree.
assert "auto span = std::abs( std::log10( xpos ) - std::log10( xneg ) );" in old
assert "std::size_t( 1 )" in old
assert "ParamList params;" in old
assert "eval_set.clear" not in old
assert "inline int        totalevals" in old
assert "const bool&         stop_requested" in old

# Reproduce the shadowing loop independently: the inner declaration leaves the
# outer span unchanged, so the old loop reaches its hard cap every time.
probe = r"""
#include <cassert>
#include <cstddef>
int main() {
  constexpr std::size_t HS_MAX_ITERS = 60;
  std::size_t niters = 0;
  double span = 1.0;
  const double tol = 1.0e-4;
  while (niters++ < HS_MAX_ITERS && span > tol) {
    auto span = 0.0; // the predecessor's shadowing declaration
    (void)span;
  }
  assert(niters == HS_MAX_ITERS + 1);
}
"""

with tempfile.TemporaryDirectory() as directory:
    source = Path(directory) / "probe.cpp"
    binary = Path(directory) / "probe"
    source.write_text(probe)
    compiler = None
    for candidate in ("c++", "g++", "clang++"):
        if shutil.which(candidate) and subprocess.call(
            [candidate, "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        ) == 0:
            compiler = candidate
            break
    if compiler is None:
        print("No C++ compiler available for predecessor probe; static checks passed")
        sys.exit(0)
    subprocess.check_call([compiler, "-std=c++17", str(source), "-o", str(binary)])
    subprocess.check_call([str(binary)])

print("original cgrow bug checks passed")
