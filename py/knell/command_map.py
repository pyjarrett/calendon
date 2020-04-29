import knell.commands as cmd
import knell.parsers as parsers

COMMAND_PARSERS = {
    # Build and test
    'clean': (parsers.parser_clean, cmd.cmd_clean, 'Remove build directories'),
    'gen': (parsers.parser_gen, cmd.cmd_gen, 'Generate build types'),
    'build': (parsers.parser_build, cmd.cmd_build, 'Do a build'),
    'check': (parsers.parser_check, cmd.cmd_check, 'Run tests'),

    # Run
    'demo': (parsers.parser_demo, cmd.cmd_demo, 'List built demos available to be run'),
    'run': (parsers.parser_run, cmd.cmd_run, 'Run a demo or a game.'),

    # Environment
    'env': (parsers.parser_env, cmd.cmd_env, 'Prints the current tool configuration'),
    'register': (parsers.parser_register, cmd.cmd_register, 'Change program registration.'),
    'default': (parsers.parser_default, cmd.cmd_default, 'Set default configuration parameters.'),

    # Command history and automation
    'source': (parsers.parser_source, cmd.cmd_source, 'Run each line from a file as a command.'),
    'save': (parsers.parser_save, cmd.cmd_save, 'Save configuration to file.'),
    'load': (parsers.parser_load, cmd.cmd_load, 'Load configuration from a file.'),

    # Development
    'pysetup': (parsers.parser_pysetup, cmd.cmd_pysetup, 'Setup virtual environment for subtools.'),
    'pycheck': (parsers.parser_pycheck, cmd.cmd_pycheck, 'Run Python linting and testing.'),
}
