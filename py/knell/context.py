from __future__ import annotations  # See PEP 484 and PEP 563.
import copy
from dataclasses import dataclass
import dataclasses
import json
import os
from typing import Dict, List, Optional

from knell.multiplatform import root_to_executable
from knell.run import run_program


def _override_flavor_from_dict(flavor: object, kv: Dict):
    """Overrides values in a flavor if they assigned in the namespace."""
    for k in kv:
        if kv[k] is not None and hasattr(flavor, k):
            setattr(flavor, k, kv[k])


@dataclass
class BuildFlavor:
    """Define enough information to do a build."""
    build_dir: str = 'build'
    build_config: str = 'Debug'
    compiler: Optional[str] = None


@dataclass
class RunFlavor:
    """Environment specification of how to do a 'run'."""
    game: Optional[str] = None
    asset_dir: Optional[str] = None
    ticks: Optional[int] = 0
    headless: bool = False

    def to_driver_args(self) -> List[str]:
        """Convert this run flavor into arguments readable by the driver."""
        args = []
        if self.game:
            args.extend(['--game', self.game])
        if self.asset_dir:
            args.extend(['--asset-dir', self.asset_dir])
        if self.ticks:
            args.extend(['--tick-limit', str(self.ticks)])
        if self.headless:
            args.append('--headless')
        print(args)
        return args

class ProjectContext:
    """A concise description of the environment in which the script will run."""

    def __init__(self, knell_home: str):
        """Creates a default context from a home directory."""
        self._knell_home = os.path.abspath(knell_home)
        self._build_flavor = BuildFlavor()
        self._run_flavor = RunFlavor()
        self._registered_programs: Dict[str, str] = {}
        self._load_config(os.path.join(self.knell_home(), '.crank'))

    def _override_from_dict(self, kv: Dict):
        _override_flavor_from_dict(self._build_flavor, kv)
        _override_flavor_from_dict(self._run_flavor, kv)

    def _load_config(self, config_path: str):
        """Loads a config from a path."""
        if not os.path.isfile(config_path):
            print(f'No config file found at {config_path}')
            return

        with open(config_path, 'r') as file:
            kv = json.load(file)
            self._registered_programs = kv.get('registered_programs', {})
            self._override_from_dict(kv)

    def _save_config(self, config_path: str):
        with open(config_path, 'w') as file:
            combined = self.dump()
            json.dump(combined, file, indent=4)

    def dump(self) -> Dict:
        combined = {}
        for flavor in [self._build_flavor, self._run_flavor]:
            for field in dataclasses.fields(flavor):
                combined[field.name] = getattr(flavor, field.name)
        combined['registered_programs'] = self._registered_programs
        return combined

    def save(self):
        self._save_config(os.path.join(self.knell_home(), '.crank'))

    def copy_with_overrides(self, kv: Dict) -> ProjectContext:
        """Creates a new context with the given overrides applied."""
        ctx = copy.deepcopy(self)
        ctx._override_from_dict(kv)
        return ctx

    def knell_home(self) -> str:
        """Project root directory."""
        return self._knell_home

    def py_dir(self) -> str:
        return os.path.abspath(os.path.join(self.knell_home(), 'py'))

    def build_dir(self) -> str:
        return os.path.abspath(os.path.join(self._knell_home, self._build_flavor.build_dir))

    def demo_dir(self) -> str:
        """Absolute directory path for where demos are stored."""
        return os.path.join(self.build_dir(), 'src', 'demos')

    def set_game(self, game):
        self._run_flavor.game = game

    def venv_dir(self) -> str:
        return os.path.abspath(os.path.join(self._knell_home, 'venv'))

    def driver_path(self) -> str:
        """The path to the driver executable, when built."""
        return os.path.join(self.build_dir(), 'src', 'driver', root_to_executable('knell-driver'))

    def build_config(self) -> str:
        return self._build_flavor.build_config

    def compiler(self) -> Optional[str]:
        return self._build_flavor.compiler

    def register_program(self, alias: str, path: str, override: bool = False) -> bool:
        if alias in self._registered_programs and not override:
            existing: str = self._registered_programs[alias]
            print(f'Trying to override {existing} of {alias} with {path}')
            return False

        self._registered_programs[alias] = path
        return True

    def has_registered_program(self, alias: str) -> bool:
        return alias in self._registered_programs

    def path_for_program(self, alias: str) -> str:
        if not self.has_registered_program(alias):
            raise ValueError(f'No registered path for {alias}')
        return self._registered_programs[alias]

    def run_driver(self) -> int:
        args = [self.driver_path()]
        args.extend(self._run_flavor.to_driver_args())
        return run_program(args, cwd=self.build_dir())


def default_knell_home():
    """Knell is assumed to be provided by the environment, or the current directory."""
    return os.environ.get('KNELL_HOME', os.getcwd())