#!/usr/bin/env python3
"""
Check a core for the kinds of mistake that only show up when someone else uses it.

    python3 extras/check_core.py              # check the core this file belongs to
    python3 extras/check_core.py --tag v1.3.1 # and that it is ready to be released

Every check is about consistency inside the core, so it needs nothing but the
files themselves and the standard library. Errors make it exit non-zero;
warnings are printed and do not.
"""

import argparse
import contextlib
import difflib
import os
import re
import sys
import tempfile
from typing import Iterator

PROBLEMS: list = []
WARNINGS: list = []


@contextlib.contextmanager
def working_directory(given: str) -> Iterator[str]:
    """The directory to work in: the one that was asked for, or a temporary one."""
    if given:
        os.makedirs(given, exist_ok=True)
        yield given
    else:
        with tempfile.TemporaryDirectory() as folder:
            yield folder


def base_version(tag: str) -> str:
    """
    The version a tag belongs to. A pre-release such as v1.3.2-rc4 belongs to
    1.3.2 and has neither a version of its own in platform.txt nor an entry of
    its own in the changelog; that is the point of trying one out.
    """
    return tag.lstrip("v").split("-", 1)[0]


def problem(text: str) -> None:
    """Record something that must be fixed before a release."""
    PROBLEMS.append(text)


def warn(text: str) -> None:
    """Record something worth a look that does not stop a release."""
    WARNINGS.append(text)


def properties(path: str) -> dict:
    """
    Read an Arduino properties file. Keys repeat the board id, values may contain
    anything, comments start with # at the beginning of a line.
    """
    found: dict = {}
    if not os.path.exists(path):
        problem(f"{os.path.basename(path)} is missing")
        return found
    with open(path, encoding="utf-8", errors="replace") as src:
        for line in src:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, value = line.split("=", 1)
            found[key.strip()] = value.strip()
    return found


def board_ids(boards: dict) -> list:
    """Every id that has a name is a board; menu declarations and the like are not."""
    return sorted(key[:-len(".name")] for key in boards
                  if key.endswith(".name") and key.count(".") == 1)


def defined_for(boards: dict, board: str, suffix: str) -> bool:
    """
    Whether a board has a property, directly or through one of its menu options:
    build.f_cpu, for instance, usually comes from the clock menu.
    """
    if f"{board}.{suffix}" in boards:
        return True
    prefix, ending = f"{board}.menu.", f".{suffix}"
    return any(k.startswith(prefix) and k.endswith(ending) for k in boards)


def check_boards(root: str, boards: dict) -> None:
    """Each board needs the properties a build depends on, and its variant folder."""
    for board in board_ids(boards):
        for suffix in ("build.mcu", "build.f_cpu", "build.board", "build.variant"):
            if not defined_for(boards, board, suffix):
                problem(f"board '{board}' has no {suffix}, not even through a menu")
        variant = boards.get(f"{board}.build.variant")
        if variant and not os.path.isdir(os.path.join(root, "variants", variant)):
            problem(f"board '{board}' uses variant '{variant}', which has no folder")


def check_orphans(boards: dict) -> None:
    """
    Every property belongs to a board, and a board is something with a name. A
    property whose prefix is no board is usually a typo in the board id, and it
    does nothing at all: the board silently lacks whatever was meant for it.
    """
    ids = set(board_ids(boards))
    orphans: dict = {}
    for key in boards:
        head = key.split(".")[0]
        if head in ids or head in ("menu", "compiler", "build", "recipe", "tools"):
            continue
        if f"{head}.name" in boards:
            continue
        orphans.setdefault(head, 0)
        orphans[head] += 1
    for head, number in sorted(orphans.items()):
        closest = difflib.get_close_matches(head, ids, n=1, cutoff=0.7)
        hint = f", did you mean '{closest[0]}'" if closest else ""
        problem(f"'{head}' has {number} propertie(s) but is not a board{hint}")


def check_menu_coverage(boards: dict) -> None:
    """
    A menu that some boards offer and others do not is worth a look: usually a
    board was forgotten when the menu was added.
    """
    ids = board_ids(boards)
    for menu in sorted({k.split(".")[2] for k in boards
                        if k.count(".") >= 3 and k.split(".")[1] == "menu"
                        and k.split(".")[0] in ids}):
        without = [b for b in ids
                   if not any(k.startswith(f"{b}.menu.{menu}.") for k in boards)]
        if without and len(without) < len(ids):
            warn(f"menu '{menu}' is missing on {', '.join(without)}")


def check_menus(boards: dict) -> None:
    """
    A menu a board uses has to be declared at the top of boards.txt, and every
    option a board configures needs a label, or the IDE shows an empty entry.
    """
    declared = {key[len("menu."):] for key in boards
                if key.startswith("menu.") and key.count(".") == 1}
    used: dict = {}
    for key in boards:
        parts = key.split(".")
        if len(parts) >= 4 and parts[1] == "menu":
            used.setdefault(parts[2], set()).add(parts[3])
    for menu in sorted(used):
        if menu not in declared:
            problem(f"menu '{menu}' is used by a board but never declared")
    for menu in sorted(declared - set(used)):
        warn(f"menu '{menu}' is declared but no board offers it")
    ids = set(board_ids(boards))
    for menu, options in sorted(used.items()):
        for option in sorted(options):
            ending = f".menu.{menu}.{option}"
            labels = [k for k in boards
                      if k.endswith(ending) and k[:-len(ending)] in ids]
            if not labels:
                problem(f"option '{option}' of menu '{menu}' has no label on any board")


