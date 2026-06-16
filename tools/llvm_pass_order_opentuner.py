#!/usr/bin/env python3
"""
OpenTuner: search LLVM `opt` pass selection and order for a .ll file.

Pipeline each trial: opt (-passes=...) -> optimized .ll (written to your path, kept)
-> llc -O0 -> .o -> clang++ -O0 -no-pie -> binary -> taskset-run and measure time.

Usage:
  source env/bin/activate
  python tools/llvm_pass_order_opentuner.py input.ll --opt-ll-out tuned.ll --test-limit 50

Each measurement prints the tried -passes= pipeline, selection/order summary, state,
and runtime (median over --exec-reps; individual reps on success). Use --quiet-trials
to disable.

Environment: OPT, LLC, CLANGXX (override tool names/paths).
"""

from __future__ import annotations

import os
import shutil
import statistics
import tempfile

import opentuner
from opentuner.measurement.interface import DefaultMeasurementInterface
from opentuner.resultsdb.models import Result
from opentuner.search.manipulator import BooleanParameter, ConfigurationManipulator
from opentuner.search.manipulator import PermutationParameter


DEFAULT_PASSES: tuple[str, ...] = (
    "early-cse",
    "mem2reg",
    "simplifycfg",
    "instcombine",
    "reassociate",
    "gvn",
    "sccp",
    "dce",
    "adce",
    "loop-simplify",
    "licm",
    "loop-deletion",
    "indvars",
    "loop-rotate",
    "tailcallelim",
)


def _enable_key(pass_name: str) -> str:
    return "enable_" + pass_name.replace("-", "_")


def _build_pass_string(cfg: dict, pass_names: tuple[str, ...]) -> str | None:
    parts: list[str] = []
    for p in cfg["pass_order"]:
        if p not in pass_names:
            continue
        if cfg.get(_enable_key(p), True):
            parts.append(p)
    if not parts:
        return None
    return ",".join(parts)


