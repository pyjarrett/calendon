from __future__ import annotations  # See PEP 484 and PEP 563.
import copy
from dataclasses import dataclass
import dataclasses
import json
import os
from typing import Dict, Optional


def _override_flavor_from_dict(flavor: object, kv: Dict):
    """Overrides values in a flavor if they assigned in the namespace."""
    for k in kv:
        if kv[k] is not None and hasattr(flavor, k):
            setattr(flavor, k, kv[k])


@dataclass
class ScriptFlavor:
    verbose: bool = False
    dry_run: bool = False


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
    run_time_seconds: Optional[int] = 0
    headless: bool = False


class ProjectContext:
    """A concise description of the environment in which the script will run."""

    def __init__(self, knell_home: str):
        """Creates a default context from a home directory."""
        self._script_flavor = ScriptFlavor()
        self._knell_home = os.path.abspath(knell_home)
        self._build_flavor = BuildFlavor()
        self._run_flavor = RunFlavor()
        self._registered_programs: Dict[str, str] = {}
        self._load_config(os.path.join(self.knell_home(), '.hammer'))

    def _override_from_dict(self, kv: Dict):
        _override_flavor_from_dict(self._script_flavor, kv)
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
            if self.is_verbose():
                print(f'Saving to {config_path}')
                print(combined)
            json.dump(combined, file, indent=4)

    def dump(self) -> Dict:
        combined = {}
        for flavor in [self._script_flavor, self._build_flavor, self._run_flavor]:
            for field in dataclasses.fields(flavor):
                combined[field.name] = getattr(flavor, field.name)
        combined['registered_programs'] = self._registered_programs
        return combined

    def save(self):
        self._save_config(os.path.join(self.knell_home(), '.hammer'))

    def copy_with_overrides(self, kv: Dict) -> ProjectContext:
        """Creates a new context with the given overrides applied."""
        ctx = copy.deepcopy(self)
        ctx._override_from_dict(kv)

        if self._script_flavor.verbose:
            print(f'Overrode: {self.__dict__}')
            print(f'Using:    {kv}')
            print(f'Result:   {self.__dict__}')
        return ctx

    def is_dry_run(self) -> bool:
        return self._script_flavor.dry_run

    def is_verbose(self) -> bool:
        return self._script_flavor.verbose

    def knell_home(self) -> str:
        """Project root directory."""
        return self._knell_home

    def py_dir(self) -> str:
        return os.path.abspath(os.path.join(self.knell_home(), 'py'))

    def build_dir(self) -> str:
        return os.path.abspath(os.path.join(self._knell_home, self._build_flavor.build_dir))

    def venv_dir(self) -> str:
        return os.path.abspath(os.path.join(self._knell_home, 'venv'))

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