def check_menu_clashes(boards: dict) -> None:
    """
    Two menus of one board must not set the same property. The old Java IDE
    applied the menus in the order they are declared, so a later menu could
    override what an earlier one set as a default; arduino-cli, and with it
    IDE 2, applies them in no fixed order, so which value wins changes from one
    build to the next. The way to keep a default that depends on another menu
    is a property of its own, e.g. chip sets build.millisdefault and the
    default option of millis sets build.millistimer={build.millisdefault}.
    """
    ids = set(board_ids(boards))
    setters: dict = {}
    for key in boards:
        parts = key.split(".", 4)
        if len(parts) == 5 and parts[1] == "menu" and parts[0] in ids:
            setters.setdefault((parts[0], parts[4]), set()).add(parts[2])
    clashes: dict = {}
    for (board, prop), menus in setters.items():
        if len(menus) > 1:
            clashes.setdefault((prop, tuple(sorted(menus))), []).append(board)
    for (prop, menus), clashing in sorted(clashes.items()):
        problem(f"'{prop}' is set by menus {' and '.join(menus)} on "
                f"{', '.join(sorted(clashing))}: which one wins is chance in IDE 2")


def check_lto(boards: dict, platform: dict) -> None:
    """
    A core that offers LTO as a menu must not put -flto into the optimization
    flags: those are used whatever the menu says, so the switch would do nothing.
    """
    has_menu = any(k.startswith("menu.LTO") for k in boards)
    if not has_menu:
        return
    for key, value in platform.items():
        if key.startswith("compiler.optimization_flags") and "-flto" in value:
            problem(f"{key} contains -flto although there is an LTO menu, "
                    "so 'LTO disabled' would not disable anything")


def check_hook_scripts(root: str, platform: dict) -> None:
    """Every script a recipe calls has to be there, on every platform."""
    pattern = re.compile(r"\{runtime\.platform\.path\}[/\\]([\w/\\.-]+)")
    for key, value in platform.items():
        if not key.startswith("recipe."):
            continue
        for relative in pattern.findall(value):
            path = os.path.join(root, relative.replace("\\", "/"))
            if not os.path.exists(path):
                problem(f"{key} calls '{relative}', which does not exist")


def up_to_repository(root: str) -> Iterator[str]:
    """
    The core's folder and those above it, up to the repository itself. The core
    may sit in a subdirectory of its repository (avr/, megaavr/), and files such
    as .gitattributes or the changelog then belong to the repository.
    """
    folder = os.path.abspath(root)
    while True:
        yield folder
        parent = os.path.dirname(folder)
        if parent == folder or os.path.exists(os.path.join(folder, ".git")):
            return
        folder = parent


def check_line_endings(root: str) -> None:
    """Windows batch files need CRLF, and git only keeps it with a rule for it."""
    rule = ""
    for folder in up_to_repository(root):
        attributes = os.path.join(folder, ".gitattributes")
        if os.path.exists(attributes):
            with open(attributes, encoding="utf-8", errors="replace") as src:
                rule += src.read()
    if ".bat" not in rule:
        for folder, _, files in os.walk(root):
            if ".git" in folder:
                continue
            if any(f.endswith(".bat") for f in files):
                problem("there are .bat files but .gitattributes has no rule for them, "
                        "so their CRLF line endings depend on whoever clones the core")
                return


def check_release(root: str, platform: dict, tag: str) -> None:
    """
    Before a release the version has to be stated in the two places that say it.
    A pre-release tag such as v1.3.2-rc1 is checked against 1.3.2: the point of
    trying one out is not having to touch the version number for every attempt.

    A version that is itself a pre-release and released as such (v2.7.0-pre5
    with version=2.7.0-pre5) is compared by its base version as well.

    A changelog is not required: not every core keeps one, and the script is
    meant to be usable in those too. It is looked for from the core's folder up
    to the repository, as CHANGELOG.md in any capitalisation. Where there is one,
    it has to have a heading for the version, which may be followed by a remark
    in brackets; where there is none, that is worth saying but not worth
    stopping a release for.
    """
    version = base_version(tag)
    stated = platform.get("version")
    if base_version(stated or "") != version:
        problem(f"platform.txt says version={stated}, but the tag says {version}")
    found = [os.path.join(folder, name) for folder in up_to_repository(root)
             for name in sorted(os.listdir(folder)) if name.lower() == "changelog.md"]
    if not found:
        warn("there is no CHANGELOG.md, so nothing was checked against it")
        return
    with open(found[0], encoding="utf-8", errors="replace") as src:
        text = src.read()
    heading = rf"^#+\s*v?{re.escape(version)}(\s*|\s+\(.*)$"
    if not re.search(heading, text, re.MULTILINE):
        problem(f"{os.path.basename(found[0])} has no entry for {version}")


def main() -> int:
    """Run every check and report what came out of it."""
    parser = argparse.ArgumentParser(description="check a core for consistency")
    parser.add_argument("--root", help="the core's folder (default: the one above this)")
    parser.add_argument("--tag", help="also check that the core is ready for this tag")
    args = parser.parse_args()

    root = args.root or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    boards = properties(os.path.join(root, "boards.txt"))
    platform = properties(os.path.join(root, "platform.txt"))
    print(f"checking {root}")
    print(f"  {len(board_ids(boards))} boards, {len(platform)} platform properties")

    check_boards(root, boards)
    check_orphans(boards)
    check_menu_coverage(boards)
    check_menus(boards)
    check_menu_clashes(boards)
    check_lto(boards, platform)
    check_hook_scripts(root, platform)
    check_line_endings(root)
    if args.tag:
        check_release(root, platform, args.tag)

    for text in WARNINGS:
        print(f"  warning: {text}")
    for text in PROBLEMS:
        print(f"  PROBLEM: {text}")
    if PROBLEMS:
        print(f"{len(PROBLEMS)} problem(s)")
        return 1
    print("no problems found")
    return 0


if __name__ == "__main__":
    sys.exit(main())