class LLVMOptPassOpenTuner(DefaultMeasurementInterface):
    def __init__(self, args, **kwargs):
        super(LLVMOptPassOpenTuner, self).__init__(args=args, **kwargs)
        self.ll_path = os.path.abspath(self.args.ll_file)
        self.opt_ll_out = os.path.abspath(self.args.opt_ll_out)
        if not os.path.isfile(self.ll_path):
            raise SystemExit(f"Input LLVM IR not found: {self.ll_path}")
        out_dir = os.path.dirname(self.opt_ll_out)
        if out_dir and not os.path.isdir(out_dir):
            raise SystemExit(f"Output directory does not exist: {out_dir}")

        self.opt_bin = shutil.which(os.environ.get("OPT", "opt")) or "opt"
        self.llc_bin = shutil.which(os.environ.get("LLC", "llc")) or "llc"
        self.clangxx_bin = shutil.which(os.environ.get("CLANGXX", "clang++")) or "clang++"

        self.pass_names: tuple[str, ...] = tuple(
            p.strip()
            for p in (self.args.pass_list or ",".join(DEFAULT_PASSES)).split(",")
            if p.strip()
        )
        if not self.pass_names:
            raise SystemExit("Empty --pass-list")
        self._trial_seq = 0

    def _format_config(self, cfg: dict) -> str:
        """Compact description: enabled passes in pass_order, then disabled list."""
        enabled = [p for p in cfg["pass_order"] if cfg.get(_enable_key(p), True)]
        disabled = [p for p in self.pass_names if not cfg.get(_enable_key(p), True)]
        dis = f" disabled={disabled}" if disabled else ""
        return f"order={enabled}{dis}"

    def _report_trial(
        self,
        desired_result,
        cfg: dict,
        passes: str | None,
        result: Result,
        rep_times: list[float] | None = None,
    ) -> Result:
        if getattr(self.args, "quiet_trials", False):
            return result
        self._trial_seq += 1
        n = self._trial_seq
        cfg_db_id = getattr(desired_result.configuration, "id", None)
        id_part = f" cfg_id={cfg_db_id}" if cfg_db_id is not None else ""
        pass_part = passes if passes is not None else "(no passes enabled)"
        t = result.time
        st = result.state
        line = (
            f"[trial {n}]{id_part} state={st} runtime_s={t}  "
            f"-passes={pass_part}  ({self._format_config(cfg)})"
        )
        if rep_times is not None and len(rep_times) > 1:
            reps = ", ".join(f"{x:.6f}" for x in rep_times)
            line += f"  reps_s=[{reps}]"
        print(line, flush=True)
        return result

    def manipulator(self):
        manip = ConfigurationManipulator()
        for p in self.pass_names:
            manip.add_parameter(BooleanParameter(_enable_key(p)))
        manip.add_parameter(PermutationParameter("pass_order", list(self.pass_names)))
        return manip

    def seed_configurations(self):
        cfg = {_enable_key(p): True for p in self.pass_names}
        cfg["pass_order"] = list(self.pass_names)
        return [cfg]

    def run(self, desired_result, input, limit):
        cfg = desired_result.configuration.data
        passes = _build_pass_string(cfg, self.pass_names)
        result = Result()

        if passes is None:
            result.state = "ERROR"
            result.time = float("inf")
            return self._report_trial(desired_result, cfg, passes, result, None)

        tmp = tempfile.mkdtemp(prefix="llvm_pass_tune_")
        obj_path = os.path.join(tmp, "tune.o")
        exe_path = os.path.join(tmp, "tune.bin")

        try:
            opt_cmd = [
                self.opt_bin,
                "-S",
                self.ll_path,
                "-o",
                self.opt_ll_out,
                f"-passes={passes}",
            ]
            r0 = self.call_program(opt_cmd, limit=limit)
            if r0["timeout"] or r0["returncode"] != 0:
                result.state = "TIMEOUT" if r0["timeout"] else "ERROR"
                result.time = float("inf")
                return self._report_trial(desired_result, cfg, passes, result, None)

            llc_cmd = [
                self.llc_bin,
                "-O0",
                "-filetype=obj",
                self.opt_ll_out,
                "-o",
                obj_path,
            ]
            r_llc = self.call_program(llc_cmd, limit=limit)
            if r_llc["timeout"] or r_llc["returncode"] != 0:
                result.state = "TIMEOUT" if r_llc["timeout"] else "ERROR"
                result.time = float("inf")
                return self._report_trial(desired_result, cfg, passes, result, None)

            link_cmd = [
                self.clangxx_bin,
                "-O0",
                "-no-pie",
                obj_path,
                "-o",
                exe_path,
                "-lm",
            ]
            r_link = self.call_program(link_cmd, limit=limit)
            if r_link["timeout"] or r_link["returncode"] != 0:
                result.state = "TIMEOUT" if r_link["timeout"] else "ERROR"
                result.time = float("inf")
                return self._report_trial(desired_result, cfg, passes, result, None)

            run_cmd = [
                "taskset",
                "-c",
                str(self.args.cpu_core),
                exe_path,
            ]

            times: list[float] = []
            for _ in range(self.args.exec_reps):
                r1 = self.call_program(run_cmd, limit=limit)
                elapsed = float(r1["time"])
                if r1["timeout"] or r1["returncode"] != 0:
                    result.state = "TIMEOUT" if r1["timeout"] else "ERROR"
                    result.time = float("inf")
                    return self._report_trial(desired_result, cfg, passes, result, times)
                times.append(elapsed)

            rep_times = times
            result.state = "OK"
            result.time = float(statistics.median(times))
            return self._report_trial(desired_result, cfg, passes, result, rep_times)
        finally:
            shutil.rmtree(tmp, ignore_errors=True)


def main():
    parser = opentuner.default_argparser()
    parser.add_argument(
        "ll_file",
        help="Input LLVM IR file (.ll)",
    )
    parser.add_argument(
        "--opt-ll-out",
        required=True,
        metavar="PATH",
        help="Where to write optimized IR from opt (updated each trial; not deleted)",
    )
    parser.add_argument(
        "--pass-list",
        metavar="NAMES",
        help="Comma-separated LLVM new-PM pass names (default: built-in set)",
    )
    parser.add_argument(
        "--exec-reps",
        type=int,
        default=5,
        help="Run the binary this many times per trial; objective is median wall time",
    )
    parser.add_argument(
        "--cpu-core",
        type=int,
        default=5,
        help="CPU core passed to taskset -c (default: 5, same as simple_compare.py)",
    )
    parser.add_argument(
        "--quiet-trials",
        action="store_true",
        help="Do not print each tried configuration and runtime",
    )
    args = parser.parse_args()
    LLVMOptPassOpenTuner.main(args)


if __name__ == "__main__":
    main()
