#!/usr/bin/env python3
"""
Compile a sketch for every board of the core, in every combination its menus
offer. A board entry that is wrong in boards.txt or platform.txt usually shows
up here and nowhere else, because nobody builds for every combination by hand.

    python3 extras/compile_all.py --fqbn-prefix XMiniCore:avr --menus clock,LTO
    python3 extras/compile_all.py --root megaavr --fqbn-prefix megaTinyCore:megaavr \
            --known-failures extras/compile_all_known_failures.txt

Where the menus of a core are not independent of each other, some combinations
are refused by the core on purpose (an #error for a timer the chip does not
have, say). Those go into a file named with --known-failures, one per line,
written as board:menu=option,... without the vendor and architecture, with the
reason after a '#'. They are then reported but do not count as failures; only a
combination that is not on the list does.

Needs arduino-cli with the core installed. Standard library apart from that.
"""

import argparse
import itertools
import os
import re
import shutil
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from check_core import (properties, board_ids,    # pylint: disable=wrong-import-position
                        working_directory)

SKETCH = """
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
"""


COMPILE_RECIPES = ("recipe.c.o.pattern", "recipe.cpp.o.pattern", "recipe.S.o.pattern",
                   "recipe.ar.pattern", "recipe.c.combine.pattern",
                   "recipe.preproc.macros")


def build_properties(platform: dict, boards: dict) -> set:
    """
    Which properties a compile recipe reads, directly or through others. The
    recipes rarely name a menu's property themselves: a menu sets build.xyz,
    something composes build.extra_flags out of it, and only that appears in the
    recipe. So follow the references until nothing new turns up.
    """
    reference = re.compile(r"\{([\w.]+)\}")
    known = dict(platform)
    for key, value in boards.items():          # a menu may compose things as well
        known.setdefault(key.split(".menu.")[-1].split(".", 1)[-1], value)
    used, pending = set(), []
    for key in COMPILE_RECIPES:
        pending += reference.findall(platform.get(key, ""))
    while pending:
        name = pending.pop()
        if name in used:
            continue
        used.add(name)
        pending += reference.findall(known.get(name, ""))
    return used


def affects_the_build(boards: dict, menu: str, relevant: set) -> bool:
    """Whether any option of this menu sets something a compile recipe reads."""
    marker = f".menu.{menu}."
    for key in boards:
        if marker not in key:
            continue
        tail = key.split(marker, 1)[1]
        if "." not in tail:
            continue
        prop = tail.split(".", 1)[1]
        if prop.startswith("compiler.") or prop in relevant:
            return True
    return False


def count_combinations(offered: dict) -> int:
    """How many combinations there would be, without building the list."""
    total = 1
    for options in offered.values():
        total *= len(options)
    return total


def menus_worth_varying(boards: dict, platform: dict) -> list:
    """
    The menus that can change the binary, saying which ones are left out. The
    others still get built, just in one of their settings rather than all.
    """
    relevant = build_properties(platform, boards)
    every = sorted({k.split(".menu.")[1].split(".")[0] for k in boards
                    if ".menu." in k})
    wanted = [m for m in every if affects_the_build(boards, m, relevant)]
    skipped = [m for m in every if m not in wanted]
    if skipped:
        print(f"not varying {', '.join(skipped)}: no compile recipe reads what they set")
    return wanted


def menu_options(boards: dict, board: str) -> dict:
    """
    Which menus this board offers and which options each has, in the order they
    are declared: the first one is what the IDE offers by default. An option is
    what has a label (board.menu.MENU.OPTION=label); properties of an option
    without one are left-overs the IDE never offers, and arduino-cli refuses an
    FQBN that names it.
    """
    offered: dict = {}
    prefix = f"{board}.menu."
    for key in boards:
        if not key.startswith(prefix):
            continue
        parts = key[len(prefix):].split(".")
        if len(parts) == 2 and parts[1] not in offered.setdefault(parts[0], []):
            offered[parts[0]].append(parts[1])
    return offered


def as_fqbn_part(menus: list, picked: tuple) -> str:
    """Turn one choice per menu into the tail of an FQBN."""
    return ",".join(f"{m}={o}" for m, o in zip(menus, picked))


def combinations(offered: dict) -> list:
    """Every combination of the menus, as a list of 'menu=option' strings."""
    if not offered:
        return [""]
    menus = sorted(offered)
    return [as_fqbn_part(menus, picked)
            for picked in itertools.product(*(offered[m] for m in menus))]


def each_value_once(offered: dict) -> list:
    """
    Enough combinations for every option of every menu to be built at least once.
    Counting all menus up at the same time needs as many builds as the longest
    menu has options, rather than their product. Catches an option that is broken
    by itself, not two that only go wrong together.
    """
    if not offered:
        return [""]
    menus = sorted(offered)
    longest = max(len(offered[m]) for m in menus)
    return [as_fqbn_part(menus, tuple(offered[m][i % len(offered[m])] for m in menus))
            for i in range(longest)]


