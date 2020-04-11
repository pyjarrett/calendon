import os
import unittest

from kn.project import BuildAndRunContext, parse_build_context_with_overrides


class BuildAndRunContextTest(unittest.TestCase):
    def test_override(self):
        args = '--build-dir special_build_dir --build-config Release --compiler gcc'.split()
        original = BuildAndRunContext()
        original.register_program('clang', '/usr/bin/clang', force=True)
        original.register_program('gcc', '/usr/bin/gcc', force=True)
        original.set_compiler_alias('clang')
        with parse_build_context_with_overrides(original, args) as overridden:
            self.assertEqual(original.compiler(), 'clang')
            self.assertEqual(overridden.compiler(), 'gcc')
            self.assertEqual(original.build_config(), 'Debug')
            self.assertEqual(overridden.build_config(), 'Release')
            self.assertEqual(os.path.basename(original.build_dir()), 'build-clang-Debug')
            self.assertEqual(os.path.basename(overridden.build_dir()), 'special_build_dir')

    def test_bad_parse(self):
        original = BuildAndRunContext()
        with parse_build_context_with_overrides(original, ['asdfasdfasdf']) as overridden:
            self.assertIsNone(overridden)


if __name__ == '__main__':
    unittest.main()
