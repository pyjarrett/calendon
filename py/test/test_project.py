import os
import unittest

from kn import base_arg_parser, parse_overrides
from kn.project import BuildAndRunContext, parse_build_context_with_overrides, add_build_args


class BuildAndRunContextTest(unittest.TestCase):
    def test_build_override_parser(self):
        args = '--build-dir special_build_dir --build-config Release --compiler gcc'
        overrides = parse_overrides(args, add_build_args(base_arg_parser()))
        original = BuildAndRunContext()
        original.register_program('clang', '/usr/bin/clang', force=True)
        original.register_program('gcc', '/usr/bin/gcc', force=True)
        original.set_compiler_alias('clang')
        with parse_build_context_with_overrides(original, overrides) as overridden:
            self.assertEqual(original.compiler(), 'clang')
            self.assertEqual(overridden.compiler(), 'gcc')
            self.assertEqual(original.build_config(), 'Debug')
            self.assertEqual(overridden.build_config(), 'Release')
            self.assertEqual(os.path.basename(original.build_dir()), 'build-clang-Debug')
            self.assertEqual(os.path.basename(overridden.build_dir()), 'special_build_dir')

    def test_bad_parse(self):
        overrides = parse_overrides('asdfasdfasdf', add_build_args(base_arg_parser()))
        self.assertIsNone(overrides)


if __name__ == '__main__':
    unittest.main()