def defaults_only(offered: dict) -> list:
    """
    One combination, the one the IDE starts with. Enough where the point is the
    platform rather than the options: the hook scripts and the handling of paths
    do not depend on which menu entry is chosen.
    """
    if not offered:
        return [""]
    menus = sorted(offered)
    return [as_fqbn_part(menus, tuple(offered[m][0] for m in menus))]


def write_sketch(folder: str) -> str:
    """
    Put a sketch where arduino-cli can build it, below a directory whose name has
    a blank in it. The sketch name itself must not have one, but everything above
    it may, and that is where an unquoted path in a recipe shows up. A work-dir
    that already has a blank in its name is one, so it does not get another.
    """
    above = folder if " " in os.path.basename(folder) else os.path.join(folder, "with blank")
    sketch = os.path.join(above, "compile_all_probe")
    os.makedirs(sketch, exist_ok=True)
    with open(os.path.join(sketch, "compile_all_probe.ino"), "w", encoding="utf-8") as out:
        out.write(SKETCH)
    return sketch


FULL_PRODUCT_LIMIT = 64          # above this, 'auto' stops building every combination


def chosen_combinations(offered: dict, coverage: str) -> list:
    """The combinations this coverage asks for."""
    if coverage == "one":
        return defaults_only(offered)
    if coverage == "each-value":
        return each_value_once(offered)
    if coverage == "full" or count_combinations(offered) <= FULL_PRODUCT_LIMIT:
        return combinations(offered)
    return each_value_once(offered)


def left_to_go(done_so_far: int, total: int, spent: float) -> str:
    """A guess at the remaining time, from how long the builds so far have taken."""
    if done_so_far == 0:
        return ""
    remaining = spent / done_so_far * (total - done_so_far)
    return f", about {remaining / 60:.0f} min left" if remaining > 90 else ""


def without_prefix(fqbn: str) -> str:
    """board:options of an FQBN, which is what the list of known failures holds."""
    return fqbn.split(":", 2)[2]


def read_known_failures(path: str) -> dict:
    """board:options -> reason, from a file of lines 'board:options  # reason'."""
    known = {}
    with open(path, encoding="utf-8") as listing:
        for line in listing:
            entry, _, reason = line.partition("#")
            if entry.strip():
                known[entry.strip()] = reason.strip()
    return known


def build_them_all(sketch: str, every: list, timeout: int, known: dict) -> tuple:
    """Build the lot, reporting progress as it goes. Returns failures and seconds."""
    print(f"{len(every)} combination(s) to build", flush=True)
    failed, started = [], time.monotonic()
    for number, fqbn in enumerate(every, start=1):
        if not compile_one(sketch, fqbn, number, len(every), timeout,
                           known.get(without_prefix(fqbn))):
            failed.append(fqbn)
        spent = time.monotonic() - started
        if number % 10 == 0 and number != len(every):
            print(f"          {number} of {len(every)} after {spent / 60:.1f} min"
                  f"{left_to_go(number, len(every), spent)}", flush=True)
    return failed, time.monotonic() - started


def tail_of(logfile: str, lines: int) -> list:
    """The last lines of a build log, because what went wrong is at the end of it."""
    try:
        with open(logfile, encoding="utf-8", errors="replace") as log:
            return log.read().strip().splitlines()[-lines:]
    except OSError:
        return []


def show_tail(logfile: str, why: str, lines: int = 15) -> None:
    """Say how far the build had got."""
    tail = tail_of(logfile, lines)
    if not tail:
        print(f"    it had not said anything at all {why}", flush=True)
        return
    print(f"    what it had got to {why}:", flush=True)
    print("      " + "\n      ".join(tail), flush=True)


def kill_the_lot(proc: subprocess.Popen) -> None:
    """
    Stop the build and everything it has started. Killing arduino-cli on its own
    leaves the compiler it is waiting for running, and under Windows those keep
    the log file open for as long as they live.
    """
    if os.name == "nt":
        subprocess.run(["taskkill", "/F", "/T", "/PID", str(proc.pid)], check=False,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    proc.kill()
    try:
        proc.wait(timeout=30)
    except subprocess.TimeoutExpired:
        pass


def compile_one(sketch: str, fqbn: str, number: int, total: int,  # pylint: disable=too-many-arguments,too-many-positional-arguments
                timeout: int, expected: str | None = None) -> bool:
    """
    Build one combination, saying how it went and how far along we are.

    The build writes into a file rather than into a pipe. Under Windows every
    process arduino-cli starts inherits that pipe, and a pipe has only reached
    its end once the last of them has let go of it, so a build that finished
    long ago can leave us waiting for output nobody is going to write. A file
    has no such end to wait for, and it outlives the build, so one that had to
    be given up on can still be read afterwards.
    """
    build = os.path.join(os.path.dirname(sketch), "build path")
    logfile = os.path.join(os.path.dirname(sketch), "build.log")
    started, code = time.monotonic(), None
    try:
        with open(logfile, "w", encoding="utf-8", errors="replace") as log:
            proc = subprocess.Popen(["arduino-cli", "compile", "--clean", "-b", fqbn,
                                     "--build-path", build, sketch],
                                    stdin=subprocess.DEVNULL, stdout=log,
                                    stderr=subprocess.STDOUT)
            try:
                code = proc.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                kill_the_lot(proc)
    except OSError as err:
        print(f"[{number:>3}/{total}] FAILED        {fqbn}\n    {err}", flush=True)
        return False
    took = time.monotonic() - started
    if code is None:
        print(f"[{number:>3}/{total}] STUCK  {timeout:5.0f} s  {fqbn}\n"
              "    still going after the time allowed, so it is not slow but stuck",
              flush=True)
        show_tail(logfile, "before it was stopped")
        return False
    if code != 0 and expected is not None:
        print(f"[{number:>3}/{total}] known  {took:5.1f} s  {fqbn}\n"
              f"    refused as expected: {expected}", flush=True)
        return False
    print(f"[{number:>3}/{total}] {'ok    ' if code == 0 else 'FAILED'} "
          f"{took:5.1f} s  {fqbn}", flush=True)
    if code != 0:
        show_tail(logfile, "before it gave up", lines=30)
    return code == 0


def main() -> int:
    """Build every combination and report which ones did not come through."""
    parser = argparse.ArgumentParser(description="compile for every board and menu")
    parser.add_argument("--fqbn-prefix", required=True, help="e.g. TinyCore:avr")
    parser.add_argument("--root", help="the core's folder (default: the one above this)")
    parser.add_argument("--menus", help="only vary these menus, comma separated")
    parser.add_argument("--all-menus", action="store_true",
                        help="also vary menus that cannot change the binary")
    parser.add_argument("--timeout", type=int, default=300, metavar="SECONDS",
                        help="give up on a build that takes longer and say so. A "
                             "build that hangs looks exactly like a slow one until "
                             "somebody puts a clock on it")
    parser.add_argument("--work-dir", metavar="DIR",
                        help="where to put the sketch and the build. A temporary "
                             "directory by default, but on Windows the build has "
                             "to sit somewhere the virus scanner has been told "
                             "about, and that means knowing where it is")
    parser.add_argument("--max-builds", type=int, default=0, metavar="N",
                        help="stop after N builds. Where the point is the platform "
                             "rather than the options, one is enough, and on Windows "
                             "a build costs a minute")
    parser.add_argument("--coverage", default="auto",
                        choices=["auto", "full", "each-value", "one"],
                        help="how much to build: every combination, every option "
                             "at least once, or just the default one. 'auto' takes "
                             "the full product while it stays small")
    parser.add_argument("--known-failures", metavar="FILE",
                        help="combinations the core refuses on purpose, one per "
                             "line as board:menu=option,... with the reason after "
                             "a '#'. They are reported but not counted as failures")
    args = parser.parse_args()

    if shutil.which("arduino-cli") is None:
        print("arduino-cli is not on PATH, so there is nothing to build with")
        return 1
    root = args.root or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    boards = properties(os.path.join(root, "boards.txt"))
    platform = properties(os.path.join(root, "platform.txt"))
    wanted = args.menus.split(",") if args.menus else None
    if wanted is None and not args.all_menus:
        wanted = menus_worth_varying(boards, platform)

    with working_directory(args.work_dir) as folder:
        sketch = write_sketch(folder)
        every = []
        for board in board_ids(boards):
            offered = menu_options(boards, board)
            if wanted is not None:
                offered = {m: o for m, o in offered.items() if m in wanted}
            for combination in chosen_combinations(offered, args.coverage):
                fqbn = f"{args.fqbn_prefix}:{board}"
                every.append(fqbn + (f":{combination}" if combination else ""))
        if args.max_builds and len(every) > args.max_builds:
            print(f"{len(every)} combinations, building the first {args.max_builds}")
            every = every[:args.max_builds]
        known = read_known_failures(args.known_failures) if args.known_failures else {}
        failed, spent = build_them_all(sketch, every, args.timeout, known)

    return summary(every, failed, known, spent)


def summary(every: list, failed: list, known: dict, spent: float) -> int:
    """Say how it went. Only failures that are not on the known list count."""
    new = [f for f in failed if without_prefix(f) not in known]
    built = {without_prefix(f) for f in every}
    gone_wrong = {without_prefix(f) for f in failed}
    now_fine = sorted(k for k in known if k in built and k not in gone_wrong)
    not_built = sorted(k for k in known if k not in built)
    print(f"{len(every)} combination(s) in {spent / 60:.1f} min, {len(failed)} failed"
          + (f", {len(failed) - len(new)} of them known" if known else ""))
    for entry in now_fine:
        print(f"on the known list but builds now, so it can come off: {entry}")
    for entry in not_built:
        print(f"on the known list but not built this time (menus changed?): {entry}")
    for fqbn in new:
        print(f"new failure: {fqbn}")
    return 1 if new else 0


if __name__ == "__main__":
    sys.exit(main())
